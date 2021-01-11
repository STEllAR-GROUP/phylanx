//  Copyright (c) 2019 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_PYBIND11_EXECUTION_TREE_HPP)
#define PHYLANX_PYBIND11_EXECUTION_TREE_HPP

#include <phylanx/phylanx.hpp>

#include <bindings/binding_helpers.hpp>

#include <pybind11/numpy.h>
#include <pybind11/pybind11.h>

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>
#include <utility>

namespace phylanx { namespace execution_tree
{
    struct variable
    {
    private:
        static primitive create_variable(
            primitive_argument_type&& value, std::string const& name);
        static primitive create_variable(std::string const& name);

        static std::size_t variable_count;

    public:
        variable(bindings::compiler_state& state,
            primitive value, pybind11::object dtype,
            pybind11::object name = pybind11::none(),
            pybind11::object constraint = pybind11::none());

        variable(bindings::compiler_state& state,
            primitive value, pybind11::object dtype,
            char const* name = "Variable",
            pybind11::object constraint = pybind11::none());

        variable(bindings::compiler_state& state,
            pybind11::array value, pybind11::object dtype,
            pybind11::object name,
            pybind11::object constraint = pybind11::none());

        variable(bindings::compiler_state& state,
            pybind11::tuple shape, pybind11::object dtype,
            pybind11::object name,
            pybind11::object constraint = pybind11::none());

        variable(bindings::compiler_state& state,
            std::string value, pybind11::object dtype,
            pybind11::object name,
            pybind11::object constraint = pybind11::none());

        variable(bindings::compiler_state& state,
            primitive_argument_type value, pybind11::object dtype,
            pybind11::object name,
            pybind11::object constraint = pybind11::none());

        variable(bindings::compiler_state& state,
            primitive_argument_type value, pybind11::object dtype,
            char const* name = "Variable",
            pybind11::object constraint = pybind11::none());

        variable(bindings::compiler_state& state,
            primitive_argument_type value, pybind11::object dtype,
            std::string const& name,
            pybind11::object constraint = pybind11::none());

        ~variable()
        {
            pybind11::gil_scoped_acquire acquire;
            dtype_.release();
            constraint_.release();
        }

        pybind11::object eval(pybind11::args args) const;

        pybind11::dtype dtype() const;
        void dtype(pybind11::object dt);

        pybind11::tuple shape() const;
        void shape(pybind11::tuple sh);

        std::string name() const
        {
            return name_;
        }

        primitive value() const
        {
            return value_;
        }
        void value(primitive new_value)
        {
            value_ = std::move(new_value);

            if (!shape_.is_none())
            {
                pybind11::gil_scoped_acquire acquire;
                shape_ = pybind11::none();
            }
        }

        bindings::compiler_state& state()
        {
            return state_;
        }
        bindings::compiler_state const& state() const
        {
            return state_;
        }

    protected:
        pybind11::object handle_return_f(
            primitive_argument_type&& result, pybind11::ssize_t itemsize) const;

        pybind11::object handle_return_i(
            primitive_argument_type&& result, pybind11::ssize_t itemsize) const;

        pybind11::object handle_return_u(
            primitive_argument_type&& result, pybind11::ssize_t itemsize) const;

        pybind11::object handle_return_b(
            primitive_argument_type&& result) const;

        pybind11::object handle_return_S(
            primitive_argument_type&& result) const;

    private:
        bindings::compiler_state& state_;
        // We cannot use pybind11::dtype here because
        // we allow this field to be None.
        pybind11::object dtype_;
        std::string name_;
        primitive value_;
        pybind11::object constraint_;
        pybind11::object shape_;
    };

#define PHYLANX_VARIABLE_OPERATION_DECLARATION(op)                             \
    /* forward operation */                                                    \
    phylanx::execution_tree::variable op##_variables(                          \
        phylanx::execution_tree::variable& lhs,                                \
        phylanx::execution_tree::variable const& rhs);                         \
                                                                               \
    phylanx::execution_tree::variable op##_variables_gen(                      \
        phylanx::execution_tree::variable& lhs,                                \
        phylanx::execution_tree::primitive_argument_type const& rhs);          \
                                                                               \
    /* reverse operation */                                                    \
    phylanx::execution_tree::variable r##op##_variables_gen(                   \
        phylanx::execution_tree::variable& rhs,                                \
        phylanx::execution_tree::primitive_argument_type const& lhs);          \
    /**/

#define PHYLANX_VARIABLE_INPLACE_OPERATION_DECLARATION(op)                     \
    /* in-place operation */                                                   \
    phylanx::execution_tree::variable i##op##_variables(                       \
        phylanx::execution_tree::variable& lhs,                                \
        phylanx::execution_tree::variable const& rhs);                         \
                                                                               \
    phylanx::execution_tree::variable i##op##_variables_gen(                   \
        phylanx::execution_tree::variable& lhs,                                \
        phylanx::execution_tree::primitive_argument_type const& rhs);          \
        /**/

    ///////////////////////////////////////////////////////////////////////////
    // declare arithmetic operations
    PHYLANX_VARIABLE_OPERATION_DECLARATION(add)                 // __add__
    PHYLANX_VARIABLE_OPERATION_DECLARATION(sub)                 // __sub__
    PHYLANX_VARIABLE_OPERATION_DECLARATION(mul)                 // __mul__
    PHYLANX_VARIABLE_OPERATION_DECLARATION(div)                 // __div__

    PHYLANX_VARIABLE_INPLACE_OPERATION_DECLARATION(add)         // __iadd__
    PHYLANX_VARIABLE_INPLACE_OPERATION_DECLARATION(sub)         // __isub__

#undef PHYLANX_VARIABLE_OPERATION_DECLARATION
#undef PHYLANX_VARIABLE_INPLACE_OPERATION_DECLARATION

    ////////////////////////////////////////////////////////////////////////////
    phylanx::execution_tree::variable unary_minus_variables_gen(    // __neg__
        phylanx::execution_tree::variable& lhs);

    phylanx::execution_tree::variable moving_average_variables(
        phylanx::execution_tree::variable& var,
        phylanx::execution_tree::variable const& value,
        phylanx::execution_tree::primitive_argument_type const& momentum);

    phylanx::execution_tree::variable moving_average_variables_gen(
        phylanx::execution_tree::variable& var,
        phylanx::execution_tree::primitive_argument_type const& value,
        phylanx::execution_tree::primitive_argument_type const& momentum);

    ////////////////////////////////////////////////////////////////////////////
    phylanx::execution_tree::variable get_variable_size(    // __len__
        phylanx::execution_tree::variable& var);

    phylanx::execution_tree::variable get_variable_item(    // __get_item__
        phylanx::execution_tree::variable& var, std::size_t i);

    phylanx::execution_tree::variable set_variable_item(    // __set_item__
        phylanx::execution_tree::variable& var, std::size_t i,
        phylanx::execution_tree::primitive_argument_type const& value);
}}

#endif
