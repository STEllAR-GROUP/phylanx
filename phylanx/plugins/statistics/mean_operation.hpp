// Copyright (c) 2018 Monil, Mohammad Alaul Haque
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef PHYLANX_MEAN_OPERATION_HPP
#define PHYLANX_MEAN_OPERATION_HPP

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/base_primitive.hpp>
#include <phylanx/plugins/statistics/statistics_base.hpp>

#include <string>
#include <utility>
#include <vector>

namespace phylanx { namespace execution_tree { namespace primitives
{
    ///////////////////////////////////////////////////////////////////////////
    namespace detail
    {
        template <typename T>
        struct statistics_mean_op;
    }

    /// \brief Mean Operation Primitive
    ///
    /// This primitive computes the arithmetic mean along the specified axis.
    /// \param operands Vector of phylanx node data objects
    ///
    /// If used inside PhySL:
    ///
    ///      mean ( input, axis (optional), keepdims (optional) )
    ///
    ///          input  : Scalar, Vector or a Matrix
    ///          axis   : The axis along which mean will be calculated
    ///          keepdims : keep dimension of input
    ///
    class mean_operation
      : public statistics<detail::statistics_mean_op, mean_operation>
    {
        using base_type =
            statistics<detail::statistics_mean_op, mean_operation>;

    public:
        static match_pattern_type const match_data;

        mean_operation() = default;

        mean_operation(primitive_arguments_type&& operands,
            std::string const& name, std::string const& codename);
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
