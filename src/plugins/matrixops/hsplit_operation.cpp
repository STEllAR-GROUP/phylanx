// Copyright (c) 2018 Maxwell Reeser
// Copyright (c) 2018 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/base_primitive.hpp>
#include <phylanx/ir/node_data.hpp>
#include <phylanx/ir/ranges.hpp>
#include <phylanx/plugins/matrixops/hsplit_operation.hpp>

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <string>
#include <utility>
#include <vector>

namespace phylanx { namespace execution_tree { namespace primitives {
    ///////////////////////////////////////////////////////////////////////////
    match_pattern_type const hsplit_operation::match_data = {match_pattern_type{
        "hsplit", std::vector<std::string>{"hsplit(_1, __2)"},
        &create_hsplit_operation, &create_primitive<hsplit_operation>, R"(
            args
            Args:

                *args (m - matrix or row vector, N - number of blocks)

            Returns:

            An N by 1 matrix of horizontal partitions of m)",
        true}};

    ///////////////////////////////////////////////////////////////////////////

    hsplit_operation::hsplit_operation(primitive_arguments_type&& operands,
        std::string const& name,
        std::string const& codename)
      : primitive_component_base(std::move(operands), name, codename)
      , dtype_(extract_dtype(name_))
    {
    }

    ///////////////////////////////////////////////////////////////////////////
    template <typename T>
    primitive_argument_type hsplit_operation::hsplit2d_helper(
        primitive_arguments_type&& args) const
    {
        std::array<std::size_t, PHYLANX_MAX_DIMENSIONS> matrix_dims =
            extract_numeric_value_dimensions(args[0], name_, codename_);

        std::size_t num_rows = matrix_dims[0];
        std::size_t num_cols = matrix_dims[1];

        std::size_t block_dims =
            extract_numeric_value_dimension(args[1], name_, codename_);

        std::vector<std::pair<std::size_t, std::size_t>> ranges;

        if (block_dims < 1)    // Scalar tiling
        {
            phylanx::ir::node_data<double> val =
                extract_numeric_value(args[1], name_, codename_);

            std::size_t num_blocks = val.scalar();
            ranges.reserve(num_blocks);

            if (num_blocks <= 0)
            {
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "hsplit_operation::eval",
                    generate_error_message(
                        "the hsplit_operation primitive can not split "
                        "matrices/vectors into fewer blocks than one"));
            }
            if (num_blocks > num_cols)
            {
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "hsplit_operation::eval",
                    generate_error_message(
                        "the hsplit_operation primitive can not split "
                        "matrices/vectors into more blocks than there "
                        "are columns"));
            }
            if (num_cols % num_blocks != 0)
            {
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "hsplit_operation::eval",
                    generate_error_message(
                        "the hsplit_operation primitive can not split "
                        "matrices/vectors unevenly"));
            }
            std::size_t block_size = num_cols / num_blocks;

            std::size_t index(num_rows);
            std::size_t tmp_block_num(num_blocks);

            for (int i = 0; i < num_cols; i += block_size)
            {
                std::pair<std::size_t, std::size_t> range(i, i + block_size);
                ranges.push_back(range);
            }
        }
        else    // Index-based tiling
        {
            auto&& indices = extract_node_data<T>(std::move(args[1]));
            auto v = indices.vector();

            if (v.size() > 0)
            {
                for (int i = 0; i < v.size(); i++)
                {
                    if (static_cast<std::size_t>(v[i]) > num_cols)
                        v[i] = num_cols;
                }

                ranges.reserve(v.size() + 1);

                // Include from 0 to first index
                ranges.push_back(std::make_pair<std::size_t, std::size_t>(
                    std::size_t(0), std::size_t(v[0])));

                // Make a list of pairs from the passed index list
                for (int i = 0; i < v.size() - 1; i++)
                {
                    ranges.push_back(std::make_pair<std::size_t, std::size_t>(
                        std::size_t(v[i]), std::size_t(v[i + 1])));
                }

                // Include from last index to last row
                ranges.push_back(std::make_pair<std::size_t, std::size_t>(
                    std::size_t(v[v.size() - 1]), std::size_t(num_cols)));
            }
            else
            {
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "hsplit_operation::eval",
                    generate_error_message(
                        "the hsplit_operation primitive requires all "
                        "index vectors to be of size at least 1"));
            }
        }

        std::int64_t num_blocks = ranges.size();

        primitive_arguments_type result;
        result.reserve(num_blocks);

        auto&& input_data = extract_node_data<T>(std::move(args[0]));
        auto m = input_data.matrix();

        for (int i = 0; i < num_blocks; i++)
        {
            if (ranges[i].second > ranges[i].first)
            {
                std::size_t diff = ranges[i].second - ranges[i].first;
                blaze::DynamicMatrix<T> block(num_rows, diff);
                for (int j = 0; j < diff; j++)
                {
                    column(block, j) = column(m, j + ranges[i].first);
                }
                result.push_back(primitive_argument_type{
                    ir::node_data<T>{std::move(block)}});
            }
            else    // Numpy's version of this function returns an empty matrix
                    // of this shape when the second index is smaller than the
                    // first
            {
                blaze::DynamicMatrix<T> block(num_rows, std::int64_t(0));
                result.push_back(primitive_argument_type{
                    ir::node_data<T>{std::move(block)}});
            }
        }
        return primitive_argument_type{std::move(result)};
    }

    primitive_argument_type hsplit_operation::hsplit2d(
        primitive_arguments_type&& args) const
    {
        switch (extract_common_type(args[0]))
        {
        case node_data_type_bool:
            return hsplit2d_helper<std::uint8_t>(std::move(args));

        case node_data_type_int64:
            return hsplit2d_helper<std::int64_t>(std::move(args));

        case node_data_type_double:
            return hsplit2d_helper<double>(std::move(args));

        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "phylanx::execution_tree::primitives::"
            "hsplit_operation::hsplit2d",
            generate_error_message(
                "the hsplit_operation primitive requires for all arguments to "
                "be numeric data types"));
    }

    ///////////////////////////////////////////////////////////////////////////
    primitive_argument_type hsplit_operation::hsplit_args(
        primitive_arguments_type&& args) const
    {
        std::size_t matrix_dims =
            extract_numeric_value_dimension(args[0], name_, codename_);

        switch (matrix_dims)
        {
        case 2:
            return hsplit2d(std::move(args));

        default:
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "hsplit_operation::hsplit_args",
                generate_error_message("left hand side operand has unsupported "
                                       "number of dimensions"));
        }
    }

    ///////////////////////////////////////////////////////////////////////////
    hpx::future<primitive_argument_type> hsplit_operation::eval(
        primitive_arguments_type const& operands,
        primitive_arguments_type const& args, eval_context ctx) const
    {
        if (operands.empty())
        {
            using storage2d_type = ir::node_data<std::int64_t>::storage2d_type;
            return hpx::make_ready_future(primitive_argument_type{
                ir::node_data<std::int64_t>{storage2d_type(0, 1)}});
        }

        bool arguments_valid = true;
        for (std::size_t i = 0; i != operands.size(); ++i)
        {
            if (!valid(operands[i]))
            {
                arguments_valid = false;
            }
        }

        if (!arguments_valid)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter, "hsplit_operation::eval",
                generate_error_message(
                    "the hsplit_operation primitive requires "
                    "that the arguments given by the operands "
                    "array are valid"));
        }

        auto this_ = this->shared_from_this();
        return hpx::dataflow(hpx::launch::sync, hpx::util::unwrapping(
            [this_ = std::move(this_)](
                primitive_arguments_type&& ops)
            -> primitive_argument_type
            {
                return this_->hsplit_args(std::move(ops));
            }),
            detail::map_operands(operands, functional::value_operand{},
                args, name_, codename_, std::move(ctx)));
    }
}}}
