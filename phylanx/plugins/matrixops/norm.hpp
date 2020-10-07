//   Copyright (c) 2020 Hartmut Kaiser
//
//   Distributed under the Boost Software License, Version 1.0. (See accompanying
//   file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/base_primitive.hpp>
#include <phylanx/execution_tree/primitives/node_data_helpers.hpp>
#include <phylanx/execution_tree/primitives/primitive_component_base.hpp>

#include <hpx/futures/future.hpp>

#include <cstdint>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace phylanx { namespace execution_tree { namespace primitives
{
    class norm
      : public primitive_component_base
      , public std::enable_shared_from_this<norm>
    {
    private:
        enum class ord_type
        {
            default_frobenius,
            frobenius,
            nuclear,
            inf,
            ninf,
            integer
        };

        ///////////////////////////////////////////////////////////////////////
        template <typename T>
        T frobenius_norm_vector(ir::node_data<T>&& data) const;
        template <typename T>
        T inf_norm_vector(ir::node_data<T>&& data) const;
        template <typename T>
        T ninf_norm_vector(ir::node_data<T>&& data) const;
        template <typename T>
        T norm_vector(ir::node_data<T>&& data, int ord) const;

        template <typename T>
        primitive_argument_type norm_helper_vector(ir::node_data<T>&& data,
            ord_type type, hpx::util::optional<int>&& ord,
            hpx::util::optional<int>&& axis, std::uint8_t keepdims,
            eval_context ctx) const;

        ///////////////////////////////////////////////////////////////////////
        template <typename T>
        T frobenius_norm_matrix(ir::node_data<T>&& data) const;
        template <typename T>
        T norm_matrix(ir::node_data<T>&& data, int ord, eval_context ctx) const;

        template <typename T>
        primitive_argument_type norm_helper_matrix(ir::node_data<T>&& data,
            ord_type type, hpx::util::optional<int>&& ord,
            hpx::util::optional<int>&& axis, std::uint8_t keepdims,
            eval_context ctx) const;

    protected:
        hpx::future<primitive_argument_type> eval(
            primitive_arguments_type const& operands,
            primitive_arguments_type const& args,
            eval_context ctx) const override;

        template <typename T>
        primitive_argument_type norm_helper(ir::node_data<T>&& data,
            ord_type type, hpx::util::optional<int>&& ord,
            hpx::util::optional<int>&& axis, std::uint8_t keepdims,
            eval_context ctx) const;

        primitive_argument_type calculate_norm(primitive_argument_type&& data,
            ord_type type, hpx::util::optional<int>&& ord,
            hpx::util::optional<int>&& axis, std::uint8_t keepdims,
            eval_context ctx) const;

    public:
        static match_pattern_type const match_data;

        norm() = default;

        norm(primitive_arguments_type&& args,
            std::string const& name, std::string const& codename);
    };

    inline primitive create_norm(hpx::id_type const& locality,
        primitive_arguments_type&& operands,
        std::string const& name = "", std::string const& codename = "")
    {
        return create_primitive_component(
            locality, "norm", std::move(operands), name, codename);
    }
}}}
