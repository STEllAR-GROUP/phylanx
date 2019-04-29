//  Copyright (c) 2017-2018 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/format_string.hpp>
#include <phylanx/util/generate_error_message.hpp>

#include <hpx/include/iostreams.hpp>
#include <hpx/include/lcos.hpp>
#include <hpx/include/naming.hpp>
#include <hpx/include/util.hpp>
#include <hpx/throw_exception.hpp>

#include <boost/utility/string_ref.hpp>

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <utility>
#include <vector>

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace execution_tree
{
    // specialize formatting of primitive_argument_types
    void format_value(std::ostream& os, boost::string_ref spec,
        primitive_argument_type const& value)
    {
        using hpx::util::detail::type_specifier;

        std::string fullspec("{:" + spec.to_string() + "}");
        if (spec.empty())
        {
            os << value;
        }
        else if (spec.ends_with(type_specifier<char const*>::value()))
        {
            hpx::util::format_to(os, fullspec, to_string(value).c_str());
        }
        else if (spec.ends_with(type_specifier<char>::value()))
        {
            if (!is_integer_operand(value))
            {
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "format_string::format_value",
                    util::generate_error_message(
                        "the format specifier %c expects to convert from "
                        "a value that is convertible to a 'char'"));
            }
            hpx::util::format_to(os, fullspec,
                static_cast<char>(extract_integer_value(value)[0]));
        }
        else if (spec.ends_with(type_specifier<signed char>::value()))
        {
            if (!is_integer_operand(value))
            {
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "format_string::format_value",
                    util::generate_error_message(
                        "the format specifier %hhd expects to convert from "
                        "a value that is convertible to a 'signed char'"));
            }
            hpx::util::format_to(os, fullspec,
                static_cast<signed char>(extract_integer_value(value)[0]));
        }
        else if (spec.ends_with(type_specifier<short>::value()))
        {
            if (!is_integer_operand(value))
            {
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "format_string::format_value",
                    util::generate_error_message(
                        "the format specifier %hd expects to convert from "
                        "a value that is convertible to a 'short'"));
            }
            hpx::util::format_to(os, fullspec,
                static_cast<short>(extract_integer_value(value)[0]));
        }
        else if (spec.ends_with(type_specifier<long long>::value()))
        {
            if (!is_integer_operand(value))
            {
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "format_string::format_value",
                    util::generate_error_message(
                        "the format specifier %lld expects to convert from "
                        "a value that is convertible to a 'long long'"));
            }
            hpx::util::format_to(os, fullspec,
                static_cast<long long>(extract_integer_value(value)[0]));
        }
        else if (spec.ends_with(type_specifier<long>::value()))
        {
            if (!is_integer_operand(value))
            {
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "format_string::format_value",
                    util::generate_error_message(
                        "the format specifier %ld expects to convert from "
                        "a value that is convertible to a 'long'"));
            }
            hpx::util::format_to(os, fullspec,
                static_cast<long>(extract_integer_value(value)[0]));
        }
        else if (spec.ends_with(type_specifier<int>::value()))
        {
            if (!is_integer_operand(value))
            {
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "format_string::format_value",
                    util::generate_error_message(
                        "the format specifier %d expects to convert from "
                        "a value that is convertible to an 'int'"));
            }
            hpx::util::format_to(os, fullspec,
                static_cast<int>(extract_integer_value(value)[0]));
        }
        else if (spec.ends_with(type_specifier<unsigned char>::value()))
        {
            if (!is_integer_operand(value))
            {
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "format_string::format_value",
                    util::generate_error_message(
                        "the format specifier %hhu expects to convert from "
                        "a value that is convertible to an 'unsigned char'"));
            }
            hpx::util::format_to(os, fullspec, static_cast<unsigned char>(
                extract_integer_value(value)[0]));
        }
        else if (spec.ends_with(type_specifier<unsigned short>::value()))
        {
            if (!is_integer_operand(value))
            {
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "format_string::format_value",
                    util::generate_error_message(
                        "the format specifier %hu expects to convert from "
                        "a value that is convertible to an 'unsigned "
                        "short'"));
            }
            hpx::util::format_to(os, fullspec, static_cast<unsigned short>(
                extract_integer_value(value)[0]));
        }
        else if (spec.ends_with(type_specifier<unsigned long long>::value()))
        {
            if (!is_integer_operand(value))
            {
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "format_string::format_value",
                    util::generate_error_message(
                        "the format specifier %llu expects to convert from "
                        "a value that is convertible to an 'unsigned long "
                        "long'"));
            }
            hpx::util::format_to(os, fullspec, static_cast<unsigned long long>(
                extract_integer_value(value)[0]));
        }
        else if (spec.ends_with(type_specifier<unsigned long>::value()))
        {
            if (!is_integer_operand(value))
            {
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "format_string::format_value",
                    util::generate_error_message(
                        "the format specifier %lu expects to convert from "
                        "a value that is convertible to an 'unsigned "
                        "long'"));
            }
            hpx::util::format_to(os, fullspec, static_cast<unsigned long>(
                extract_integer_value(value)[0]));
        }
        else if (spec.ends_with(type_specifier<unsigned int>::value()))
        {
            if (!is_integer_operand(value))
            {
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "format_string::format_value",
                    util::generate_error_message(
                        "the format specifier %u expects to convert from "
                        "a value that is convertible to an 'unsigned "
                        "int'"));
            }
            hpx::util::format_to(os, fullspec,
                static_cast<unsigned int>(extract_integer_value(value)[0]));
        }
        else if (spec.ends_with(type_specifier<long double>::value()))
        {
            if (!is_numeric_operand(value))
            {
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "format_string::format_value",
                    util::generate_error_message(
                        "the format specifier %Lf expects to convert from "
                        "a value that is convertible to a 'long double'"));
            }
            hpx::util::format_to(os, fullspec,
                static_cast<long double>(extract_numeric_value(value)[0]));
        }
        else if (spec.ends_with(type_specifier<double>::value()))
        {
            if (!is_numeric_operand(value))
            {
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "format_string::format_value",
                    util::generate_error_message(
                        "the format specifier %lf expects to convert from "
                        "a value that is convertible to a 'double'"));
            }
            hpx::util::format_to(os, fullspec,
                static_cast<double>(extract_numeric_value(value)[0]));
        }
        else if (spec.ends_with(type_specifier<float>::value()))
        {
            if (!is_numeric_operand(value))
            {
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "format_string::format_value",
                    util::generate_error_message(
                        "the format specifier %f expects to convert from "
                        "a value that is convertible to a 'float'"));
            }
            hpx::util::format_to(os, fullspec,
                static_cast<float>(extract_numeric_value(value)[0]));
        }
        else
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "format_string::format_value",
                util::generate_error_message(
                    "invalid format specifier: " + spec.to_string()));
        }
    }
}}

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace execution_tree { namespace primitives
{
    ///////////////////////////////////////////////////////////////////////////
    primitive create_format_string(hpx::id_type const& locality,
        primitive_arguments_type&& operands,
        std::string const& name, std::string const& codename)
    {
        static std::string type("format");
        return create_primitive_component(
            locality, type, std::move(operands), name, codename);
    }

    match_pattern_type const format_string::match_data =
    {
        hpx::util::make_tuple("format",
            std::vector<std::string>{"format(_1, __2)"},
            &create_format_string, &create_primitive<format_string>,
            R"(s,args

            Args:

                s (string) : a format string
                *args (arg list, optional) : a list of arguments

            Returns:

            A formatted string, with each `{}` in s replaced by
            a value from `*args`.)")
    };

    ///////////////////////////////////////////////////////////////////////////
    format_string::format_string(
            primitive_arguments_type&& operands,
            std::string const& name, std::string const& codename)
      : primitive_component_base(std::move(operands), name, codename)
    {}

    hpx::future<primitive_argument_type> format_string::eval(
        primitive_arguments_type const& operands,
        primitive_arguments_type const& args, eval_context ctx) const
    {
        if (operands.empty())
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "format_string::eval",
                generate_error_message(
                    "the format_string primitive expects to be invoked with "
                    "at least one argument"));
        }

        auto this_ = this->shared_from_this();
        return hpx::dataflow(hpx::launch::sync, hpx::util::unwrapping(
            [this_ = std::move(this_)](primitive_arguments_type&& args)
            ->  primitive_argument_type
            {
                if (!is_string_operand(args[0]))
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "format_string::eval",
                        this_->generate_error_message(
                            "the format_string primitive expects a formatting "
                                "string as its first argument"));
                }

                std::string fmt = extract_string_value(
                    args[0], this_->name_, this_->codename_);

                std::vector<hpx::util::detail::format_arg> fargs;
                fargs.reserve(args.size());
                for (auto it = args.begin() + 1; it != args.end(); ++it)
                {
                    fargs.emplace_back(*it);
                }

                return primitive_argument_type{
                    hpx::util::detail::format(fmt, fargs.data(), fargs.size())};
            }),
            detail::map_operands(
                operands, functional::value_operand{}, args, name_, codename_,
                std::move(ctx)));
    }
}}}
