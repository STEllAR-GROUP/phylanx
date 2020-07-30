//  Copyright (c) 2017 Alireza Kheirkhahan
//  Copyright (c) 2020 Bita Hasheminezhad
//  Copyright (c) 2020 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/annotation.hpp>
#include <phylanx/execution_tree/localities_annotation.hpp>
#include <phylanx/execution_tree/locality_annotation.hpp>
#include <phylanx/execution_tree/meta_annotation.hpp>
#include <phylanx/execution_tree/primitives/node_data_helpers.hpp>
#include <phylanx/execution_tree/tiling_annotations.hpp>
#include <phylanx/ir/node_data.hpp>
#include <phylanx/plugins/dist_matrixops/tile_calculation_helper.hpp>
#include <phylanx/plugins/fileio/dist_file_read_csv.hpp>
#include <phylanx/plugins/fileio/file_read_csv_impl.hpp>
#include <phylanx/util/detail/range_dimension.hpp>

#include <hpx/include/lcos.hpp>
#include <hpx/include/naming.hpp>
#include <hpx/include/util.hpp>
#include <hpx/errors/throw_exception.hpp>

#include <array>
#include <atomic>
#include <cstddef>
#include <cstdint>
#include <fstream>
#include <iomanip>
#include <memory>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include <blaze/Math.h>
#include <blaze_tensor/Math.h>

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace execution_tree { namespace primitives
{
    ///////////////////////////////////////////////////////////////////////////
    match_pattern_type const dist_file_read_csv::match_data =
    {
        hpx::util::make_tuple("file_read_csv_d",
            std::vector<std::string>{R"(
                file_read_csv_d(
                    _1_filename,
                    __arg(_2_mode3d, false),
                    __arg(_3_page_nrows, 1),
                    __arg(_4_tiling_type, "sym"),
                    __arg(_5_intersection, nil),
                    __arg(_6_name, ""),
                    __arg(_7_numtiles, num_localities())
                )
            )"},
            &create_dist_file_read_csv, &create_primitive<dist_file_read_csv>,
            R"(filename, tiling_type, intersection, name, numtiles
            Args:

                filename (string) : file name including its path.
                mode3d (bool, optional) : If sets to true, the result will be a
                    3d array. Each page_nrows rows of the data is stored in a
                    new page of the result.
                page_nrows (int, optional) : it is used only whenmode3d is true.
                    It determines the number of rows in each page of the
                    resulted array.
                tiling_type (string, optional): defaults to `sym` which is a
                    balanced way of tiling among all the numtiles localities.
                    Other options are `page`, `row` or `column` tiling. For a
                    vector all these three tiling_type are the same.
                intersection (int or a tuple of ints, optional): the size of
                    overlapped part on each dimension. If an integer is given,
                    that would be the intersection length on all dimensions
                    that are tiled. The middle parts get to have two
                    intersections, one with the tile before it and one with the
                    tile after it.
                name (string, optional): the array given name. If not given, a
                    globally unique name will be generated.
                numtiles (int, optional): number of tiles of the returned array.
                    if not given it sets to the number of localities in the
                    application.

            Returns:

            Returns a distributed array representing the contents of the given
            csv file.
            )")
    };

    ///////////////////////////////////////////////////////////////////////////
    dist_file_read_csv::dist_file_read_csv(primitive_arguments_type&& operands,
        std::string const& name, std::string const& codename)
      : primitive_component_base(std::move(operands), name, codename)
    {}

    ///////////////////////////////////////////////////////////////////////////
    namespace detail
    {
        static std::atomic<std::size_t> csv_count(0);
        std::string generate_csv_name(std::string&& given_name)
        {
            if (given_name.empty())
            {
                return "csv_file_" + std::to_string(++csv_count);
            }

            return std::move(given_name);
        }
    }

    ///////////////////////////////////////////////////////////////////////////
    primitive_argument_type dist_file_read_csv::dist_read_2d(
        std::ifstream&& infile, std::string const& filename,
        std::string const& tiling_type,
        std::array<std::size_t, PHYLANX_MAX_DIMENSIONS> const& intersections,
        std::string&& given_name, std::uint32_t numtiles) const
    {
        std::vector<double> data;
        std::size_t n_rows, n_cols;
        std::tie(data, n_rows, n_cols) =
            read_helper(std::move(infile), filename);

        std::int64_t row_start, column_start;
        std::size_t row_size, column_size;
        std::uint32_t tile_idx = hpx::get_locality_id();

        std::tie(row_start, column_start, row_size, column_size) =
            tile_calculation::tile_calculation_2d(
                tile_idx, n_rows, n_cols, numtiles, tiling_type);

        // adding overlap
        if (row_size != n_rows && intersections[0] != 0)    // rows overlap
        {
            std::tie(row_start, row_size) =
                tile_calculation::tile_calculation_overlap_1d(
                    row_start, row_size, n_rows, intersections[0]);
        }
        if (column_size != n_cols &&
            intersections[1] != 0)    // columns overlap
        {
            std::tie(column_start, column_size) =
                tile_calculation::tile_calculation_overlap_1d(
                    column_start, column_size, n_cols, intersections[1]);
        }

        tiling_information_2d tile_info(
            tiling_span(row_start, row_start + row_size),
            tiling_span(column_start, column_start + column_size));

        locality_information locality_info(tile_idx, numtiles);
        annotation locality_ann = locality_info.as_annotation();

        std::string base_name =
            detail::generate_csv_name(std::move(given_name));

        annotation_information ann_info(
            std::move(base_name), 0);    //generation 0

        auto attached_annotation =
            std::make_shared<annotation>(localities_annotation(locality_ann,
                tile_info.as_annotation(name_, codename_), ann_info, name_,
                codename_));

        blaze::DynamicMatrix<double> whole_data(n_rows, n_cols, data.data());
        blaze::DynamicMatrix<double> result =
            blaze::submatrix(std::move(whole_data), row_start, column_start,
                row_size, column_size);

        return primitive_argument_type(result, attached_annotation);
    }

    ///////////////////////////////////////////////////////////////////////////
    primitive_argument_type dist_file_read_csv::dist_read_3d(
        std::ifstream&& infile, std::string const& filename,
        std::int64_t given_nrows, std::string const& tiling_type,
        std::array<std::size_t, PHYLANX_MAX_DIMENSIONS> const& intersections,
        std::string&& given_name, std::uint32_t numtiles) const
    {
        std::vector<double> data;
        std::size_t n_rows, n_cols;
        std::tie(data, n_rows, n_cols) =
            read_helper(std::move(infile), filename);
        std::size_t n_pages = static_cast<std::size_t>(n_rows / given_nrows);

        if (n_rows % given_nrows != 0)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter, "file_read_csv::read_3d",
                util::generate_error_message(
                    "the number of rows in the csv file is not divisible by "
                    "the given number of rows in a page"));
        }

        std::int64_t page_start, row_start, column_start;
        std::size_t page_size, row_size, column_size;
        std::uint32_t tile_idx = hpx::get_locality_id();

        std::tie(page_start, row_start, column_start, page_size, row_size,
            column_size) = tile_calculation::tile_calculation_3d(tile_idx,
            n_pages, given_nrows, n_cols, numtiles, tiling_type);

        // adding overlap
        if (page_size != n_pages && intersections[0] != 0)    // pages overlap
        {
            std::tie(page_start, page_size) =
                tile_calculation::tile_calculation_overlap_1d(
                    page_start, page_size, n_pages, intersections[0]);
        }
        if (row_size != given_nrows && intersections[1] != 0)    // rows overlap
        {
            std::tie(row_start, row_size) =
                tile_calculation::tile_calculation_overlap_1d(
                    row_start, row_size, given_nrows, intersections[1]);
        }
        if (column_size != n_cols &&
            intersections[2] != 0)    // columns overlap
        {
            std::tie(column_start, column_size) =
                tile_calculation::tile_calculation_overlap_1d(
                    column_start, column_size, n_cols, intersections[2]);
        }

        tiling_information_3d tile_info(
            tiling_span(page_start, page_start + page_size),
            tiling_span(row_start, row_start + row_size),
            tiling_span(column_start, column_start + column_size));

        locality_information locality_info(tile_idx, numtiles);
        annotation locality_ann = locality_info.as_annotation();

        std::string base_name =
            detail::generate_csv_name(std::move(given_name));

        annotation_information ann_info(
            std::move(base_name), 0);    //generation 0

        auto attached_annotation =
            std::make_shared<annotation>(localities_annotation(locality_ann,
                tile_info.as_annotation(name_, codename_), ann_info, name_,
                codename_));

        blaze::DynamicTensor<double> whole_data(
            n_pages, given_nrows, n_cols, data.data());
        blaze::DynamicTensor<double> result =
            blaze::subtensor(std::move(whole_data), page_start, row_start,
                column_start, page_size, row_size, column_size);

        return primitive_argument_type(result, attached_annotation);
    }

    ///////////////////////////////////////////////////////////////////////////
    hpx::future<primitive_argument_type> dist_file_read_csv::eval(
        primitive_arguments_type const& operands,
        primitive_arguments_type const& args, eval_context ctx) const
    {
        if (operands.size() < 2 || operands.size() > 7)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter, "dist_file_read_csv::eval",
                generate_error_message("the file_read_csv_d primitive requires "
                                       "at least 2 and at most 5 operands"));
        }

        if (!valid(operands[0]))
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "dist_file_read_csv::eval",
                generate_error_message(
                    "the file_read_csv_d primitive requires that the given "
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
                    std::size_t numdims = mode3d ? 3 : 2;

                    std::int64_t page_nrows;
                    if (args.size() > 2 && valid(args[2]))
                    {
                        page_nrows =
                            extract_scalar_positive_integer_value_strict(
                                std::move(args[2]), this_->name_,
                                this_->codename_);
                    }

                    // using balanced symmetric tiles as the default
                    std::string tiling_type = "sym";
                    if (args.size() > 3 && valid(args[3]))
                    {
                        tiling_type = extract_string_value(
                            std::move(args[3]), this_->name_, this_->codename_);
                        if ((tiling_type != "sym" && tiling_type != "page") &&
                            tiling_type != "row" && tiling_type != "column")
                        {
                            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                                "dist_file_read_csv::eval",
                                this_->generate_error_message(
                                    "invalid tiling_type. The tiling_type can "
                                    "be one of these: `sym`, `page`, `row` or "
                                    "`column`"));
                        }
                    }

                    std::array<std::size_t, PHYLANX_MAX_DIMENSIONS>
                        intersections{0};
                    if (args.size() > 4 && valid(args[4]))
                    {
                        if (is_list_operand_strict(args[4]))
                        {
                            ir::range&& intersection_list =
                                extract_list_value_strict(std::move(args[4]),
                                    this_->name_, this_->codename_);

                            if (intersection_list.size() != 1 &&
                                intersection_list.size() != numdims)
                            {
                                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                                    "dist_constant::eval",
                                    this_->generate_error_message(
                                        "intersection should have the same "
                                        "number of dimensions as the array, or "
                                        "be represented with an integer for "
                                        "all dimensions"));
                            }
                            intersections =
                                util::detail::extract_nonneg_range_dimensions(
                                    intersection_list, this_->name_,
                                    this_->codename_);
                        }
                        else if (is_numeric_operand(args[4]))
                        {
                            intersections[0] =
                                extract_scalar_nonneg_integer_value_strict(
                                    std::move(args[4]), this_->name_,
                                    this_->codename_);

                            // we assume all dimensions have the same
                            // intersection length which is the given one
                            for (std::size_t i = 1; i != PHYLANX_MAX_DIMENSIONS;
                                 ++i)
                            {
                                if (i == numdims)
                                    break;
                                intersections[i] = intersections[0];
                            }
                        }
                        else
                        {
                            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                                "dist_file_read_csv::eval",
                                this_->generate_error_message(
                                    "intersection can be an integer or a list "
                                    "of integers"));
                        }
                    }

                    std::string given_name = "";
                    if (args.size() > 5 && valid(args[5]))
                    {
                        given_name = extract_string_value(std::move(args[5]),
                            this_->name_, this_->codename_);
                    }

                    std::uint32_t numtiles =
                        hpx::get_num_localities(hpx::launch::sync);
                    if (args.size() > 6 && valid(args[6]))
                    {
                        extract_scalar_positive_integer_value_strict(
                            std::move(args[6]), this_->name_, this_->codename_);
                    }

                    std::ifstream infile(filename.c_str(), std::ios::in);

                    if (!infile.is_open())
                    {
                        throw std::runtime_error(this_->generate_error_message(
                            "couldn't open file: " + filename));
                    }

                    if (mode3d)
                    {
                        return this_->dist_read_3d(std::move(infile), filename,
                            page_nrows, tiling_type, intersections,
                            std::move(given_name), numtiles);
                    }

                    // dist_file_read_csv never considers 1d arrays. It is a
                    // dataframe ether representing a matrix or a tensor
                    return this_->dist_read_2d(std::move(infile), filename,
                             tiling_type, intersections,
                            std::move(given_name), numtiles);
                }),
            detail::map_operands(operands, functional::value_operand{}, args,
                name_, codename_, std::move(ctx)));
    }
}}}
