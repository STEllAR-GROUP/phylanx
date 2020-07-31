// Copyright (c) 2020 Hartmut Kaiser
// Copyright (c) 2020 Nanmiao Wu
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_PRIMITIVES_ALL_GATHER)
#define PHYLANX_PRIMITIVES_ALL_GATHER

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/annotation.hpp>
#include <phylanx/execution_tree/localities_annotation.hpp>
#include <phylanx/execution_tree/primitives/base_primitive.hpp>
#include <phylanx/execution_tree/primitives/node_data_helpers.hpp>
#include <phylanx/execution_tree/primitives/primitive_component_base.hpp>
#include <phylanx/ir/node_data.hpp>

#include <hpx/assertion.hpp>
#include <hpx/errors/throw_exception.hpp>
#include <hpx/include/lcos.hpp>
#include <hpx/include/naming.hpp>
#include <hpx/include/util.hpp>
#include <hpx/collectives.hpp>

#include <array>
#include <cstddef>
#include <cstdint>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

namespace phylanx { namespace dist_matrixops { namespace primitives {

    class all_gather
      : public execution_tree::primitives::primitive_component_base
      , public std::enable_shared_from_this<all_gather>
    {
    public:
        static execution_tree::match_pattern_type const match_data;

        all_gather() = default;

        all_gather(execution_tree::primitive_arguments_type&& operands,
            std::string const& name, std::string const& codename);

    protected:
        hpx::future<execution_tree::primitive_argument_type> eval(
            execution_tree::primitive_arguments_type const& operands,
            execution_tree::primitive_arguments_type const& args,
            execution_tree::eval_context ctx) const override;
    private:
        template <typename T>
        execution_tree::primitive_argument_type all_gather2d(
            ir::node_data<T>&& arr,
        execution_tree::localities_information&& locs) const;

        execution_tree::primitive_argument_type all_gather2d(
            execution_tree::primitive_argument_type&& arr) const;
    };

    inline execution_tree::primitive create_all_gather(
        hpx::id_type const& locality,
        execution_tree::primitive_arguments_type&& operands,
        std::string const& name = "", std::string const& codename = "")
    {
        return create_primitive_component(
            locality, "all_gather_d", std::move(operands), name, codename);
    }
}}}    // namespace phylanx::execution_tree::primitives

#endif
