// Copyright (c) 2017 Bibek Wagle
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
#ifndef PHYLANX_ROW_SLICING_HPP
#define PHYLANX_ROW_SLICING_HPP

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/base_primitive.hpp>
#include <phylanx/execution_tree/primitives/primitive_component_base.hpp>
#include <phylanx/ir/node_data.hpp>

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
    /// \brief Row Slicing Primitive
    ///
    /// This primitive returns a slice of the original data.
    /// \param operands Vector of phylanx node data objects of
    /// size either three or four
    ///
    /// If used inside PhySL:
    ///
    ///      slice_row (input, '(row_start, row_stop, steps(optional)) )
    ///
    ///          input : Scalar, Vector or a Matrix
    ///          row_start     : Starting index of the slice
    ///          row_stop      : Stopping index of the slice
    ///          steps          : Go from row_start to row_stop in steps
    ///  Note: Indices and steps can have negative values and negative values
    ///  indicate direction, similar to python.
    ///
    class row_slicing_operation
      : public primitive_component_base
      , public std::enable_shared_from_this<row_slicing_operation>
    {
    protected:
        using arg_type = ir::node_data<double>;
        // using args_type = std::vector<arg_type>;
        using storage0d_type = typename arg_type::storage0d_type;
        using storage1d_type = typename arg_type::storage1d_type;
        using storage2d_type = typename arg_type::storage2d_type;

        hpx::future<primitive_argument_type> eval(
            std::vector<primitive_argument_type> const& operands,
            std::vector<primitive_argument_type> const& args) const;

    public:
        static match_pattern_type const match_data;

        row_slicing_operation() = default;

        row_slicing_operation(std::vector<primitive_argument_type>&& operands,
            std::string const& name, std::string const& codename);

        hpx::future<primitive_argument_type> eval(
            std::vector<primitive_argument_type> const& params) const override;

    private:
        std::vector<std::int64_t> create_list_row(std::int64_t start,
            std::int64_t stop, std::int64_t step,
            std::size_t array_length) const;
        primitive_argument_type row_slicing0d(
            arg_type&& arg) const;
        primitive_argument_type row_slicing1d(
            arg_type&& arg, std::vector<double> extracted) const;
        primitive_argument_type row_slicing2d(
            arg_type&& arg, std::vector<double> extracted) const;
    };

    inline primitive create_row_slicing_operation(hpx::id_type const& locality,
        std::vector<primitive_argument_type>&& operands,
        std::string const& name = "", std::string const& codename = "")
    {
        return create_primitive_component(
            locality, "slice_row", std::move(operands), name, codename);
    }
}}}

#endif //PHYLANX_ROW_SLICING_HPP
