// Copyright (c) 2017 Bibek Wagle
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef PHYLANX_SLICING_OPERATION_1153_10242017_HPP
#define PHYLANX_SLICING_OPERATION_1153_10242017_HPP

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/base_primitive.hpp>
#include <phylanx/execution_tree/primitives/primitive_component_base.hpp>
#include <phylanx/ir/node_data.hpp>
#include <phylanx/util/small_vector.hpp>

#include <hpx/lcos/future.hpp>

#include <array>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace phylanx { namespace execution_tree { namespace primitives
{
    class slicing_operation
      : public primitive_component_base
      , public std::enable_shared_from_this<slicing_operation>
    {
    protected:
        hpx::future<primitive_argument_type> eval(
            primitive_arguments_type const& operands,
            primitive_arguments_type const& args,
            eval_context ctx) const override;

    public:
        static std::vector<match_pattern_type> const match_data;

        slicing_operation() = default;

        /**
        * @brief Slicing Primitive
        *
        * This primitive returns a slice of the original data.
        *
        * If used inside PhySL:
        *
        *      slice (input,
        *           list(row_start, row_stop, row_steps),
        *           list(col_start, col_stop, col_steps)
        *      )
        *
        *      input: Scalar, Vector, Matrix, or a list
        *          row_start     : Starting index of the slice (row)
        *          row_stop      : Stopping index of the slice (row)
        *          row_steps     : Go from row_start to row_stop in row_steps
        *          col_start     : Starting index of the slice (column)
        *          col_stop      : Stopping index of the slice (column)
        *          col_steps     : Go from col_start to col_stop in steps
        *
        *  Quirks: In case the input is a vector, row_start, row_stop and row_steps
        *  determine the result. col_start, col_stop and col_step are ignored internally.
        *
        *  Note: Indices and steps can have negative values and negative values
        *        indicate direction, similar to python.
        */

        slicing_operation(primitive_arguments_type&& operands,
            std::string const& name, std::string const& codename);

    private:
        static std::string extract_function_name(std::string const& name);

        bool slice_rows_;
        bool slice_columns_;
#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
        bool slice_pages_;
#endif
    };

    inline primitive create_slicing_operation(hpx::id_type const& locality,
        primitive_arguments_type&& operands,
        std::string const& name = "", std::string const& codename = "")
    {
        return create_primitive_component(
            locality, "slice", std::move(operands), name, codename);
    }
}}}

#endif //PHYLANX_SLICING_OPERATION_1153_10242017_HPP
