// Copyright (c) 2018 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/node_data_helpers.hpp>
#include <phylanx/ir/node_data.hpp>
#include <phylanx/plugins/matrixops/count_nonzero_operation.hpp>

#include <hpx/include/lcos.hpp>
#include <hpx/include/naming.hpp>
#include <hpx/include/util.hpp>
#include <hpx/errors/throw_exception.hpp>

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
    match_pattern_type const count_nonzero_operation::match_data =
    {
        hpx::make_tuple("count_nonzero",
            std::vector<std::string>{"count_nonzero(_1)"},
            &create_count_nonzero_operation,
            &create_primitive<count_nonzero_operation>,
            R"(ar
            Args:

                ar (array) : a numeric array of values

            Returns:

            The number of array elements that are not zero.)"
            )
    };

    ///////////////////////////////////////////////////////////////////////////
    count_nonzero_operation::count_nonzero_operation(
            primitive_arguments_type&& operands,
            std::string const& name, std::string const& codename)
      : primitive_component_base(std::move(operands), name, codename)
    {}

    namespace detail
    {
        template <typename T>
        std::int64_t count_nonzero0d(ir::node_data<T>&& arg)
        {
            return arg.scalar() ? 1 : 0;
        }
    }

    primitive_argument_type count_nonzero_operation::count_nonzero0d(
        primitive_argument_type&& arg) const
    {
        switch (extract_common_type(arg))
        {
        case node_data_type_bool:
            return primitive_argument_type{detail::count_nonzero0d(
                extract_node_data<std::uint8_t>(std::move(arg)))};

        case node_data_type_int64:
            return primitive_argument_type{detail::count_nonzero0d(
                extract_node_data<std::int64_t>(std::move(arg)))};

        case node_data_type_double:
            return primitive_argument_type{detail::count_nonzero0d(
                extract_node_data<double>(std::move(arg)))};

        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "count_nonzero_operation::count_nonzero0d",
            util::generate_error_message(
                "unsupported operand type", name_, codename_));
    }

    namespace detail
    {
        template <typename T>
        std::int64_t count_nonzero1d(ir::node_data<T>&& arg)
        {
            return blaze::nonZeros(arg.vector());
        }
    }

    primitive_argument_type count_nonzero_operation::count_nonzero1d(
        primitive_argument_type&& arg) const
    {
        switch (extract_common_type(arg))
        {
        case node_data_type_bool:
            return primitive_argument_type{detail::count_nonzero1d(
                extract_node_data<std::uint8_t>(std::move(arg)))};

        case node_data_type_int64:
            return primitive_argument_type{detail::count_nonzero1d(
                extract_node_data<std::int64_t>(std::move(arg)))};

        case node_data_type_double:
            return primitive_argument_type{detail::count_nonzero1d(
                extract_node_data<double>(std::move(arg)))};

        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "count_nonzero_operation::count_nonzero1d",
            util::generate_error_message(
                "unsupported operand type", name_, codename_));
    }

    namespace detail
    {
        template <typename T>
        std::int64_t count_nonzero2d(ir::node_data<T>&& arg)
        {
            return blaze::nonZeros(arg.matrix());
        }
    }

    primitive_argument_type count_nonzero_operation::count_nonzero2d(
        primitive_argument_type&& arg) const
    {
        switch (extract_common_type(arg))
        {
        case node_data_type_bool:
            return primitive_argument_type{detail::count_nonzero2d(
                extract_node_data<std::uint8_t>(std::move(arg)))};

        case node_data_type_int64:
            return primitive_argument_type{detail::count_nonzero2d(
                extract_node_data<std::int64_t>(std::move(arg)))};

        case node_data_type_double:
            return primitive_argument_type{detail::count_nonzero2d(
                extract_node_data<double>(std::move(arg)))};

        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "count_nonzero_operation::count_nonzero2d",
            util::generate_error_message(
                "unsupported operand type", name_, codename_));
    }

    ///////////////////////////////////////////////////////////////////////////
    hpx::future<primitive_argument_type> count_nonzero_operation::eval(
        primitive_arguments_type const& operands,
        primitive_arguments_type const& args, eval_context ctx) const
    {
        if (operands.size() != 1)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "count_nonzero_operation::eval",
                util::generate_error_message(
                    "the count_nonzero_operation primitive requires"
                        "exactly one operand",
                        name_, codename_));
        }

        if (!valid(operands[0]))
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "count_nonzero_operation::eval",
                util::generate_error_message(
                    "the count_nonzero_operation primitive requires that "
                        "the arguments given by the operands array "
                        "is valid",
                    name_, codename_));
        }

        auto this_ = this->shared_from_this();
        return hpx::dataflow(hpx::launch::sync, hpx::unwrapping(
            [this_ = std::move(this_)](primitive_argument_type&& arg)
            -> primitive_argument_type
            {
                switch (extract_numeric_value_dimension(
                    arg, this_->name_, this_->codename_))
                {
                case 0:
                    return this_->count_nonzero0d(std::move(arg));

                case 1:
                    return this_->count_nonzero1d(std::move(arg));

                case 2:
                    return this_->count_nonzero2d(std::move(arg));

                default:
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "count_nonzero_operation::eval",
                        util::generate_error_message(
                            "left hand side operand has unsupported "
                                "number of dimensions",
                            this_->name_, this_->codename_));
                }
            }),
            value_operand(operands[0], args, name_, codename_, std::move(ctx)));
    }
}}}
