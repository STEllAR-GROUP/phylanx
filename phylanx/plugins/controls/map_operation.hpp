// Copyright (c) 2018 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_MAP_OPERATION_MAR_08_2018_1153PM)
#define PHYLANX_MAP_OPERATION_MAR_08_2018_1153PM

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/base_primitive.hpp>
#include <phylanx/execution_tree/primitives/primitive_component_base.hpp>

#include <hpx/lcos/future.hpp>

#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace phylanx { namespace execution_tree { namespace primitives
{
    class map_operation
      : public primitive_component_base
      , public std::enable_shared_from_this<map_operation>
    {
    public:
        static match_pattern_type const match_data;

        map_operation() = default;

        map_operation(std::vector<primitive_argument_type>&& operands,
            std::string const& name, std::string const& codename);

        hpx::future<primitive_argument_type> eval(
            std::vector<primitive_argument_type> const& args) const override;

        bool bind(std::vector<primitive_argument_type> const& params,
            bind_mode mode) const override
        {
            return false;
        }

    protected:
        hpx::future<primitive_argument_type> eval(
            std::vector<primitive_argument_type> const& operands,
            std::vector<primitive_argument_type> const& args) const;

        hpx::future<primitive_argument_type> map_1(
            std::vector<primitive_argument_type> const& operands,
            std::vector<primitive_argument_type> const& args) const;
        hpx::future<primitive_argument_type> map_n(
            std::vector<primitive_argument_type> const& operands,
            std::vector<primitive_argument_type> const& args) const;

        primitive_argument_type map_1_scalar(
            primitive const* p, primitive_argument_type&& arg) const;
        primitive_argument_type map_1_vector(
            primitive const* p, primitive_argument_type&& arg) const;
        primitive_argument_type map_1_matrix(
            primitive const* p, primitive_argument_type&& arg) const;

        primitive_argument_type map_n_lists(primitive const* p,
            std::vector<primitive_argument_type>&& args) const;

        primitive_argument_type map_n_scalar(primitive const* p,
            std::vector<primitive_argument_type>&& args) const;
        primitive_argument_type map_n_vector(primitive const* p,
            std::vector<primitive_argument_type>&& args) const;
        primitive_argument_type map_n_matrix(primitive const* p,
            std::vector<primitive_argument_type>&& args) const;
    };

    inline primitive create_map_operation(hpx::id_type const& locality,
        std::vector<primitive_argument_type>&& operands,
        std::string const& name = "", std::string const& codename = "")
    {
        return create_primitive_component(
            locality, "map", std::move(operands), name, codename);
    }
}}}

#endif
