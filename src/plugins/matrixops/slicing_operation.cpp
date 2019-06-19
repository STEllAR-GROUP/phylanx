// Copyright (c) 2017-2018 Bibek Wagle
// Copyright (c) 2017-2018 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/compiler/parse_primitive_name.hpp>
#include <phylanx/execution_tree/primitives/slice.hpp>
#include <phylanx/ir/node_data.hpp>
#include <phylanx/plugins/matrixops/slicing_operation.hpp>
#include <phylanx/util/slicing_helpers.hpp>

#include <hpx/include/lcos.hpp>
#include <hpx/include/naming.hpp>
#include <hpx/include/util.hpp>
#include <hpx/throw_exception.hpp>
#include <hpx/util/assert.hpp>

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <numeric>
#include <string>
#include <utility>
#include <vector>

///////////////////////////////////////////////////////////////////////////////
namespace phylanx {namespace execution_tree {    namespace primitives
{
    ///////////////////////////////////////////////////////////////////////////
    constexpr char const* const docstr = R"(
        v, ind
        Args:

            v (list) : a list to take a slice from
            ind (int or list) : index or slice range

        Returns:

        If `ind` is an integer, this operation returns an element of the
        list. If it is negative, indexing is done from the back of the list.
        Alternatively, `ind` may consist of a list of values which serve as
        the `start`, `stop`, and (optionally) `step` of a Python range. In this
        case, the return value is a new list with the values of `v` described
        by the range.
    )";

    ///////////////////////////////////////////////////////////////////////////
    std::vector<match_pattern_type> const slicing_operation::match_data =
    {
        match_pattern_type{"slice",
            std::vector<std::string>{"slice(_1, __2)"},
            &create_slicing_operation,
            &create_primitive<slicing_operation>,
            docstr
        },

        match_pattern_type{"slice_row",
            std::vector<std::string>{"slice_row(_1, __2)"},
            &create_slicing_operation,
            &create_primitive<slicing_operation>,
            docstr
        },

        match_pattern_type{"slice_column",
            std::vector<std::string>{"slice_column(_1, __2)"},
            &create_slicing_operation,
            &create_primitive<slicing_operation>,
            docstr
        },

        match_pattern_type{"tuple_slice",
            std::vector<std::string>{"tuple_slice(_1, _2)"},
            &create_slicing_operation,
            &create_primitive<slicing_operation>,
            docstr
        }
#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
      , match_pattern_type{"slice_page",
            std::vector<std::string>{"slice_page(_1, __2)"},
            &create_slicing_operation,
            &create_primitive<slicing_operation>,
            docstr
        }
#endif
    };

    ///////////////////////////////////////////////////////////////////////////
    slicing_operation::slicing_operation(
            primitive_arguments_type&& operands,
            std::string const& name, std::string const& codename)
      : primitive_component_base(std::move(operands), name, codename)
      , slice_rows_(false)
      , slice_columns_(false)
#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
      , slice_pages_(false)
#endif
      , is_tuple_slice_(false)
    {
        auto func_name = compiler::extract_primitive_name(name_);
        if (func_name == "slice_row")
        {
            slice_rows_ = true;
        }
        else if (func_name == "slice_column")
        {
            slice_columns_ = true;
        }
#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
        else if (func_name == "slice_page")
        {
            slice_pages_ = true;
        }
#endif
        else if (func_name == "tuple_slice")
        {
            is_tuple_slice_ = true;
        }
    }

    ///////////////////////////////////////////////////////////////////////////
    hpx::future<primitive_argument_type> slicing_operation::eval(
        primitive_arguments_type const& operands,
        primitive_arguments_type const& args, eval_context ctx) const
    {
#if !defined(PHYLANX_HAVE_BLAZE_TENSOR)
        if (operands.empty() || operands.size() > 3)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "phylanx::execution_tree::primitives::"
                "slicing_operation::eval",
                generate_error_message(
                    "the slicing_operation primitive requires "
                    "either one, two or three arguments"));
        }

        if ((slice_rows_ || slice_columns_) && operands.size() > 2)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "phylanx::execution_tree::primitives::"
                    "slicing_operation::eval",
                generate_error_message(
                    "the column/row-slicing_operation primitive requires "
                        "either one or two arguments"));
        }
#else
        if (operands.empty() || operands.size() > 4)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "phylanx::execution_tree::primitives::"
                "slicing_operation::eval",
                generate_error_message(
                    "the slicing_operation primitive requires "
                    "either one, two, three or four arguments"));
        }

        if ((slice_rows_ || slice_columns_ || slice_pages_) &&
            operands.size() > 2)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "phylanx::execution_tree::primitives::"
                    "slicing_operation::eval",
                generate_error_message(
                    "the page/column/row-slicing_operation primitive requires "
                        "either one or two arguments"));
        }
#endif
        if (is_tuple_slice_ && operands_.size() != 2)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "phylanx::execution_tree::primitives::"
                "slicing_operation::eval",
                generate_error_message(
                    "the tuple_slice primitive requires exactly two arguments"));
        }

        if (!valid(operands[0]) || (is_tuple_slice_ && !valid(operands[1])))
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "slicing_operation::eval",
                generate_error_message(
                    "the slicing_operation primitive requires "
                        "that the first argument given is valid"));
        }

        auto this_ = this->shared_from_this();
        return hpx::dataflow(hpx::launch::sync, hpx::util::unwrapping(
            [this_ = std::move(this_)](primitive_arguments_type&& ops)
            ->  primitive_argument_type
            {
                primitive_arguments_type args;
                if (this_->is_tuple_slice_)
                {
                    // in case of tuple_slice the first (and only) argument has
                    // to be a list of indices
                    auto&& r = extract_list_value_strict(
                        std::move(ops[1]), this_->name_, this_->codename_);
                    args.reserve(r.size() + 1);

                    // target has to be taken verbatim
                    args.push_back(std::move(ops[0]));

                    // evaluate all list elements
                    for (auto && op : r)
                    {
                        args.push_back(value_operand_sync(
                            std::move(op), this_->name_, this_->codename_));
                    }
                }
                else
                {
                    args = std::move(ops);
                }

                switch (args.size())
                {
                case 1:
                    return std::move(args[0]);

                case 2:
                    {
                        if (this_->slice_rows_)
                        {
                            return slice(args[0], args[1],
                                this_->name_, this_->codename_);
                        }
                        else if (this_->slice_columns_)
                        {
                            return slice(args[0], {}, args[1],
                                this_->name_, this_->codename_);
                        }
#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
                        else if (this_->slice_pages_)
                        {
                            return slice(args[0], args[1], {},
                                this_->name_, this_->codename_);
                        }
#endif
                        else
                        {
                            return slice(args[0], args[1],
                                this_->name_, this_->codename_);
                        }
                    }

                case 3:
                    return slice(args[0], args[1], args[2],
                        this_->name_, this_->codename_);

#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
                case 4:
                    return slice(args[0], args[1], args[2], args[3],
                        this_->name_, this_->codename_);
#endif
                default:
                    break;
                }

                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "phylanx::execution_tree::primitives::"
                        "slicing_operation::eval",
                    this_->generate_error_message(
                        "the slicing_operation primitive requires "
                            "either one, two, or three arguments"));
            }),
            detail::map_operands(
                operands, functional::value_operand{}, args,
                name_, codename_, std::move(ctx)));
    }
}}}
