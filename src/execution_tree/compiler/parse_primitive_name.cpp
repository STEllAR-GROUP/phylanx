//  Copyright (c) 2018 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/compiler/parse_primitive_name.hpp>

#include <hpx/throw_exception.hpp>
#include <hpx/runtime/naming_fwd.hpp>

#include <cstdint>
#include <string>
#include <utility>

// Uncomment this if you want to enable debugging
// #define BOOST_SPIRIT_QI_DEBUG

#include <boost/spirit/include/qi_attr.hpp>
#include <boost/spirit/include/qi_char.hpp>
#include <boost/spirit/include/qi_nonterminal.hpp>
#include <boost/spirit/include/qi_numeric.hpp>
#include <boost/spirit/include/qi_operator.hpp>
#include <boost/spirit/include/qi_optional.hpp>
#include <boost/spirit/include/qi_parse.hpp>
#include <boost/spirit/include/qi_string.hpp>
#include <boost/fusion/include/adapt_struct.hpp>

BOOST_FUSION_ADAPT_STRUCT(
    phylanx::execution_tree::compiler::primitive_name_parts,
    (std::uint32_t, locality)
    (std::string, primitive)
    (std::int64_t, sequence_number)
    (std::string, instance)
    (std::int64_t, compile_id)
    (std::int64_t, tag1)
    (std::int64_t, tag2)
)

namespace phylanx { namespace execution_tree { namespace compiler
{
    namespace detail
    {
        namespace qi = boost::spirit::qi;

        template <typename Iterator>
        struct primitive_name_parser
          : qi::grammar<Iterator, primitive_name_parts()>
        {
            primitive_name_parser()
              : primitive_name_parser::base_type(start)
            {
                start =
                         qi::lit("/phylanx")
                    >>  (  (qi::lit('$') >> qi::uint_)
                        |   qi::attr(hpx::naming::invalid_locality_id)
                        )
                    >>   qi::lit("/") >> primitive
                    >>   qi::lit('$') >> qi::uint_
                    >> ((qi::lit('$') >> instance) | qi::attr(""))
                    >>   qi::lit('/') >> qi::uint_
                    >>   qi::lit('$') >> qi::uint_
                    >> ((qi::lit('$') >> qi::uint_) | qi::attr(-1))
                    ;

                primitive = +(qi::char_ - qi::lit('$'));
                instance = +(qi::char_ - qi::lit('/'));

                // Debugging support.
                BOOST_SPIRIT_DEBUG_NODES(
                    (start)
                    (primitive)
                    (instance)
                );
            }

            qi::rule<Iterator, primitive_name_parts()> start;
            qi::rule<Iterator, std::string()> primitive;
            qi::rule<Iterator, std::string()> instance;
        };
    }

    ///////////////////////////////////////////////////////////////////////////
    // Split the given primitive name into its parts
    primitive_name_parts parse_primitive_name(std::string const& name)
    {
        primitive_name_parts data;

        auto begin = name.begin();
        bool result = boost::spirit::qi::parse(begin, name.end(),
            detail::primitive_name_parser<std::string::const_iterator>(), data);

        if (!result || begin != name.end())
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "phylanx::execution_tree::compiler::parse_primitive_name",
                "could not (fully) parse the given primitive name: " + name);
        }

        return data;
    }

    bool parse_primitive_name(std::string const& name, primitive_name_parts& parts)
    {
        auto begin = name.begin();
        bool result = boost::spirit::qi::parse(begin, name.end(),
            detail::primitive_name_parser<std::string::const_iterator>(), parts);

        return result && begin == name.end();
    }

    ///////////////////////////////////////////////////////////////////////////
    // compose a new primitive name from the given parts
    std::string compose_primitive_name(primitive_name_parts const& parts)
    {
        if (parts.primitive.empty())
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "phylanx::execution_tree::compiler::compose_primitive_name",
                "primitive type was not specified");
        }

        std::string result("/phylanx");
        if (parts.locality != hpx::naming::invalid_locality_id)
        {
            result += "$" + std::to_string(parts.locality);
        }
        result += "/" + parts.primitive;
        result += "$" + std::to_string(
            parts.sequence_number == -1 ? 0 : parts.sequence_number);

        if (!parts.instance.empty())
        {
            result += "$" + parts.instance;
        }

        result += "/" + std::to_string(
            parts.compile_id == -1 ? 0 : parts.compile_id);
        result += "$" + std::to_string(parts.tag1 < 0 ? 0 : parts.tag1);

        // column is optional
        if (parts.tag2 != -1)
        {
            result += '$' + std::to_string(parts.tag2);
        }

        return result;
    }

    // Compose a primitive display name from the given parts
    std::string compose_primitive_display_name(
        primitive_name_parts const& parts)
    {
        if (parts.primitive.empty())
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "phylanx::execution_tree::compiler::compose_primitive_display_name",
                "primitive type was not specified");
        }

        std::string result = parts.primitive;
        if (result.size() > 2 && result[0] == '_' && result[1] == '_')
        {
            // strip leading "__"
            result.erase(0, 2);
        }

        if (!parts.instance.empty())
        {
            result += "/" + parts.instance;
        }

        if (parts.locality != hpx::naming::invalid_locality_id)
        {
            result += "/L#" + std::to_string(parts.locality);
        }

        if (parts.tag1 >= 0 || parts.tag2 != -1)
        {
            result += "(" + std::to_string(parts.tag1 < 0 ? 0 : parts.tag1);

            // column is optional
            if (parts.tag2 != -1)
            {
                result += ", " + std::to_string(parts.tag2);
            }

            result += ")";
        }

        return result;
    }

    // Return display name for a given primitive name
    std::string primitive_display_name(std::string const& name)
    {
        primitive_name_parts parts = parse_primitive_name(name);
        return compose_primitive_display_name(parts);
    }

    ///////////////////////////////////////////////////////////////////////////
    // Extract the primitive name from the given component name
    std::string extract_primitive_name(std::string const& name)
    {
        compiler::primitive_name_parts name_parts;
        if (!parse_primitive_name(name, name_parts))
        {
            std::string::size_type p = name.find_first_of("$");
            if (p != std::string::npos)
            {
                return name.substr(0, p);
            }
            return name;
        }
        return std::move(name_parts.primitive);
    }

    // Extract the function/variable name from the given component name
    std::string extract_instance_name(std::string const& name)
    {
        compiler::primitive_name_parts name_parts;
        if (!parse_primitive_name(name, name_parts))
        {
            std::string::size_type p = name.find_first_of("$");
            if (p != std::string::npos)
            {
                std::string::size_type p1 = name.find_first_of("$", p);
                if (p1 != std::string::npos)
                {
                    return name.substr(p1);
                }
            }
            return name;
        }
        return std::move(name_parts.instance);
    }
}}}



