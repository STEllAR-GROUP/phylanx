// Copyright (c) 2018 Tianyi Zhang
// Copyright (c) 2018 Bita Hasheminezhad
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef PHYLANX_GENERIC_OPERATION_HPP
#define PHYLANX_GENERIC_OPERATION_HPP

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/base_primitive.hpp>
#include <phylanx/execution_tree/primitives/primitive_component_base.hpp>
#include <phylanx/ir/node_data.hpp>

#include <hpx/lcos/future.hpp>

#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace phylanx { namespace execution_tree { namespace primitives
{
    class generic_operation
      : public primitive_component_base
      , public std::enable_shared_from_this<generic_operation>
    {
    protected:
        hpx::future<primitive_argument_type> eval(
            primitive_arguments_type const& operands,
            primitive_arguments_type const& args,
            eval_context ctx) const override;

        using arg_type = ir::node_data<double>;
        using args_type = std::vector<arg_type, arguments_allocator<arg_type>>;

        using dynamic_vector_type = ir::node_data<double>::storage1d_type;

        using dynamic_matrix_type = ir::node_data<double>::storage2d_type;

    public:
        static std::vector<match_pattern_type> const match_data;

        generic_operation() = default;

        generic_operation(primitive_arguments_type&& operands,
            std::string const& name, std::string const& codename);

    public:
        using scalar_function = double(double);
        using matrix_vector_function = arg_type(arg_type&&);

        using scalar_function_ptr = scalar_function*;
        using matrix_vector_function_ptr = matrix_vector_function*;

    private:
        primitive_argument_type generic0d(arg_type&& op) const;
        primitive_argument_type generic1d(arg_type&& op) const;
        primitive_argument_type generic2d(arg_type&& op) const;

        scalar_function_ptr get_0d_map(std::string const& name) const;
        matrix_vector_function_ptr get_1d_map(std::string const& name) const;
        matrix_vector_function_ptr get_2d_map(std::string const& name) const;

        scalar_function_ptr func0d_;
        matrix_vector_function_ptr func1d_;
        matrix_vector_function_ptr func2d_;
    };

    inline primitive create_generic_operation(hpx::id_type const& locality,
        primitive_arguments_type&& operands,
        std::string const& name = "", std::string const& codename = "")
    {
        return create_primitive_component(
            locality, "__gen", std::move(operands), name, codename);
    }
}}}

#endif    //PHYLANX_GENERIC_OPERATION_HPP
