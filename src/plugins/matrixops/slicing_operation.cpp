// Copyright (c) 2017-2018 Bibek Wagle
// Copyright (c) 2017-2018 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/compiler/primitive_name.hpp>
#include <phylanx/execution_tree/primitives/slice.hpp>
#include <phylanx/execution_tree/primitives/slicing_helpers.hpp>
#include <phylanx/ir/node_data.hpp>
#include <phylanx/plugins/matrixops/slicing_operation.hpp>
#include <phylanx/util/small_vector.hpp>

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
namespace phylanx {namespace execution_tree {    namespace primitives {

    ///////////////////////////////////////////////////////////////////////////
    std::vector<match_pattern_type> const slicing_operation::match_data =
    {
        hpx::util::make_tuple("slice",
            std::vector<std::string>{"slice(_1, __2)"},
            &create_slicing_operation,
            &create_primitive<slicing_operation>),

        hpx::util::make_tuple("slice_row",
            std::vector<std::string>{"slice_row(_1, __2)"},
            &create_slicing_operation,
            &create_primitive<slicing_operation>),

        hpx::util::make_tuple("slice_column",
            std::vector<std::string>{"slice_column(_1, __2)"},
            &create_slicing_operation,
            &create_primitive<slicing_operation>)
    };

    ///////////////////////////////////////////////////////////////////////////
    slicing_operation::slicing_operation(
            std::vector<primitive_argument_type>&& operands,
            std::string const& name, std::string const& codename)
      : primitive_component_base(std::move(operands), name, codename)
      , slice_rows_(false)
      , slice_columns_(false)
    {
        auto func_name = extract_function_name(name_);
        if (func_name == "slice_row")
        {
            slice_rows_ = true;
        }
        else if (func_name == "slice_column")
        {
            slice_columns_ = true;
        }
    }

    ///////////////////////////////////////////////////////////////////////////
    std::string slicing_operation::extract_function_name(
        std::string const& name)
    {
        compiler::primitive_name_parts name_parts;
        if (!compiler::parse_primitive_name(name, name_parts))
        {
            std::string::size_type p = name.find_first_of("$");
            if (p != std::string::npos)
            {
                return name.substr(0, p);
            }
        }

        return name_parts.primitive;
    }

    hpx::future<primitive_argument_type> slicing_operation::eval(
        std::vector<primitive_argument_type> const& operands,
        std::vector<primitive_argument_type> const& args) const
    {
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

        if (!valid(operands[0]))
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "slicing_operation::eval",
                generate_error_message(
                    "the slicing_operation primitive requires "
                        "that the first argument given is valid"));
        }

        auto this_ = this->shared_from_this();
        return hpx::dataflow(hpx::launch::sync, hpx::util::unwrapping(
            [this_](util::small_vector<primitive_argument_type>&& args)
            ->  primitive_argument_type
            {
                switch (args.size())
                {
                case 1:
                    return std::move(args[0]);

                case 2:
                    {
                        if (this_->slice_rows_)
                        {
                            return slice(args[0], args[1], this_->name_,
                                this_->codename_);
                        }
                        else if (this_->slice_columns_)
                        {
                            return slice(args[0], {}, args[1], this_->name_,
                                this_->codename_);
                        }
                        else
                        {
                            return slice(args[0], args[1], this_->name_,
                                this_->codename_);
                        }
                    }

                case 3:
                    return slice(args[0], args[1], args[2], this_->name_,
                        this_->codename_);

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
            detail::map_operands_sv(
                operands, functional::value_operand{}, args, name_, codename_));
    }

    //////////////////////////////////////////////////////////////////////////
    hpx::future<primitive_argument_type> slicing_operation::eval(
        std::vector<primitive_argument_type> const& args, eval_mode) const
    {
        if (this->no_operands())
        {
            return eval(args, noargs);
        }
        return eval(this->operands(), args);
    }
}}}
