//  Copyright (c) 2018 Shahrzad Shirzad
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_PLUGINS_LINEAR_SOLVERS_PRIMITIVES_MAY_11_2018_1030AM)
#define PHYLANX_PLUGINS_LINEAR_SOLVERS_PRIMITIVES_MAY_11_2018_1030AM

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
    class linear_solver
      : public primitive_component_base
      , public std::enable_shared_from_this<linear_solver>
    {
    protected:
        hpx::future<primitive_argument_type> eval(
            primitive_arguments_type const& operands,
            primitive_arguments_type const& args,
            eval_context ctx) const override;

        using arg_type = ir::node_data<double>;
        using args_type = std::vector<arg_type, arguments_allocator<arg_type>>;
        using storage1d_type = typename arg_type::storage1d_type;
        using storage2d_type = typename arg_type::storage2d_type;

    public:
        static std::vector<match_pattern_type> const match_data;
        linear_solver() = default;

        linear_solver(primitive_arguments_type&& operands,
            std::string const& name, std::string const& codename);

        using vector_function = arg_type(args_type&&);
        using vector_function_ul = arg_type(
            arg_type&&, arg_type&&, std::string);

        using vector_function_ptr = vector_function*;
        using vector_function_ptr_ul = vector_function_ul*;

    private:
        vector_function_ptr get_lin_solver_map(std::string const& name) const;
        vector_function_ptr_ul get_lin_solver_map_ul(
            std::string const& name) const;

        vector_function_ptr func_;
        vector_function_ptr_ul func_ul_;

        primitive_argument_type calculate_linear_solver(args_type&& args) const;
        primitive_argument_type calculate_linear_solver(
            arg_type&& lhs, arg_type&& rhs, std::string ul) const;
    };

    inline primitive create_linear_solver(hpx::id_type const& locality,
        primitive_arguments_type&& operands,
        std::string const& name = "", std::string const& codename = "")
    {
        return create_primitive_component(
            locality, "_linear_solver", std::move(operands), name, codename);
    }
}}}

#endif
