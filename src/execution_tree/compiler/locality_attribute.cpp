//  Copyright (c) 2018 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/compiler/locality_attribute.hpp>

#include <hpx/throw_exception.hpp>

#include <cstdint>
#include <string>

// Uncomment this if you want to enable debugging
// #define BOOST_SPIRIT_QI_DEBUG

#include <boost/spirit/include/qi_nonterminal.hpp>
#include <boost/spirit/include/qi_numeric.hpp>
#include <boost/spirit/include/qi_operator.hpp>
#include <boost/spirit/include/qi_parse.hpp>
#include <boost/spirit/include/qi_string.hpp>

namespace phylanx { namespace execution_tree { namespace compiler
{
    namespace detail
    {
        namespace qi = boost::spirit::qi;

        template <typename Iterator>
        struct locality_attribute_parser
          : qi::grammar<Iterator, std::uint32_t()>
        {
            locality_attribute_parser()
              : locality_attribute_parser::base_type(start)
            {
                start = qi::lit("L#") >> qi::uint_;

                // Debugging support.
                BOOST_SPIRIT_DEBUG_NODES(
                    (start)
                );
            }

            qi::rule<Iterator, std::uint32_t()> start;
        };
    }

    ///////////////////////////////////////////////////////////////////////////
    // Attempt to parse the given attribute as a locality specification
    hpx::id_type parse_locality_attribute(std::string const& name)
    {
        std::uint32_t locality_id = std::uint32_t(-1);

        auto begin = name.begin();
        bool result = boost::spirit::qi::parse(begin, name.end(),
            detail::locality_attribute_parser<std::string::const_iterator>(),
            locality_id);

        if (!result || begin != name.end())
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "phylanx::execution_tree::compiler::parse_locality_attribute",
                "could not (fully) parse the given attribute: " + name);
        }

        return hpx::naming::get_id_from_locality_id(locality_id);
    }

    bool parse_locality_attribute(std::string const& name, hpx::id_type& locality)
    {
        std::uint32_t locality_id = std::uint32_t(-1);

        auto begin = name.begin();
        bool result = boost::spirit::qi::parse(begin, name.end(),
            detail::locality_attribute_parser<std::string::const_iterator>(),
            locality_id);

        if (result && begin == name.end())
        {
            locality = hpx::naming::get_id_from_locality_id(locality_id);
            return true;
        }

        return false;
    }

    ///////////////////////////////////////////////////////////////////////////
    // Compose an attribute referencing a locality
    std::string compose_locality_attribute(hpx::id_type const& locality)
    {
        std::string result("L#");
        result +=
            std::to_string(hpx::naming::get_locality_id_from_id(locality));
        return result;
    }
}}}



