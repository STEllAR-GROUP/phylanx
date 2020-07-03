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
#include <phylanx/plugins/fileio/dist_file_read_csv.hpp>
#include <phylanx/plugins/fileio/file_read_csv_impl.hpp>

#include <hpx/include/lcos.hpp>
#include <hpx/include/naming.hpp>
#include <hpx/include/util.hpp>
#include <hpx/errors/throw_exception.hpp>
//#include <hpx/runtime/threads/run_as_os_thread.hpp>

#include <atomic>
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
                    __arg(_2_tiling_type, "sym"),
                    __arg(_3_intersection, nil),
                    __arg(_4_name, ""),
                    __arg(_5_numtiles, num_localities())
                )
            )"},
            &create_dist_file_read_csv, &create_primitive<dist_file_read_csv>,
            R"(filename, tiling_type, intersection, name, numtiles
            Args:

                filename (string) : file name including its path.
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

            Returns a distributed array representing the contents of a the
            given csv file.
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
        static std::atomic<std::size_t> const_count(0);
        std::string generate_csv_name(std::string&& given_name)
        {
            if (given_name.empty())
            {
                return "csv_file_" + std::to_string(++const_count);
            }

            return std::move(given_name);
        }
    }

    ///////////////////////////////////////////////////////////////////////////
    hpx::future<primitive_argument_type> dist_file_read_csv::eval(
        primitive_arguments_type const& operands,
        primitive_arguments_type const& args, eval_context ctx) const
    {
        if (operands.size() < 2 || operands.size() > 5)
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

                    // using balanced symmetric tiles as the default
                    std::string tiling_type = "sym";
                    if (valid(args[1]))
                    {
                        tiling_type = extract_string_value(
                            std::move(args[1]), this_->name_, this_->codename_);
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
                    if (valid(args[2]))
                    {
                        //if (is_list_operand_strict(args[2]))
                        //{
                        //    ir::range&& intersection_list =
                        //        extract_list_value_strict(std::move(args[2]),
                        //            this_->name_, this_->codename_);

                            //if (intersection_list.size() != 1 &&
                            //    intersection_list.size() != numdims)
                            //{
                            //    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                            //        "dist_constant::eval",
                            //        this_->generate_error_message(
                            //            "intersection should have the same "
                            //            "number of dimensions as the array, or "
                            //            "be represented with an integer for "
                            //            "all dimensions"));
                            //}
                        //    intersections =
                        //        util::detail::extract_nonneg_range_dimensions(
                        //            intersection_list, this_->name_,
                        //            this_->codename_);
                        //}
                        //else if (is_numeric_operand(args[2]))
                        //{
                        //    intersections[0] =
                        //        extract_scalar_nonneg_integer_value_strict(
                        //            std::move(args[2]), this_->name_,
                        //            this_->codename_);

                        //    // we assume all dimensions have the same
                        //    // intersection length which is the given one
                        //    for (std::size_t i = 1; i != PHYLANX_MAX_DIMENSIONS;
                        //         ++i)
                        //    {
                        //        if (i == numdims)
                        //            break;
                        //        intersections[i] = intersections[0];
                        //    }
                        //}
                        //else
                        //{
                        //    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        //        "dist_constant::eval",
                        //        this_->generate_error_message(
                        //            "intersection can be an integer or a list "
                        //            "of integers"));
                        //}
                    }

                    std::string given_name = "";
                    if (valid(args[3]))
                    {
                        given_name = extract_string_value(std::move(args[3]),
                            this_->name_, this_->codename_);
                    }

                    std::uint32_t numtiles =
                        hpx::get_num_localities(hpx::launch::sync);
                    if (valid(args[4]))
                    {
                        extract_scalar_positive_integer_value_strict(
                            std::move(args[4]), this_->name_, this_->codename_);
                    }

                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "dist_constant::eval",
                        util::generate_error_message(
                            "the given shape is of an unsupported "
                            "dimensionality",
                            this_->name_, this_->codename_));
                }),
            detail::map_operands(operands, functional::value_operand{}, args,
                name_, codename_, std::move(ctx)));
    }
}}}
