// Copyright (c) 2018 Bibek Wagle
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef PHYLANX_SET_OPERATION_1153_02182018_HPP
#define PHYLANX_SET_OPERATION_1153_02182018_HPP

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/base_primitive.hpp>
#include <phylanx/execution_tree/primitives/primitive_component_base.hpp>

#include <hpx/lcos/future.hpp>

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace phylanx { namespace execution_tree { namespace primitives
{
    ///
    /// \brief Set Primitive
    ///
    /// This primitive sets new value to  a slice of the original data.
    /// \param operands Vector of phylanx node data objects of
    /// size eight
    ///
    /// If used inside PhySL:
    ///
    ///      slice (input, row_start, row_stop, row_steps
    ///                , col_start, col_stop, col_steps , value_to_set
    ///          )
    ///
    ///          input         : Vector or a Matrix
    ///          row_start     : Starting index of the slice (row)
    ///          row_stop      : Stopping index of the slice (row)
    ///          row_steps     : Go from row_start to row_stop in row_steps
    ///          col_start     : Starting index of the slice (column)
    ///          col_stop      : Stopping index of the slice (column)
    ///          col_steps     : Go from col_start to col_stop in steps
    ///          value_to_set  : value to set to the referenced region in the input
    ///
    ///  Note: Indices and steps can have negative values and negative values
    ///  indicate direction, similar to python.
    ///
    class set_operation
      : public primitive_component_base
      , public std::enable_shared_from_this<set_operation>
    {
    protected:
        using arg_type = ir::node_data<double>;
        using args_type = std::vector<arg_type>;
        using storage0d_type = typename arg_type::storage0d_type;
        using storage1d_type = typename arg_type::storage1d_type;
        using storage2d_type = typename arg_type::storage2d_type;

    public:
        static match_pattern_type const match_data;

        set_operation() = default;

        set_operation(std::vector<primitive_argument_type>&& operands,
            std::string const& name, std::string const& codename);

        hpx::future<primitive_argument_type> eval(
            std::vector<primitive_argument_type> const& params) const override;

    private:
        bool check_set_parameters(std::int64_t start, std::int64_t stop,
            std::int64_t step, std::size_t array_length) const;
        std::vector<std::int64_t> create_list_set(std::int64_t start,
            std::int64_t stop, std::int64_t step,
            std::size_t array_length) const;
        primitive_argument_type set0d(args_type&& args) const;
        primitive_argument_type set1d(args_type&& args) const;
        primitive_argument_type set2d(args_type&& args) const;
        hpx::future<primitive_argument_type> eval(
            std::vector<primitive_argument_type> const& operands,
            std::vector<primitive_argument_type> const& args) const;
    };

    inline primitive create_set_operation(hpx::id_type const& locality,
        std::vector<primitive_argument_type>&& operands,
        std::string const& name = "", std::string const& codename = "")
    {
        return create_primitive_component(
            locality, "set", std::move(operands), name, codename);
    }
}}}

#endif //PHYLANX_SET_OPERATION_1153_02182018_HPP
