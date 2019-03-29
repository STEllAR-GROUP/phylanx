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
        static primitive create_variable(primitive_argument_type&& value);
        static std::size_t variable_count;

    public:
        variable(primitive value, pybind11::object dtype, pybind11::object name,
            pybind11::object constraint);

        variable(pybind11::array value, pybind11::object dtype,
            pybind11::object name, pybind11::object constraint);

        variable(std::string value, pybind11::object dtype,
            pybind11::object name, pybind11::object constraint);

        variable(primitive_argument_type value, pybind11::object dtype,
            pybind11::object name, pybind11::object constraint);

//         template <typename T>
//         variable(std::vector<T> value, pybind11::dtype dtype,
//                 pybind11::object name, pybind11::object constraint)
//           : value_(ir::node_data<T>{std::move(value)})
//           , dtype_(pybind11::dtype(std::move(dtype)))
//           , name_(name.is_none() ? "" : name.cast<std::string>())
//           , constraint_(std::move(constraint))
//         {}
//         template <typename T>
//         variable(std::vector<std::vector<T>> value,
//                 pybind11::object dtype, pybind11::object name,
//                 pybind11::object constraint)
//           : value_(ir::node_data<T>{std::move(value)})
//           , dtype_(pybind11::dtype(std::move(dtype)))
//           , name_(name.is_none() ? "" : name.cast<std::string>())
//           , constraint_(std::move(constraint))
//         {}
// #if defined(PHYLANX_HAVE_BLAZE_TENSOR)
//         template <typename T>
//         variable(std::vector<std::vector<std::vector<T>>> value,
//                 pybind11::object dtype, pybind11::object name,
//                 pybind11::object constraint)
//           : value_(ir::node_data<T>{std::move(value)})
//           , dtype_(pybind11::dtype(std::move(dtype)))
//           , name_(name.is_none() ? "" : name.cast<std::string>())
//           , constraint_(std::move(constraint))
//         {}
// #endif

        ~variable()
        {
            pybind11::gil_scoped_acquire acquire;
            dtype_.release();
            constraint_.release();
        }

        pybind11::handle eval(pybind11::args args) const;

        pybind11::dtype dtype() const
        {
            return dtype_;
        };

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
        primitive value_;
        std::string name_;
        pybind11::object constraint_;
    };
}}

#endif
