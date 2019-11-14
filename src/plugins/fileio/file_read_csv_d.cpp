//  Copyright (c) 2017 Alireza Kheirkhahan
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/annotation.hpp>
#include <phylanx/ir/node_data.hpp>
#include <phylanx/ir/ranges.hpp>
#include <phylanx/plugins/fileio/file_read_csv_d.hpp>

#include <hpx/collectives.hpp>
#include <hpx/errors/throw_exception.hpp>
#include <hpx/include/components.hpp>
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
                locality (list) : annotation for locality

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
        annotation tmp;
        if (!locality_info.has_key("locality"))
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter, "file_read_csv_d::read",
                generate_error_message("malformed annotation"));
        }
        ir::range data = locality_info.get_data();
        ir::range_iterator first = data.begin();
        ir::range_iterator second = data.begin();
        ++second;
        std::size_t me =
            phylanx::execution_tree::extract_scalar_integer_value_strict(
                *first);
        if (me != hpx::get_locality_id())
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "phylanx::execution_tree::primitives::file_read_csv_d::read",
                generate_error_message("supplied locality id is different from "
                                       "execution locality id"));
        }
        std::size_t num_locs =
            phylanx::execution_tree::extract_scalar_integer_value_strict(
                *second);
        std::size_t num_rows = 0;    // Need a way to get this. With num_locs,
        // we can do a barrier, and let locality 0 count the rows and
        // broadcast the result to all participating elements. I think
        // we need a basename for that though
        /*hpx::lcos::barrier barrier("file_read_csv_d_" + filename, num_locs, me);
        barrier.wait();*/

        // Want to find what localities are participating, and which element
        // this locality is in the list, to determine which portion of the
        // result matrix it holds
        std::vector<std::size_t> locs =
            hpx::all_to_all(("all_to_all_" + filename).c_str(), me, num_locs,
                std::size_t(-1), me)
                .get();
        std::sort(locs.begin(), locs.end());
        std::size_t me_idx = std::distance(
            locs.begin(), std::find(locs.begin(), locs.end(), me));

        // This check might be excessive
        if (std::find(locs.begin(), locs.end(), me) == locs.end())
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "phylanx::execution_tree::primitives::file_read_csv_d::read",
                generate_error_message("all_to_all returned locality list "
                                       "without current locality"));
        }

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
            auto begin_local = line.begin();
            if (n_rows != 0)
            {
                n_rows++;
                continue;
            }
            else if (boost::spirit::qi::parse(begin_local, line.end(),
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
                // Need to decide whether this would be an error or
                // if only locality 0 keeps the scalar
                return primitive_argument_type{
                    ir::node_data<double>{matrix_array[0]}};
            }

            // vector
            std::size_t my_part = 
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
        phylanx::execution_tree::annotation loc_info(
            phylanx::execution_tree::extract_list_value_strict(operands[1]));

        auto this_ = this->shared_from_this();
        return hpx::threads::run_as_os_thread(
            [filename = std::move(filename), loc_info = std::move(loc_info),
                this_ = std::move(this_)]() -> primitive_argument_type {
                return this_->read(filename, loc_info);
            });
    }
}}}    // namespace phylanx::execution_tree::primitives
