// Copyright (c) 2018 Bita Hasheminezhad
// Copyright (c) 2018 Shahrzad Shirzad
// Copyright (c) 2018 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/ir/node_data.hpp>
#include <phylanx/plugins/matrixops/reshape_operation.hpp>
#include <phylanx/util/matrix_iterators.hpp>

#include <hpx/include/lcos.hpp>
#include <hpx/include/naming.hpp>
#include <hpx/include/util.hpp>
#include <hpx/throw_exception.hpp>
#include <hpx/util/iterator_facade.hpp>
#include <hpx/util/optional.hpp>

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include <blaze/Math.h>

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace execution_tree { namespace primitives
{
    ///////////////////////////////////////////////////////////////////////////
    match_pattern_type const reshape_operation::match_data =
    {
        hpx::util::make_tuple("reshape",
        std::vector<std::string>{"reshape(_1,_2)"},
        &create_reshape_operation, &create_primitive<reshape_operation>,
        "")
    };

    ///////////////////////////////////////////////////////////////////////////
    reshape_operation::reshape_operation(primitive_arguments_type&& operands,
        std::string const& name, std::string const& codename)
        : primitive_component_base(std::move(operands), name, codename)
        , dtype_(extract_dtype(name_))
    {}

    bool reshape_operation::validate_shape(std::int64_t n, ir::range&& arg) const
    {
        if (arg.size() == 1)
        {
            auto first = extract_scalar_integer_value(*arg.begin());
            if (first == -1)
                return true;
            else
                return first == n;
        }
        else if (arg.size() == 2)
        {
            auto first = extract_scalar_integer_value(*arg.begin());
            auto second = extract_scalar_integer_value(*arg.end());
            if (second == -1)
            {
                if (n % first == 0)
                    return true;
                return false;
            }
            else
            {
                return first * second == n;
            }
        }
        else
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "reshape_operation::validate_shape",
                util::generate_error_message("The given shape has an invalid "
                    "number of dimensions",
                    name_, codename_));
        }
    }

    template <typename T>
    primitive_argument_type reshape_operation::reshape0d(ir::node_data<T>&& arr,
        ir::range&& arg) const
    {
        switch(arg.size())
        {
        case 1:
            return primitive_argument_type{
                blaze::DynamicVector<T>{arr.scalar()}};
        case 2:
            return primitive_argument_type{
                blaze::DynamicMatrix<T>{1, 1, arr.scalar()}};
        default:
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "reshape_operation::eval",
                util::generate_error_message("operand a has an invalid "
                    "number of dimensions",
                    name_, codename_));
        }
    }

    primitive_argument_type reshape_operation::reshape0d(
        primitive_argument_type&& arr, ir::range&& arg) const
    {
        switch (extract_common_type(arr))
        {
        case node_data_type_bool:
            return reshape0d(
                extract_boolean_value_strict(std::move(arr), name_, codename_),
                std::move(arg));

        case node_data_type_int64:
            return reshape0d(
                extract_integer_value_strict(std::move(arr), name_, codename_),
                std::move(arg));

        case node_data_type_double:
            return reshape0d(
                extract_numeric_value_strict(std::move(arr), name_, codename_),
                std::move(arg));

        case node_data_type_unknown:
            return reshape0d(
                extract_numeric_value(std::move(arr), name_, codename_),
                std::move(arg));

        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "phylanx::execution_tree::primitives::reshape_operation::reshape0d",
            generate_error_message(
                "the reshape primitive requires for all arguments to "
                "be numeric data types"));
    }

    ///////////////////////////////////////////////////////////////////////////
    hpx::future<primitive_argument_type> reshape_operation::eval(
        primitive_arguments_type const& operands,
        primitive_arguments_type const& args,
        eval_context ctx) const
    {
        if (operands.size() != 2)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "reshape_operation::eval",
                util::generate_error_message(
                    "the reshape_operation primitive requires exactly two "
                    "operands",
                    name_, codename_));
        }

        for (auto const& i : operands)
        {
            if (!valid(i))
            {
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "reshape_operation::eval",
                    util::generate_error_message(
                        "the reshape_operation primitive requires that the "
                        "arguments given by the operands array are "
                        "valid",
                        name_, codename_));
            }
        }

        auto this_ = this->shared_from_this();
        return hpx::dataflow(hpx::launch::sync,
            hpx::util::unwrapping([this_ = std::move(this_)](
                                      primitive_argument_type&& arr,
                                      ir::range&& arg)
                                      -> primitive_argument_type {
                auto a_dims = extract_numeric_value_dimension(
                    arr, this_->name_, this_->codename_);

                switch (a_dims)
                {
                case 0:
                    if (this_->validate_shape(1, std::move(arg)))
                        return this_->reshape0d(std::move(arr), std::move(arg));

                    //case 1:
                    //    return this_->reshape1d(std::move(arr), std::move(args));

                    //case 2:
                    //    return this_->reshape2d(std::move(arr), std::move(args));

                default:
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "reshape_operation::eval",
                        util::generate_error_message("operand a has an invalid "
                            "number of dimensions",
                            this_->name_, this_->codename_));
                }
            }),
            value_operand(operands[0], args, name_, codename_, ctx),
            list_operand(operands[1], args, name_, codename_, ctx));
    }
}}}
