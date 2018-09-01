//  Copyright (c) 2017 Alireza Kheirkhahan
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/ir/node_data.hpp>
#include <phylanx/plugins/fileio/file_read_csv.hpp>

#include <hpx/include/lcos.hpp>
#include <hpx/include/naming.hpp>
#include <hpx/include/util.hpp>
#include <hpx/throw_exception.hpp>

#include <boost/spirit/include/qi_char.hpp>
#include <boost/spirit/include/qi_list.hpp>
#include <boost/spirit/include/qi_lit.hpp>
#include <boost/spirit/include/qi_parse.hpp>
#include <boost/spirit/include/qi_real.hpp>

#include <cstddef>
#include <fstream>
#include <string>
#include <utility>
#include <vector>

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace execution_tree { namespace primitives
{
    ///////////////////////////////////////////////////////////////////////////
    match_pattern_type const file_read_csv::match_data =
    {
        hpx::util::make_tuple("file_read_csv",
            std::vector<std::string>{"file_read_csv(_1)"},
            &create_file_read_csv, &create_primitive<file_read_csv>)
    };

    ///////////////////////////////////////////////////////////////////////////
    file_read_csv::file_read_csv(
            primitive_arguments_type && operands,
            std::string const& name, std::string const& codename)
      : primitive_component_base(std::move(operands), name, codename)
    {}

    // read data from given file and return content
    hpx::future<primitive_argument_type> file_read_csv::eval(
        primitive_arguments_type const& args) const
    {
        if (operands_.size() != 1)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "phylanx::execution_tree::primitives::file_read_csv::"
                    "eval",
                util::generate_error_message(
                    "the file_read_csv primitive requires exactly one "
                        "literal argument",
                    name_, codename_));
        }

        if (!valid(operands_[0]))
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "phylanx::execution_tree::primitives::file_read_csv::"
                    "eval",
                util::generate_error_message(
                    "the file_read_csv primitive requires that the given "
                        "operand is valid",
                    name_, codename_));
        }

        std::string filename =
            string_operand_sync(operands_[0], args, name_, codename_);
        std::ifstream infile(filename.c_str(), std::ios::in);

        if (!infile.is_open())
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "phylanx::execution_tree::primitives::file_read_csv::eval",
                util::generate_error_message(
                    "couldn't open file: " + filename,
                    name_, codename_));
        }

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
                        HPX_THROW_EXCEPTION(hpx::invalid_data,
                            "phylanx::execution_tree::primitives::"
                                "file_read_csv::eval",
                            util::generate_error_message(
                                "wrong data format, different number of "
                                    "element in this row " +
                                    filename + ':' + std::to_string(n_rows),
                                name_, codename_));
                    }
                    n_rows++;
                }
                current_line.clear();
            }
            else
            {
                HPX_THROW_EXCEPTION(hpx::invalid_data,
                    "phylanx::execution_tree::primitives::file_read_csv::eval",
                    util::generate_error_message(
                        "wrong data format " + filename + ':' +
                            std::to_string(n_rows),
                        name_, codename_));
            }
        }

        if (n_rows == 1)
        {
            if (n_cols == 1)
            {
                // scalar value
                return hpx::make_ready_future(primitive_argument_type{
                    ir::node_data<double>{matrix_array[0]}});
            }

            // vector
            blaze::DynamicVector<double> vector(n_cols, matrix_array.data());

            return hpx::make_ready_future(primitive_argument_type{
                ir::node_data<double>{std::move(vector)}});
        }

        // matrix
        blaze::DynamicMatrix<double> matrix(
            n_rows, n_cols, matrix_array.data());

        return hpx::make_ready_future(
            primitive_argument_type{ir::node_data<double>{std::move(matrix)}});
    }
}}}
