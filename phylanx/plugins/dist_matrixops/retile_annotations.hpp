// Copyright (c) 2020 Bita Hasheminezhad
// Copyright (c) 2020 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_EXECUTION_TREE_PRIMITIVES_RETILE_ANNOTATIONS)
#define PHYLANX_EXECUTION_TREE_PRIMITIVES_RETILE_ANNOTATIONS

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/base_primitive.hpp>
#include <phylanx/execution_tree/primitives/node_data_helpers.hpp>
#include <phylanx/execution_tree/primitives/primitive_component_base.hpp>

#include <hpx/lcos/future.hpp>

#include <array>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace phylanx { namespace dist_matrixops { namespace primitives
{
    class retile_annotations
      : public execution_tree::primitives::primitive_component_base
      , public std::enable_shared_from_this<retile_annotations>
    {
    public:
        static execution_tree::match_pattern_type const match_data;

        retile_annotations() = default;

        retile_annotations(execution_tree::primitive_arguments_type&& operands,
            std::string const& name, std::string const& codename);

    protected:
        hpx::future<execution_tree::primitive_argument_type> eval(
            execution_tree::primitive_arguments_type const& operands,
            execution_tree::primitive_arguments_type const& args,
            execution_tree::eval_context ctx) const override;

    private:
        execution_tree::primitive_argument_type retile1d(
            execution_tree::primitive_argument_type&& arr,
            std::string const& tiling_type, std::size_t intersection,
            std::uint32_t numtiles, ir::range&& new_tiling) const;
        template <typename T>
        execution_tree::primitive_argument_type retile1d(ir::node_data<T>&& arr,
            std::string const& tiling_type, std::size_t intersection,
            std::uint32_t numtiles, ir::range&& new_tiling,
            execution_tree::localities_information&& arr_localities) const;

        execution_tree::primitive_argument_type retile2d(
            execution_tree::primitive_argument_type&& arr,
            std::string const& tiling_type,
            std::array<std::size_t, PHYLANX_MAX_DIMENSIONS> const& intersection,
            std::uint32_t numtiles, ir::range&& new_tiling) const;
        template <typename T>
        execution_tree::primitive_argument_type retile2d(ir::node_data<T>&& arr,
            std::string const& tiling_type,
            std::array<std::size_t, PHYLANX_MAX_DIMENSIONS> const& intersection,
            std::uint32_t numtiles, ir::range&& new_tiling,
            execution_tree::localities_information&& arr_localities) const;
    };

    inline execution_tree::primitive create_retile_annotations(
        hpx::id_type const& locality,
        execution_tree::primitive_arguments_type&& operands,
        std::string const& name = "", std::string const& codename = "")
    {
        return create_primitive_component(
            locality, "retile_d", std::move(operands), name, codename);
    }
}}}

#endif
