// Copyright (c) 2018 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_PRIMITIVES_NONZERO_AUG_05_2018_0404PM)
#define PHYLANX_PRIMITIVES_NONZERO_AUG_05_2018_0404PM

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/base_primitive.hpp>
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
    class nonzero_where
      : public primitive_component_base
      , public std::enable_shared_from_this<nonzero_where>
    {
    protected:
        hpx::future<primitive_argument_type> eval(
            std::vector<primitive_argument_type> const& operands,
            std::vector<primitive_argument_type> const& args) const;

    public:
        static std::vector<match_pattern_type> const match_data;

        nonzero_where() = default;

        nonzero_where(std::vector<primitive_argument_type>&& operands,
            std::string const& name, std::string const& codename);

        hpx::future<primitive_argument_type> eval(
            std::vector<primitive_argument_type> const& params,
            eval_mode) const override;

    private:
        struct visit_nonzero;
        struct visit_where;

        static std::string extract_function_name(std::string const& name);

        template <typename T>
        primitive_argument_type nonzero_elements(ir::node_data<T>&& op) const;

        template <typename R, typename T>
        primitive_argument_type where_elements0d(ir::node_data<T>&& op,
            primitive_argument_type&& lhs, primitive_argument_type&& rhs) const;
        template <typename R, typename T>
        primitive_argument_type where_elements1d(ir::node_data<T>&& op,
            primitive_argument_type&& lhs, primitive_argument_type&& rhs) const;
        template <typename R, typename T>
        primitive_argument_type where_elements2d(ir::node_data<T>&& op,
            primitive_argument_type&& lhs, primitive_argument_type&& rhs) const;

        template <typename R, typename T>
        primitive_argument_type where_elements(ir::node_data<T>&& op,
            primitive_argument_type&& lhs, primitive_argument_type&& rhs) const;

        bool nonzero_;
        bool where_;
    };

    inline primitive create_nonzero(
        hpx::id_type const& locality,
        std::vector<primitive_argument_type>&& operands,
        std::string const& name = "", std::string const& codename = "")
    {
        return create_primitive_component(
            locality, "nonzero", std::move(operands), name, codename);
    }

    inline primitive create_where(
        hpx::id_type const& locality,
        std::vector<primitive_argument_type>&& operands,
        std::string const& name = "", std::string const& codename = "")
    {
        return create_primitive_component(
            locality, "where", std::move(operands), name, codename);
    }
}}}

#endif


