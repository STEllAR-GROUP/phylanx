// Copyright (c) 2020 Rory Hector
// Copyright (c) 2017-2018 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_MATRIX_INV_OPERATION)
#define PHYLANX_MATRIX_INV_OPERATION

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/base_primitive.hpp>
#include <phylanx/execution_tree/primitives/node_data_helpers.hpp>
#include <phylanx/execution_tree/primitives/primitive_component_base.hpp>
#include <phylanx/ir/node_data.hpp>

#include <hpx/futures/future.hpp>

#include <memory>
#include <string>
#include <utility>
#include <vector>

#include <blaze/Math.h>

namespace phylanx { namespace dist_matrixops { namespace primitives {

    class dist_inverse
      : public execution_tree::primitives::primitive_component_base
      , public std::enable_shared_from_this<dist_inverse>
    {
    protected:
        hpx::future<execution_tree::primitive_argument_type> eval(
            execution_tree::primitive_arguments_type const& operands,
            execution_tree::primitive_arguments_type const& args,
            execution_tree::eval_context ctx) const override;

    public:
        // Declare match_data type that appears in cpp file
        static execution_tree::match_pattern_type const match_data;

        dist_inverse() = default;

        dist_inverse(execution_tree::primitive_arguments_type&& operands,
            std::string const& name, std::string const& codename);

    private:
        template <typename T>
        execution_tree::primitive_argument_type distGaussInv(
            ir::node_data<T>&& arg,
            execution_tree::localities_information&& lhs_localities) const;
        execution_tree::primitive_argument_type distGaussInv(
            execution_tree::primitive_argument_type&& lhs) const;
    };

    inline execution_tree::primitive create_dist_inverse(
        hpx::id_type const& locality,
        execution_tree::primitive_arguments_type&& operands,
        std::string const& name = "", std::string const& codename = "")
    {
        return create_primitive_component(
            locality, "inverse_d", std::move(operands), name, codename);
    }
}}}    // namespace phylanx::dist_matrixops::primitives

#endif
