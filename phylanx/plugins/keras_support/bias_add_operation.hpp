// Copyright (c) 2019 Bita Hasheminezhad
// Copyright (c) 2019 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_PRIMITIVES_KERAS_BIAS_ADD_OPERATION)
#define PHYLANX_PRIMITIVES_KERAS_BIAS_ADD_OPERATION

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/base_primitive.hpp>
#include <phylanx/execution_tree/primitives/primitive_component_base.hpp>

#include <hpx/lcos/future.hpp>

#include <cstddef>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace phylanx { namespace execution_tree { namespace primitives {
/// \brief Adds a bias array to an array
/// \param x     The 3d (tensor) or 4d array (the quaternion)
/// \param bias  The array to be added to x. It can be a vector or have
///              1 dimension less than x.

    class bias_add_operation
        : public primitive_component_base
        , public std::enable_shared_from_this<bias_add_operation>
    {
    protected:
        hpx::future<primitive_argument_type> eval(
            primitive_arguments_type const& operands,
            primitive_arguments_type const& args,
            eval_context ctx) const override;

    public:
        static match_pattern_type const match_data;

        bias_add_operation() = default;

        bias_add_operation(primitive_arguments_type&& operands,
            std::string const& name, std::string const& codename);

    private:
        primitive_argument_type bias_add3d(
            ir::node_data<double>&& arg, ir::node_data<double>&& bias) const;
    };
    inline primitive create_bias_add_operation(
        hpx::id_type const& locality, primitive_arguments_type&& operands,
        std::string const& name = "", std::string const& codename = "")
    {
        return create_primitive_component(locality, "bias_add",
            std::move(operands), name, codename);
    }
}}}

#endif
