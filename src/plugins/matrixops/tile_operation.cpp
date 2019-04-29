// Copyright (c) 2018 Bita Hasheminezhad
// Copyright (c) 2018 Parsa Amini
// Copyright (c) 2018 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/ir/node_data.hpp>
#include <phylanx/plugins/matrixops/tile_operation.hpp>
#include <phylanx/util/matrix_iterators.hpp>

#include <hpx/include/lcos.hpp>
#include <hpx/include/naming.hpp>
#include <hpx/include/util.hpp>
#include <hpx/throw_exception.hpp>
#include <hpx/util/iterator_facade.hpp>
#include <hpx/util/optional.hpp>

#include <cstdint>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include <blaze/Math.h>
#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
#include <blaze_tensor/Math.h>
#endif
///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace execution_tree { namespace primitives
{
    ///////////////////////////////////////////////////////////////////////////
    match_pattern_type const tile_operation::match_data = {
        hpx::util::make_tuple("tile", std::vector<std::string>{"tile(_1,_2)"},
            &create_tile_operation, &create_primitive<tile_operation>,
            R"(a, reps
            Args:

                a (array_like) : input array
                reps (integer or tuple of integers): Number of repetitions of
                a along each axis.

            Returns:

            Constructs an array by repeating a, the number of times given by
            reps.)")};

    ///////////////////////////////////////////////////////////////////////////
    tile_operation::tile_operation(primitive_arguments_type&& operands,
        std::string const& name, std::string const& codename)
        : primitive_component_base(std::move(operands), name, codename)
    {}

    ///////////////////////////////////////////////////////////////////////////
    bool tile_operation::validate_reps(ir::range const& arg) const
    {
        if (arg.size() == 1)
        {
            if (extract_scalar_integer_value_strict(*arg.begin()) < 0)
                return false;
            return true;
        }
        else if (arg.size() == 2)
        {
            auto it = arg.begin();
            if (extract_scalar_integer_value_strict(*it) < 0)
                return false;
            if (extract_scalar_integer_value_strict(*++it) < 0)
                return false;
            return true;
        }
        else if (arg.size() == 3)
        {
            auto it = arg.begin();
            if (extract_scalar_integer_value_strict(*it) < 0)
                return false;
            if (extract_scalar_integer_value_strict(*++it) < 0)
                return false;
            if (extract_scalar_integer_value_strict(*++it) < 0)
                return false;
            return true;
        }
        else
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "tile_operation::validate_reps",
                util::generate_error_message(
                    "Tiling to >3d is not supported", name_, codename_));
    }

    ///////////////////////////////////////////////////////////////////////////
    template <typename T>
    primitive_argument_type tile_operation::tile0d_1arg(
        ir::node_data<T>&& arr, ir::range&& arg) const
    {
        auto rep = extract_scalar_integer_value_strict(*arg.begin());

        blaze::DynamicVector<T> result(rep, arr.scalar());
        return primitive_argument_type{std::move(result)};
    }

    template <typename T>
    primitive_argument_type tile_operation::tile0d_2args(
        ir::node_data<T>&& arr, ir::range&& arg) const
    {
        auto it = arg.begin();
        auto row = extract_scalar_integer_value_strict(*it);
        auto column = extract_scalar_integer_value_strict(*++it);

        blaze::DynamicMatrix<T> result(row, column, arr.scalar());
        return primitive_argument_type{std::move(result)};
    }

#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
    template <typename T>
    primitive_argument_type tile_operation::tile0d_3args(
        ir::node_data<T>&& arr, ir::range&& arg) const
    {
        auto it = arg.begin();
        auto page = extract_scalar_integer_value_strict(*it);
        auto row = extract_scalar_integer_value_strict(*++it);
        auto column = extract_scalar_integer_value_strict(*++it);

        blaze::DynamicTensor<T> result(page, row, column, arr.scalar());
        return primitive_argument_type{std::move(result)};
    }
#endif

    template <typename T>
    primitive_argument_type tile_operation::tile0d(ir::node_data<T>&& arr,
        ir::range&& arg) const
    {
        switch (arg.size())
        {
        case 1:
            return tile0d_1arg(std::move(arr), std::move(arg));

        case 2:
            return tile0d_2args(std::move(arr), std::move(arg));

#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
        case 3:
            return tile0d_3args(std::move(arr), std::move(arg));
#endif

        default:
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "tile_operation::tile0d",
                util::generate_error_message(
                    "tiling to >3d is not supported",
                    name_, codename_));
        }
    }

    primitive_argument_type tile_operation::tile0d(
        primitive_argument_type&& arr, ir::range&& arg) const
    {
        switch (extract_common_type(arr))
        {
        case node_data_type_bool:
            return tile0d(
                extract_boolean_value_strict(std::move(arr), name_, codename_),
                std::move(arg));

        case node_data_type_int64:
            return tile0d(
                extract_integer_value_strict(std::move(arr), name_, codename_),
                std::move(arg));

        case node_data_type_double:
            return tile0d(
                extract_numeric_value_strict(std::move(arr), name_, codename_),
                std::move(arg));

        case node_data_type_unknown:
            return tile0d(
                extract_numeric_value(std::move(arr), name_, codename_),
                std::move(arg));

        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "phylanx::execution_tree::primitives::tile_operation::tile0d",
            generate_error_message(
                "the tile primitive requires for all arguments to "
                "be numeric data types"));
    }

    ///////////////////////////////////////////////////////////////////////////
    template <typename T>
    primitive_argument_type tile_operation::tile1d_1arg(
        ir::node_data<T>&& arr, ir::range&& arg) const
    {
        auto v = arr.vector();
        auto rep = extract_scalar_integer_value_strict(*arg.begin());

        blaze::DynamicVector<T> result(rep * v.size());
        for (auto i = 0; i < rep; ++i)
        {
            blaze::subvector(result, i * v.size(), v.size()) = v;
        }
        return primitive_argument_type{std::move(result)};
    }

    template <typename T>
    primitive_argument_type tile_operation::tile1d_2args(
        ir::node_data<T>&& arr, ir::range&& arg) const
    {
        auto v = arr.vector();
        auto it = arg.begin();
        auto row = extract_scalar_integer_value_strict(*it);
        auto column = extract_scalar_integer_value_strict(*++it);

        blaze::DynamicMatrix<T> result(row, column * v.size());
        for (auto r = 0; r < row; ++r)
            for (auto c = 0; c < column; ++c)
                blaze::row(
                    blaze::submatrix(result, r, c * v.size(), 1, v.size()), 0) =
                    blaze::trans(v);

        return primitive_argument_type{std::move(result)};
    }

#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
    template <typename T>
    primitive_argument_type tile_operation::tile1d_3args(
        ir::node_data<T>&& arr, ir::range&& arg) const
    {
        auto v = arr.vector();
        auto it = arg.begin();
        auto page = extract_scalar_integer_value_strict(*it);
        auto row = extract_scalar_integer_value_strict(*++it);
        auto column = extract_scalar_integer_value_strict(*++it);

        blaze::DynamicTensor<T> result(page, row, column * v.size());
        for (auto p = 0; p < page; ++p)
            for (auto r = 0; r < row; ++r)
                for (auto c = 0; c < column; ++c)
                    blaze::row(
                        blaze::pageslice(blaze::subtensor(result, p, r,
                                             c * v.size(), 1, 1, v.size()),
                            0),
                        0) = blaze::trans(v);

        return primitive_argument_type{std::move(result)};
    }
#endif

    template <typename T>
    primitive_argument_type tile_operation::tile1d(ir::node_data<T>&& arr,
        ir::range&& arg) const
    {
        switch (arg.size())
        {
        case 1:
            return tile1d_1arg(std::move(arr), std::move(arg));

        case 2:
            return tile1d_2args(std::move(arr), std::move(arg));

#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
        case 3:
            return tile1d_3args(std::move(arr), std::move(arg));
#endif

        default:
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "tile_operation::tile1d",
                util::generate_error_message(
                    "tiling to >3d is not supported",
                    name_, codename_));
        }
    }

    primitive_argument_type tile_operation::tile1d(
        primitive_argument_type&& arr, ir::range&& arg) const
    {
        switch (extract_common_type(arr))
        {
        case node_data_type_bool:
            return tile1d(
                extract_boolean_value_strict(std::move(arr), name_, codename_),
                std::move(arg));

        case node_data_type_int64:
            return tile1d(
                extract_integer_value_strict(std::move(arr), name_, codename_),
                std::move(arg));

        case node_data_type_double:
            return tile1d(
                extract_numeric_value_strict(std::move(arr), name_, codename_),
                std::move(arg));

        case node_data_type_unknown:
            return tile1d(
                extract_numeric_value(std::move(arr), name_, codename_),
                std::move(arg));

        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "phylanx::execution_tree::primitives::tile_operation::tile1d",
            generate_error_message(
                "the tile primitive requires for all arguments to "
                "be numeric data types"));
    }

    ///////////////////////////////////////////////////////////////////////////
    template <typename T>
    primitive_argument_type tile_operation::tile2d_1arg(
        ir::node_data<T>&& arr, ir::range&& arg) const
    {
        auto m = arr.matrix();
        auto rep = extract_scalar_integer_value_strict(*arg.begin());

        blaze::DynamicMatrix<T> result(m.rows(), rep * m.columns());
        for (auto i = 0; i < rep; ++i)
        {
            blaze::submatrix(
                result, 0, i * m.columns(), m.rows(), m.columns()) = m;
        }
        return primitive_argument_type{std::move(result)};
    }

    template <typename T>
    primitive_argument_type tile_operation::tile2d_2args(
        ir::node_data<T>&& arr, ir::range&& arg) const
    {
        auto m = arr.matrix();
        auto it = arg.begin();
        auto row = extract_scalar_integer_value_strict(*it);
        auto column = extract_scalar_integer_value_strict(*++it);

        blaze::DynamicMatrix<T> result(row * m.rows(), column * m.columns());
        for (auto r = 0; r < row; ++r)
            for (auto c = 0; c < column; ++c)
                blaze::submatrix(result, r * m.rows(), c * m.columns(),
                    m.rows(), m.columns()) = m;

        return primitive_argument_type{std::move(result)};
    }

#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
    template <typename T>
    primitive_argument_type tile_operation::tile2d_3args(
        ir::node_data<T>&& arr, ir::range&& arg) const
    {
        auto m = arr.matrix();
        auto it = arg.begin();
        auto page = extract_scalar_integer_value_strict(*it);
        auto row = extract_scalar_integer_value_strict(*++it);
        auto column = extract_scalar_integer_value_strict(*++it);

        blaze::DynamicTensor<T> result(
            page, row * m.rows(), column * m.columns());

        for (auto p = 0; p < page; ++p)
            for (auto r = 0; r < row; ++r)
                for (auto c = 0; c < column; ++c)
                    blaze::pageslice(
                        blaze::subtensor(result, p, r * m.rows(),
                            c * m.columns(), 1, m.rows(), m.columns()),
                        0) = m;

        return primitive_argument_type{std::move(result)};
    }
#endif

    template <typename T>
    primitive_argument_type tile_operation::tile2d(ir::node_data<T>&& arr,
        ir::range&& arg) const
    {
        switch (arg.size())
        {
        case 1:
            return tile2d_1arg(std::move(arr), std::move(arg));

        case 2:
            return tile2d_2args(std::move(arr), std::move(arg));

#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
        case 3:
            return tile2d_3args(std::move(arr), std::move(arg));
#endif

        default:
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "tile_operation::tile2d",
                util::generate_error_message(
                    "tiling to >3d is not supported",
                    name_, codename_));
        }
    }

    primitive_argument_type tile_operation::tile2d(
        primitive_argument_type&& arr, ir::range&& arg) const
    {
        switch (extract_common_type(arr))
        {
        case node_data_type_bool:
            return tile2d(
                extract_boolean_value_strict(std::move(arr), name_, codename_),
                std::move(arg));

        case node_data_type_int64:
            return tile2d(
                extract_integer_value_strict(std::move(arr), name_, codename_),
                std::move(arg));

        case node_data_type_double:
            return tile2d(
                extract_numeric_value_strict(std::move(arr), name_, codename_),
                std::move(arg));

        case node_data_type_unknown:
            return tile2d(
                extract_numeric_value(std::move(arr), name_, codename_),
                std::move(arg));

        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "phylanx::execution_tree::primitives::tile_operation::tile2d",
            generate_error_message(
                "the tile primitive requires for all arguments to "
                "be numeric data types"));
    }

    ///////////////////////////////////////////////////////////////////////////
#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
    template <typename T>
    primitive_argument_type tile_operation::tile3d_1arg(
        ir::node_data<T>&& arr, ir::range&& arg) const
    {
        auto t = arr.tensor();
        auto rep = extract_scalar_integer_value_strict(*arg.begin());

        blaze::DynamicTensor<T> result(t.pages(), t.rows(), rep * t.columns());
        for (auto i = 0; i < rep; ++i)
        {
            blaze::subtensor(result, 0, 0, i * t.columns(), t.pages(), t.rows(),
                t.columns()) = t;
        }
        return primitive_argument_type{std::move(result)};
    }

    template <typename T>
    primitive_argument_type tile_operation::tile3d_2args(
        ir::node_data<T>&& arr, ir::range&& arg) const
    {
        auto t = arr.tensor();
        auto it = arg.begin();
        auto row = extract_scalar_integer_value_strict(*it);
        auto column = extract_scalar_integer_value_strict(*++it);

        blaze::DynamicTensor<T> result(
            t.pages(), row * t.rows(), column * t.columns());
        for (auto r = 0; r < row; ++r)
            for (auto c = 0; c < column; ++c)
                blaze::subtensor(result, 0, r * t.rows(), c * t.columns(),
                    t.pages(), t.rows(), t.columns()) = t;

        return primitive_argument_type{std::move(result)};
    }

    template <typename T>
    primitive_argument_type tile_operation::tile3d_3args(
        ir::node_data<T>&& arr, ir::range&& arg) const
    {
        auto t = arr.tensor();
        auto it = arg.begin();
        auto page = extract_scalar_integer_value_strict(*it);
        auto row = extract_scalar_integer_value_strict(*++it);
        auto column = extract_scalar_integer_value_strict(*++it);

        blaze::DynamicTensor<T> result(
            page * t.pages(), row * t.rows(), column * t.columns());

        for (auto p = 0; p < page; ++p)
            for (auto r = 0; r < row; ++r)
                for (auto c = 0; c < column; ++c)
                    blaze::subtensor(result, p * t.pages(), r * t.rows(),
                        c * t.columns(), t.pages(), t.rows(), t.columns()) = t;

        return primitive_argument_type{std::move(result)};
    }

    template <typename T>
    primitive_argument_type tile_operation::tile3d(ir::node_data<T>&& arr,
        ir::range&& arg) const
    {
        switch (arg.size())
        {
        case 1:
            return tile3d_1arg(std::move(arr), std::move(arg));

        case 2:
            return tile3d_2args(std::move(arr), std::move(arg));

        case 3:
            return tile3d_3args(std::move(arr), std::move(arg));

        default:
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "tile_operation::tile3d",
                util::generate_error_message(
                    "tiling to >3d is not supported",
                    name_, codename_));
        }
    }

    primitive_argument_type tile_operation::tile3d(
        primitive_argument_type&& arr, ir::range&& arg) const
    {
        switch (extract_common_type(arr))
        {
        case node_data_type_bool:
            return tile3d(
                extract_boolean_value_strict(std::move(arr), name_, codename_),
                std::move(arg));

        case node_data_type_int64:
            return tile3d(
                extract_integer_value_strict(std::move(arr), name_, codename_),
                std::move(arg));

        case node_data_type_double:
            return tile3d(
                extract_numeric_value_strict(std::move(arr), name_, codename_),
                std::move(arg));

        case node_data_type_unknown:
            return tile3d(
                extract_numeric_value(std::move(arr), name_, codename_),
                std::move(arg));

        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "phylanx::execution_tree::primitives::tile_operation::tile3d",
            generate_error_message(
                "the tile primitive requires for all arguments to "
                "be numeric data types"));
    }
#endif

    ///////////////////////////////////////////////////////////////////////////
    hpx::future<primitive_argument_type> tile_operation::eval(
        primitive_arguments_type const& operands,
        primitive_arguments_type const& args, eval_context ctx) const
    {
        if (operands.size() != 2)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "tile_operation::eval",
                util::generate_error_message(
                    "the tile_operation primitive requires exactly two "
                    "operands",
                    name_, codename_));
        }

        for (auto const& i : operands)
        {
            if (!valid(i))
            {
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "tile_operation::eval",
                    util::generate_error_message(
                        "the tile_operation primitive requires that the "
                        "arguments given by the operands array are valid",
                        name_, codename_));
            }
        }

        auto&& op0 = value_operand(operands[0], args, name_, codename_, ctx);

        auto this_ = this->shared_from_this();
        return hpx::dataflow(hpx::launch::sync,
            [this_ = std::move(this_)](
                    hpx::future<primitive_argument_type>&& f1,
                    hpx::future<ir::range>&& f2)
            -> primitive_argument_type
            {
                auto&& arr = f1.get();
                auto&& arg = f2.get();

                auto arr_dims_num = extract_numeric_value_dimension(
                    arr, this_->name_, this_->codename_);

                if (this_->validate_reps(arg))
                {
                    switch (arr_dims_num)
                    {
                    case 0:
                        return this_->tile0d(std::move(arr), std::move(arg));

                    case 1:
                        return this_->tile1d(std::move(arr), std::move(arg));

                    case 2:
                        return this_->tile2d(std::move(arr), std::move(arg));

#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
                    case 3:
                        return this_->tile3d(std::move(arr), std::move(arg));
#endif

                    default:
                        HPX_THROW_EXCEPTION(hpx::bad_parameter,
                            "tile_operation::eval",
                            this_->generate_error_message(
                                "operand a has an invalid number of dimensions"));
                    }
                }
                else
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "tile_operation::eval",
                        this_->generate_error_message(
                            "negative dimensions are not allowed"));
                }
            },
            std::move(op0),
            list_operand(operands[1], args, name_, codename_, std::move(ctx)));
    }
}}}
