// Copyright (c) 2020 Hartmut Kaiser
// Copyright (c) 2020 Nanmiao Wu

// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_PRIMITIVES_DIST_DIAG)
#define PHYLANX_PRIMITIVES_DIST_DIAG

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/annotation.hpp>
#include <phylanx/execution_tree/localities_annotation.hpp>
#include <phylanx/execution_tree/primitives/base_primitive.hpp>
#include <phylanx/execution_tree/primitives/node_data_helpers.hpp>
#include <phylanx/execution_tree/primitives/primitive_component_base.hpp>
#include <phylanx/ir/node_data.hpp>

#include <hpx/lcos/future.hpp>

#include <cstdint>
#include <memory>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

namespace phylanx { namespace dist_matrixops { namespace primitives {
    class dist_diag
      : public execution_tree::primitives::primitive_component_base
      , public std::enable_shared_from_this<dist_diag>
    {
    public:
        static execution_tree::match_pattern_type const match_data;

        dist_diag() = default;

        dist_diag(execution_tree::primitive_arguments_type&& operands,
            std::string const& name, std::string const& codename);

    protected:
        hpx::future<execution_tree::primitive_argument_type> eval(
            execution_tree::primitive_arguments_type const& operands,
            execution_tree::primitive_arguments_type const& args,
            execution_tree::eval_context ctx) const override;

    private:
        template <typename T>
        execution_tree::primitive_argument_type dist_diag1d(
            ir::node_data<T>&& arr, std::int64_t k,
            std::string const& tiling_type, std::uint32_t tile_idx,
            std::uint32_t numtiles,
            execution_tree::localities_information&& arr_localities) const;

        execution_tree::primitive_argument_type dist_diag1d(
            execution_tree::primitive_argument_type&& arr, std::int64_t k,
            std::string const& tiling_type, std::uint32_t tile_idx,
            std::uint32_t numtiles) const;

        template <typename T>
        execution_tree::primitive_argument_type diag_1d_helper(
            ir::node_data<T>&& arr, std::int64_t k,
            execution_tree::localities_information&& arr_localities,
            std::int64_t row_start, std::int64_t column_start,
            std::size_t row_size, std::size_t column_size,
            std::int64_t des_start, std::int64_t des_size,
            std::int64_t num_band) const;

    };

    inline execution_tree::primitive create_dist_diag(
        hpx::id_type const& locality,
        execution_tree::primitive_arguments_type&& operands,
        std::string const& name = "", std::string const& codename = "")
    {
        return create_primitive_component(
            locality, "diag_d", std::move(operands), name, codename);
    }
}}}    // namespace phylanx::dist_matrixops::primitives

#endif
