// Copyright (c) 2018 Hartmut Kaiser
// Copyright (c) 2018 Bita Hasheminezhad
// Copyright (c) 2018 Parsa Amini
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_PLUGINS_MATRIXOPS_SQUEEZE_OPERATION)
#define PHYLANX_PLUGINS_MATRIXOPS_SQUEEZE_OPERATION

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/base_primitive.hpp>
#include <phylanx/execution_tree/primitives/node_data_helpers.hpp>
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
/// \brief squeeze removes single-dimensional entries from the shape of an
///        array.
/// \param a         The scalar, vector, or matrix to perform squeeze over
/// \param axis      Optional. If provided, squeeze is calculated along the
///                  provided axis for >2d arrays.

    class squeeze_operation
        : public primitive_component_base
        , public std::enable_shared_from_this<squeeze_operation>
    {
    protected:
        hpx::future<primitive_argument_type> eval(
            primitive_arguments_type const& operands,
            primitive_arguments_type const& args,
            eval_context ctx) const override;

    public:
        static match_pattern_type const match_data;

        squeeze_operation() = default;

        squeeze_operation(primitive_arguments_type&& operands,
            std::string const& name, std::string const& codename);

    private:
        primitive_argument_type squeeze0d(primitive_argument_type&& arg) const;
        primitive_argument_type squeeze1d(primitive_argument_type&& arg) const;
        primitive_argument_type squeeze2d(primitive_argument_type&& arg) const;

        template <typename T>
        primitive_argument_type squeeze1d(ir::node_data<T>&& arg) const;
        template <typename T>
        primitive_argument_type squeeze2d(ir::node_data<T>&& arg) const;

    private:
        node_data_type dtype_;
    };
    inline primitive create_squeeze_operation(hpx::id_type const& locality,
        primitive_arguments_type&& operands,
        std::string const& name = "", std::string const& codename = "")
    {
        return create_primitive_component(
            locality, "squeeze", std::move(operands), name, codename);
    }
}}}

#endif
