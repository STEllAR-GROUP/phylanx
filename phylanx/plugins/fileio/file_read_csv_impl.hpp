//  Copyright (c) 2017 Alireza Kheirkhahan
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_PRIMITIVES_FILE_READ_CSV_IMPL_HPP)
#define PHYLANX_PRIMITIVES_FILE_READ_CSV_IMPL_HPP

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/base_primitive.hpp>
#include <phylanx/execution_tree/primitives/primitive_component_base.hpp>
#include <phylanx/plugins/fileio/file_read_csv.hpp>

#include <hpx/futures/future.hpp>

#include <hpx/include/lcos.hpp>
#include <hpx/include/naming.hpp>
#include <hpx/include/util.hpp>
#include <hpx/runtime/threads/run_as_os_thread.hpp>

#include <boost/spirit/include/qi_char.hpp>
#include <boost/spirit/include/qi_list.hpp>
#include <boost/spirit/include/qi_lit.hpp>
#include <boost/spirit/include/qi_parse.hpp>
#include <boost/spirit/include/qi_real.hpp>

#include <cstddef>
#include <fstream>
#include <iomanip>
#include <memory>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include <blaze/Math.h>

namespace phylanx { namespace execution_tree { namespace primitives
{
    // read data from given file and return content
    inline primitive_argument_type file_read_csv::read(
        std::ifstream&& infile, std::string const& filename) const
    {
        std::string line;
        bool header_parsed = false;
        std::vector<double> matrix_array, current_line;
        std::size_t n_rows = 0, n_cols = 0;
        std::size_t before_readln = 0, after_readln = 0;

        while (std::getline(infile, line))
        {
            before_readln = matrix_array.size();

            auto begin_local = line.begin();
            if (boost::spirit::qi::parse(begin_local, line.end(),
                    boost::spirit::qi::double_ % ',', current_line))
            {
                if (begin_local == line.end() || header_parsed)
                {
                    header_parsed = true;

                    matrix_array.insert(matrix_array.end(),
                        current_line.begin(), current_line.end());

                    after_readln = matrix_array.size();
                    if (n_rows == 0)
                    {
                        n_cols = matrix_array.size();
                    }
                    else if (n_cols != (after_readln - before_readln))
                    {
                        throw std::runtime_error(generate_error_message(
                            "wrong data format, different number "
                            "of element in this row " +
                            filename + ':' + std::to_string(n_rows)));
                    }
                    n_rows++;
                }
                current_line.clear();
            }
            else
            {
                throw std::runtime_error(
                    generate_error_message("wrong data format " + filename +
                        ':' + std::to_string(n_rows)));
            }
        }

        if (n_rows == 1)
        {
            if (n_cols == 1)
            {
                // scalar value
                return primitive_argument_type{
                    ir::node_data<double>{matrix_array[0]}};
            }

            // vector
            blaze::DynamicVector<double> vector(n_cols, matrix_array.data());

            return primitive_argument_type{
                ir::node_data<double>{std::move(vector)}};
        }

        // matrix
        blaze::DynamicMatrix<double> matrix(
            n_rows, n_cols, matrix_array.data());

        return primitive_argument_type{
            ir::node_data<double>{std::move(matrix)}};
    }
}}}

#endif
