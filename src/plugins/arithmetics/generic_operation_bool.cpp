// Copyright (c) 2019 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/compiler/parse_primitive_name.hpp>
#include <phylanx/execution_tree/primitives/node_data_helpers.hpp>
#include <phylanx/ir/node_data.hpp>
#include <phylanx/plugins/arithmetics/generic_operation_bool.hpp>

#include <hpx/include/lcos.hpp>
#include <hpx/include/naming.hpp>
#include <hpx/include/util.hpp>
#include <hpx/throw_exception.hpp>
#include <hpx/util/assert.hpp>

#include <cstddef>
#include <cstdint>
#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace execution_tree { namespace primitives
{
///////////////////////////////////////////////////////////////////////////////
#define PHYLANX_GEN_MATCH_DATA(name)                                           \
    match_pattern_type                                                         \
    {                                                                          \
        name, std::vector<std::string>{name "(_1)"},                           \
            &create_generic_operation_bool,                                    \
            &create_primitive<generic_operation_bool>,                         \
            "arg\n"                                                            \
            "Args:\n"                                                          \
            "\n"                                                               \
            "    arg (float) : a floating point number\n"                      \
            "\n"                                                               \
            "Returns:\n"                                                       \
            "\n"                                                               \
            "This function implements function `" name "` from Python's "      \
            "math library.",                                                   \
            true                                                               \
    }                                                                          \
    /**/

    std::vector<match_pattern_type> const
        generic_operation_bool::match_data = {
            PHYLANX_GEN_MATCH_DATA("isnan"),
            PHYLANX_GEN_MATCH_DATA("isinf"),
            PHYLANX_GEN_MATCH_DATA("isneginf"),
            PHYLANX_GEN_MATCH_DATA("isposinf"),
            PHYLANX_GEN_MATCH_DATA("isfinite")
        };

#undef PHYLANX_GEN_MATCH_DATA

    ///////////////////////////////////////////////////////////////////////////
    generic_operation_bool::generic_operation_bool(
        primitive_arguments_type && operands,
        std::string const& name, std::string const& codename)
      : primitive_component_base(std::move(operands), name, codename)
      , func_name_(extract_function_name(name))
      , dtype_(extract_dtype(name_))
    {
    }

    ///////////////////////////////////////////////////////////////////////////
    template <typename T>
    primitive_argument_type generic_operation_bool::generic0d_bool(
        arg_type<T>&& op) const
    {
        return primitive_argument_type{
            get_0d_function<T>(func_name_, name_, codename_)(op.scalar())};
    }

    template <typename T>
    primitive_argument_type generic_operation_bool::generic1d_bool(
        arg_type<T>&& op) const
    {
        return primitive_argument_type{
            get_1d_function<T>(func_name_, name_, codename_)(std::move(op))};
    }

    template <typename T>
    primitive_argument_type generic_operation_bool::generic2d_bool(
        arg_type<T>&& op) const
    {
        return primitive_argument_type{
            get_2d_function<T>(func_name_, name_, codename_)(std::move(op))};
    }

#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
    template <typename T>
    primitive_argument_type generic_operation_bool::generic3d_bool(
        arg_type<T>&& op) const
    {
        return primitive_argument_type{
            get_3d_function<T>(func_name_, name_, codename_)(std::move(op))};
    }
#endif

    ///////////////////////////////////////////////////////////////////////////
    primitive_argument_type generic_operation_bool::generic0d_bool(
        primitive_argument_type&& op) const
    {
        node_data_type t = dtype_;
        if (t == node_data_type_unknown)
        {
            t = node_data_type_double;  // use double by default
        }

        switch (t)
        {
        case node_data_type_int64:
            return generic0d_bool(
                extract_integer_value(std::move(op), name_, codename_));

        case node_data_type_double:
        case node_data_type_bool:
        case node_data_type_unknown:
            return generic0d_bool(
                extract_numeric_value(std::move(op), name_, codename_));
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "generic_operation_bool::generic0d_bool",
            generate_error_message("operand has unsupported type"));
    }

    primitive_argument_type generic_operation_bool::generic1d_bool(
        primitive_argument_type&& op) const
    {
        node_data_type t = dtype_;
        if (t == node_data_type_unknown)
        {
            t = node_data_type_double;  // use double by default
        }

        switch (t)
        {
        case node_data_type_int64:
            return generic1d_bool(
                extract_integer_value(std::move(op), name_, codename_));

        case node_data_type_double:
        case node_data_type_bool:
        case node_data_type_unknown:
            return generic1d_bool(
                extract_numeric_value(std::move(op), name_, codename_));
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "generic_operation_bool::generic1d_bool",
            generate_error_message("operand has unsupported type"));
    }

    primitive_argument_type generic_operation_bool::generic2d_bool(
        primitive_argument_type&& op) const
    {
        node_data_type t = dtype_;
        if (t == node_data_type_unknown)
        {
            t = node_data_type_double;  // use double by default
        }

        switch (t)
        {
        case node_data_type_int64:
            return generic2d_bool(
                extract_integer_value(std::move(op), name_, codename_));

        case node_data_type_double:
        case node_data_type_bool:
        case node_data_type_unknown:
            return generic2d_bool(
                extract_numeric_value(std::move(op), name_, codename_));
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "generic_operation_bool::generic2d_bool",
            generate_error_message("operand has unsupported type"));
    }

#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
    primitive_argument_type generic_operation_bool::generic3d_bool(
        primitive_argument_type&& op) const
    {
        node_data_type t = dtype_;
        if (t == node_data_type_unknown)
        {
            t = node_data_type_double;  // use double by default
        }

        switch (t)
        {
        case node_data_type_int64:
            return generic3d_bool(
                extract_integer_value(std::move(op), name_, codename_));

        case node_data_type_double:
        case node_data_type_bool:
        case node_data_type_unknown:
            return generic3d_bool(
                extract_numeric_value(std::move(op), name_, codename_));
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "generic_operation_bool::generic3d_bool",
            generate_error_message("operand has unsupported type"));
    }
#endif

    ///////////////////////////////////////////////////////////////////////////
    hpx::future<primitive_argument_type> generic_operation_bool::eval(
        primitive_arguments_type const& operands,
        primitive_arguments_type&& args, eval_context ctx) const
    {
        if (operands.size() != 1)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "generic_operation_bool::eval",
                generate_error_message(
                    "the generic_operation_bool primitive requires"
                    "exactly one operand"));
        }

        if (!valid(operands[0]))
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "generic_operation_bool::eval",
                generate_error_message(
                    "the generic_operation_bool primitive requires "
                    "that the arguments given by the operands "
                    "array is valid"));
        }

        auto this_ = this->shared_from_this();
        return hpx::dataflow(hpx::launch::sync, hpx::util::unwrapping(
            [this_ = std::move(this_)](primitive_argument_type&& op)
            -> primitive_argument_type
            {
                std::size_t dims = extract_numeric_value_dimension(
                    op, this_->name_, this_->codename_);
                switch (dims)
                {
                case 0:
                    return this_->generic0d_bool(std::move(op));

                case 1:
                    return this_->generic1d_bool(std::move(op));

                case 2:
                    return this_->generic2d_bool(std::move(op));

#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
                case 3:
                    return this_->generic3d_bool(std::move(op));
#endif
                default:
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "generic_operation_bool::eval",
                        this_->generate_error_message(
                            "left hand side operand has unsupported "
                            "number of dimensions"));
                }
            }),
            value_operand(operands[0], std::move(args),
                name_, codename_, std::move(ctx)));
    }
}}}
