//  Copyright (c) 2019 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_PYBIND11_EXECUTION_TREE_HPP)
#define PHYLANX_PYBIND11_EXECUTION_TREE_HPP

#include <phylanx/phylanx.hpp>

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
        static std::size_t variable_count;

    public:
        variable(primitive value, pybind11::object dtype,
            pybind11::object name = pybind11::none(),
            pybind11::object constraint = pybind11::none());

        variable(pybind11::array value, pybind11::object dtype,
            pybind11::object name, pybind11::object constraint);

        variable(std::string value, pybind11::object dtype,
            pybind11::object name, pybind11::object constraint);

        variable(primitive_argument_type value, pybind11::object dtype,
            pybind11::object name, pybind11::object constraint);

        ~variable()
        {
            pybind11::gil_scoped_acquire acquire;
            dtype_.release();
            constraint_.release();
        }

        pybind11::handle eval(pybind11::args args) const;

        pybind11::dtype dtype() const;

        std::string name() const
        {
            return name_;
        }

        primitive value() const
        {
            return value_;
        }

    protected:
        pybind11::handle handle_return_f(
            primitive_argument_type&& result, pybind11::ssize_t itemsize) const;

        pybind11::handle handle_return_i(
            primitive_argument_type&& result, pybind11::ssize_t itemsize) const;

        pybind11::handle handle_return_u(
            primitive_argument_type&& result, pybind11::ssize_t itemsize) const;

        pybind11::handle handle_return_b(
            primitive_argument_type&& result) const;

        pybind11::handle handle_return_S(
            primitive_argument_type&& result) const;

    private:
        pybind11::dtype dtype_;
        std::string name_;
        primitive value_;
        pybind11::object constraint_;
    };
}}

#endif
