//  Copyright (c) 2017-2020 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/compiler/primitive_name.hpp>
#include <phylanx/util/generate_error_message.hpp>

#include <hpx/modules/format.hpp>

#include <string>
#include <utility>
#include <vector>

namespace phylanx { namespace util
{
    std::string generate_error_message(std::string const& msg,
        execution_tree::compiler::primitive_name_parts const& parts,
        std::string const& codename)
    {
        return generate_error_message(msg,
            execution_tree::compiler::compose_primitive_name(parts), codename);
    }

    namespace detail
    {
        std::string format_frame(int frame_no, std::string&& frame)
        {
            return hpx::util::format(
                "  [{}]: {}\n", frame_no, std::move(frame));
        }
    }

    std::string generate_error_message(std::string const& msg,
        std::string const& name, std::string const& codename,
        std::vector<std::string>&& frames)
    {
        std::string trace = "\n<stack frames>\n";

        int frame_no = 0;
        for (auto&& frame : frames)
        {
            trace += detail::format_frame(++frame_no, std::move(frame));
        }

        trace += detail::format_frame(++frame_no,
            generate_error_message("(PhySL)\n", name, codename));

        return trace + msg;
    }

    std::string generate_error_message(std::string const& msg,
        std::string const& name, std::string const& codename)
    {
        if (!name.empty())
        {
            execution_tree::compiler::primitive_name_parts parts;

            if (execution_tree::compiler::parse_primitive_name(name, parts))
            {
                std::string line_col;
                if (parts.tag1 != -1 && parts.tag2 != -1)
                {
                    line_col = hpx::util::format(
                        "(" PHYLANX_FORMAT_SPEC(1) ", "
                            PHYLANX_FORMAT_SPEC(2) ")",
                        parts.tag1, parts.tag2);
                }

                if (!parts.instance.empty())
                {
                    return hpx::util::format(
                        PHYLANX_FORMAT_SPEC(1) PHYLANX_FORMAT_SPEC(2)
                            ": " PHYLANX_FORMAT_SPEC(3)
                            "$"  PHYLANX_FORMAT_SPEC(4)
                            ":: " PHYLANX_FORMAT_SPEC(5),
                        codename.empty() ? "<unknown>" : codename, line_col,
                        parts.primitive, parts.instance, msg);
                }

                return hpx::util::format(
                    PHYLANX_FORMAT_SPEC(1) PHYLANX_FORMAT_SPEC(2)
                        ": " PHYLANX_FORMAT_SPEC(3)
                        ":: " PHYLANX_FORMAT_SPEC(4),
                    codename.empty() ? "<unknown>" : codename, line_col,
                    parts.primitive, msg);
            }

            return hpx::util::format(
                PHYLANX_FORMAT_SPEC(1)
                    ": "  PHYLANX_FORMAT_SPEC(2)
                    ":: " PHYLANX_FORMAT_SPEC(3),
                codename.empty() ? "<unknown>" : codename, name, msg);
        }

        return hpx::util::format(
            PHYLANX_FORMAT_SPEC(1) ": "  PHYLANX_FORMAT_SPEC(2),
            codename.empty() ? "<unknown>" : codename, msg);
    }
}}

