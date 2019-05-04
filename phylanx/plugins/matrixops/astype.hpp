//   Copyright (c) 2019 Hartmut Kaiser
//
//   Distributed under the Boost Software License, Version 1.0. (See accompanying
//   file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_PRIMITIVES_ASTYPE_2019_APR_17_0704PM)
#define PHYLANX_PRIMITIVES_ASTYPE_2019_APR_17_0704PM

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/base_primitive.hpp>
#include <phylanx/execution_tree/primitives/node_data_helpers.hpp>
#include <phylanx/execution_tree/primitives/primitive_component_base.hpp>
#include <phylanx/ir/node_data.hpp>

#include <hpx/lcos/future.hpp>

#include <cstdint>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace phylanx { namespace execution_tree { namespace primitives
{
    class astype
      : public primitive_component_base
      , public std::enable_shared_from_this<astype>
    {
    protected:
        hpx::future<primitive_argument_type> eval(
            primitive_arguments_type const& operands,
            primitive_arguments_type const& args,
            eval_context ctx) const override;

    public:
        static match_pattern_type const match_data;

        astype() = default;

        astype(primitive_arguments_type&& operands,
            std::string const& name, std::string const& codename);

    private:
        template <typename T>
        primitive_argument_type astype_helper(ir::node_data<T>&& op) const;
        primitive_argument_type astype_nd(
            primitive_argument_type&& op, node_data_type dtype) const;
    };

    ///////////////////////////////////////////////////////////////////////////
    inline primitive create_astype(hpx::id_type const& locality,
        primitive_arguments_type&& operands,
        std::string const& name = "", std::string const& codename = "")
    {
        return create_primitive_component(
            locality, "astype", std::move(operands), name, codename);
    }
}}}

#endif
