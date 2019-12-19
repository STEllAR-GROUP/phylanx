//  Copyright (c) 2017 Alireza Kheirkhahan
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/annotation.hpp>
#include <phylanx/execution_tree/locality_annotation.hpp>
#include <phylanx/execution_tree/meta_annotation.hpp>
#include <phylanx/execution_tree/primitives/base_primitive.hpp>
#include <phylanx/ir/node_data.hpp>
#include <phylanx/ir/ranges.hpp>
#include <phylanx/plugins/fileio/file_read_csv_d.hpp>
#include <phylanx/util/generate_error_message.hpp>

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

using pair_size_t_size_t = std::pair<std::size_t, std::size_t>;
HPX_REGISTER_ALLTOALL(
    pair_size_t_size_t, file_read_csv_d_all_to_all_coordination);

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
    // Could also allow for default behavior which includes all available
    // localities. Might need to specify tiling though

    ///////////////////////////////////////////////////////////////////////////
    file_read_csv_d::file_read_csv_d(primitive_arguments_type&& operands,
        std::string const& name, std::string const& codename)
      : primitive_component_base(std::move(operands), name, codename)
    {
    }

    primitive_argument_type file_read_csv_d::read_scalar(std::string filename,
        std::pair<std::size_t, std::size_t> locality_info,
        std::pair<std::size_t, std::size_t> dims) const
    {
        std::size_t my_idx = locality_info.first;
        std::size_t num_locs = locality_info.second;

        double val;
        if (my_idx == 0)
        {
            std::ifstream infile(filename.c_str(), std::ios::in);

            if (!infile.is_open())
            {
                throw std::runtime_error(
                    generate_error_message("couldn't open file: " + filename));
            }

            std::string line;
            bool header_parsed = false;
            std::vector<double> matrix_array, current_line;

            while (std::getline(infile, line))
            {
                auto begin_local = line.begin();
                if (boost::spirit::qi::parse(begin_local, line.end(),
                        boost::spirit::qi::double_ % ',', current_line))
                {
                    if (begin_local == line.end() || header_parsed)
                    {
                        header_parsed = true;

                        if (current_line.size() == 1)
                        {
                            val = current_line[0];
                        }
                        else
                        {
                            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                                "file_read_csv_d::read_scalar",
                                generate_error_message("file size read error"));
                        }
                    }
                    current_line.clear();
                }
                else
                {
                    throw std::runtime_error(
                        generate_error_message("wrong data format " + filename +
                            ':' + std::to_string(0)));
                }
            }
        }

        annotation tile;

        if (my_idx == 0)
        {
            annotation tmp(ir::range{"tile",
                primitive_argument_type(ir::range{"columns", 0ll, 1ll}),
                primitive_argument_type(ir::range{"rows", 0ll, 1ll})});
            tile = std::move(tmp);
        }
        else
        {
            annotation tmp(ir::range{"tile",
                primitive_argument_type(ir::range{"columns", 0ll, 0ll}),
                primitive_argument_type(ir::range{"rows", 0ll, 0ll})});
            tile = std::move(tmp);
        }
        std::string name = filename + "file_read_csv_d";
        annotation_information ann_info{std::move(name), 0ll};
        annotation locality(
            ir::range{"locality", (long long) my_idx, (long long) num_locs});

        annotation localities = localities_annotation(
            locality, std::move(tile), ann_info, name_, codename_);

        // How do I crawl this? Stay simple, make it work. Then optimize

        if (my_idx == 0)
        {
            primitive_argument_type ret(val);
            ret.set_annotation(std::move(localities), name_, codename_);
            return ret;
        }
        else
        {
            // Maybe what this should do is broadcast the value to all
            // other localities
            primitive_argument_type ret;
            ret.set_annotation(std::move(localities), name_, codename_);
            return ret;
        }
    }

    primitive_argument_type file_read_csv_d::read_vector(std::string filename,
        std::pair<std::size_t, std::size_t> locality_info,
        std::pair<std::size_t, std::size_t> dims) const
    {
        std::size_t my_idx = locality_info.first;
        std::size_t num_locs = locality_info.second;

        std::size_t my_share = dims.second / num_locs;
        std::size_t others_share = dims.second / num_locs;
        if (dims.second % num_locs != 0)
        {
            others_share++;
            if (my_idx == (num_locs - 1))
            {
                my_share = (dims.second - (num_locs - 1) * (my_share + 1));
            }
            else
            {
                my_share++;
            }
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

        while (std::getline(infile, line))
        {
            auto begin_local = line.begin();
            if (boost::spirit::qi::parse(begin_local, line.end(),
                    boost::spirit::qi::double_ % ',', current_line))
            {
                if (begin_local == line.end() || header_parsed)
                {
                    header_parsed = true;

                    matrix_array.insert(matrix_array.end(),
                        current_line.begin() + my_idx * others_share,
                        current_line.begin() + my_idx * others_share +
                            my_share);
                }
                current_line.clear();
            }
            else
            {
                throw std::runtime_error(generate_error_message(
                    "wrong data format " + filename + ':' + std::to_string(0)));
            }
        }

        annotation tile(ir::range{"tile",
            primitive_argument_type(ir::range{"rows", 0ll, 1ll}),
            primitive_argument_type(
                ir::range{"columns", (long long) (my_idx * others_share),
                    (long long) (my_idx * others_share + my_share)})});

        std::string name = filename;

        annotation_information ann_info{
            std::move((filename + "file_read_csv_d")), 0ll};
        annotation locality(
            ir::range{"locality", (long long) my_idx, (long long) num_locs});

        annotation localities = localities_annotation(
            locality, std::move(tile), ann_info, name_, codename_);

        // How do I crawl this? Stay simple, make it work. Then optimize
        blaze::DynamicVector<double> vector(my_share, matrix_array.data());

        primitive_argument_type ret(vector);
        ret.set_annotation(std::move(localities), name_, codename_);
        return ret;
    }

    primitive_argument_type file_read_csv_d::read_matrix(std::string filename,
        std::pair<std::size_t, std::size_t> locality_info,
        std::pair<std::size_t, std::size_t> dims) const
    {
        std::size_t my_idx = locality_info.first;
        std::size_t num_locs = locality_info.second;

        // All of this really depends on the tiling
        // Maybe we can add that later though
        std::size_t my_rows = dims.first / num_locs;
        std::size_t others_rows = dims.first / num_locs;
        if (dims.first % num_locs != 0)
        {
            others_rows++;
            if (my_idx == (num_locs - 1))
            {
                my_rows = (dims.first - (num_locs - 1) * (my_rows + 1));
            }
            else
            {
                my_rows++;
            }
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
        std::size_t start_row = my_idx * others_rows;
        std::size_t current_row = 0;
        std::size_t before_readln = 0, after_readln = 0;

        while (std::getline(infile, line))
        {
            before_readln = matrix_array.size();

            auto begin_local = line.begin();
            if (current_row < start_row)
            {
                current_row++;
            }
            else if (current_row - start_row > my_rows)
            {
                break;
            }
            else if (boost::spirit::qi::parse(begin_local, line.end(),
                         boost::spirit::qi::double_ % ',', current_line))
            {
                if (begin_local == line.end() || header_parsed)
                {
                    header_parsed = true;

                    matrix_array.insert(matrix_array.end(),
                        current_line.begin(), current_line.end());

                    after_readln = matrix_array.size();
                    current_row++;
                }
                current_line.clear();
            }
            else
            {
                throw std::runtime_error(generate_error_message(
                    "wrong data format " + filename + ':' + std::to_string(0)));
            }
        }

        annotation tile(ir::range{"tile",
            primitive_argument_type(ir::range{"columns", 0ll, (long long) dims.second}),
            primitive_argument_type(ir::range{
                    "rows", (long long) start_row,
                (long long) (start_row + my_rows)})});

        annotation_information ann_info{
            std::move((filename + "file_read_csv_d")), 0ll};
        annotation locality(
            ir::range{"locality", (long long) my_idx, (long long) num_locs});

        annotation localities = localities_annotation(
            locality, std::move(tile), ann_info, name_, codename_);

        // How do I crawl this? Stay simple, make it work. Then optimize
        blaze::DynamicMatrix<double> mat(
            my_rows, dims.second, matrix_array.data());

        primitive_argument_type ret(mat);
        ret.set_annotation(std::move(localities), name_, codename_);
        return ret;
    }

    std::pair<std::size_t, std::size_t> file_read_csv_d::get_dims(
        std::string filename) const
    {
        std::ifstream infile(filename.c_str(), std::ios::in);

        if (!infile.is_open())
        {
            throw std::runtime_error(
                generate_error_message("couldn't open file: " + filename));
        }

        std::string line;
        bool header_parsed = false;
        std::vector<double> current_line;
        std::size_t n_rows = 0, n_cols = 0;

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

        return std::make_pair(n_rows, n_cols);
    }

    primitive_argument_type file_read_csv_d::read(std::string filename,
        phylanx::execution_tree::annotation&& locality_info) const
    {
        // Here, the locality info is being interpreted as:
        // ("locality", my index in group, number of localities)
        annotation tmp;
        if (locality_info.get_type() != "locality")
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter, "file_read_csv_d::read",
                generate_error_message("missing locality annotation"));
        }
        ir::range data = locality_info.get_data();
        ir::range_iterator first = data.begin();
        ir::range_iterator second = data.begin();
        ++second;

        std::size_t my_idx =
            phylanx::execution_tree::extract_scalar_integer_value_strict(
                *first);
        std::size_t num_locs =
            phylanx::execution_tree::extract_scalar_integer_value_strict(
                *second);

        std::pair<std::size_t, std::size_t> my_locality_info(my_idx, num_locs);
        std::pair<std::size_t, std::size_t> dims = get_dims(filename);

        if (dims.first == 1)
        {
            if (dims.second == 1)
            {
                // Read scalar value
                // scalar value
                // Need to decide whether this would be an error or
                // if only locality 0 keeps the scalar
                return primitive_argument_type{
                    read_scalar(filename, my_locality_info, dims)};
            }
            return primitive_argument_type{
                read_vector(filename, my_locality_info, dims)};
        }
        else
        {
            return primitive_argument_type{
                read_matrix(filename, my_locality_info, dims)};
        }

        return primitive_argument_type{};
    }    // namespace primitives

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
        auto f = value_operand(operands[1], args, name_, codename_, ctx);
        phylanx::execution_tree::annotation loc_info(
            phylanx::execution_tree::extract_list_value_strict(f.get()));

        auto this_ = this->shared_from_this();

        return hpx::dataflow(
            hpx::launch::sync,
            [this_ = std::move(this_)](std::string&& filename,
                phylanx::execution_tree::annotation&& loc_info)
                -> primitive_argument_type {
                return this_->read(filename, std::move(loc_info));
            },
            std::move(filename), std::move(loc_info));
        return hpx::future<primitive_argument_type>();
    }
}}}    // namespace phylanx::execution_tree::primitives
