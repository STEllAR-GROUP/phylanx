//  Copyright (c) 2017 Alireza Kheirkhahan
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/annotation.hpp>
#include <phylanx/ir/node_data.hpp>
#include <phylanx/plugins/fileio/file_read_csv_d.hpp>

#include <hpx/errors/throw_exception.hpp>
#include <hpx/include/lcos.hpp>
#include <hpx/include/naming.hpp>
#include <hpx/include/util.hpp>
#include <hpx/lcos/barrier.hpp>
#include <hpx/runtime/threads/run_as_os_thread.hpp>

#include <boost/spirit/include/qi_char.hpp>
#include <boost/spirit/include/qi_list.hpp>
#include <boost/spirit/include/qi_lit.hpp>
#include <boost/spirit/include/qi_parse.hpp>
#include <boost/spirit/include/qi_real.hpp>

#include <cstddef>
#include <fstream>
#include <memory>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace execution_tree { namespace primitives {
    ///////////////////////////////////////////////////////////////////////////
    match_pattern_type const file_read_csv_d::match_data = {
        hpx::util::make_tuple("file_read_csv_d",
            std::vector<std::string>{"file_read_csv_d(_1, _2)"},
            &create_file_read_csv_d, &create_primitive<file_read_csv_d>,
            R"(fname
            Args:

                fname (string) : file name
                locality (annotation) : annotation for locality

            Returns:

            Returns a matrix representation of the contents of a
            csv file distributed across a number of localities.)")};

    ///////////////////////////////////////////////////////////////////////////
    file_read_csv_d::file_read_csv_d(primitive_arguments_type&& operands,
        std::string const& name, std::string const& codename)
      : primitive_component_base(std::move(operands), name, codename)
    {
    }

    primitive_argument_type file_read_csv_d::read(std::string filename,
        phylanx::execution_tree::annotation&& locality_info)
    {
        ir::range loc_of = locality_info.get_data();
        if (extract_numeric_value_dimension(
                *loc_of.begin(), name_, codename_) != 0 ||
            extract_numeric_value_dimension(
                *(loc_of.begin() + 1), name_, codename_) != 0)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter, "file_read_csv_d::read",
                generate_error_message(
                    "locality annotation has elements of wrong dimension"));
        }
        std::size_t me = phylanx::execution_tree::extract_scalar_integer_value(
            *loc_of.begin());
        std::size_t num_locs =
            phylanx::execution_tree::extract_scalar_integer_value(
                *(loc_of.begin() + 1));
        std::size_t num_rows = 0;    // Need a way to get this. With num_locs,
        // we can do a barrier, and let locality 0 count the rows and
        // broadcast the result to all participating elements. I think
        // we need a basename for that though
        hpx::lcos::barrier barrier("file_read_csv_d_" + filename, num_locs, me);
        barrier.wait();

        std::ifstream infile(filename.c_str(), std::ios::in);

        if (!infile.is_open())
        {
            throw std::runtime_error(
                generate_error_message("couldn't open file: " + filename));
        }

        std::string line;
        bool header_parsed = false;
        std::vector<double> matrix_array, current_line;
        std::size_t n_rows = 0, n_cols = 0;
        std::size_t before_readln = 0, after_readln = 0;
        while (std::getline(infile, line))
        {
        }
        while (std::getline(infile, line))
        {
            before_readln = matrix_array.size();

            auto begin_local = line.begin();
            if (n_rows == 0 &&
                boost::spirit::qi::parse(begin_local, line.end(),
                    boost::spirit::qi::double_ % ',', current_line))
            {
                if (begin_local == line.end() || header_parsed)
                {
                    header_parsed = true;

                    /*matrix_array.insert(matrix_array.end(),
                        current_line.begin(), current_line.end());*/

                    if (n_rows == 0)
                    {
                        n_cols = current_line.size();
                    }
                    n_rows++;
                }
                current_line.clear();
            }
            else if (n_rows != 0)
            {
                n_rows++;
                continue;
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

    // read data from given file and return content
    hpx::future<primitive_argument_type> file_read_csv_d::eval(
        primitive_arguments_type const& operands,
        primitive_arguments_type const& args, eval_context ctx) const
    {
        if (operands.size() != 2)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "phylanx::execution_tree::primitives::file_read_csv_d::eval",
                generate_error_message(
                    "the file_read_csv_d primitive requires exactly two "
                    "arguments"));
        }

        if (!valid(operands[0]) || !valid(operands[1]))
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "phylanx::execution_tree::primitives::file_read_csv_d::eval",
                generate_error_message(
                    "the file_read_csv_d primitive requires that the given "
                    "operands are valid"));
        }

        std::string filename = string_operand_sync(
            operands[0], args, name_, codename_, std::move(ctx));

        auto this_ = this->shared_from_this();
        return hpx::threads::run_as_os_thread(
            [filename = std::move(filename),
                this_ = std::move(this_)]() -> primitive_argument_type {
                std::ifstream infile(filename.c_str(), std::ios::in);

                if (!infile.is_open())
                {
                    throw std::runtime_error(this_->generate_error_message(
                        "couldn't open file: " + filename));
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
                                throw std::runtime_error(
                                    this_->generate_error_message(
                                        "wrong data format, different number "
                                        "of element in this row " +
                                        filename + ':' +
                                        std::to_string(n_rows)));
                            }
                            n_rows++;
                        }
                        current_line.clear();
                    }
                    else
                    {
                        throw std::runtime_error(
                            this_->generate_error_message("wrong data format " +
                                filename + ':' + std::to_string(n_rows)));
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
                    blaze::DynamicVector<double> vector(
                        n_cols, matrix_array.data());

                    return primitive_argument_type{
                        ir::node_data<double>{std::move(vector)}};
                }

                // matrix
                blaze::DynamicMatrix<double> matrix(
                    n_rows, n_cols, matrix_array.data());

                return primitive_argument_type{
                    ir::node_data<double>{std::move(matrix)}};
            });
    }
}}}    // namespace phylanx::execution_tree::primitives
