// Copyright (c) 2018 Tianyi Zhang
// Copyright (c) 2018 Bita Hasheminezhad
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/compiler/parse_primitive_name.hpp>
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
        "math library.",                                                       \
        true                                                                   \
    }                                                                          \
    /**/

    std::vector<std::pair<match_pattern_type, bool>> const
        generic_operation::match_data = {
            { PHYLANX_GEN_MATCH_DATA("absolute"), true },
            { PHYLANX_GEN_MATCH_DATA("floor"), false },
            { PHYLANX_GEN_MATCH_DATA("ceil"), false },
            { PHYLANX_GEN_MATCH_DATA("trunc"), false },
            { PHYLANX_GEN_MATCH_DATA("rint"), false },
            { PHYLANX_GEN_MATCH_DATA("conj"), false },
            { PHYLANX_GEN_MATCH_DATA("real"), false },
            { PHYLANX_GEN_MATCH_DATA("imag"), false },
            { PHYLANX_GEN_MATCH_DATA("sqrt"), false },
            { PHYLANX_GEN_MATCH_DATA("invsqrt"), false },
            { PHYLANX_GEN_MATCH_DATA("cbrt"), false },
            { PHYLANX_GEN_MATCH_DATA("invcbrt"), false },
            { PHYLANX_GEN_MATCH_DATA("exp"), false },
            { PHYLANX_GEN_MATCH_DATA("exp2"), false },
            { PHYLANX_GEN_MATCH_DATA("exp10"), false },
            { PHYLANX_GEN_MATCH_DATA("log"), false },
            { PHYLANX_GEN_MATCH_DATA("log2"), false },
            { PHYLANX_GEN_MATCH_DATA("log10"), false },
            { PHYLANX_GEN_MATCH_DATA("sin"), false },
            { PHYLANX_GEN_MATCH_DATA("cos"), false },
            { PHYLANX_GEN_MATCH_DATA("tan"), false },
            { PHYLANX_GEN_MATCH_DATA("sinh"), false },
            { PHYLANX_GEN_MATCH_DATA("cosh"), false },
            { PHYLANX_GEN_MATCH_DATA("tanh"), false },
            { PHYLANX_GEN_MATCH_DATA("arcsin"), false },
            { PHYLANX_GEN_MATCH_DATA("arccos"), false },
            { PHYLANX_GEN_MATCH_DATA("arctan"), false },
            { PHYLANX_GEN_MATCH_DATA("arcsinh"), false },
            { PHYLANX_GEN_MATCH_DATA("arccosh"), false },
            { PHYLANX_GEN_MATCH_DATA("arctanh"), false },
            { PHYLANX_GEN_MATCH_DATA("erf"), false },
            { PHYLANX_GEN_MATCH_DATA("erfc"), false },
            { PHYLANX_GEN_MATCH_DATA("square"), true },
            { PHYLANX_GEN_MATCH_DATA("sign"), true },
            { PHYLANX_GEN_MATCH_DATA("normalize"), false },
            { PHYLANX_GEN_MATCH_DATA("trace"), false },
    };

#undef PHYLANX_GEN_MATCH_DATA

    ///////////////////////////////////////////////////////////////////////////
    namespace detail
    {
        bool extract_argument_handling_mode(std::string const& funcname,
            std::string const& name, std::string const& codename)
        {
            for (auto && pattern : generic_operation::match_data)
            {
                if (pattern.first.primitive_type_ == funcname)
                    return pattern.second;
            }

            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "detail::extract_argument_handling_mode",
                util::generate_error_message(
                    "unknown function requested: " + funcname, name, codename));
        }
    }

    generic_operation::generic_operation(primitive_arguments_type&& operands,
            std::string const& name, std::string const& codename)
      : primitive_component_base(std::move(operands), name, codename)
      , func_name_(extract_function_name(name))
      , dtype_(extract_dtype(name_))
      , retain_argument_type_(detail::extract_argument_handling_mode(
            func_name_, name_, codename_))
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
        node_data_type t = dtype_;
        if (t == node_data_type_unknown)
        {
            t = retain_argument_type_ ? extract_common_type(op) :
                                        node_data_type_double;
        }

        switch (t)
        {
        case node_data_type_int64:
            return generic0d(
                extract_integer_value(std::move(op), name_, codename_));

        case node_data_type_double:
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
        node_data_type t = dtype_;
        if (t == node_data_type_unknown)
        {
            t = retain_argument_type_ ? extract_common_type(op) :
                                        node_data_type_double;
        }

        switch (t)
        {
        case node_data_type_int64:
            return generic1d(
                extract_integer_value(std::move(op), name_, codename_));

        case node_data_type_double:
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
        node_data_type t = dtype_;
        if (t == node_data_type_unknown)
        {
            t = retain_argument_type_ ? extract_common_type(op) :
                                        node_data_type_double;
        }

        switch (t)
        {
        case node_data_type_int64:
            return generic2d(
                extract_integer_value(std::move(op), name_, codename_));

        case node_data_type_double:
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
        node_data_type t = dtype_;
        if (t == node_data_type_unknown)
        {
            t = retain_argument_type_ ? extract_common_type(op) :
                                        node_data_type_double;
        }

        switch (t)
        {
        case node_data_type_int64:
            return generic3d(
                extract_integer_value(std::move(op), name_, codename_));

        case node_data_type_double:
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
        primitive_arguments_type&& args, eval_context ctx) const
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
        return hpx::dataflow(hpx::launch::sync, hpx::util::unwrapping(
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
            value_operand(operands[0], std::move(args),
                name_, codename_, std::move(ctx)));
    }
}}}
