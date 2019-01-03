// Copyright (c) 2018 Monil, Mohammad Alaul Haque
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
#ifndef PHYLANX_MEAN_OPERATION_HPP
#define PHYLANX_MEAN_OPERATION_HPP


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
    ///
    /// \brief Mean Operation Primitive
    ///
    /// This primitive computes the arithmetic mean along the specified axis.
    /// \param operands Vector of phylanx node data objects
    ///
    /// If used inside PhySL:
    ///
    ///      mean ( input, axis (optional) )
    ///
    ///          input : Scalar, Vector or a Matrix
    ///          axis     : The axis along which mean will be calculated
    ///
    class mean_operation
      : public primitive_component_base
      , public std::enable_shared_from_this<mean_operation>
    {
    protected:
        hpx::future<primitive_argument_type> eval(
            primitive_arguments_type const& operands,
            primitive_arguments_type const& args,
            eval_context ctx) const override;

    public:
        static match_pattern_type const match_data;

        mean_operation() = default;

        mean_operation(primitive_arguments_type&& operands,
            std::string const& name, std::string const& codename);

    private:
        primitive_argument_type mean0d(primitive_argument_type&& arg,
            bool has_axis = false, std::int64_t axis = 0) const;
        primitive_argument_type mean1d(primitive_argument_type&& arg,
            bool has_axis = false, std::int64_t axis = 0) const;
        primitive_argument_type mean2d(primitive_argument_type&& arg,
            bool has_axis = false, std::int64_t axis = 0) const;

        template <typename T>
        primitive_argument_type mean1d(ir::node_data<T>&& arg) const;

        template <typename T>
        primitive_argument_type mean2d_flatten(ir::node_data<T>&& arg) const;
        template <typename T>
        primitive_argument_type mean2d_x_axis(ir::node_data<T>&& arg) const;
        template <typename T>
        primitive_argument_type mean2d_y_axis(ir::node_data<T>&& arg) const;
        template <typename T>
        primitive_argument_type mean2d(ir::node_data<T>&& arg,
            bool has_axis, std::int64_t axis) const;

    private:
        node_data_type dtype_;
    };

    inline primitive create_mean_operation(hpx::id_type const& locality,
        primitive_arguments_type&& operands,
        std::string const& name = "", std::string const& codename = "")
    {
        return create_primitive_component(
            locality, "mean", std::move(operands), name, codename);
    }
}}}

#endif //PHYLANX_MEAN_OPERATION_HPP
