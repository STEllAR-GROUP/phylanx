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
#include <phylanx/plugins/dist_matrixops/tile_calculation_helper.hpp>
#include <phylanx/plugins/matrixops/concatenate.hpp>
#include <phylanx/util/distributed_matrix.hpp>
#include <phylanx/util/distributed_vector.hpp>
#include <phylanx/util/generate_error_message.hpp>
#include <phylanx/util/index_calculation_helper.hpp>


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
    namespace detail
    {
        template <typename T>
        execution_tree::primitive_argument_type concatenate2d_axis0(
            execution_tree::primitive_arguments_type&& args,
            std::string const& name, std::string const& codename)
        {
            std::size_t args_size = args.size();
            std::array<std::size_t, PHYLANX_MAX_DIMENSIONS> prevdim =
                extract_numeric_value_dimensions(args[0], name, codename);
            std::size_t total_rows = 0;
            for (std::size_t i = 0; i != args_size; ++i)
            {
                if (extract_numeric_value_dimension(args[i],
                    name, codename) != 2)
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "dist_matrixops::primitives::all_gather::"
                        "detail::concatenate2d_axis0",
                        util::generate_error_message(
                            "all the input arrays must have "
                            "the same number of dimensions", name, codename));
                }

                std::array<std::size_t, PHYLANX_MAX_DIMENSIONS> dim =
                    extract_numeric_value_dimensions(args[i], name, codename);

                if (i != 0 && prevdim[1] != dim[1])
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "dist_matrixops::primitives::"
                        "all_gather::detail::concatenate2d_axis0",
                        util::generate_error_message(
                            "all the input array dimensions except for "
                            "the concatenation axis must match exactly ",
                            name, codename));
                }

                total_rows += dim[0];
                prevdim = dim;
            }
            blaze::DynamicMatrix<T> result(total_rows, prevdim[1]);
            std::size_t step = 0;
            for (auto&& arg : args)
            {
                auto&& val =
                    execution_tree::extract_node_data<T>(std::move(arg));
                std::size_t num_rows = val.dimension(0);
                auto m = val.matrix();
                for (std::size_t j = 0; j != num_rows; ++j)
                {
                    blaze::row(result, j + step) = blaze::row(m, j);
                }
                step += num_rows;
            }
            return execution_tree::primitive_argument_type{
                ir::node_data<T>{std::move(result)}};
        }

        template <typename T>
        execution_tree::primitive_argument_type concatenate2d_axis1(
            execution_tree::primitive_arguments_type&& args,
            std::string const& name, std::string const& codename)
        {
            std::size_t args_size = args.size();

            std::array<std::size_t, PHYLANX_MAX_DIMENSIONS> prevdim =
                extract_numeric_value_dimensions(args[0], name, codename);

            std::size_t total_cols = 0;
            for (std::size_t i = 0; i != args_size; ++i)
            {
                if (extract_numeric_value_dimension(args[i],
                    name, codename) != 2)
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "dist_matrixops::primitives::"
                        "all_gather::detail::concatenate2d_axis1",
                        util::generate_error_message(
                            "all the input arrays must have "
                            "the same number of dimensions", name, codename));
                }

                std::array<std::size_t, PHYLANX_MAX_DIMENSIONS> dim =
                    extract_numeric_value_dimensions(args[i], name, codename);

                if (i != 0 && prevdim[0] != dim[0])
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "dist_matrixops::primitives::"
                        "all_gather::detail::concatenate2d_axis1",
                        util::generate_error_message(
                            "all the input array dimensions except for"
                            "the concatenation axis must match exactly",
                            name, codename));
                }

                total_cols += dim[1];
                prevdim = dim;
            }

            blaze::DynamicMatrix<T> result(prevdim[0], total_cols);

            std::size_t step = 0;
            for (auto&& arg : args)
            {
                auto&& val =
                    execution_tree::extract_node_data<T>(std::move(arg));
                std::size_t num_cols = val.dimension(1);
                auto m = val.matrix();
                for (std::size_t j = 0; j != num_cols; ++j)
                {
                    blaze::column(result, j + step) = blaze::column(m, j);
                }
                step += num_cols;
            }

            return execution_tree::primitive_argument_type{
                ir::node_data<T>{std::move(result)}};
        }

        template <typename T>
        execution_tree::primitive_argument_type concatenate2d(
            execution_tree::primitive_arguments_type&& args, std::int64_t axis,
            std::string const& name, std::string const& codename)
        {
            switch (axis)
            {
            case 0:
                return detail::concatenate2d_axis0<T>(
                    std::move(args), name, codename);
            case 1:
                return detail::concatenate2d_axis1<T>(
                    std::move(args), name, codename);
            default:
                break;
            }
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "dist_matrixops::primitives::"
                "all_gather::detail::concatenate2d",
                util::generate_error_message(
                    "axis is out of bounds of dimension", name, codename));
        }
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
        // use hpx::all_gather to get a vector of values
        auto p = hpx::all_gather(
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
                generate_error_message(
                    "invalid tiling_type. The tiling_type can"
                    "be `row` or `column`"));
        }

        primitive_arguments_type ops;
        ops.reserve(p.size());
        for (auto && op : p)
        {
            ops.push_back(
                primitive_argument_type{ir::node_data<T>{std::move(op)}});
        }

        return detail::concatenate2d<T>(std::move(ops), axis, name_, codename_);
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

