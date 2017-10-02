//  Copyright (c) 2017 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/ir/node_data.hpp>

#include <cstddef>
#include <iosfwd>

namespace phylanx { namespace ir
{
    namespace detail
    {
        template <typename Range>
        void print_array(std::ostream& out, Range const& r, std::size_t size)
        {
            out << "[";
            for (std::size_t i = 0; i != size; ++i)
            {
                if (i != 0)
                {
                    out << ", ";
                }
                out << std::to_string(r[i]);
            }
            out << "]";
        }
    }

    ///////////////////////////////////////////////////////////////////////////
    std::ostream& operator<<(std::ostream& out, node_data<double> const& nd)
    {
        switch (nd.num_dimensions())
        {
        case 0:
            out << std::to_string(nd[0]);
            break;

        case 1:
            detail::print_array(out, nd, nd.size());
            break;

        case 2:
            {
                auto const& data = nd.matrix();
                for (std::size_t row = 0; row != data.rows(); ++row)
                {
                    if (row != 0)
                        out << ", ";
                    detail::print_array(out, data.row(row), data.cols());
                }
            }
            break;

        default:
            break;
        }
        return out;
    }
}}
