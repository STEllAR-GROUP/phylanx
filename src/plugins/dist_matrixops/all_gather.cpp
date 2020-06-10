// Copyright (c) 2020 Hartmut Kaiser
// Copyright (c) 2020 Nanmiao Wu
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/annotation.hpp>
#include <phylanx/execution_tree/locality_annotation.hpp>
#include <phylanx/execution_tree/localities_annotation.hpp>
#include <phylanx/execution_tree/meta_annotation.hpp>
#include <phylanx/execution_tree/primitives/annotate_primitive.hpp>
#include <phylanx/execution_tree/primitives/node_data_helpers.hpp>
#include <phylanx/execution_tree/tiling_annotations.hpp>
#include <phylanx/ir/node_data.hpp>
#include <phylanx/plugins/dist_matrixops/all_gather.hpp>
#include <phylanx/plugins/dist_matrixops/tile_calculation_helper.hpp>
#include <phylanx/plugins/matrixops/concatenate.hpp>
#include <phylanx/util/distributed_matrix.hpp>
#include <phylanx/util/distributed_vector.hpp>
#include <phylanx/util/generate_error_message.hpp>
#include <phylanx/util/index_calculation_helper.hpp>


#include <hpx/assertion.hpp>
#include <hpx/errors/throw_exception.hpp>
#include <hpx/include/lcos.hpp>
#include <hpx/include/naming.hpp>
#include <hpx/include/util.hpp>
#include <hpx/modules/collectives.hpp>



#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include <blaze/Math.h>

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace dist_matrixops { namespace primitives
{
    ///////////////////////////////////////////////////////////////////////////
    execution_tree::match_pattern_type const all_gather::match_data =
    {
        hpx::util::make_tuple("all_gather", std::vector<std::string>{R"(
                all_gather_d(
                    _1_local_result
                )
            )"},
            &create_all_gather,
            &execution_tree::create_primitive<all_gather>, R"(
            local_result
            Args:

                local_result (array) : a distributed array. A scalar, vector,
                    or matrix.

            Returns:

                A future holding a 1-D array or 2-D array with all values send
                    by all participating localities.)"
            )
    };

    ///////////////////////////////////////////////////////////////////////////
    all_gather::all_gather(
        execution_tree::primitive_arguments_type&& operands,
        std::string const& name, std::string const& codename)
      : primitive_component_base(std::move(operands), name, codename)
    {}

    ///////////////////////////////////////////////////////////////////////////
    namespace detail {
        
    }

    ///////////////////////////////////////////////////////////////////////////
    template <typename T>
    execution_tree::primitive_argument_type all_gather::all_gather2d(
        ir::node_data<T>&& arr,
        execution_tree::localities_information&& locs) const
    {
        using namespace execution_tree;
        auto m = arr.matrix();
        blaze::DynamicMatrix<T> res_value(m.rows(), m.columns());
        res_value = m;
        // use hpx::all_gather to get the whole vector of values
        auto overall_result = hpx::all_gather(
            ("all_gather_" + locs.annotation_.name_).c_str(),
            res_value, locs.locality_.num_localities_,
            std::size_t(-1),
            locs.locality_.locality_id_)
                .get();
        // row and column dimensions of the whole array 
        std::size_t rows_dim, cols_dim;
        rows_dim = locs.rows();
        cols_dim = locs.columns();
        // check the tiling type is column-tiling or row-tiling
        std::int64_t axis; // along with the array will be joined
        if (m.rows() == rows_dim)
        {
            // column-tiling
            axis = 1;
        }
        else if (m.columns() == cols_dim)
        {
            // row-tiling
            axis = 0;
        }
        else
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "all_gather::detail::all_gather2d",
                    util::generate_error_message(
                        "invalid tiling_type. The tiling_type can"
                        "be `row` or `column`"));
        }
        //concatenate the vector of values according to the tiling-type
        return execution_tree::primitives::concatenate::
            concatenate2d_helper<T>(primitive_argument_type
            {overall_result}, axis);
    }

    ///////////////////////////////////////////////////////////////////////////
    execution_tree::primitive_argument_type all_gather::all_gather2d(
        execution_tree::primitive_argument_type&& arr) const
    {
        using namespace execution_tree;

        execution_tree::localities_information locs =
            extract_localities_information(arr, name_, codename_);

        std::size_t ndim = locs.num_dimensions();
        if (ndim > 2 || ndim < 1)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "all_gather::all_gather2d",
                util::generate_error_message(
                    "the operand has incompatible dimensionalities"));
        }

        switch (extract_common_type(arr))
        {
        case node_data_type_bool:
            return all_gather2d(
                extract_boolean_value_strict(std::move(arr), name_, codename_),
                std::move(locs));

        case node_data_type_int64:
            return all_gather2d(
                extract_integer_value_strict(std::move(arr), name_, codename_),
                std::move(locs));

        case node_data_type_unknown:
            return all_gather2d(
                extract_numeric_value(std::move(arr), name_, codename_),
                std::move(locs));

        case node_data_type_double:
            return all_gather2d(
                extract_numeric_value_strict(std::move(arr), name_, codename_),
                std::move(locs));

        default:
            break;
        }

    }

    ///////////////////////////////////////////////////////////////////////////

    hpx::future<execution_tree::primitive_argument_type> all_gather::eval(
        execution_tree::primitive_arguments_type const& operands,
        execution_tree::primitive_arguments_type const& args,
        execution_tree::eval_context ctx) const
    {
        if (operands.empty() || operands.size() > 1)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "all_gather::eval",
                generate_error_message(
                    "the all_gather primitive requires"
                    "1 operands"));
        }

        if (!valid(operands[0]))
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "all_gather::eval",
                generate_error_message(
                    "the all_gather primitive requires the first argument"
                    "given by the operands array is valid"));
        }


        auto this_ = this->shared_from_this();
        return hpx::dataflow(hpx::launch::sync,
            hpx::util::unwrapping(
                [this_ = std::move(this_)](
                    execution_tree::primitive_arguments_type&& args)
                    -> execution_tree::primitive_argument_type
                    {
                        using namespace execution_tree;

                        switch (extract_numeric_value_dimension(
                            std::move(args[0]), this_->name_, this_->codename_))
                        {

                            case 1:
                                HPX_FALLTHROUGH;

                            case 2:
                                return this_->all_gather2d(std::move(args[0]));

                            default:
                                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                                    "all_gather::eval",
                                    this_->generate_error_message(
                                        "operand a has an invalid number of "
                                        "dimensions"));
                        }
                    }),
        execution_tree::primitives::detail::map_operands(operands,
            execution_tree::functional::value_operand{}, args, name_,
            codename_, std::move(ctx)));
    }
}}}    // namespace phylanx::dist_matrixops::primitives

