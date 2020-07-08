//  Copyright (c) 2017 Alireza Kheirkhahan
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/ir/node_data.hpp>
#include <phylanx/plugins/fileio/file_read_csv.hpp>
#include <phylanx/plugins/fileio/file_read_csv_impl.hpp>

#include <hpx/include/lcos.hpp>
#include <hpx/include/naming.hpp>
#include <hpx/include/util.hpp>
#include <hpx/errors/throw_exception.hpp>

#include <cstdint>
#include <fstream>
#include <iomanip>
#include <memory>
#include <stdexcept>
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
            std::vector<std::string>{R"(
                file_read_csv(
                    _1_filename,
                    __arg(_2_mode3d, false),
                    __arg(_3_page_nrows, 1)
                )
            )"},
            &create_file_read_csv, &create_primitive<file_read_csv>,
            R"(filename, mode3d, page_nrows
            Args:

                filename (string) : file name
                mode3d (bool, optional) : If sets to true, the result will be a
                    3d array. Each page_nrows rows of the data is stored in a
                    new page of the result.
                page_nrows (int, optional) : it is used only whenmode3d is true.
                    It determines the number of rows in each page of the
                    resulted array.

            Returns:

            Returns an array representation of the contents of a
            csv file.)"
            )
    };

    ///////////////////////////////////////////////////////////////////////////
    file_read_csv::file_read_csv(
            primitive_arguments_type && operands,
            std::string const& name, std::string const& codename)
      : primitive_component_base(std::move(operands), name, codename)
    {}

    ///////////////////////////////////////////////////////////////////////////
    inline primitive_argument_type file_read_csv::read(
        std::ifstream&& infile, std::string const& filename) const
    {
        std::vector<double> data;
        std::size_t n_rows, n_cols;
        std::tie(data, n_rows, n_cols) =
            read_helper(std::move(infile), filename);

        if (n_rows == 1)
        {
            if (n_cols == 1)
            {
                // scalar value
                return primitive_argument_type{
                    ir::node_data<double>{data[0]}};
            }

            // vector
            blaze::DynamicVector<double> vector(n_cols, data.data());

            return primitive_argument_type{
                ir::node_data<double>{std::move(vector)}};
        }

        // matrix
        blaze::DynamicMatrix<double> matrix(n_rows, n_cols, data.data());

        return primitive_argument_type{
            ir::node_data<double>{std::move(matrix)}};
    }

    inline primitive_argument_type file_read_csv::read_3d(
        std::ifstream&& infile, std::string const& filename,
        std::int64_t given_nrows) const
    {
        std::vector<double> data;
        std::size_t n_rows, n_cols;
        std::tie(data, n_rows, n_cols) =
            read_helper(std::move(infile), filename);

        if (n_rows % given_nrows != 0)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter, "file_read_csv::read_3d",
                util::generate_error_message(
                    "the number of rows in the csv file is not divisible by "
                    "the given number of rows in a page"));
        }

        // tensor
        blaze::DynamicTensor<double> result(
            static_cast<std::size_t>(n_rows / given_nrows), given_nrows, n_cols,
            data.data());

        return primitive_argument_type{
            ir::node_data<double>{std::move(result)}};
    }

    ///////////////////////////////////////////////////////////////////////////
    hpx::future<primitive_argument_type> file_read_csv::eval(
        primitive_arguments_type const& operands,
        primitive_arguments_type const& args, eval_context ctx) const
    {
        if (operands.empty() || operands.size() > 3)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "phylanx::execution_tree::primitives::file_read_csv::eval",
                generate_error_message("the file_read_csv primitive requires "
                                       "at least one and at most 3 operands."));
        }

        if (!valid(operands[0]))
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "phylanx::execution_tree::primitives::file_read_csv::eval",
                generate_error_message(
                    "the file_read_csv primitive requires that the given "
                        "operand is valid"));
        }

        auto this_ = this->shared_from_this();
        return hpx::dataflow(hpx::launch::sync, hpx::util::unwrapping(
            [this_ = std::move(this_)](
                primitive_arguments_type&& args)
                -> primitive_argument_type
            {

                std::string filename = extract_string_value_strict(
                    std::move(args[0]), this_->name_, this_->codename_);

                bool mode3d = false;
                if (args.size() > 1 && valid(args[1]))
                {
                    mode3d = extract_scalar_boolean_value(
                        std::move(args[1]), this_->name_, this_->codename_);
                }

                std::int64_t page_nrows;
                if (args.size() > 2 && valid(args[2]))
                {
                    page_nrows = extract_scalar_positive_integer_value_strict(
                        std::move(args[2]), this_->name_, this_->codename_);
                }

                std::ifstream infile(filename.c_str(), std::ios::in);

                if (!infile.is_open())
                {
                    throw std::runtime_error(this_->generate_error_message(
                        "couldn't open file: " + filename));
                }

                if (mode3d)
                {
                    return this_->read_3d(
                        std::move(infile), filename, page_nrows);
                }
                return this_->read(std::move(infile), filename);

                }),
            detail::map_operands(operands, functional::value_operand{}, args,
                name_, codename_, std::move(ctx)));
    }
}}}
