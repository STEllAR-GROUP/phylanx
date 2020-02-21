// Copyright (c) 2017-2020 Hartmut Kaiser
// Copyright (c) 2020 Bita Hasheminezhad
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_PRIMITIVES_DIST_CONSTANT)
#define PHYLANX_PRIMITIVES_DIST_CONSTANT

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/annotation.hpp>
#include <phylanx/execution_tree/localities_annotation.hpp>
#include <phylanx/execution_tree/primitives/base_primitive.hpp>
#include <phylanx/execution_tree/primitives/node_data_helpers.hpp>
#include <phylanx/execution_tree/primitives/primitive_component_base.hpp>

#include <hpx/lcos/future.hpp>

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace phylanx { namespace dist_matrixops { namespace primitives
{
    class dist_constant
      : public execution_tree::primitives::primitive_component_base
      , public std::enable_shared_from_this<dist_constant>
    {
    protected:
        hpx::future<execution_tree::primitive_argument_type> eval(
            execution_tree::primitive_arguments_type const& operands,
            execution_tree::primitive_arguments_type const& args,
            execution_tree::eval_context ctx) const override;

        using operand_type = ir::node_data<double>;

    public:
        static execution_tree::match_pattern_type const match_data;

        dist_constant() = default;

        dist_constant(execution_tree::primitive_arguments_type&& operands,
            std::string const& name, std::string const& codename);

    private:
        template <typename T>
        execution_tree::primitive_argument_type constant1d_helper(
            execution_tree::primitive_argument_type&& value, std::size_t dim,
            std::size_t const& tile_idx, std::size_t const& numtiles,
            std::string const& tiling_type, std::string const& name,
            std::string const& codename) const;

        execution_tree::primitive_argument_type constant1d(
            execution_tree::primitive_argument_type&& value,
            operand_type::dimensions_type const& dims,
            std::size_t const& tile_idx, std::size_t const& numtiles,
            std::string const& tiling_type,
            execution_tree::node_data_type dtype, std::string const& name_,
            std::string const& codename_) const;
        //execution_tree::primitive_argument_type constant2d(
        //    execution_tree::primitive_argument_type&& arg) const;
        //execution_tree::primitive_argument_type constant2d(
        //    execution_tree::primitive_argument_type&& arg,
        //    ir::node_data<std::int64_t>&& axes) const;

        //template <typename T>
        //execution_tree::primitive_argument_type constant2d(
        //    ir::node_data<T>&& arg,
        //    execution_tree::localities_information&& localities) const;

        //execution_tree::primitive_argument_type constant3d(
        //    execution_tree::primitive_argument_type&& arg) const;
        //execution_tree::primitive_argument_type constant3d(
        //    execution_tree::primitive_argument_type&& arg,
        //    ir::node_data<std::int64_t>&& axes) const;

        //template <typename T>
        //execution_tree::primitive_argument_type constant3d(
        //    ir::node_data<T>&& arg,
        //    execution_tree::localities_information&& localities) const;
        //template <typename T>
        //execution_tree::primitive_argument_type constant3d(
        //    ir::node_data<T>&& arg, ir::node_data<std::int64_t>&& axes,
        //    execution_tree::localities_information&& localities) const;

    };

    inline execution_tree::primitive
    create_dist_constant(hpx::id_type const& locality,
        execution_tree::primitive_arguments_type&& operands,
        std::string const& name = "", std::string const& codename = "")
    {
        return execution_tree::create_primitive_component(
            locality, "constant_d", std::move(operands), name, codename);
    }
}}}

#endif
