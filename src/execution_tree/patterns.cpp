//  Copyright (c) 2017-2018 Hartmut Kaiser
//
//  Distributed under the Boost Software License}, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives.hpp>
#include <phylanx/execution_tree/compile.hpp>

#include <utility>
#include <string>
#include <vector>
#include <set>
#include <iostream>
#include <map>

namespace phylanx { namespace execution_tree
{
    ///////////////////////////////////////////////////////////////////////////
    std::vector<std::pair<std::string, match_pattern_type>> registered_patterns;

    void show_patterns()
    {
        std::cout << "patterns: " << std::endl;
        for(auto i : list_patterns())
        {
            std::cout << "pattern: " << i.first << std::endl;
            for(auto s : i.second)
            {
                std::cout << "  matches: " << s << std::endl;
            }
        }
    }

    std::map<std::string,std::vector<std::string>> list_patterns()
    {
        std::map<std::string,std::vector<std::string>> result;
        std::map<std::string,std::set<std::string>> found;
        for(auto p : get_all_known_patterns())
        {
            std::string pat = hpx::util::get<0>(p);
            for(auto pat2 : hpx::util::get<1>(hpx::util::get<1>(p)))
            {
                found[pat].insert(pat2);
            }
        }
        for(auto p = found.begin();p != found.end();++p)
        {
            for(auto p2 : p->second)
            {
                result[p->first].push_back(p2);
            }
        }
        return result;
    }

    std::string find_help(const std::string& s)
    {
        for(auto p : get_all_known_patterns())
        {
            std::string pat = hpx::util::get<0>(p);
            std::string help = hpx::util::get<4>(hpx::util::get<1>(p));
            if(pat == s)
                return help;
            for(auto pat2 : hpx::util::get<1>(hpx::util::get<1>(p)))
            {
                std::string p = pat2;
                for(int i=0;i<p.size();i++) {
                    if(p[i] == '(') {
                        p.resize(i);
                        break;
                    }
                }
                if(p == s)
                    return help;
            }
        }
        return "No help available.";
    }

    void register_pattern(
        std::string const& name, match_pattern_type const& pattern)
    {
        registered_patterns.push_back(std::make_pair(name, pattern));
    }

    ///////////////////////////////////////////////////////////////////////////
    namespace detail
    {
        ///////////////////////////////////////////////////////////////////////
#define PHYLANX_MATCH_DATA(type)                                               \
    hpx::util::make_tuple(hpx::util::get<0>(primitives::type::match_data),     \
        primitives::type::match_data)                                          \
/**/
#define PHYLANX_MATCH_DATA_VERBATIM(type)                                      \
    hpx::util::make_tuple(                                                     \
        hpx::util::get<0>(primitives::type), primitives::type)                 \
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
                PHYLANX_MATCH_DATA(variable)
            };

            // patterns registered from external primitive plugins
            for (auto const& pattern : registered_patterns)
            {
                patterns.push_back(
                    hpx::util::make_tuple(pattern.first, pattern.second));
            }

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
