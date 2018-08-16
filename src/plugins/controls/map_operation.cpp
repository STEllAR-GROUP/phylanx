// Copyright (c) 2018 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/plugins/controls/map_operation.hpp>

#include <hpx/include/lcos.hpp>
#include <hpx/include/naming.hpp>
#include <hpx/include/util.hpp>
#include <hpx/util/assert.hpp>
#include <hpx/throw_exception.hpp>

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include <blaze/Blaze.h>

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace execution_tree { namespace primitives
{
    ///////////////////////////////////////////////////////////////////////////
    match_pattern_type const map_operation::match_data =
    {
        hpx::util::make_tuple("map",
            std::vector<std::string>{"map(_1, __2)"},
            &create_map_operation, &create_primitive<map_operation>,
            "synopsis: map(func, list)\n"
            "Evaluate the function `func` for each item in list `list` "
            "and return the result as a new list.\n"
            "\n"
            "Example:\n"
            "``\n"
            "  map(lambda(x, x + 1), make_list(1, 2, 3))\n"
            "``\n"
            "Produces `[2.0, 3.0, 4.0]`. "
            "TODO")
    };

    ///////////////////////////////////////////////////////////////////////////
    map_operation::map_operation(
            std::vector<primitive_argument_type>&& operands,
            std::string const& name, std::string const& codename)
      : primitive_component_base(std::move(operands), name, codename)
    {}

    ///////////////////////////////////////////////////////////////////////////
    primitive_argument_type map_operation::map_1_scalar(
        primitive const* p, primitive_argument_type&& arg) const
    {
        return p->eval(hpx::launch::sync, std::move(arg));
    }

    namespace detail
    {
        template <typename T>
        struct map_1_vector
        {
            using vector_type = typename ir::node_data<T>::storage1d_type;
            using vector_view_type =
                typename ir::node_data<T>::custom_storage1d_type;

            static vector_type call(primitive const* p,
                vector_view_type const& vec, std::string const& name,
                std::string const& codename)
            {
                vector_type result(vec.size(), T{0});

                std::size_t i = 0;
                for (auto && val : vec)
                {
                    auto r = p->eval(hpx::launch::sync,
                        primitive_argument_type{std::move(val)});

                    if (valid(r))
                    {
                        auto num_result =
                            extract_numeric_value(std::move(r), name, codename);

                        if (num_result.num_dimensions() != 0)
                        {
                            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                                "detail::map_1_vector::call",
                                util::generate_error_message(
                                    "the invoked lambda returned an unexpected "
                                    "type ("
                                    "should be a scalar value)",
                                    name, codename));
                        }

                        result[i++] = num_result.scalar();
                    }
                }

                return result;
            }
        };
    }

    primitive_argument_type map_operation::map_1_vector(
        primitive const* p, primitive_argument_type&& arg) const
    {
        if (is_numeric_operand(arg))
        {
            auto v = extract_numeric_value(std::move(arg), name_, codename_);
            HPX_ASSERT(v.num_dimensions() == 1);
            return primitive_argument_type{
                ir::node_data<double>{detail::map_1_vector<double>::call(
                    p, v.vector(), name_, codename_)}};
        }

        if (is_integer_operand(arg))
        {
            auto v = extract_integer_value(std::move(arg), name_, codename_);
            HPX_ASSERT(v.num_dimensions() == 1);
            return primitive_argument_type{ir::node_data<std::int64_t>{
                detail::map_1_vector<std::int64_t>::call(
                    p, v.vector(), name_, codename_)}};
        }

        if (is_boolean_operand(arg))
        {
            auto v = extract_boolean_value(std::move(arg), name_, codename_);
            HPX_ASSERT(v.num_dimensions() == 1);
            return primitive_argument_type{ir::node_data<std::uint8_t>{
                detail::map_1_vector<std::uint8_t>::call(
                    p, v.vector(), name_, codename_)}};
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "map_operation::map_1_vector",
            generate_error_message("unexpected numeric type"));
    }

    namespace detail
    {
        template <typename T>
        struct map_1_matrix
        {
            using vector_type = typename ir::node_data<T>::storage1d_type;

            using matrix_type = typename ir::node_data<T>::storage2d_type;
            using matrix_view_type =
                typename ir::node_data<T>::custom_storage2d_type;

            static matrix_type call(primitive const* p,
                matrix_view_type const& m, std::string const& name,
                std::string const& codename)
            {
                matrix_type result(m.rows(), m.columns(), T{0});

                for (std::size_t i = 0; i != m.rows(); ++i)
                {
                    vector_type row{blaze::trans(blaze::row(m, i))};

                    auto r = p->eval(hpx::launch::sync,
                        primitive_argument_type{std::move(row)});

                    if (valid(r))
                    {
                        auto num_result =
                            extract_numeric_value(std::move(r), name, codename);

                        if (num_result.num_dimensions() != 1)
                        {
                            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                                "detail::map_1_matrix::call",
                                util::generate_error_message(
                                    "the invoked lambda returned an unexpected "
                                    "type (should be a vector value)",
                                    name, codename));
                        }

                        blaze::row(result, i) =
                            blaze::trans(num_result.vector());
                    }
                }

                return result;
            }
        };
    }

    primitive_argument_type map_operation::map_1_matrix(
        primitive const* p, primitive_argument_type&& arg) const
    {
        if (is_numeric_operand(arg))
        {
            auto m = extract_numeric_value(std::move(arg), name_, codename_);
            HPX_ASSERT(m.num_dimensions() == 2);
            return primitive_argument_type{
                ir::node_data<double>{detail::map_1_matrix<double>::call(
                    p, m.matrix(), name_, codename_)}};
        }

        if (is_integer_operand(arg))
        {
            auto m = extract_integer_value(std::move(arg), name_, codename_);
            HPX_ASSERT(m.num_dimensions() == 2);
            return primitive_argument_type{ir::node_data<std::int64_t>{
                detail::map_1_matrix<std::int64_t>::call(
                    p, m.matrix(), name_, codename_)}};
        }

        if (is_boolean_operand(arg))
        {
            auto m = extract_boolean_value(std::move(arg), name_, codename_);
            HPX_ASSERT(m.num_dimensions() == 2);
            return primitive_argument_type{ir::node_data<std::uint8_t>{
                detail::map_1_matrix<std::uint8_t>::call(
                    p, m.matrix(), name_, codename_)}};
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "map_operation::map_1_matrix",
            generate_error_message("unexpected numeric type"));
    }

    ///////////////////////////////////////////////////////////////////////////
    hpx::future<primitive_argument_type> map_operation::map_1(
        std::vector<primitive_argument_type> const& operands,
        std::vector<primitive_argument_type> const& args) const
    {
        auto this_ = this->shared_from_this();
        return hpx::dataflow(hpx::launch::sync,
            [this_](util::future_or_value<primitive_argument_type>&& f,
                    util::future_or_value<primitive_argument_type>&& l)
            -> primitive_argument_type
            {
                auto && bound_func = f.get();
                auto && arg = l.get();

                primitive const* p = util::get_if<primitive>(&bound_func);
                if (p == nullptr)
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "map_operation::eval",
                        this_->generate_error_message(
                            "the first argument to map must be an invocable "
                                "object"));
                }

                if (is_list_operand_strict(arg))
                {
                    // Sequentially evaluate all elements in the given list
                    ir::range&& list = extract_list_value_strict(
                        std::move(arg), this_->name_, this_->codename_);

                    std::vector<primitive_argument_type> result;
                    result.reserve(list.size());

                    for (auto && elem : list)
                    {
                        // Evaluate function for each of the argument sets
                        result.push_back(
                            p->eval(hpx::launch::sync, std::move(elem)));
                    }

                    return primitive_argument_type{std::move(result)};
                }

                if (is_numeric_operand(arg))
                {
                    std::size_t dim = extract_numeric_value_dimension(
                        arg, this_->name_, this_->codename_);

                    switch (dim)
                    {
                    case 0:
                        return this_->map_1_scalar(p, std::move(arg));

                    case 1:
                        return this_->map_1_vector(p, std::move(arg));

                    case 2:
                        return this_->map_1_matrix(p, std::move(arg));

                    default:
                        break;
                    }
                }

                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "map_operation::map_1",
                    this_->generate_error_message(
                        "the second argument to map must be an iterable "
                            "object (a list or a numeric type)"));
            },
            value_operand_fov(operands[0], args, name_, codename_,
                eval_dont_evaluate_lambdas),
            value_operand_fov(operands[1], args, name_, codename_));
    }

    ///////////////////////////////////////////////////////////////////////////
    namespace detail
    {
        bool all_list_operands(std::vector<primitive_argument_type> const& args)
        {
            return std::all_of(args.begin(), args.end(), &is_list_operand_strict);
        }

        bool all_numeric_operands(
            std::vector<primitive_argument_type> const& args)
        {
            return std::all_of(args.begin(), args.end(), &is_numeric_operand);
        }

        std::size_t extract_numeric_value_dimension(
            std::vector<primitive_argument_type> const& args,
            std::string const& name, std::string const& codename)
        {
            std::size_t dimension = 0;
            std::array<std::size_t, 2> dimensions;

            for (auto const& arg : args)
            {
                std::size_t dim =
                    extract_numeric_value_dimension(arg, name, codename);
                std::array<std::size_t, 2> dims =
                    extract_numeric_value_dimensions(arg, name, codename);

                if (dimension == 0)
                {
                    dimension = dim;
                    dimensions = dims;
                }

                if (dim != dimension)
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "detail::extract_numeric_value_dimension",
                        util::generate_error_message(
                            "all numeric arguments must have the same shape",
                            name, codename));
                }
            }

            return dimension;
        }
    }

    primitive_argument_type map_operation::map_n_lists(
        primitive const* p, std::vector<primitive_argument_type>&& args) const
    {
        std::vector<ir::range> lists;
        lists.reserve(args.size());

        for (auto && arg : args)
        {
            lists.emplace_back(
                extract_list_value_strict(std::move(arg), name_, codename_));
        }

        // Make sure all lists have the same size
        std::size_t size = lists[0].size();
        for (auto const& list : lists)
        {
            if (list.size() != size)
            {
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "map_operation::map_n_lists",
                    generate_error_message(
                        "all list arguments must have the same length"));
            }
        }

        // Sequentially evaluate all operations
        std::size_t numlists = lists.size();

        std::vector<ir::range_iterator> iters;
        iters.reserve(numlists);

        for (auto const& j : lists)
        {
            iters.push_back(j.begin());
        }

        std::vector<primitive_argument_type> result;
        result.reserve(size);

        for (std::size_t i = 0; i != size; ++i)
        {
            std::vector<primitive_argument_type> args;
            args.reserve(numlists);

            // Each invocation has its own argument set
            for (ir::range_iterator& j : iters)
            {
                args.push_back(*j++);
            }

            // Evaluate function for each of the argument sets
            result.push_back(p->eval(hpx::launch::sync, std::move(args)));
        }

        return primitive_argument_type{std::move(result)};
    }

    primitive_argument_type map_operation::map_n_scalar(primitive const* p,
        std::vector<primitive_argument_type>&& args) const
    {
        return p->eval(hpx::launch::sync, std::move(args));
    }

    primitive_argument_type map_operation::map_n_vector(primitive const* p,
        std::vector<primitive_argument_type>&& args) const
    {
        std::vector<ir::node_data<double>> values;
        values.reserve(args.size());

        for (auto && arg : args)
        {
            values.emplace_back(extract_numeric_value(std::move(arg)));
        }

        std::size_t size = values[0].vector().size();
        ir::node_data<double>::storage1d_type result(size, 0.0);

        for (std::size_t i = 0; i != size; ++i)
        {
            std::vector<primitive_argument_type> params;
            params.reserve(args.size());
            for (std::size_t j = 0; j != args.size(); ++j)
            {
                params.emplace_back(values[j].vector()[i]);
            }

            auto r = p->eval(hpx::launch::sync, std::move(params));
            if (valid(r))
            {
                auto num_result =
                    extract_numeric_value(std::move(r), name_, codename_);

                if (num_result.num_dimensions() != 0)
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "map_operation::map_n_vector",
                        generate_error_message(
                            "the invoked lambda returned an unexpected type ("
                            "should be a scalar value)"));
                }

                result[i] = num_result.scalar();
            }
        }

        return primitive_argument_type{std::move(result)};
    }

    primitive_argument_type map_operation::map_n_matrix(primitive const* p,
        std::vector<primitive_argument_type>&& args) const
    {
        std::vector<ir::node_data<double>> values;
        values.reserve(args.size());

        for (auto && arg : args)
        {
            values.emplace_back(extract_numeric_value(std::move(arg)));
        }

        auto m0 = values[0].matrix();
        ir::node_data<double>::storage2d_type result(
            m0.rows(), m0.columns(), 0.0);

        for (std::size_t i = 0; i != m0.rows(); ++i)
        {
            std::vector<primitive_argument_type> params;
            params.reserve(args.size());
            for (std::size_t j = 0; j != args.size(); ++j)
            {
                using vector_type = ir::node_data<double>::storage1d_type;
                vector_type row{
                    blaze::trans(blaze::row(values[j].matrix(), i))};
                params.emplace_back(std::move(row));
            }

            auto r = p->eval(hpx::launch::sync, std::move(params));

            if (valid(r))
            {
                auto num_result =
                    extract_numeric_value(std::move(r), name_, codename_);

                if (num_result.num_dimensions() != 1)
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "map_operation::map_n_matrix",
                        generate_error_message(
                            "the invoked lambda returned an unexpected type ("
                            "should be a vector value)"));
                }

                blaze::row(result, i) = blaze::trans(num_result.vector());
            }
        }

        return primitive_argument_type{std::move(result)};
    }

    ///////////////////////////////////////////////////////////////////////////
    hpx::future<primitive_argument_type> map_operation::map_n(
        std::vector<primitive_argument_type> const& operands,
        std::vector<primitive_argument_type> const& args) const
    {
        // all remaining operands have to be lists
        std::vector<primitive_argument_type> lists;
        std::copy(operands.begin() + 1, operands.end(),
            std::back_inserter(lists));

        auto this_ = this->shared_from_this();
        return hpx::dataflow(hpx::launch::sync, hpx::util::unwrapping(
            [this_](primitive_argument_type&& bound_func,
                std::vector<primitive_argument_type>&& args)
            ->  primitive_argument_type
            {
                primitive const* p = util::get_if<primitive>(&bound_func);
                if (p == nullptr)
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "map_operation::eval",
                        this_->generate_error_message(
                            "the first argument to map must be an invocable "
                                "object"));
                }

                if (detail::all_list_operands(args))
                {
                    return this_->map_n_lists(p, std::move(args));
                }

                if (detail::all_numeric_operands(args))
                {
                    std::size_t dim = detail::extract_numeric_value_dimension(
                        args, this_->name_, this_->codename_);

                    switch (dim)
                    {
                    case 0:
                        return this_->map_n_scalar(p, std::move(args));

                    case 1:
                        return this_->map_n_vector(p, std::move(args));

                    case 2:
                        return this_->map_n_matrix(p, std::move(args));

                    default:
                        break;
                    }
                }

                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "map_operation::map_n",
                    this_->generate_error_message(
                        "all but the first arguments to map must be "
                            "compatible iterable objects (all lists or all "
                            "numeric)"));
            }),
            value_operand(operands_[0], args, name_, codename_,
                eval_dont_evaluate_lambdas),
            detail::map_operands(
                lists, functional::value_operand{}, args, name_, codename_));
    }

    hpx::future<primitive_argument_type> map_operation::eval(
        std::vector<primitive_argument_type> const& operands,
        std::vector<primitive_argument_type> const& args) const
    {
        if (operands.size() < 2)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "map_operation::eval",
                generate_error_message(
                    "the map_operation primitive requires "
                        "at least two operands"));
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
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "map_operation::eval",
                generate_error_message(
                    "the map_operation primitive requires that the "
                        "arguments given by the operands array are valid"));
        }

        // the first argument must be an invokable
        if (util::get_if<primitive>(&operands_[0]) == nullptr)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "map_operation::eval",
                generate_error_message(
                    "the first argument to map must be an invocable object"));
        }

        // handle common case separately
        if (operands.size() == 2)
        {
            return map_1(operands, args);
        }

        return map_n(operands, args);
    }

    // Start iteration over given for statement
    hpx::future<primitive_argument_type> map_operation::eval(
        std::vector<primitive_argument_type> const& args) const
    {
        if (this->no_operands())
        {
            return eval(args, noargs);
        }
        return eval(this->operands(), args);
    }
}}}
