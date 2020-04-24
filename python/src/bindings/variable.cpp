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

#include <hpx/assertion.hpp>
#include <hpx/format.hpp>

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>
#include <utility>

namespace phylanx { namespace execution_tree
{
    namespace detail
    {
        pybind11::dtype to_dtype(pybind11::object o)
        {
            if (pybind11::isinstance<pybind11::str>(o))
            {
                return pybind11::dtype(pybind11::cast<std::string>(o));
            }

//            HPX_ASSERT(pybind11::isinstance<pybind11::dtype>(o));
            return pybind11::dtype(std::move(o));
        }
    }

    ///////////////////////////////////////////////////////////////////////////
    variable::variable(bindings::compiler_state& state, primitive value,
        pybind11::object dtype, pybind11::object name,
        pybind11::object constraint)
      : state_(state)
      , dtype_(detail::to_dtype(std::move(dtype)))
      , name_(name.is_none() ?
                hpx::util::format("Variable_{}", ++variable_count) :
                name.cast<std::string>())
      , value_(std::move(value))
      , constraint_(std::move(constraint))
      , shape_(pybind11::none())
    {
    }

    variable::variable(bindings::compiler_state& state, primitive value,
        pybind11::object dtype, char const* name, pybind11::object constraint)
      : state_(state)
      , dtype_(detail::to_dtype(std::move(dtype)))
      , name_(hpx::util::format("{}_{}", name, ++variable_count))
      , value_(std::move(value))
      , constraint_(std::move(constraint))
      , shape_(pybind11::none())
    {
    }

    variable::variable(bindings::compiler_state& state, pybind11::array value,
        pybind11::object dtype, pybind11::object name,
        pybind11::object constraint)
      : state_(state)
      , dtype_(detail::to_dtype(
            dtype.is_none() ? value.dtype() : std::move(dtype)))
      , name_(name.is_none() ?
                hpx::util::format("Variable_{}", ++variable_count) :
                name.cast<std::string>())
      , value_(create_variable(
            pybind11::cast<primitive_argument_type>(std::move(value)), name_))
      , constraint_(std::move(constraint))
      , shape_(pybind11::none())
    {}

    variable::variable(bindings::compiler_state& state, pybind11::tuple shape,
        pybind11::object dtype, pybind11::object name,
        pybind11::object constraint)
      : state_(state)
      , dtype_(detail::to_dtype(std::move(dtype)))
      , name_(name.is_none() ?
                hpx::util::format("Variable_{}", ++variable_count) :
                name.cast<std::string>())
      , value_(create_variable(name_))
      , constraint_(std::move(constraint))
      , shape_(std::move(shape))
    {}

    variable::variable(bindings::compiler_state& state, std::string value,
        pybind11::object dtype, pybind11::object name,
        pybind11::object constraint)
      : state_(state)
      , dtype_(detail::to_dtype(
            dtype.is_none() ? pybind11::dtype("S") : std::move(dtype)))
      , name_(name.is_none() ?
                hpx::util::format("Variable_{}", ++variable_count) :
                name.cast<std::string>())
      , value_(
            create_variable(primitive_argument_type(std::move(value)), name_))
      , constraint_(std::move(constraint))
      , shape_(pybind11::none())
    {}

    variable::variable(bindings::compiler_state& state,
        primitive_argument_type value, pybind11::object dtype,
        pybind11::object name, pybind11::object constraint)
      : state_(state)
      , dtype_(detail::to_dtype(std::move(dtype)))
      , name_(name.is_none() ?
                hpx::util::format("Variable_{}", ++variable_count) :
                name.cast<std::string>())
      , value_(is_primitive_operand(value) ?
                primitive_operand(std::move(value)) :
                create_variable(std::move(value), name_))
      , constraint_(std::move(constraint))
      , shape_(pybind11::none())
    {}

    variable::variable(bindings::compiler_state& state,
        primitive_argument_type value, pybind11::object dtype, char const* name,
        pybind11::object constraint)
      : state_(state)
      , dtype_(detail::to_dtype(std::move(dtype)))
      , name_(hpx::util::format("{}_{}", name, ++variable_count))
      , value_(is_primitive_operand(value) ?
                primitive_operand(std::move(value)) :
                create_variable(std::move(value), name_))
      , constraint_(std::move(constraint))
      , shape_(pybind11::none())
    {}

    variable::variable(bindings::compiler_state& state,
        primitive_argument_type value, pybind11::object dtype,
        std::string const& name, pybind11::object constraint)
      : state_(state)
      , dtype_(detail::to_dtype(std::move(dtype)))
      , name_(hpx::util::format("{}_{}", name, ++variable_count))
      , value_(is_primitive_operand(value) ?
                primitive_operand(std::move(value)) :
                create_variable(std::move(value), name_))
      , constraint_(std::move(constraint))
      , shape_(pybind11::none())
    {}

    ///////////////////////////////////////////////////////////////////////////
    primitive variable::create_variable(
        primitive_argument_type&& value, std::string const& name)
    {
        return create_primitive_component(
            hpx::find_here(), "variable", std::move(value), name, "", false);
    }

    primitive variable::create_variable(std::string const& name)
    {
        return create_primitive_component(hpx::find_here(), "variable",
            primitive_argument_type{}, name, "", false);
    }

    std::size_t variable::variable_count = 0;

    ///////////////////////////////////////////////////////////////////////////
    pybind11::dtype variable::dtype() const
    {
        if (dtype_.is_none())
        {
            return bindings::extract_dtype(primitive_argument_type{value_});
        }
        return dtype_;
    }

    void variable::dtype(pybind11::object dt)
    {
        dtype_ = detail::to_dtype(std::move(dt));
    }

    ///////////////////////////////////////////////////////////////////////////
    pybind11::tuple variable::shape() const
    {
        if (shape_.is_none())
        {
            return bindings::extract_shape(primitive_argument_type{value_});
        }
        return pybind11::tuple(shape_);
    }

    void variable::shape(pybind11::tuple sh)
    {
        shape_ = sh;
    }

    ///////////////////////////////////////////////////////////////////////////
    namespace detail
    {
        template <typename T>
        pybind11::object convert_array(primitive_argument_type&& result)
        {
            pybind11::array_t<T> arr = pybind11::reinterpret_steal<
                pybind11::object>(
                pybind11::detail::make_caster<primitive_argument_type>::cast(
                    std::move(result), pybind11::return_value_policy::move,
                    pybind11::handle()));
            return arr;
        }
    }

    pybind11::object variable::handle_return_f(
        primitive_argument_type&& result, pybind11::ssize_t itemsize) const
    {
        if (itemsize == 4)
        {
            return detail::convert_array<float>(std::move(result));
        }

        HPX_ASSERT(itemsize == 8);
        return detail::convert_array<double>(std::move(result));
    }

    pybind11::object variable::handle_return_i(
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

    pybind11::object variable::handle_return_u(
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

    pybind11::object variable::handle_return_b(
        primitive_argument_type&& result) const
    {
        pybind11::bool_ b = pybind11::reinterpret_steal<pybind11::object>(
            pybind11::detail::make_caster<primitive_argument_type>::cast(
                std::move(result), pybind11::return_value_policy::move,
                pybind11::handle()));
        return b;
    }

    pybind11::object variable::handle_return_S(
        primitive_argument_type&& result) const
    {
        pybind11::str s = pybind11::reinterpret_steal<pybind11::object>(
            pybind11::detail::make_caster<primitive_argument_type>::cast(
                std::move(result), pybind11::return_value_policy::move,
                pybind11::handle()));
        return s;
    }

    ///////////////////////////////////////////////////////////////////////////
    pybind11::object variable::eval(pybind11::args args) const
    {
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

        static std::string varname("variable::eval");
        primitive_argument_type result = value_operand_sync(
            primitive_argument_type{value_}, std::move(fargs),
            varname, state().codename_);

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

        return pybind11::reinterpret_steal<pybind11::object>(
            pybind11::detail::make_caster<primitive_argument_type>::cast(
                std::move(result), pybind11::return_value_policy::move,
                pybind11::handle()));
    }

    ////////////////////////////////////////////////////////////////////////////
#define PHYLANX_VARIABLE_OPERATION(op, name)                                   \
    /* forward operation */                                                    \
    phylanx::execution_tree::variable op##_variables_gen(                      \
        phylanx::execution_tree::variable& lhs,                                \
        phylanx::execution_tree::primitive_argument_type const& rhs)           \
    {                                                                          \
        pybind11::gil_scoped_release release;                                  \
        return hpx::threads::run_as_hpx_thread(                                \
            [&]() -> phylanx::execution_tree::variable {                       \
                using namespace phylanx::execution_tree;                       \
                primitive_arguments_type args;                                 \
                args.reserve(2);                                               \
                args.emplace_back(primitive_argument_type{lhs.value()});       \
                args.emplace_back(rhs);                                        \
                auto var = primitives::create_##op##_operation(                \
                    hpx::find_here(), std::move(args), #op "_variables_gen",   \
                    lhs.state().codename_);                                    \
                pybind11::gil_scoped_acquire acquire;                          \
                return phylanx::execution_tree::variable{                      \
                    lhs.state(), std::move(var), lhs.dtype(), name};           \
            });                                                                \
    }                                                                          \
                                                                               \
    phylanx::execution_tree::variable op##_variables(                          \
        phylanx::execution_tree::variable& lhs,                                \
        phylanx::execution_tree::variable const& rhs)                          \
    {                                                                          \
        return op##_variables_gen(lhs, primitive_argument_type{rhs.value()});  \
    }                                                                          \
                                                                               \
    /* reverse operation */                                                    \
    phylanx::execution_tree::variable r##op##_variables_gen(                   \
        phylanx::execution_tree::variable& rhs,                                \
        phylanx::execution_tree::primitive_argument_type const& lhs)           \
    {                                                                          \
        pybind11::gil_scoped_release release;                                  \
        return hpx::threads::run_as_hpx_thread(                                \
            [&]() -> phylanx::execution_tree::variable {                       \
                using namespace phylanx::execution_tree;                       \
                primitive_arguments_type args;                                 \
                args.reserve(2);                                               \
                args.emplace_back(lhs);                                        \
                args.emplace_back(primitive_argument_type{rhs.value()});       \
                auto var = primitives::create_##op##_operation(                \
                    hpx::find_here(), std::move(args),                         \
                    "r" #op "_variables_gen", rhs.state().codename_);          \
                pybind11::gil_scoped_acquire acquire;                          \
                return phylanx::execution_tree::variable{                      \
                    rhs.state(), std::move(var), rhs.dtype(), name};           \
            });                                                                \
    }                                                                          \
    /**/

#define PHYLANX_VARIABLE_INPLACE_OPERATION(op, __name)                         \
    /* in-place operation */                                                   \
    phylanx::execution_tree::variable i##op##_variables_gen(                   \
        phylanx::execution_tree::variable& lhs,                                \
        phylanx::execution_tree::primitive_argument_type const& rhs)           \
    {                                                                          \
        pybind11::gil_scoped_release release;                                  \
        return hpx::threads::run_as_hpx_thread(                                \
            [&]() -> phylanx::execution_tree::variable {                       \
                using namespace phylanx::execution_tree;                       \
                                                                               \
                /* create operation */                                         \
                primitive_arguments_type args;                                 \
                args.reserve(2);                                               \
                args.emplace_back(primitive_argument_type{lhs.value()});       \
                args.emplace_back(rhs);                                        \
                primitive op = primitives::create_##op##_operation(            \
                    hpx::find_here(), std::move(args),                         \
                    "i" #op "_variables_gen", lhs.state().codename_);          \
                                                                               \
                /* set the new expression tree to the lhs variable */          \
                lhs.value(std::move(op));                                      \
                                                                               \
                /* return */                                                   \
                pybind11::gil_scoped_acquire acquire;                          \
                return phylanx::execution_tree::variable{                      \
                    lhs.state(), lhs.value(), lhs.dtype(), __name};            \
            });                                                                \
    }                                                                          \
                                                                               \
    phylanx::execution_tree::variable i##op##_variables(                       \
        phylanx::execution_tree::variable& lhs,                                \
        phylanx::execution_tree::variable const& rhs)                          \
    {                                                                          \
        return i##op##_variables_gen(                                          \
            lhs, primitive_argument_type{rhs.value()});                        \
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
    phylanx::execution_tree::variable unary_minus_variables_gen(    // __neg__
        phylanx::execution_tree::variable& target)
    {
        pybind11::gil_scoped_release release;

        return hpx::threads::run_as_hpx_thread(
            [&]() -> phylanx::execution_tree::variable {

                using namespace phylanx::execution_tree;

                primitive_arguments_type args;
                args.reserve(1);
                args.emplace_back(target.value());

                auto var = primitives::create_unary_minus_operation(
                    hpx::find_here(), std::move(args),
                    "unary_minus_variables_gen", target.state().codename_);

                pybind11::gil_scoped_acquire acquire;

                return phylanx::execution_tree::variable{
                    target.state(), std::move(var), target.dtype(), "Neg"};
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

        return hpx::threads::run_as_hpx_thread(
            [&]() -> phylanx::execution_tree::variable {

                // create operations
                primitive_arguments_type args;
                args.reserve(2);
                args.emplace_back(primitive_argument_type{1.0});
                args.emplace_back(momentum);
                primitive op1 = primitives::create_sub_operation(
                    hpx::find_here(), std::move(args), "moving_average",
                    var.state().codename_);

                args.reserve(2);
                args.emplace_back(var.value());
                args.emplace_back(value);
                primitive op2 = primitives::create_sub_operation(
                    hpx::find_here(), std::move(args), "moving_average",
                    var.state().codename_);

                args.reserve(2);
                args.emplace_back(std::move(op1));
                args.emplace_back(std::move(op2));
                primitive op3 = primitives::create_mul_operation(
                    hpx::find_here(), std::move(args), "moving_average",
                    var.state().codename_);

                args.reserve(2);
                args.emplace_back(var.value());
                args.emplace_back(std::move(op3));
                primitive result = primitives::create_sub_operation(
                    hpx::find_here(), std::move(args), "moving_average",
                    var.state().codename_);

                // set the new expression tree to the target variable
                var.value(std::move(result));

                pybind11::gil_scoped_acquire acquire;

                return phylanx::execution_tree::variable{
                    var.state(), var.value(), var.dtype(), "AssignMovingAvg"};
            });
    }

    phylanx::execution_tree::variable moving_average_variables(
        phylanx::execution_tree::variable& var,
        phylanx::execution_tree::variable const& value,
        phylanx::execution_tree::primitive_argument_type const& momentum)
    {
        return moving_average_variables_gen(
            var, primitive_argument_type{value.value()}, momentum);
    }

    ////////////////////////////////////////////////////////////////////////////
    phylanx::execution_tree::variable get_variable_size(    // __len__
        phylanx::execution_tree::variable& var)
    {
        // return length if shape is known
        if (!var.shape().is_none())
        {
            pybind11::tuple t(var.shape());
            if (t.size() != 0)
            {
                return phylanx::execution_tree::variable(var.state(),
                    phylanx::execution_tree::primitive_argument_type(
                        t[0].cast<std::int64_t>()),
                    pybind11::dtype("int64"), "Size");
            }
            // fall through if tuple is empty
        }

        pybind11::gil_scoped_release release;       // release GIL

        return hpx::threads::run_as_hpx_thread(
            [&]() -> phylanx::execution_tree::variable {

                static util::hashed_string funcname("_get_variable_size");
                static std::string expr(
                    "define(_get_variable_size, x, shape(x, 0))");

                auto& state = var.state();
                auto* p = state.eval_ctx_.get_var(funcname.key());
                if (p == nullptr)
                {
                    phylanx::execution_tree::compile(state.codename_,
                        funcname.key(), expr, state.eval_snippets_,
                        state.eval_env_)
                        .run(state.eval_ctx_);

                    // now, the variable should have been defined
                    p = state.eval_ctx_.get_var(funcname.key());
                    HPX_ASSERT(p != nullptr);
                }

                // now compose a target-reference that binds our variable to the
                // compiled function
                primitive_arguments_type args;
                args.emplace_back(var.value());

                auto result = phylanx::execution_tree::bind_arguments(
                    state.codename_, funcname.key(), state.eval_snippets_, *p,
                    std::move(args));

                pybind11::gil_scoped_acquire acquire;       // acquire GIL

                return phylanx::execution_tree::variable(
                    var.state(), std::move(result.arg_), var.dtype(), "Size");
            });
    }

    phylanx::execution_tree::variable get_variable_item(    // __get_item__
        phylanx::execution_tree::variable& var, std::size_t i)
    {
        if (var.shape().is_none())
        {
            throw pybind11::index_error();
        }

        pybind11::tuple t(var.shape());
        if (t.size() == 0)
        {
            throw pybind11::index_error();
        }

        if (i >= std::size_t(t[0].cast<std::int64_t>()))
        {
            throw pybind11::stop_iteration();
        }

        pybind11::gil_scoped_release release;       // release GIL

        return hpx::threads::run_as_hpx_thread(
            [&]() -> phylanx::execution_tree::variable {

                static util::hashed_string funcname("_get_variable_item");
                static std::string expr(
                    "define(_get_variable_item, x, i, slice(x, i))");

                auto& state = var.state();
                auto* p = state.eval_ctx_.get_var(funcname.key());
                if (p == nullptr)
                {
                    phylanx::execution_tree::compile(state.codename_,
                        funcname.key(), expr, state.eval_snippets_,
                        state.eval_env_)
                        .run(state.eval_ctx_);

                    // now, the variable should have been defined
                    p = state.eval_ctx_.get_var(funcname.key());
                    HPX_ASSERT(p != nullptr);
                }

                // now compose a target-reference that binds our variable to the
                // compiled function
                primitive_arguments_type args;
                args.emplace_back(var.value());
                args.emplace_back(std::int64_t(i));

                auto result = phylanx::execution_tree::bind_arguments(
                    state.codename_, funcname.key(), state.eval_snippets_, *p,
                    std::move(args));

                pybind11::gil_scoped_acquire acquire;    // acquire GIL

                return phylanx::execution_tree::variable{var.state(),
                    std::move(result.arg_), var.dtype(),
                    hpx::util::format("GetItem_{}", i)};
            });
    }

    ////////////////////////////////////////////////////////////////////////////
    phylanx::execution_tree::variable set_variable_item(    // __set_item__
        phylanx::execution_tree::variable& var, std::size_t i,
        phylanx::execution_tree::primitive_argument_type const& value)
    {
        if (var.shape().is_none())
        {
            throw pybind11::index_error();
        }

        pybind11::tuple t(var.shape());
        if (t.size() == 0)
        {
            throw pybind11::index_error();
        }

        if (i >= std::size_t(t[0].cast<std::int64_t>()))
        {
            throw pybind11::stop_iteration();
        }

        pybind11::gil_scoped_release release;       // release GIL

        return hpx::threads::run_as_hpx_thread(
            [&]() -> phylanx::execution_tree::variable {

                static util::hashed_string funcname("_set_variable_item");
                static std::string expr("define(_set_variable_item, x, i, val, "
                                        "store(slice(x, i), val))");

                auto& state = var.state();
                auto* p = state.eval_ctx_.get_var(funcname.key());
                if (p == nullptr)
                {
                    phylanx::execution_tree::compile(state.codename_,
                        funcname.key(), expr, state.eval_snippets_,
                        state.eval_env_)
                        .run(state.eval_ctx_);

                    // now, the variable should have been defined
                    p = state.eval_ctx_.get_var(funcname.key());
                    HPX_ASSERT(p != nullptr);
                }

                // now compose a target-reference that binds our variable to the
                // compiled function
                primitive_arguments_type args;
                args.emplace_back(var.value());
                args.emplace_back(std::int64_t(i));
                args.emplace_back(std::move(value));

                auto result = phylanx::execution_tree::bind_arguments(
                    state.codename_, funcname.key(), state.eval_snippets_, *p,
                    std::move(args));

                pybind11::gil_scoped_acquire acquire;    // acquire GIL

                return phylanx::execution_tree::variable{var.state(),
                    var.value(), var.dtype(),
                    hpx::util::format("SetItem_{}", i)};
            });
    }
}}
