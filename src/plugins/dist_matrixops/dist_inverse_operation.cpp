// Copyright (c) 2020 Rory Hector
// Copyright (c) 2018-2020 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/annotation.hpp>
#include <phylanx/execution_tree/localities_annotation.hpp>
#include <phylanx/execution_tree/locality_annotation.hpp>
#include <phylanx/execution_tree/meta_annotation.hpp>
#include <phylanx/execution_tree/primitives/node_data_helpers.hpp>
#include <phylanx/execution_tree/tiling_annotations.hpp>
#include <phylanx/ir/node_data.hpp>
#include <phylanx/plugins/dist_matrixops/dist_inverse_operation.hpp>
#include <phylanx/plugins/dist_matrixops/tile_calculation_helper.hpp>
#include <phylanx/util/detail/bad_swap.hpp>

#include <hpx/errors/throw_exception.hpp>
#include <hpx/include/lcos.hpp>
#include <hpx/include/naming.hpp>
#include <hpx/include/util.hpp>
#include <hpx/lcos/future.hpp>

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

#include <blaze/Math.h>
#include <blaze_tensor/Math.h>

#include <phylanx/util/distributed_matrix.hpp>

// This is just boilerplate
namespace phylanx { namespace dist_matrixops { namespace primitives {
    // Declare the help_string details, just for explaining what
    // a primitive is doing, if needed.
    constexpr char const* const help_string = R"(
        inverse_d(matrix)
        Args:

            blaze dynamic matrix

        Returns:

            the inverse of the matrix
        )";

    // For the plugin file, you need to tell Phylanx these things about
    // your primtiive in a tuple
    // (1) name of the primitive, e.g. inverse_d
    // (2) a description of how the primitive can be invoked in the compiler
    //        _k means the primitive should be invoked with k arguments
    // (3) a creation function (defined in header)
    // (4) a connection between the name and the type (always the same)
    // (5) describes the usage of the primitive
    execution_tree::match_pattern_type const dist_inverse::match_data = {
        hpx::util::make_tuple("inverse_d", std::vector<std::string>{R"(
			inverse_d(
				_1_matrix
				)
			)"},
            &create_dist_inverse,
            &execution_tree::create_primitive<dist_inverse>, help_string)};

    // Pretty much boilerplate, just change the name, or may need to use
    // std::vector<primitive_arguments_type>&& for multiple args.
    // Passes the arguments along to the constructor of the base class.
    // Stores the three arguments as members.
    // Name = name of primitive instance, Codename = file used to generate it
    dist_inverse::dist_inverse(
        execution_tree::primitive_arguments_type&& operands,
        std::string const& name, std::string const& codename)
      : primitive_component_base(std::move(operands), name, codename)
    {
    }

    // This is where the computation of the inverse should be performed.
    template <typename T>
    execution_tree::primitive_argument_type dist_inverse::distGaussInv(
        ir::node_data<T>&& arg,
        execution_tree::localities_information&& lhs_localities) const
    {
        if (lhs_localities.num_dimensions() != 2)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "dist_dot_operation::distGaussInv",
                generate_error_message("the input must be a 2d matrix"));
        }

        std::cout << "This is locality "
                  << lhs_localities.locality_.locality_id_ << std::endl;

        std::cout << "Data belonging to this locality:"<< std::endl;
        std::cout << arg.matrix() << std::endl;

        std::cout << "Total number of localities = "
                  << lhs_localities.locality_.num_localities_ << std::endl;

        std::cout << "The localities information is as follows: " << std::endl;
        std::cout << "   Columns = " << lhs_localities.columns() << std::endl;
        std::cout << "   Rows = " << lhs_localities.rows() << std::endl;
        std::cout << "   Size = " << lhs_localities.size() << std::endl;

        std::cout << "Tile information: " << std::endl;
        std::size_t numTilesSeen = 1;
        for (auto const& lhsTile : lhs_localities.tiles_)
        {
            std::cout << "    "<< "Tile " << numTilesSeen <<
                " Dimension = "<< lhsTile.dimension() << std::endl;
            std::cout << "    "
                      << "Tile " << numTilesSeen
                      << " Span Name 1 = " << lhsTile.get_span_name(1) << std::endl;
            numTilesSeen++;
        }

        util::distributed_matrix<T> lhs_data(lhs_localities.annotation_.name_,
        arg.matrix(), lhs_localities.locality_.num_localities_,
        lhs_localities.locality_.locality_id_);

        execution_tree::tiling_span const& lhs_span =
            lhs_localities.get_span(1);

        blaze::DynamicMatrix<T> result_matrix(
            arg.dimension(0), lhs_localities.columns(), T{0});

        std::cout << result_matrix << std::endl;

        //for (auto const& lhs_tile : lhs_localities.tiles_)
        //{
        //    lhs_data.fetch(1).get();
        //}

        blaze::DynamicMatrix<double> myMatrix = arg.matrix();
        std::size_t numRows = myMatrix.rows();
        std::size_t numCols = myMatrix.columns();

        // Definition of a nxn double row-major identity matrix
        blaze::DynamicMatrix<double> invMatrix =
            blaze::IdentityMatrix<double>(numRows);
        //invMatrix = lhs_data.fetch(0, 1, 2, 0, lhs_tile.spans_[1].size()).get();

        //arg = blaze::inv(arg.matrix());
        execution_tree::primitive_argument_type result{std::move(arg)};
        return result;
    }

    // This is where the computation of the inverse should be performed.
    execution_tree::primitive_argument_type dist_inverse::distGaussInv(
        execution_tree::primitive_argument_type&& lhs) const
    {
        using namespace execution_tree;

        execution_tree::localities_information lhs_localities =
            extract_localities_information(lhs, name_, codename_);

        switch (extract_numeric_value_dimension(lhs, name_, codename_))
        {
        case 2:
            // DO INVERSE
            std::cout << "Input checks out, now sending for calculation.";
            std::cout << std::endl;
            return distGaussInv(
                extract_numeric_value(std::move(lhs), name_, codename_),
                std::move(lhs_localities));
        default:
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                " dist_inverse::distGaussInv",
                generate_error_message("left hand side operand has unsupported "
                                       "number of dimensions"));
        }
    }

    // Call the evaluation function
    // Petty much boilerplate, just gotta change names and stuff
    // if the constructor was called without an operand, just pass back
    // the args to the function, otherwise, pass the operands
    hpx::future<execution_tree::primitive_argument_type> dist_inverse::eval(
        execution_tree::primitive_arguments_type const& operands,
        execution_tree::primitive_arguments_type const& args,
        execution_tree::eval_context ctx) const

    {
        using namespace execution_tree;

        // Check to make sure there is exactly one item to invert
        if (operands.size() != 1)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter, "dist_inverse::eval",
                generate_error_message(
                    "the gaussian inverse operation primitive requires"
                    "exactly one operand"));
        }

        // Check if there are no valid operands
        if (!valid(operands[0]))
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter, "dist_inverse::eval",
                generate_error_message(
                    "the gaussian_inverse_operation primitive requires that "
                    "the arguments given by the operands array is valid"));
        }

        //-auto f = value_operand(operands[0], args, name_, codename_, ctx);

        // Get a future to the result of the actual computation
        auto this_ = this->shared_from_this();
        return hpx::dataflow(hpx::launch::sync,
            hpx::util::unwrapping(
                [this_ = std::move(this_)](primitive_arguments_type&& args)
                    -> primitive_argument_type {
                    return this_->distGaussInv(std::move(args[0]));
                }),
            execution_tree::primitives::detail::map_operands(operands,
                execution_tree::functional::value_operand{}, args, name_,
                codename_, std::move(ctx)));
    }
}}}    // namespace phylanx::dist_matrixops::primitives
