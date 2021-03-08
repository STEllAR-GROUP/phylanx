//  Copyright (c) 2017-2019 Hartmut Kaiser
//  Copyright (c) 2019 Maxwell Reeser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_PRIMITIVES_DIST_CANNON_PRODUCT_OCT_17_2019_1130AM)
#define PHYLANX_PRIMITIVES_DIST_CANNON_PRODUCT_OCT_17_2019_1130AM

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/localities_annotation.hpp>
#include <phylanx/execution_tree/primitives/base_primitive.hpp>
#include <phylanx/execution_tree/primitives/primitive_component_base.hpp>
#include <phylanx/ir/node_data.hpp>

#include <hpx/futures/future.hpp>

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

namespace phylanx { namespace dist_matrixops { namespace primitives {

    class dist_cannon_product
      : public execution_tree::primitives::primitive_component_base
      , public std::enable_shared_from_this<dist_cannon_product>
    {
    protected:
        hpx::future<execution_tree::primitive_argument_type> eval(
            execution_tree::primitive_arguments_type const& operands,
            execution_tree::primitive_arguments_type const& args,
            execution_tree::eval_context ctx) const override;

    public:
        static execution_tree::match_pattern_type const match_data;

        dist_cannon_product() = default;

        dist_cannon_product(execution_tree::primitive_arguments_type&& operands,
            std::string const& name, std::string const& codename);

    private:
        template <typename T>
        execution_tree::primitive_argument_type product(ir::node_data<T>&& lhs,
            ir::node_data<T>&& rhs,
            execution_tree::localities_information&& lhs_localities,
            execution_tree::localities_information const& rhs_localities) const;

        template <typename T>
        execution_tree::primitive_argument_type dot2d2d(ir::node_data<T>&& lhs,
            ir::node_data<T>&& rhs,
            execution_tree::localities_information&& lhs_localities,
            execution_tree::localities_information const& rhs_localities) const;

        execution_tree::primitive_argument_type dot2d(
            execution_tree::primitive_argument_type&&,
            execution_tree::primitive_argument_type&&) const;

        execution_tree::primitive_argument_type dot_nd(
            execution_tree::primitive_argument_type&& lhs,
            execution_tree::primitive_argument_type&& rhs) const;

    private:
        std::int64_t get_transferred_bytes(bool reset) const;

        mutable std::int64_t transferred_bytes_;
    };

    inline execution_tree::primitive create_dist_cannon_product(
        hpx::id_type const& locality,
        execution_tree::primitive_arguments_type&& operands,
        std::string const& name = "", std::string const& codename = "")
    {
        return execution_tree::create_primitive_component(
            locality, "cannon_product_d", std::move(operands), name, codename);
    }
}}}
#endif
