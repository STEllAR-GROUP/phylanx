//  Copyright (c) 2018 Parsa Amini
//  Copyright (c) 2018 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/argmax.hpp>
#include <phylanx/ir/node_data.hpp>
#include <phylanx/util/matrix_iterators.hpp>

#include <hpx/include/lcos.hpp>
#include <hpx/include/naming.hpp>
#include <hpx/include/util.hpp>
#include <hpx/throw_exception.hpp>
#include <hpx/util/iterator_facade.hpp>

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include <blaze/Math.h>

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace execution_tree { namespace primitives
{
    ///////////////////////////////////////////////////////////////////////////
    primitive create_argmax(hpx::id_type const& locality,
        std::vector<primitive_argument_type>&& operands,
        std::string const& name, std::string const& codename)
    {
        static std::string type("argmax");
        return create_primitive_component(
            locality, type, std::move(operands), name, codename);
    }

    match_pattern_type const argmax::match_data =
    {
        hpx::util::make_tuple("argmax",
            std::vector<std::string>{"argmax(_1, _2)"},
            &create_argmax, &create_primitive<argmax>)
    };

    ///////////////////////////////////////////////////////////////////////////
    argmax::argmax(std::vector<primitive_argument_type>&& operands,
            std::string const& name, std::string const& codename)
      : primitive_component_base(std::move(operands), name, codename)
    {}

    ///////////////////////////////////////////////////////////////////////////
    primitive_argument_type argmax::argmax0d(args_type && args) const
    {
        // `axis` is optional
        if (args.size() == 2)
        {
            // `axis` must be a scalar if provided
            if (args[1].num_dimensions() != 0)
            {
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "argmax::argmax0d",
                    execution_tree::generate_error_message(
                        "operand axis must be a scalar", name_,
                        codename_));
            }
            const int axis = args[1].scalar();
            // `axis` can only be -1 or 0
            if (axis < -1 || axis > 0)
            {
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "argmax::argmax0d",
                    execution_tree::generate_error_message(
                        "operand axis can only between -1 and 0 for "
                        "an a operand that is 0d",
                        name_, codename_));
            }
        }

        return 0ul;
    }

    ///////////////////////////////////////////////////////////////////////////
    primitive_argument_type argmax::argmax1d(args_type && args) const
    {
        // `axis` is optional
        if (args.size() == 2)
        {
            // `axis` must be a scalar if provided
            if (args[1].num_dimensions() != 0)
            {
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "argmax::argmax1d",
                    execution_tree::generate_error_message(
                        "operand axis must be a scalar", name_,
                        codename_));
            }
            const int axis = args[1].scalar();
            // `axis` can only be -1 or 0
            if (axis < -1 || axis > 0)
            {
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "argmax::argmax1d",
                    execution_tree::generate_error_message(
                        "operand axis can only between -1 and 0 for "
                        "an a operand that is 1d",
                        name_, codename_));
            }
        }

        auto a = args[0].vector();

        // a should not be empty
        if (a.size() == 0)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "argmax::argmax1d",
                execution_tree::generate_error_message(
                    "attempt to get argmax of an empty sequence",
                    name_, codename_));
        }

        // Find the maximum value among the elements
        const auto max_it = std::max_element(a.begin(), a.end());

        // Return max's index
        return std::distance(a.begin(), max_it);
    }

    ///////////////////////////////////////////////////////////////////////////
    primitive_argument_type argmax::argmax2d_flatten(arg_type && arg_a) const
    {
        using phylanx::util::matrix_row_iterator;

        auto a = arg_a.matrix();

        const matrix_row_iterator<decltype(a)> a_begin(a);
        const matrix_row_iterator<decltype(a)> a_end(a, a.rows());

        val_type global_max = 0.;
        std::size_t global_index = 0ul;
        std::size_t passed_rows = 0ul;
        for (auto it = a_begin; it != a_end; ++it, ++passed_rows)
        {
            const auto local_max = std::max_element(it->begin(), it->end());
            const auto local_max_val = *local_max;

            if (local_max_val > global_max)
            {
                global_max = local_max_val;
                global_index = std::distance(it->begin(), local_max) +
                    passed_rows * it->size();
            }
        }
        return global_index;
    }

    primitive_argument_type argmax::argmax2d_x_axis(arg_type && arg_a) const
    {
        using phylanx::util::matrix_row_iterator;

        auto a = arg_a.matrix();

        const matrix_row_iterator<decltype(a)> a_begin(a);
        const matrix_row_iterator<decltype(a)> a_end(a, a.rows());

        std::vector<primitive_argument_type> result;
        for (auto it = a_begin; it != a_end; ++it)
        {
            const auto local_max = std::max_element(it->begin(), it->end());
            std::int64_t index = std::distance(it->begin(), local_max);
            result.emplace_back(primitive_argument_type(index));
        }
        return primitive_argument_type{result};
    }
    primitive_argument_type argmax::argmax2d_y_axis(arg_type && arg_a) const
    {
        using phylanx::util::matrix_column_iterator;

        auto a = arg_a.matrix();

        const matrix_column_iterator<decltype(a)> a_begin(a);
        const matrix_column_iterator<decltype(a)> a_end(a, a.columns());

        std::vector<primitive_argument_type> result;
        for (auto it = a_begin; it != a_end; ++it)
        {
            const auto local_max = std::max_element(it->begin(), it->end());
            std::int64_t index = std::distance(it->begin(), local_max);
            result.emplace_back(primitive_argument_type(index));
        }
        return primitive_argument_type{result};
    }

    primitive_argument_type argmax::argmax2d(args_type && args) const
    {
        // a should not be empty
        if (args[0].matrix().rows() == 0 ||
            args[0].matrix().columns() == 0)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "argmax::argmax2d",
                execution_tree::generate_error_message(
                    "attempt to get argmax of an empty sequence",
                    name_, codename_));
        }

        // `axis` is optional
        if (args.size() == 1)
        {
            // Option 1: Flatten and find max
            return argmax2d_flatten(std::move(args[0]));
        }

        // `axis` must be a scalar if provided
        if (args[1].num_dimensions() != 0)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "argmax::argmax2d",
                execution_tree::generate_error_message(
                    "operand axis must be a scalar", name_, codename_));
        }
        const int axis = args[1].scalar();
        // `axis` can only be -2, -1, 0, or 1
        if (axis < -2 || axis > 1)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "argmax::argmax2d",
                execution_tree::generate_error_message(
                    "operand axis can only between -2 and 1 for an a "
                    "operand that is 2d",
                    name_, codename_));
        }
        switch (axis)
        {
        // Option 2: Find max among rows
        case -2: HPX_FALLTHROUGH;
        case 0:
            return argmax2d_x_axis(std::move(args[0]));

        // Option 3: Find max among columns
        case -1: HPX_FALLTHROUGH;
        case 1:
            return argmax2d_y_axis(std::move(args[0]));

        default:
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "argmax::argmax2d",
                execution_tree::generate_error_message(
                    "operand a has an invalid number of "
                    "dimensions",
                    name_, codename_));
        }


    }

    hpx::future<primitive_argument_type> argmax::eval(
        std::vector<primitive_argument_type> const& operands,
        std::vector<primitive_argument_type> const& args) const
    {
        if (operands.empty() || operands.size() > 2)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "argmax::eval",
                execution_tree::generate_error_message(
                    "the argmax primitive requires exactly one or two "
                        "operands",
                    name_, codename_));
        }

        for (auto const& i : operands)
        {
            if (!valid(i))
            {
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "argmax::eval",
                    execution_tree::generate_error_message(
                        "the argmax primitive requires that the "
                        "arguments given by the operands array are "
                        "valid",
                        name_, codename_));
            }
        }


        auto this_ = this->shared_from_this();
        return hpx::dataflow(hpx::util::unwrapping(
            [this_](args_type&& args) -> primitive_argument_type
            {
                std::size_t a_dims = args[0].num_dimensions();
                switch (a_dims)
                {
                case 0:
                    return this_->argmax0d(std::move(args));

                case 1:
                    return this_->argmax1d(std::move(args));

                case 2:
                    return this_->argmax2d(std::move(args));

                default:
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "argmax::eval",
                        execution_tree::generate_error_message(
                            "operand a has an invalid "
                            "number of dimensions",
                        this_->name_, this_->codename_));
                }
            }),
            // TODO: Check what value -1 is going to turn into.
            // node_data of doubles?
            detail::map_operands(
                operands, functional::numeric_operand{}, args,
                name_, codename_));
    }

    hpx::future<primitive_argument_type> argmax::eval(
        std::vector<primitive_argument_type> const& args) const
    {
        if (operands_.empty())
        {
            return eval(args, noargs);
        }
        return eval(operands_, args);
    }
}}}
