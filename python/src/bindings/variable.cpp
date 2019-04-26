//  Copyright (c) 2019 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/phylanx.hpp>

#include <pybind11/numpy.h>
#include <pybind11/pybind11.h>

#include <bindings/binding_helpers.hpp>
#include <bindings/type_casters.hpp>
#include <bindings/variable.hpp>

#include <hpx/util/assert.hpp>
#include <hpx/util/format.hpp>

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>
#include <utility>

namespace phylanx { namespace execution_tree
{
    ///////////////////////////////////////////////////////////////////////////
    variable::variable(primitive value, pybind11::object dtype,
            pybind11::object name, pybind11::object constraint)
      : dtype_(std::move(dtype))
      , name_(name.is_none() ?
              hpx::util::format("Variable_{}", ++variable_count) :
              name.cast<std::string>())
      , value_(std::move(value))
      , constraint_(std::move(constraint))
    {
    }

    variable::variable(pybind11::array value, pybind11::object dtype,
        pybind11::object name, pybind11::object constraint)
      : dtype_(dtype.is_none() ? value.dtype() : std::move(dtype))
      , name_(name.is_none() ?
                hpx::util::format("Variable_{}", ++variable_count) :
                name.cast<std::string>())
      , value_(create_variable(
            pybind11::cast<primitive_argument_type>(std::move(value)), name_))
      , constraint_(std::move(constraint))
    {}

    variable::variable(std::string value, pybind11::object dtype,
        pybind11::object name, pybind11::object constraint)
      : dtype_(dtype.is_none() ? pybind11::dtype("S") : std::move(dtype))
      , name_(name.is_none() ?
                hpx::util::format("Variable_{}", ++variable_count) :
                name.cast<std::string>())
      , value_(
            create_variable(primitive_argument_type(std::move(value)), name_))
      , constraint_(std::move(constraint))
    {}

    variable::variable(primitive_argument_type value, pybind11::object dtype,
            pybind11::object name, pybind11::object constraint)
      : dtype_(std::move(dtype))
      , name_(name.is_none() ?
              hpx::util::format("Variable_{}", ++variable_count) :
              name.cast<std::string>())
      , value_(is_primitive_operand(value) ?
              primitive_operand(std::move(value)) :
              create_variable(std::move(value), name_))
      , constraint_(std::move(constraint))
    {}

    variable::variable(primitive value, pybind11::object dtype,
            char const* name, pybind11::object constraint)
      : dtype_(std::move(dtype))
      , name_(hpx::util::format("{}_{}", name, ++variable_count))
      , value_(is_primitive_operand(value) ?
              primitive_operand(std::move(value)) :
              create_variable(std::move(value), name_))
      , constraint_(std::move(constraint))
    {}

    variable::variable(primitive_argument_type value, pybind11::object dtype,
            char const* name, pybind11::object constraint)
      : dtype_(std::move(dtype))
      , name_(hpx::util::format("{}_{}", name, ++variable_count))
      , value_(is_primitive_operand(value) ?
              primitive_operand(std::move(value)) :
              create_variable(std::move(value), name_))
      , constraint_(std::move(constraint))
    {}

    ///////////////////////////////////////////////////////////////////////////
    primitive variable::create_variable(
        primitive_argument_type&& value, std::string const& name)
    {
        pybind11::gil_scoped_release release;       // release GIL

        auto f = [&]() {
            return create_primitive_component(hpx::find_here(), "variable",
                std::move(value), name, "", false);
        };

        if (hpx::threads::get_self_ptr() == nullptr)
        {
            return hpx::threads::run_as_hpx_thread(f);
        }
        return f();
    }

    std::size_t variable::variable_count = 0;

    ///////////////////////////////////////////////////////////////////////////
    pybind11::dtype variable::dtype() const
    {
        if (dtype_.is_none())
        {
            return bindings::extract_dtype(value_);
        }
        return dtype_;
    }

    ///////////////////////////////////////////////////////////////////////////
    namespace detail
    {
        template <typename T>
        pybind11::handle convert_array(primitive_argument_type&& result)
        {
            pybind11::array_t<T> arr = pybind11::cast(std::move(result),
                pybind11::return_value_policy::move);
            return arr.release();
        }
    }

    pybind11::handle variable::handle_return_f(
        primitive_argument_type&& result, pybind11::ssize_t itemsize) const
    {
        if (itemsize == 4)
        {
            return detail::convert_array<float>(std::move(result));
        }

        HPX_ASSERT(itemsize == 8);
        return detail::convert_array<double>(std::move(result));
    }

    pybind11::handle variable::handle_return_i(
        primitive_argument_type&& result, pybind11::ssize_t itemsize) const
    {
        if (itemsize == 2)
        {
            return detail::convert_array<std::int16_t>(std::move(result));
        }
        else if (itemsize == 4)
        {
            return detail::convert_array<std::int32_t>(std::move(result));
        }

        HPX_ASSERT(itemsize == 8);
        return detail::convert_array<std::int64_t>(std::move(result));
    }

    pybind11::handle variable::handle_return_u(
        primitive_argument_type&& result, pybind11::ssize_t itemsize) const
    {
        if (itemsize == 2)
        {
            return detail::convert_array<std::uint16_t>(std::move(result));
        }
        else if (itemsize == 4)
        {
            return detail::convert_array<std::uint32_t>(std::move(result));
        }

        HPX_ASSERT(itemsize == 8);
        return detail::convert_array<std::uint64_t>(std::move(result));
    }

    pybind11::handle variable::handle_return_b(
        primitive_argument_type&& result) const
    {
        pybind11::bool_ b = pybind11::cast(
            std::move(result), pybind11::return_value_policy::move);
        return b.release();
    }

    pybind11::handle variable::handle_return_S(
        primitive_argument_type&& result) const
    {
        pybind11::str s = pybind11::cast(
            std::move(result), pybind11::return_value_policy::move);
        return s.release();
    }

    ///////////////////////////////////////////////////////////////////////////
    pybind11::handle variable::eval(pybind11::args args) const
    {
        HPX_ASSERT(hpx::threads::get_self_ptr() != nullptr);

        phylanx::execution_tree::primitive_arguments_type keep_alive;
        keep_alive.reserve(args.size());
        phylanx::execution_tree::primitive_arguments_type fargs;
        fargs.reserve(args.size());

        {
            pybind11::gil_scoped_acquire acquire;
            for (auto const& item : args)
            {
                using phylanx::execution_tree::primitive_argument_type;

                primitive_argument_type value =
                    item.cast<primitive_argument_type>();

                keep_alive.emplace_back(std::move(value));
                fargs.emplace_back(extract_ref_value(keep_alive.back()));
            }
        }

        primitive_argument_type result =
            value_operand_sync(value_, std::move(fargs));

        // re-acquire GIL
        pybind11::gil_scoped_acquire acquire;

        // access dtype of result, if necessary
        if (!dtype_.is_none())
        {
            switch (dtype_.kind())
            {
            case 'b':   // boolean
                if (!is_boolean_operand_strict(result))
                {
                    return handle_return_b(std::move(result));
                }
                break;

            case 'i':   // signed integer
                if (!is_integer_operand_strict(result) || dtype_.itemsize() != 8)
                {
                    return handle_return_i(std::move(result), dtype_.itemsize());
                }
                break;

            case 'u':   // unsigned integer
                return handle_return_u(std::move(result), dtype_.itemsize());

            case 'f':   // floating-point
                if (!is_numeric_operand_strict(result) || dtype_.itemsize() != 8)
                {
                    return handle_return_f(std::move(result), dtype_.itemsize());
                }
                break;

            case 'O':   // object
                break;

            case 'S':   // (byte-)string
                if (!is_string_operand_strict(result))
                {
                    return handle_return_S(std::move(result));
                }
                break;

            case 'c': HPX_FALLTHROUGH;  // complex floating-point
            case 'm': HPX_FALLTHROUGH;  // timedelta
            case 'M': HPX_FALLTHROUGH;  // datetime
            case 'U': HPX_FALLTHROUGH;  // Unicode
            case 'V':                   // void
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "variable::eval",
                        hpx::util::format("unsupported dtype: {}", dtype_.kind()));
                }
                break;
            }
        }

        pybind11::object h = pybind11::cast(std::move(result),
            pybind11::return_value_policy::move);
        return h.release();
    }

    ////////////////////////////////////////////////////////////////////////////
#define PHYLANX_VARIABLE_OPERATION(op, name)                                   \
    /* forward operation */                                                    \
    phylanx::execution_tree::variable op##_variables_gen(                      \
        phylanx::execution_tree::variable const& lhs,                          \
        phylanx::execution_tree::primitive_argument_type const& rhs)           \
    {                                                                          \
        using namespace phylanx::execution_tree;                               \
        pybind11::gil_scoped_release release;                                  \
        return hpx::threads::run_as_hpx_thread([&]() {                         \
            primitive_arguments_type args;                                     \
            args.reserve(2);                                                   \
            args.emplace_back(lhs.value());                                    \
            args.emplace_back(rhs);                                            \
            auto p = primitives::create_##op##_operation(                      \
                hpx::find_here(), std::move(args));                            \
            pybind11::gil_scoped_acquire acquire;                              \
            return phylanx::execution_tree::variable{                          \
                std::move(p), lhs.dtype(), name};                              \
        });                                                                    \
    }                                                                          \
                                                                               \
    phylanx::execution_tree::variable op##_variables(                          \
        phylanx::execution_tree::variable const& lhs,                          \
        phylanx::execution_tree::variable const& rhs)                          \
    {                                                                          \
        return op##_variables_gen(lhs, rhs.value());                           \
    }                                                                          \
                                                                               \
    /* reverse operation */                                                    \
    phylanx::execution_tree::variable r##op##_variables_gen(                   \
        phylanx::execution_tree::variable const& rhs,                          \
        phylanx::execution_tree::primitive_argument_type const& lhs)           \
    {                                                                          \
        using namespace phylanx::execution_tree;                               \
        pybind11::gil_scoped_release release;                                  \
        return hpx::threads::run_as_hpx_thread([&]() {                         \
            primitive_arguments_type args;                                     \
            args.reserve(2);                                                   \
            args.emplace_back(lhs);                                            \
            args.emplace_back(rhs.value());                                    \
            auto p = primitives::create_##op##_operation(                      \
                hpx::find_here(), std::move(args));                            \
            pybind11::gil_scoped_acquire acquire;                              \
            return phylanx::execution_tree::variable{                          \
                std::move(p), rhs.dtype(), name};                              \
        });                                                                    \
    }                                                                          \
    /**/

#define PHYLANX_VARIABLE_INPLACE_OPERATION(op, __name)                         \
    /* in-place operation */                                                   \
    phylanx::execution_tree::variable i##op##_variables_gen(                   \
        phylanx::execution_tree::variable& lhs,                                \
        phylanx::execution_tree::primitive_argument_type const& rhs)           \
    {                                                                          \
        using namespace phylanx::execution_tree;                               \
                                                                               \
        pybind11::gil_scoped_release release;                                  \
        return hpx::threads::run_as_hpx_thread([&]() {                         \
            /* create operation */                                             \
            primitive_arguments_type args;                                     \
            args.reserve(2);                                                   \
            args.emplace_back(lhs.value());                                    \
            args.emplace_back(rhs);                                            \
            primitive op = primitives::create_##op##_operation(                \
                hpx::find_here(), std::move(args));                            \
                                                                               \
            pybind11::gil_scoped_acquire acquire;                              \
                                                                               \
            /* set the new expression tree to the lhs variable */              \
            lhs.value(std::move(op));                                          \
                                                                               \
            /* return */                                                       \
            return phylanx::execution_tree::variable{                          \
                lhs.value(), lhs.dtype(), __name};                             \
        });                                                                    \
    }                                                                          \
                                                                               \
    phylanx::execution_tree::variable i##op##_variables(                       \
        phylanx::execution_tree::variable& lhs,                                \
        phylanx::execution_tree::variable const& rhs)                          \
    {                                                                          \
        return i##op##_variables_gen(lhs, rhs.value());                        \
    }                                                                          \
        /**/

    ///////////////////////////////////////////////////////////////////////////
    // implement arithmetic operations
    PHYLANX_VARIABLE_OPERATION(add, "Add")                      // __add__
    PHYLANX_VARIABLE_OPERATION(sub, "Sub")                      // __sub__
    PHYLANX_VARIABLE_OPERATION(mul, "Mul")                      // __mul__
    PHYLANX_VARIABLE_OPERATION(div, "Mul")                      // __div__

    PHYLANX_VARIABLE_INPLACE_OPERATION(add, "AssignAdd")        // __iadd__
    PHYLANX_VARIABLE_INPLACE_OPERATION(sub, "AssignSub")        // __isub__

#undef PHYLANX_VARIABLE_OPERATION
#undef PHYLANX_VARIABLE_INPLACE_OPERATION

    ///////////////////////////////////////////////////////////////////////////
    phylanx::execution_tree::variable unary_minus_variables(    // __neg__
        phylanx::execution_tree::variable const& target)
    {
        using namespace phylanx::execution_tree;
        pybind11::gil_scoped_release release;
        return hpx::threads::run_as_hpx_thread([&]() {
            primitive_arguments_type args;
            args.reserve(1);
            auto p = primitives::create_unary_minus_operation(
                hpx::find_here(), std::move(args));
            pybind11::gil_scoped_acquire acquire;
            args.emplace_back(target.value());
            return phylanx::execution_tree::variable{
                std::move(p), target.dtype(), "Neg"};
        });
    }

    // The moving average of 'variable' updated with 'value' is:
    //
    //      variable * momentum + value * (1 - momentum)
    //
    // The returned Operation sets 'variable' to the newly computed moving
    // average, by performing this subtraction:
    //
    //      variable -= (1 - momentum) * (variable - value)
    //
    phylanx::execution_tree::variable moving_average_variables_gen(
        phylanx::execution_tree::variable& var,
        phylanx::execution_tree::primitive_argument_type const& value,
        phylanx::execution_tree::primitive_argument_type const& momentum)
    {
        pybind11::gil_scoped_release release;
        return hpx::threads::run_as_hpx_thread([&]() {
            // create operations
            primitive_arguments_type args;
            args.reserve(2);
            args.emplace_back(primitive_argument_type{1.0});
            args.emplace_back(momentum);
            primitive op1 = primitives::create_sub_operation(
                hpx::find_here(), std::move(args));

            args.reserve(2);
            args.emplace_back(var.value());
            args.emplace_back(value);
            primitive op2 = primitives::create_sub_operation(
                hpx::find_here(), std::move(args));

            args.reserve(2);
            args.emplace_back(std::move(op1));
            args.emplace_back(std::move(op2));
            primitive op3 = primitives::create_mul_operation(
                hpx::find_here(), std::move(args));

            args.reserve(2);
            args.emplace_back(var.value());
            args.emplace_back(std::move(op3));
            primitive result = primitives::create_sub_operation(
                hpx::find_here(), std::move(args));

            pybind11::gil_scoped_acquire acquire;

            // set the new expression tree to the target variable
            var.value(std::move(result));

            return phylanx::execution_tree::variable{
                var.value(), var.dtype(), "AssignMovingAvg"};
        });
    }

    phylanx::execution_tree::variable moving_average_variables(
        phylanx::execution_tree::variable& var,
        phylanx::execution_tree::variable const& value,
        phylanx::execution_tree::primitive_argument_type const& momentum)
    {
        return moving_average_variables_gen(var, value.value(), momentum);
    }
}}
