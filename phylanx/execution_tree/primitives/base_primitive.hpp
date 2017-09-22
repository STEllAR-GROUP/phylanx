//  Copyright (c) 2017 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_PRIMITIVES_BASE_PRIMITIVE_SEP_05_2017_1102AM)
#define PHYLANX_PRIMITIVES_BASE_PRIMITIVE_SEP_05_2017_1102AM

#include <phylanx/config.hpp>
#include <phylanx/ast/node.hpp>
#include <phylanx/ir/node_data.hpp>

#include <hpx/include/components.hpp>
#include <hpx/include/util.hpp>

#include <utility>
#include <vector>

namespace phylanx { namespace execution_tree { namespace primitives
{
    class HPX_COMPONENT_EXPORT base_primitive
      : public hpx::traits::detail::component_tag
    {
    public:
        base_primitive() = default;
        virtual ~base_primitive() = default;

        hpx::future<ir::node_data<double>> eval_nonvirtual()
        {
            return eval();
        }
        virtual hpx::future<ir::node_data<double>> eval() const = 0;

    public:
        HPX_DEFINE_COMPONENT_ACTION(base_primitive,
            eval_nonvirtual, eval_action);
    };
}}}

// Declaration of serialization support for the local_file actions
HPX_REGISTER_ACTION_DECLARATION(
    phylanx::execution_tree::primitives::base_primitive::eval_action,
    phylanx_primitive_eval_action);

namespace phylanx { namespace execution_tree
{
    class HPX_COMPONENT_EXPORT primitive
      : public hpx::components::client_base<primitive,
            primitives::base_primitive>
    {
    private:
        using base_type =
            hpx::components::client_base<primitive, primitives::base_primitive>;

    public:
        primitive() = default;

        explicit primitive(hpx::id_type const& id)
          : base_type(id)
        {
        }
        explicit primitive(hpx::id_type && id)
          : base_type(std::move(id))
        {
        }
        primitive(hpx::future<hpx::id_type> && fid)
          : base_type(std::move(fid))
        {
        }

        hpx::future<ir::node_data<double>> eval() const;
    };
}}

namespace phylanx { namespace execution_tree { namespace primitives
{
    ///////////////////////////////////////////////////////////////////////////
    // Factory functions
    using factory_function_type =
        hpx::util::function_nonser<primitive(hpx::id_type,
            std::vector<ast::literal_value_type>&&, std::vector<primitive>&&)>;

    template <typename Primitive>
    primitive create(hpx::id_type locality,
        std::vector<ast::literal_value_type>&& literals,
        std::vector<primitive>&& operands)
    {
        return primitive(hpx::new_<Primitive>(
            locality, std::move(literals), std::move(operands)));
    }
}}}

#endif


