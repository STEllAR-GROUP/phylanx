//  Copyright (c) 2017-2019 Hartmut Kaiser
//
//  Distributed under the Boost Software License}, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives.hpp>
#include <phylanx/execution_tree/compile.hpp>

#include <cstddef>
#include <iostream>
#include <map>
#include <set>
#include <string>
#include <utility>
#include <vector>

namespace phylanx { namespace execution_tree
{
    ///////////////////////////////////////////////////////////////////////////
    std::vector<hpx::util::tuple<std::string, match_pattern_type, std::string>>
        registered_patterns;

    void show_patterns()
    {
        show_patterns(std::cout);
    }

    void show_patterns(std::ostream& ostrm)
    {
        ostrm << "patterns: " << std::endl;
        for(auto const& i : list_patterns())
        {
            ostrm << "pattern: " << i.first << std::endl;
            for (auto const& s : i.second)
            {
                ostrm << "  matches: " << s << std::endl;
            }
        }
    }

    std::map<std::string,std::vector<std::string>> list_patterns()
    {
        std::map<std::string, std::vector<std::string>> result;
        std::map<std::string, std::set<std::string>> found;

        for (auto const& p : get_all_known_patterns())
        {
            std::string const& pat = hpx::util::get<0>(p);
            for(auto const& pat2 : hpx::util::get<1>(p).patterns_)
            {
                found[pat].insert(pat2);
            }
        }

        for (auto const& it : found)
        {
            for (auto const& p2 : it.second)
            {
                result[it.first].push_back(p2);
            }
        }

        return result;
    }

    std::string find_help(std::string const& s)
    {
        for(auto const& p : get_all_known_patterns())
        {
            std::string const& pat = hpx::util::get<0>(p);
            std::string const& help = hpx::util::get<1>(p).help_string_;
            if (pat == s)
            {
                return help;
            }

            for (auto const& pat2 : hpx::util::get<1>(p).patterns_)
            {
                std::string p = pat2;
                for (std::size_t i = 0; i != p.size(); ++i)
                {
                    if (p[i] == '(')
                    {
                        p.resize(i);
                        break;
                    }
                }

                if (p == s)
                {
                    return help;
                }
            }
        }
        return "No help available.";
    }

    void register_pattern(std::string const& name,
        match_pattern_type const& pattern, std::string const& fullpath)
    {
        registered_patterns.emplace_back(name, pattern, fullpath);
    }

    ///////////////////////////////////////////////////////////////////////////
    namespace detail
    {
        ///////////////////////////////////////////////////////////////////////
#define PHYLANX_MATCH_DATA(type)                                               \
    hpx::util::make_tuple(primitives::type::match_data.primitive_type_,        \
        primitives::type::match_data)                                          \
/**/
#define PHYLANX_MATCH_DATA_VERBATIM(type)                                      \
    hpx::util::make_tuple(                                                     \
        primitives::type.primitive_type_, primitives::type)                    \
/**/

        pattern_list get_all_known_patterns()
        {
            pattern_list patterns =
            {
                // debugging support
                PHYLANX_MATCH_DATA(console_output),
                PHYLANX_MATCH_DATA(debug_output),
                PHYLANX_MATCH_DATA(enable_tracing),
                PHYLANX_MATCH_DATA(format_string),
                PHYLANX_MATCH_DATA(string_output),
                PHYLANX_MATCH_DATA(assert_condition),

                // special purpose primitives
                PHYLANX_MATCH_DATA(store_operation),
                PHYLANX_MATCH_DATA(phytype),
                PHYLANX_MATCH_DATA(phyname),

                // compiler-specific (internal) primitives
                PHYLANX_MATCH_DATA(access_argument),
                PHYLANX_MATCH_DATA(call_function),
                PHYLANX_MATCH_DATA(target_reference),

                PHYLANX_MATCH_DATA(access_function),
                PHYLANX_MATCH_DATA(access_variable),
                PHYLANX_MATCH_DATA(define_variable),
                PHYLANX_MATCH_DATA_VERBATIM(define_variable::match_data_define),
                PHYLANX_MATCH_DATA(function),
                PHYLANX_MATCH_DATA(lambda),
                PHYLANX_MATCH_DATA(variable),
                PHYLANX_MATCH_DATA(variable_factory)
            };

            // patterns registered from external primitive plugins
            for (auto const& pattern : registered_patterns)
            {
                patterns.push_back(hpx::util::make_tuple(
                    hpx::util::get<0>(pattern), hpx::util::get<1>(pattern)));
            }

            patterns.push_back(hpx::util::make_tuple(
                "locality", primitives::locality_match_data));

            return patterns;
        }

#undef PHYLANX_MATCH_DATA_VERBATIM
#undef PHYLANX_MATCH_DATA
    }

    pattern_list const& get_all_known_patterns()
    {
        static pattern_list patterns = detail::get_all_known_patterns();
        return patterns;
    }
}}
