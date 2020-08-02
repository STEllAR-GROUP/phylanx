// Copyright (c) 2019 Shahrzad Shirzad
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
#include <phylanx/util/distributed_matrix.hpp>
#include <phylanx/util/generate_error_message.hpp>


#include <hpx/assert.hpp>
#include <hpx/errors/throw_exception.hpp>
#include <hpx/include/lcos.hpp>
#include <hpx/include/naming.hpp>
#include <hpx/include/util.hpp>
#include <hpx/modules/collectives.hpp>


#include <array>
#include <cstddef>
#include <cstdint>
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
        hpx::util::make_tuple("all_gather_d", std::vector<std::string>{R"(
                all_gather_d(
                    _1_local_result
                )
            )"},
            &create_all_gather,
            &execution_tree::create_primitive<all_gather>, R"(
            local_result
            Arg:

                local_result (array) : a distributed array. A vector or matrix.

            Returns:

                A future holding a 2-D array with all values send
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
    template <typename T>
    execution_tree::primitive_argument_type all_gather::all_gather2d(
        ir::node_data<T>&& arr,
        execution_tree::localities_information&& locs) const
    {
        using namespace execution_tree;

        blaze::DynamicMatrix<T> m = arr.matrix();
        // use hpx::all_gather to get a vector of values
        auto p = hpx::all_gather(
            ("all_gather_" + locs.annotation_.name_).c_str(),
            m, locs.locality_.num_localities_,
            std::size_t(-1),
            locs.locality_.locality_id_)
                .get();

        // row and column dimensions of the whole array
        std::size_t rows_dim, cols_dim;
        rows_dim = locs.rows(name_, codename_);
        cols_dim = locs.columns(name_, codename_);

        blaze::DynamicMatrix<T> result(rows_dim, cols_dim);

        bool tiled_col, tiled_row;
        tiled_col = locs.is_column_tiled(name_, codename_);
        tiled_row = locs.is_row_tiled(name_, codename_);

        // check the tiling type is column-tiling or row-tiling
        std::int64_t axis; // along with the array will be joined
        if (tiled_col)
        {
            // column-tiling
            std::size_t step = 0;
            for (auto&& arg : p)
            {
                auto&& val = ir::node_data<T>{std::move(arg)};
                std::size_t num_rows = val.dimension(0);
                std::size_t num_cols = val.dimension(1);
                auto m = val.matrix();
                blaze::submatrix(
                    result, 0, step, num_rows, num_cols) = m;
                step += num_cols;
            }
        }
        else if (tiled_row)
        {
            // row-tiling
            std::size_t step = 0;
            for (auto&& arg : p)
            {
                auto&& val = ir::node_data<T>{std::move(arg)};
                std::size_t num_rows = val.dimension(0);
                std::size_t num_cols = val.dimension(1);
                auto n = val.matrix();
                blaze::submatrix(
                    result, step, 0, num_rows, num_cols) = n;
                step += num_rows;
            }
        }
        else
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "all_gather::detail::all_gather2d",
                generate_error_message(
                    "invalid tiling_type. The tiling_type can"
                    "be `row` or `column`"));
        }

        return execution_tree::primitive_argument_type{
                ir::node_data<T>{std::move(result)}};
    }

    ///////////////////////////////////////////////////////////////////////////
    execution_tree::primitive_argument_type all_gather::all_gather2d(
        execution_tree::primitive_argument_type&& arr) const
    {
        using namespace execution_tree;

        execution_tree::localities_information locs =
            extract_localities_information(arr, name_, codename_);

        std::size_t ndim = locs.num_dimensions();

        if (ndim != 2 && ndim != 0)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter, "all_gather::all_gather2d",
                generate_error_message(
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

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "dist_matrixops::primitives::all_gather::all_gather2d",
            generate_error_message(
                "the all_gather_d primitive requires for all arguments to "
                "be numeric data types"));

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

