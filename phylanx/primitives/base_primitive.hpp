//  Copyright (c) 2017 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_PRIMITIVES_BASE_PRIMITIVE_SEP_05_2017_1102AM)
#define PHYLANX_PRIMITIVES_BASE_PRIMITIVE_SEP_05_2017_1102AM

#include <phylanx/config.hpp>
#include <phylanx/ir/node_data.hpp>

#include <hpx/include/components.hpp>

#include <utility>

namespace phylanx { namespace primitives
{
    class primitive;

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
        virtual hpx::future<ir::node_data<double>> eval() = 0;

    public:
        HPX_DEFINE_COMPONENT_ACTION(base_primitive,
            eval_nonvirtual, eval_action);
    };
}}

// Declaration of serialization support for the local_file actions
HPX_REGISTER_ACTION_DECLARATION(
    phylanx::primitives::base_primitive::eval_action,
    phylanx_primitive_eval_action);

namespace phylanx { namespace primitives
{
    class HPX_COMPONENT_EXPORT primitive
      : public hpx::components::client_base<primitive, base_primitive>
    {
    private:
        using base_type =
            hpx::components::client_base<primitive, base_primitive>;

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

#endif


