//  Copyright (c) 2017-2019 Hartmut Kaiser
//  Copyright (c) 2017 Parsa Amini
//  Copyright (c) 2019 Bita Hasheminezhad
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_DIST_MATRIXOPS_TENSORDOT_IMPL_JUN_17_2019_0828AM)
#define PHYLANX_DIST_MATRIXOPS_TENSORDOT_IMPL_JUN_17_2019_0828AM

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/annotation.hpp>
#include <phylanx/execution_tree/localities_annotation.hpp>
#include <phylanx/execution_tree/locality_annotation.hpp>
#include <phylanx/execution_tree/meta_annotation.hpp>
#include <phylanx/execution_tree/primitives/node_data_helpers.hpp>
#include <phylanx/ir/node_data.hpp>
#include <phylanx/plugins/common/dot_operation_nd.hpp>
#include <phylanx/plugins/dist_matrixops/dist_dot_operation.hpp>
#include <phylanx/util/distributed_vector.hpp>

#include <hpx/assertion.hpp>
#include <hpx/collectives/all_reduce.hpp>
#include <hpx/include/lcos.hpp>
#include <hpx/include/naming.hpp>
#include <hpx/include/util.hpp>
#include <hpx/throw_exception.hpp>

#include <cstddef>
#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#include <blaze/Math.h>
#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
#include <blaze_tensor/Math.h>
#endif

using std_int64_t = std::int64_t;
using std_uint8_t = std::uint8_t;

////////////////////////////////////////////////////////////////////////////////
REGISTER_DISTRIBUTED_VECTOR_DECLARATION(double);
REGISTER_DISTRIBUTED_VECTOR_DECLARATION(std_int64_t);
REGISTER_DISTRIBUTED_VECTOR_DECLARATION(std_uint8_t);

HPX_REGISTER_ALLREDUCE_DECLARATION(double);
HPX_REGISTER_ALLREDUCE_DECLARATION(std_int64_t);
HPX_REGISTER_ALLREDUCE_DECLARATION(std_uint8_t);

using blaze_vector_double = blaze::DynamicVector<double>;
using blaze_vector_std_int64_t = blaze::DynamicVector<std::int64_t>;
using blaze_vector_std_uint8_t = blaze::DynamicVector<std::uint8_t>;

HPX_REGISTER_ALLREDUCE_DECLARATION(blaze_vector_double);
HPX_REGISTER_ALLREDUCE_DECLARATION(blaze_vector_std_int64_t);
HPX_REGISTER_ALLREDUCE_DECLARATION(blaze_vector_std_uint8_t);

////////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace dist_matrixops { namespace primitives
{
    ////////////////////////////////////////////////////////////////////////////
    template <typename T>
    execution_tree::primitive_argument_type dist_dot_operation::dot0d(
        ir::node_data<T>&& lhs, ir::node_data<T>&& rhs) const
    {
        switch (rhs.num_dimensions())
        {
        case 0:
            // If is_scalar(lhs) && is_scalar(rhs)
            return common::dot0d0d(std::move(lhs), std::move(rhs));

        case 1:
            // If is_scalar(lhs) && is_vector(rhs)
            return common::dot0d1d(std::move(lhs), std::move(rhs));

        case 2:
            // If is_scalar(lhs) && is_matrix(rhs)
            return common::dot0d2d(std::move(lhs), std::move(rhs));

#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
        case 3:
            // If is_scalar(lhs) && is_tensor(rhs)
            return common::dot0d3d(std::move(lhs), std::move(rhs));
#endif

        default:
            // lhs_order == 1 && rhs_order != 2
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "dist_dot_operation::dot0d",
                generate_error_message(
                    "the operands have incompatible number of dimensions"));
        }
    }

    ////////////////////////////////////////////////////////////////////////////
    template <typename T>
    execution_tree::primitive_argument_type dist_dot_operation::dot1d1d(
        ir::node_data<T>&& lhs, ir::node_data<T>&& rhs,
        execution_tree::localities_information const& lhs_localities,
        execution_tree::localities_information const& rhs_localities) const
    {
        if (lhs_localities.dimension() < 1 || rhs_localities.dimension() < 1)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "dist_dot_operation::dot1d1d",
                generate_error_message(
                    "the operands have incompatible dimensionalities"));
        }

        if (lhs_localities.size() != rhs_localities.size())
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "dist_dot_operation::dot1d1d",
                generate_error_message(
                    "the operands have incompatible size"));
        }

        // construct a distributed vector object for the rhs
        util::distributed_vector<T> rhs_data(
            rhs_localities.annotation_.name_, rhs.vector(),
            rhs_localities.locality_.num_localities_,
            rhs_localities.locality_.locality_id_);

        // use the local tile of lhs and calculate the dot product with all
        // corresponding tiles of rhs
        std::size_t lhs_span_index = 0;
        if (!lhs_localities.has_span(0))
        {
            HPX_ASSERT(lhs_localities.has_span(1));
            lhs_span_index = 1;
        }

        execution_tree::tiling_span const& lhs_span =
            lhs_localities.get_span(lhs_span_index);

        // go over all tiles of rhs vector
        T dot_result = T{0};

        std::uint32_t loc = 0;
        for (auto const& rhs_tile : rhs_localities.tiles_)
        {
            std::size_t rhs_span_index = 0;
            if (!rhs_tile.spans_[0].is_valid())
            {
                HPX_ASSERT(rhs_tile.spans_[1].is_valid());
                rhs_span_index = 1;
            }

            execution_tree::tiling_span const& rhs_span =
                rhs_tile.spans_[rhs_span_index];

            execution_tree::tiling_span intersection;
            if (!intersect(lhs_span, rhs_span, intersection))
            {
                ++loc;
                continue;
            }

            // project global coordinates onto local ones
            execution_tree::tiling_span lhs_intersection =
                lhs_localities.project_coords(
                    lhs_localities.locality_.locality_id_, lhs_span_index,
                    intersection);
            execution_tree::tiling_span rhs_intersection =
                rhs_localities.project_coords(
                    loc, rhs_span_index, intersection);

            if (rhs_localities.locality_.locality_id_ == loc)
            {
                // calculate the dot product with local tile
                dot_result += T{blaze::dot(
                    blaze::subvector(lhs.vector(), lhs_intersection.start_,
                        lhs_intersection.size()),
                    blaze::subvector(*rhs_data, rhs_intersection.start_,
                        rhs_intersection.size()))};
            }
            else
            {
                // calculate the dot product with remote tile
                dot_result += T{blaze::dot(
                    blaze::subvector(lhs.vector(), lhs_intersection.start_,
                        lhs_intersection.size()),
                    rhs_data.fetch(loc, rhs_intersection.start_,
                            rhs_intersection.stop_).get())};
            }
            ++loc;
        }

        // collect overall result if left hand side vector is distributed
        if (lhs_localities.locality_.num_localities_ > 1)
        {
            lhs = hpx::all_reduce(
                ("all_reduce_" + lhs_localities.annotation_.name_).c_str(),
                dot_result, std::plus<T>{},
                lhs_localities.locality_.num_localities_, std::size_t(-1),
                lhs_localities.locality_.locality_id_)
                      .get();
        }
        else
        {
            lhs = dot_result;
        }

        return execution_tree::primitive_argument_type{std::move(lhs)};
    }

    template <typename T>
    execution_tree::primitive_argument_type dist_dot_operation::dot1d2d(
        ir::node_data<T>&& lhs, ir::node_data<T>&& rhs) const
    {
        if (lhs.size() != rhs.dimension(0))
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "dist_dot_operation::dot1d2d",
                generate_error_message(
                    "the operands have incompatible number of dimensions"));
        }
        // lhs = blaze::trans(rhs.matrix()) * lhs.vector();
        lhs = blaze::trans(blaze::trans(lhs.vector()) * rhs.matrix());
        return execution_tree::primitive_argument_type{std::move(lhs)};
    }

#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
    template <typename T>
    execution_tree::primitive_argument_type dist_dot_operation::dot1d3d(
        ir::node_data<T>&& lhs, ir::node_data<T>&& rhs) const
    {
        if (lhs.size() != rhs.dimension(1))
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "dist_dot_operation::dot1d3d",
                generate_error_message(
                    "the operands have incompatible number of dimensions"));
        }

        auto t = rhs.tensor();
        blaze::DynamicMatrix<T> result(t.pages(), t.columns());

        for (std::size_t i = 0; i != t.pages(); ++i)
            blaze::row(blaze::submatrix(result, i, 0, 1, t.columns()), 0) =
                blaze::trans(lhs.vector()) * blaze::pageslice(t, i);

        return execution_tree::primitive_argument_type{std::move(result)};
    }
#endif

    // lhs_num_dims == 1
    // Case 1: Inner product of two vectors
    // Case 2: Inner product of a vector and an array of vectors
    // Case 3: Inner product of a matrix (tensor slice)
    template <typename T>
    execution_tree::primitive_argument_type dist_dot_operation::dot1d(
        ir::node_data<T>&& lhs, ir::node_data<T>&& rhs,
        execution_tree::localities_information const& lhs_localities,
        execution_tree::localities_information const& rhs_localities) const
    {
        switch (rhs.num_dimensions())
        {
        case 0:
            // If is_vector(lhs) && is_scalar(rhs)
            return common::dot1d0d(std::move(lhs), std::move(rhs));

        case 1:
            // If is_vector(lhs) && is_vector(rhs)
            return dot1d1d(
                std::move(lhs), std::move(rhs), lhs_localities, rhs_localities);

        case 2:
            // If is_vector(lhs) && is_matrix(rhs)
            return dot1d2d(std::move(lhs), std::move(rhs));

#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
        case 3:
            // If is_vector(lhs) && is_tensor(rhs)
            return dot1d3d(std::move(lhs), std::move(rhs));
#endif

        default:
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "dist_dot_operation::dot1d",
                generate_error_message(
                    "the operands have incompatible number of dimensions"));
        }
    }

    ///////////////////////////////////////////////////////////////////////////
    template <typename T>
    execution_tree::primitive_argument_type dist_dot_operation::dot2d1d(
        ir::node_data<T>&& lhs, ir::node_data<T>&& rhs,
        execution_tree::localities_information&& lhs_localities,
        execution_tree::localities_information const& rhs_localities) const
    {
        if (lhs_localities.dimension() < 2 || rhs_localities.dimension() < 1)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "dist_dot_operation::dot2d1d",
                generate_error_message(
                    "the operands have incompatible dimensionalities"));
        }

        if (lhs_localities.columns() != rhs_localities.size())
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "dist_dot_operation::dot2d1d",
                generate_error_message(
                    "the operands have incompatible number of dimensions"));
        }

        // construct a distributed vector object for the rhs
        util::distributed_vector<T> rhs_data(
            rhs_localities.annotation_.name_, rhs.vector(),
            rhs_localities.locality_.num_localities_,
            rhs_localities.locality_.locality_id_);

        // use the local tile of lhs and calculate the dot product with all
        // corresponding tiles of rhs
        execution_tree::tiling_span const& lhs_span =
            lhs_localities.get_span(0);

        // go over all tiles of rhs vector, the result size is determined by the
        // number of rows of the lhs tile
        blaze::DynamicVector<T> dot_result(lhs.dimension(0), T{0});

        std::uint32_t loc = 0;
        for (auto const& rhs_tile : rhs_localities.tiles_)
        {
            std::size_t rhs_span_index = 0;
            if (!rhs_tile.spans_[0].is_valid())
            {
                HPX_ASSERT(rhs_tile.spans_[1].is_valid());
                rhs_span_index = 1;
            }

            execution_tree::tiling_span const& rhs_span =
                rhs_tile.spans_[rhs_span_index];

            execution_tree::tiling_span intersection;
            if (!intersect(lhs_span, rhs_span, intersection))
            {
                ++loc;
                continue;
            }

            // project global coordinates onto local ones
            execution_tree::tiling_span lhs_intersection =
                lhs_localities.project_coords(
                    lhs_localities.locality_.locality_id_, 0, intersection);
            execution_tree::tiling_span rhs_intersection =
                rhs_localities.project_coords(
                    loc, rhs_span_index, intersection);

            if (rhs_localities.locality_.locality_id_ == loc)
            {
                // calculate the dot product with local tile
                dot_result +=
                    blaze::submatrix(lhs.matrix(), 0, lhs_intersection.start_,
                        lhs.dimension(0), lhs_intersection.size()) *
                    blaze::subvector(*rhs_data, rhs_intersection.start_,
                        rhs_intersection.size());
            }
            else
            {
                // calculate the dot product with remote tile
                dot_result +=
                    blaze::submatrix(lhs.matrix(), 0, lhs_intersection.start_,
                        lhs.dimension(0), lhs_intersection.size()) *
                    rhs_data.fetch(loc, rhs_intersection.start_,
                        rhs_intersection.stop_).get();
            }
            ++loc;
        }

        // collect overall result if left hand side vector is distributed
        execution_tree::primitive_argument_type result;
        if (lhs_localities.locality_.num_localities_ > 1)
        {
            if (lhs.dimension(1) == lhs_localities.columns())
            {
                // If the lhs number of columns is equal to the overall number
                // of columns we don't have to all_reduce the result. Instead,
                // the overall result is a tiled vector.
                result = execution_tree::primitive_argument_type{
                    std::move(dot_result)};

                // Generate new tiling annotation for the result vector
                execution_tree::tiling_information_1d tile_info(
                    execution_tree::tiling_information_1d::columns,
                    lhs_localities.get_span(1));

                ++lhs_localities.annotation_.generation_;

                auto locality_ann = lhs_localities.locality_.as_annotation();
                result.set_annotation(
                    execution_tree::localities_annotation(locality_ann,
                        tile_info.as_annotation(name_, codename_),
                        lhs_localities.annotation_, name_, codename_),
                    name_, codename_);
            }
            else
            {
                result = execution_tree::primitive_argument_type{hpx::all_reduce(
                    ("all_reduce_" + lhs_localities.annotation_.name_).c_str(),
                    dot_result, blaze::Add{},
                    lhs_localities.locality_.num_localities_, std::size_t(-1),
                    lhs_localities.locality_.locality_id_)
                          .get()};
            }
        }
        else
        {
            // the result is completely local, no need to all_reduce it
            result =
                execution_tree::primitive_argument_type{std::move(dot_result)};

            // if the rhs is distributed we should synchronize with all
            // connected localities however to avoid for the distributed vector
            // going out of scope
            if (rhs_localities.locality_.num_localities_ > 1)
            {
                hpx::lcos::barrier b(
                    "barrier_" + rhs_localities.annotation_.name_,
                    rhs_localities.locality_.num_localities_,
                    rhs_localities.locality_.locality_id_);
                b.wait();
            }
        }
        return result;
    }

    template <typename Matrix1, typename Matrix2>
    execution_tree::primitive_argument_type dist_dot_operation::dot2d2d(
        Matrix1&& lhs, Matrix2&& rhs) const
    {
        if (lhs.columns() != rhs.rows())
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "dist_dot_operation::dot2d2d",
                generate_error_message(
                    "the operands have incompatible number of dimensions"));
        }
        using T = blaze::ElementType_t<typename std::decay<Matrix1>::type>;
        blaze::DynamicMatrix<T> result = lhs * rhs;
        return execution_tree::primitive_argument_type{std::move(result)};
    }

#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
    template <typename T>
    execution_tree::primitive_argument_type dist_dot_operation::dot2d3d(
        ir::node_data<T>&& lhs, ir::node_data<T>&& rhs) const
    {
        if (lhs.dimension(1) != rhs.dimension(1))
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "dist_dot_operation::dot2d3d",
                generate_error_message(
                    "the operands have incompatible number of dimensions"));
        }

        auto m = lhs.matrix();
        auto t = rhs.tensor();

        blaze::DynamicTensor<T> result(m.rows(), t.pages(), t.columns());

        for (std::size_t i = 0; i != t.pages(); ++i)
            blaze::rowslice(
                blaze::subtensor(result, 0, i, 0, m.rows(), 1, t.columns()),
                0) = blaze::trans(m * blaze::pageslice(t, i));

        return execution_tree::primitive_argument_type{std::move(result)};
    }
#endif

    // lhs_num_dims == 2
    // Multiply a matrix with a vector
    // Regular matrix multiplication
    template <typename T>
    execution_tree::primitive_argument_type dist_dot_operation::dot2d(
        ir::node_data<T>&& lhs, ir::node_data<T>&& rhs,
        execution_tree::localities_information&& lhs_localities,
        execution_tree::localities_information const& rhs_localities) const
    {
        switch (rhs.num_dimensions())
        {
        case 0:
            // If is_matrix(lhs) && is_scalar(rhs)
            return common::dot2d0d(std::move(lhs), std::move(rhs));

        case 1:
            // If is_matrix(lhs) && is_vector(rhs)
            return dot2d1d(std::move(lhs), std::move(rhs),
                std::move(lhs_localities), rhs_localities);

        case 2:
            // If is_matrix(lhs) && is_matrix(rhs)
            return dot2d2d(lhs.matrix(), rhs.matrix());

#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
        case 3:
            // If is_matrix(lhs) && is_tensor(rhs)
            return dot2d3d(std::move(lhs), std::move(rhs));
#endif

        default:
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "dist_dot_operation::dot2d",
                generate_error_message(
                    "the operands have incompatible number of dimensions"));
        }
    }

    ///////////////////////////////////////////////////////////////////////////
#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
    template <typename T>
    execution_tree::primitive_argument_type dist_dot_operation::dot3d1d(
        ir::node_data<T>&& lhs, ir::node_data<T>&& rhs) const
    {
        if (lhs.dimension(2) != rhs.size())
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "dist_dot_operation::dot3d1d",
                generate_error_message(
                    "the operands have incompatible number of dimensions"));
        }
        auto t = lhs.tensor();
        blaze::DynamicMatrix<T> result(t.pages(), t.rows());

        for (std::size_t i = 0; i != t.pages(); ++i)
            blaze::row(blaze::submatrix(result, i, 0, 1, t.rows()), 0) =
                blaze::trans(blaze::pageslice(t, i) * rhs.vector());

        return execution_tree::primitive_argument_type{std::move(result)};
    }

    template <typename T>
    execution_tree::primitive_argument_type dist_dot_operation::dot3d2d(
        ir::node_data<T>&& lhs, ir::node_data<T>&& rhs) const
    {
        if (lhs.dimension(2) != rhs.dimension(0))
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "dist_dot_operation::dot3d2d",
                generate_error_message(
                    "the operands have incompatible number of dimensions"));
        }

        auto m = rhs.matrix();
        auto t = lhs.tensor();

        blaze::DynamicTensor<T> result(t.pages(), t.rows(), m.columns());

        for (std::size_t i = 0; i != t.pages(); ++i)
            blaze::pageslice(
                blaze::subtensor(result, i, 0, 0, 1, t.rows(), m.columns()),
                0) = blaze::pageslice(t, i) * m;

        return execution_tree::primitive_argument_type{std::move(result)};
    }

    template <typename T>
    execution_tree::primitive_argument_type dist_dot_operation::dot3d3d(
        ir::node_data<T>&& lhs, ir::node_data<T>&& rhs) const
    {
        if (lhs.dimension(2) != rhs.dimension(1))
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "dist_dot_operation::dot3d3d",
                generate_error_message(
                    "the operands have incompatible number of dimensions"));
        }
        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "dist_dot_operation::dot3d3d",
            generate_error_message(
                "it is not supported by Phylanx yet"));
    }

    // lhs_num_dims == 3
    template <typename T>
    execution_tree::primitive_argument_type dist_dot_operation::dot3d(
        ir::node_data<T>&& lhs, ir::node_data<T>&& rhs,
        execution_tree::localities_information const& lhs_localities,
        execution_tree::localities_information const& rhs_localities) const
    {
        switch (rhs.num_dimensions())
        {
        case 0:
            // If is_tensor(lhs) && is_scalar(rhs)
            return common::dot3d0d(std::move(lhs), std::move(rhs));

        case 1:
            // If is_tensor(lhs) && is_vector(rhs)
            return dot3d1d(std::move(lhs), std::move(rhs));

        case 2:
            // If is_tensor(lhs) && is_matrix(rhs)
            return dot3d2d(std::move(lhs), std::move(rhs));

        case 3:
            // If is_tensor(lhs) && is_tensor(rhs)
            return dot3d3d(std::move(lhs), std::move(rhs));

        default:
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "dist_dot_operation::dot3d",
                generate_error_message(
                    "the operands have incompatible number of dimensions"));
        }
    }
#endif
}}}

#endif
