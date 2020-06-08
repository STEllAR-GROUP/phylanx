// Copyright (c) 2020 Nanmiao Wu
// Copyright (c) 2020 Bita Hasheminezhad
// Copyright (c) 2017-2020 Hartmut Kaiser

// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_PRIMITIVES_DIST_IDENTITY)
#define PHYLANX_PRIMITIVES_DIST_IDENTITY

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/annotation.hpp>
#include <phylanx/execution_tree/localities_annotation.hpp>
#include <phylanx/execution_tree/primitives/base_primitive.hpp>
#include <phylanx/execution_tree/primitives/node_data_helpers.hpp>
#include <phylanx/execution_tree/primitives/primitive_component_base.hpp>

#include <hpx/futures/future.hpp>

#include <cstdint>
#include <memory>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

namespace phylanx { namespace dist_matrixops { namespace primitives {
    class dist_identity
      : public execution_tree::primitives::primitive_component_base
      , public std::enable_shared_from_this<dist_identity>
    {
    public:
        static execution_tree::match_pattern_type const match_data;

        dist_identity() = default;

        dist_identity(execution_tree::primitive_arguments_type&& operands,
            std::string const& name, std::string const& codename);

    protected:
        hpx::future<execution_tree::primitive_argument_type> eval(
            execution_tree::primitive_arguments_type const& operands,
            execution_tree::primitive_arguments_type const& args,
            execution_tree::eval_context ctx) const override;

        template <typename T>
        execution_tree::primitive_argument_type dist_identity_helper(
            std::int64_t&& sz, std::uint32_t const& tile_idx,
            std::uint32_t const& numtiles, std::string&& given_name,
            std::string const& tiling_type) const;

        execution_tree::primitive_argument_type dist_identity_nd(
            std::int64_t&& sz, std::uint32_t const& tile_idx,
            std::uint32_t const& numtiles, std::string&& given_name,
            std::string const& tiling_type,
            execution_tree::node_data_type dtype) const;
    };

    inline execution_tree::primitive create_dist_identity(
        hpx::id_type const& locality,
        execution_tree::primitive_arguments_type&& operands,
        std::string const& name = "", std::string const& codename = "")
    {
        return create_primitive_component(
            locality, "identity_d", std::move(operands), name, codename);
    }
}}}    // namespace phylanx::dist_matrixops::primitives

#endif
