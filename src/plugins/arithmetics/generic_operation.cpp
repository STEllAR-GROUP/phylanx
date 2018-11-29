// Copyright (c) 2018 Tianyi Zhang
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/compiler/primitive_name.hpp>
#include <phylanx/execution_tree/primitives/node_data_helpers.hpp>
#include <phylanx/ir/node_data.hpp>
#include <phylanx/plugins/arithmetics/generic_operation.hpp>

#include <hpx/include/lcos.hpp>
#include <hpx/include/naming.hpp>
#include <hpx/include/util.hpp>
#include <hpx/throw_exception.hpp>
#include <hpx/util/assert.hpp>

#include <cstddef>
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
    match_pattern_type{name, std::vector<std::string>{name "(_1)"},            \
        &create_generic_operation, &create_primitive<generic_operation>,       \
        "arg\n"                                                                \
        "Args:\n"                                                              \
        "\n"                                                                   \
        "    arg (float) : a floating point number\n"                          \
        "\n"                                                                   \
        "Returns:\n"                                                           \
        "\n"                                                                   \
        "This function implements function `" name "` from Python's "          \
        "math library."                                                        \
    }                                                                          \
    /**/

    std::vector<match_pattern_type> const generic_operation::match_data = {
        PHYLANX_GEN_MATCH_DATA("amin"),
        PHYLANX_GEN_MATCH_DATA("amax"),
        PHYLANX_GEN_MATCH_DATA("absolute"),
        PHYLANX_GEN_MATCH_DATA("floor"),
        PHYLANX_GEN_MATCH_DATA("ceil"),
        PHYLANX_GEN_MATCH_DATA("trunc"),
        PHYLANX_GEN_MATCH_DATA("rint"),
        PHYLANX_GEN_MATCH_DATA("conj"),
        PHYLANX_GEN_MATCH_DATA("real"),
        PHYLANX_GEN_MATCH_DATA("imag"),
        PHYLANX_GEN_MATCH_DATA("sqrt"),
        PHYLANX_GEN_MATCH_DATA("invsqrt"),
        PHYLANX_GEN_MATCH_DATA("cbrt"),
        PHYLANX_GEN_MATCH_DATA("invcbrt"),
        PHYLANX_GEN_MATCH_DATA("exp"),
        PHYLANX_GEN_MATCH_DATA("exp2"),
        PHYLANX_GEN_MATCH_DATA("exp10"),
        PHYLANX_GEN_MATCH_DATA("log"),
        PHYLANX_GEN_MATCH_DATA("log2"),
        PHYLANX_GEN_MATCH_DATA("log10"),
        PHYLANX_GEN_MATCH_DATA("sin"),
        PHYLANX_GEN_MATCH_DATA("cos"),
        PHYLANX_GEN_MATCH_DATA("tan"),
        PHYLANX_GEN_MATCH_DATA("arcsin"),
        PHYLANX_GEN_MATCH_DATA("arccos"),
        PHYLANX_GEN_MATCH_DATA("arctan"),
        PHYLANX_GEN_MATCH_DATA("arcsinh"),
        PHYLANX_GEN_MATCH_DATA("arccosh"),
        PHYLANX_GEN_MATCH_DATA("arctanh"),
        PHYLANX_GEN_MATCH_DATA("erf"),
        PHYLANX_GEN_MATCH_DATA("erfc"),
    };

#undef PHYLANX_GEN_MATCH_DATA

    ///////////////////////////////////////////////////////////////////////////
    namespace detail
    {
        std::string extract_function_name(std::string const& name)
        {
            compiler::primitive_name_parts name_parts;
            if (!compiler::parse_primitive_name(name, name_parts))
            {
                return name;
            }

            return name_parts.primitive;
        }
    }

    ///////////////////////////////////////////////////////////////////////////
    generic_operation::generic_operation(
        primitive_arguments_type && operands,
        std::string const& name, std::string const& codename)
      : primitive_component_base(std::move(operands), name, codename)
      , func_name_(detail::extract_function_name(name))
    {
    }

    ///////////////////////////////////////////////////////////////////////////
    template <typename T>
    primitive_argument_type generic_operation::generic0d(arg_type<T>&& op) const
    {
        return primitive_argument_type{
            get_0d_function<T>(func_name_, name_, codename_)(op.scalar())};
    }

    template <typename T>
    primitive_argument_type generic_operation::generic1d(arg_type<T>&& op) const
    {
        return primitive_argument_type{
            get_1d_function<T>(func_name_, name_, codename_)(std::move(op))};
    }

    template <typename T>
    primitive_argument_type generic_operation::generic2d(arg_type<T>&& op) const
    {
        return primitive_argument_type{
            get_2d_function<T>(func_name_, name_, codename_)(std::move(op))};
    }

#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
    template <typename T>
    primitive_argument_type generic_operation::generic3d(arg_type<T>&& op) const
    {
        return primitive_argument_type{
            get_3d_function<T>(func_name_, name_, codename_)(std::move(op))};
    }
#endif

    ///////////////////////////////////////////////////////////////////////////
    primitive_argument_type generic_operation::generic0d(
        primitive_argument_type&& op) const
    {
        switch (extract_common_type(op))
        {
        case node_data_type_double:
            return generic0d(
                extract_numeric_value_strict(std::move(op), name_, codename_));

        case node_data_type_int64:
            return generic0d(
                extract_integer_value_strict(std::move(op), name_, codename_));

        case node_data_type_bool:
        case node_data_type_unknown:
            return generic0d(
                extract_numeric_value(std::move(op), name_, codename_));
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "generic_operation::generic0d",
            generate_error_message("operand has unsupported type"));
    }

    primitive_argument_type generic_operation::generic1d(
        primitive_argument_type&& op) const
    {
        switch (extract_common_type(op))
        {
        case node_data_type_double:
            return generic1d(
                extract_numeric_value_strict(std::move(op), name_, codename_));

        case node_data_type_int64:
            return generic1d(
                extract_integer_value_strict(std::move(op), name_, codename_));

        case node_data_type_bool:
        case node_data_type_unknown:
            return generic1d(
                extract_numeric_value(std::move(op), name_, codename_));
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "generic_operation::generic1d",
            generate_error_message("operand has unsupported type"));
    }

    primitive_argument_type generic_operation::generic2d(
        primitive_argument_type&& op) const
    {
        switch (extract_common_type(op))
        {
        case node_data_type_double:
            return generic2d(
                extract_numeric_value_strict(std::move(op), name_, codename_));

        case node_data_type_int64:
            return generic2d(
                extract_integer_value_strict(std::move(op), name_, codename_));

        case node_data_type_bool:
        case node_data_type_unknown:
            return generic2d(
                extract_numeric_value(std::move(op), name_, codename_));
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "generic_operation::generic2d",
            generate_error_message("operand has unsupported type"));
    }

#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
    primitive_argument_type generic_operation::generic3d(
        primitive_argument_type&& op) const
    {
        switch (extract_common_type(op))
        {
        case node_data_type_double:
            return generic3d(
                extract_numeric_value_strict(std::move(op), name_, codename_));

        case node_data_type_int64:
            return generic3d(
                extract_integer_value_strict(std::move(op), name_, codename_));

        case node_data_type_bool:
        case node_data_type_unknown:
            return generic3d(
                extract_numeric_value(std::move(op), name_, codename_));
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "generic_operation::generic3d",
            generate_error_message("operand has unsupported type"));
    }
#endif

    ///////////////////////////////////////////////////////////////////////////
    hpx::future<primitive_argument_type> generic_operation::eval(
        primitive_arguments_type const& operands,
        primitive_arguments_type const& args, eval_context ctx) const
    {
        if (operands.size() != 1)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "generic_operation::eval",
                generate_error_message(
                    "the generic_operation primitive requires"
                    "exactly one operand"));
        }

        if (!valid(operands[0]))
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "generic_operation::eval",
                generate_error_message(
                    "the generic_operation primitive requires "
                    "that the arguments given by the operands "
                    "array is valid"));
        }

        auto this_ = this->shared_from_this();
        return hpx::dataflow(hpx::launch::sync,
            hpx::util::unwrapping(
                [this_ = std::move(this_)](primitive_argument_type&& op)
                -> primitive_argument_type
                {
                    std::size_t dims = extract_numeric_value_dimension(
                        op, this_->name_, this_->codename_);
                    switch (dims)
                    {
                    case 0:
                        return this_->generic0d(std::move(op));

                    case 1:
                        return this_->generic1d(std::move(op));

                    case 2:
                        return this_->generic2d(std::move(op));

#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
                    case 3:
                        return this_->generic3d(std::move(op));
#endif
                    default:
                        HPX_THROW_EXCEPTION(hpx::bad_parameter,
                            "generic_operation::eval",
                            this_->generate_error_message(
                                "left hand side operand has unsupported "
                                "number of dimensions"));
                    }
                }),
            value_operand(operands[0], args, name_, codename_, std::move(ctx)));
    }
}}}
