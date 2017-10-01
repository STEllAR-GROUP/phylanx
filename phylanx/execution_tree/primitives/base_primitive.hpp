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
    ///////////////////////////////////////////////////////////////////////////
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

    ///////////////////////////////////////////////////////////////////////////
    using primitive_value_type = phylanx::util::variant<
            phylanx::ast::nil
          , bool
          , std::int64_t
          , std::string
          , phylanx::ir::node_data<double>
          , primitive
        >;


    // a literal value is valid of its not nil{}
    inline bool valid(primitive_value_type const& val)
    {
        return val.index() != 0;
    }

    ///////////////////////////////////////////////////////////////////////////
    PHYLANX_EXPORT  primitive_value_type to_primitive_value_type(
        ast::literal_value_type && val);

    PHYLANX_EXPORT ir::node_data<double> extract_literal_value(
        primitive_value_type const& val);
}}

namespace phylanx { namespace execution_tree { namespace primitives
{
    ///////////////////////////////////////////////////////////////////////////
    // Factory functions
    using factory_function_type =
        hpx::util::function_nonser<
            primitive(hpx::id_type, std::vector<primitive_value_type>&&)
        >;

    template <typename Primitive>
    primitive create(hpx::id_type locality,
        std::vector<primitive_value_type>&& operands)
    {
        return primitive(hpx::new_<Primitive>(locality, std::move(operands)));
    }

    namespace detail
    {
        template <typename T, typename F>
        auto map_operands(std::vector<T> const& in, F && f)
        ->  std::vector<decltype(hpx::util::invoke(f, std::declval<T>()))>
        {
            std::vector<decltype(hpx::util::invoke(f, std::declval<T>()))> out;
            out.reserve(in.size());

            for (auto const& d : in)
            {
                out.push_back(hpx::util::invoke(f, d));
            }
            return out;
        }
    }
}}}

#endif


