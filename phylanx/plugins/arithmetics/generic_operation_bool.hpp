// Copyright (c) 2019 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef PHYLANX_GENERIC_OPERATION_BOOL_HPP
#define PHYLANX_GENERIC_OPERATION_BOOL_HPP

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/base_primitive.hpp>
#include <phylanx/execution_tree/primitives/node_data_helpers.hpp>
#include <phylanx/execution_tree/primitives/primitive_component_base.hpp>
#include <phylanx/ir/node_data.hpp>

#include <hpx/lcos/future.hpp>

#include <cstdint>
#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace phylanx { namespace execution_tree { namespace primitives
{
    class generic_operation_bool
      : public primitive_component_base
      , public std::enable_shared_from_this<generic_operation_bool>
    {
    protected:
        hpx::future<primitive_argument_type> eval(
            primitive_arguments_type const& operands,
            primitive_arguments_type const& args,
            eval_context ctx) const override;

        template <typename T>
        using arg_type = ir::node_data<T>;

    public:
        static std::vector<match_pattern_type> const match_data;

        generic_operation_bool() = default;

        generic_operation_bool(primitive_arguments_type&& operands,
            std::string const& name, std::string const& codename);

    public:
        template <typename T>
        using scalar_function = bool(T);
        template <typename T>
        using matrix_vector_function = arg_type<std::uint8_t>(arg_type<T>&&);

        template <typename T>
        using scalar_function_ptr = scalar_function<T>*;
        template <typename T>
        using matrix_vector_function_ptr = matrix_vector_function<T>*;

    private:
        template <typename T>
        primitive_argument_type generic0d_bool(arg_type<T>&& op) const;
        template <typename T>
        primitive_argument_type generic1d_bool(arg_type<T>&& op) const;
        template <typename T>
        primitive_argument_type generic2d_bool(arg_type<T>&& op) const;

        primitive_argument_type generic0d_bool(
            primitive_argument_type&& op) const;
        primitive_argument_type generic1d_bool(
            primitive_argument_type&& op) const;
        primitive_argument_type generic2d_bool(
            primitive_argument_type&& op) const;

        template <typename T>
        static std::map<std::string, scalar_function_ptr<T>> const&
            get_0d_map();
        template <typename T>
        static std::map<std::string, matrix_vector_function_ptr<T>> const&
            get_1d_map();
        template <typename T>
        static std::map<std::string, matrix_vector_function_ptr<T>> const&
            get_2d_map();

        template <typename T>
        static scalar_function_ptr<T> get_0d_function(
            std::string const& funcname, std::string const& name,
            std::string const& codename);
        template <typename T>
        static matrix_vector_function_ptr<T> get_1d_function(
            std::string const& funcname, std::string const& name,
            std::string const& codename);
        template <typename T>
        static matrix_vector_function_ptr<T> get_2d_function(
            std::string const& funcname, std::string const& name,
            std::string const& codename);

#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
        template <typename T>
        primitive_argument_type generic3d_bool(arg_type<T>&& op) const;
        primitive_argument_type generic3d_bool(
            primitive_argument_type&& op) const;

        template <typename T>
        static std::map<std::string, matrix_vector_function_ptr<T>> const&
            get_3d_map();
        template <typename T>
        static matrix_vector_function_ptr<T> get_3d_function(
            std::string const& funcname, std::string const& name,
            std::string const& codename);
#endif
    private:
        std::string func_name_;
        node_data_type dtype_;
    };

    inline primitive create_generic_operation_bool(hpx::id_type const& locality,
        primitive_arguments_type&& operands,
        std::string const& name = "", std::string const& codename = "")
    {
        return create_primitive_component(
            locality, "__gen_bool", std::move(operands), name, codename);
    }
}}}

#endif    //PHYLANX_GENERIC_OPERATION_HPP
