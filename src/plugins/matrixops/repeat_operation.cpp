// Copyright (c) 2018 Bita Hasheminezhad
// Copyright (c) 2018 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/ir/node_data.hpp>
#include <phylanx/plugins/matrixops/repeat_operation.hpp>
#include <phylanx/util/matrix_iterators.hpp>

#include <hpx/include/lcos.hpp>
#include <hpx/include/naming.hpp>
#include <hpx/include/util.hpp>
#include <hpx/throw_exception.hpp>
#include <hpx/util/iterator_facade.hpp>
#include <hpx/util/optional.hpp>

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include <blaze/Math.h>
#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
#include <blaze_tensor/Math.h>
#include <phylanx/util/tensor_iterators.hpp>
#endif

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace execution_tree { namespace primitives
{
    ///////////////////////////////////////////////////////////////////////////
    match_pattern_type const repeat_operation::match_data = {
        hpx::util::make_tuple("repeat",
            std::vector<std::string>{"repeat(_1,_2)", "repeat(_1,_2,_3)"},
            &create_repeat_operation, &create_primitive<repeat_operation>,
            R"(a, repeats, axis
            Args:

                a (array) : a scalar, a vector, a
                   matrix or a tensor
                repeats (integer or a vector of integers) : The number of
                   repetitions for each element. repeats is broadcasted to
                   fit the shape of the given axis.
                axis (optional, integer): an axis to repeat along. By default,
                   flattened input is used.

            Returns:

            Repeated array which has the same shape as a, except along the
            given axis. In case of no axis for matrices flatten result is
            returned)")};

    ///////////////////////////////////////////////////////////////////////////
    repeat_operation::repeat_operation(primitive_arguments_type&& operands,
        std::string const& name, std::string const& codename)
        : primitive_component_base(std::move(operands), name, codename)
    {}

    bool repeat_operation::validate_repetition(
        ir::node_data<val_type>&& rep) const
    {
        if (rep.num_dimensions() == 0) {
            if (rep.scalar() < 0)
                return false;
        }
        else if (rep.num_dimensions() == 1)
        {
            auto v = rep.vector();
            for (auto it = v.begin(); it != v.end(); ++it)
                if (*it < 0)
                    return false;
        }
        return true;
    }

    template <typename T>
    primitive_argument_type repeat_operation::repeat0d0d(ir::node_data<T>&& arg,
        ir::node_data<val_type>&& rep) const
    {
        blaze::DynamicVector<T> result(rep.scalar(), arg.scalar());
        return primitive_argument_type{std::move(result)};
    }

    template <typename T>
    primitive_argument_type repeat_operation::repeat0d1d(ir::node_data<T>&& arg,
        ir::node_data<val_type>&& rep) const
    {
        auto v = rep.vector();
        if (v.size() == 1)
        {
            blaze::DynamicVector<T> result(v[0], arg.scalar());
            return primitive_argument_type{std::move(result)};
        }
        else
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "repeat_operation::repeat0d1d",
                generate_error_message("the repetition should be a scalar or a "
                                       "unit-size vector for scalar values."));
    }

    template <typename T>
    primitive_argument_type repeat_operation::repeat0d(ir::node_data<T>&& arg,
        ir::node_data<val_type>&& rep,
        hpx::util::optional<val_type> axis) const
    {
        if (axis && axis.value() != 0 && axis.value() != -1)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "repeat_operation::repeat0d",
                generate_error_message(
                    "the repeat_operation primitive requires operand axis to be "
                    "either 0 or -1 for scalar values."));
        }
        switch (rep.num_dimensions())
        {

        case 0:
            return repeat0d0d(std::move(arg), std::move(rep));

        case 1:
            return repeat0d1d(std::move(arg), std::move(rep));

        default:
            break;
        }
        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "repeat_operation::repeat0d",
            generate_error_message("the repetition should be a scalar or a "
                                   "unit-size vector for scalar values."));
    }

    ///////////////////////////////////////////////////////////////////////////
    template <typename T>
    primitive_argument_type repeat_operation::repeat1d0d(ir::node_data<T>&& arg,
        val_type&& rep) const
    {
        auto arr = arg.vector();

        blaze::DynamicVector<T> result(rep * arr.size());

        std::size_t c = 0;
        for (auto it = arr.begin(); it != arr.end(); ++it, ++c)
        {
            blaze::subvector(result, c * rep, rep) = *it;
        }

        return primitive_argument_type{std::move(result)};
    }

    template <typename T>
    primitive_argument_type repeat_operation::repeat1d1d(ir::node_data<T>&& arg,
        ir::node_data<val_type>&& rep) const
    {
        auto v = rep.vector();
        if (v.size() == 1)
        {
            return repeat1d0d(std::move(arg), std::move(v[0]));
        }
        else
        {
            auto arr = arg.vector();
            if (v.size() == arr.size())
            {
                blaze::DynamicVector<T> result(blaze::sum(v));

                std::size_t c = 0;       // counter on result's elements
                auto it2 = v.begin();    // iterator over repetition vector
                for (auto it1 = arr.begin(); it1 != arr.end(); ++it1, ++it2)
                {
                    blaze::subvector(result, c, *it2) = *it1;
                    c += *it2;
                }
                return primitive_argument_type{std::move(result)};
            }
        }
        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "repeat_operation::repeat1d1d",
            generate_error_message(
                "the repetition should be a unit-size "
                "vector or a vector of size a for vectors."));
    }

    template <typename T>
    primitive_argument_type repeat_operation::repeat1d(ir::node_data<T>&& arg,
        ir::node_data<val_type>&& rep,
        hpx::util::optional<val_type> axis) const
    {
        if (axis && axis.value() != 0 && axis.value() != -1)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "repeat_operation::repeat1d",
                generate_error_message(
                    "the repeat_operation primitive requires operand axis to be "
                    "either 0 or -1 for vectors."));
        }
        switch (rep.num_dimensions())
        {

        case 0:
            return repeat1d0d(std::move(arg), std::move(rep.scalar()));

        case 1:
            return repeat1d1d(std::move(arg), std::move(rep));

        default:
            break;
        }
        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "repeat_operation::repeat1d",
            generate_error_message(
                "the repetition should be a scalar or a vector."));
    }

    ///////////////////////////////////////////////////////////////////////////
    template <typename T>
    primitive_argument_type repeat_operation::repeat2d0d_axis0(
        ir::node_data<T>&& arg,
        val_type&& rep) const
    {
        auto m = arg.matrix();
        blaze::DynamicMatrix<T> result(m.rows() * rep, m.columns());

        for (std::size_t i = 0; i != result.rows(); ++i)
        {
            blaze::row(result, i) =
                blaze::row(m, static_cast<std::int64_t>(i / rep));
        }

        return primitive_argument_type{std::move(result)};
    }

    template <typename T>
    primitive_argument_type repeat_operation::repeat2d1d_axis0(
        ir::node_data<T>&& arg,
        ir::node_data<val_type>&& rep) const
    {
        auto v = rep.vector();
        if (v.size() == 1)
        {
            return repeat2d0d_axis0(std::move(arg), std::move(v[0]));
        }
        else
        {
            auto m = arg.matrix();
            if (v.size() == m.rows())
            {
                blaze::DynamicMatrix<T> result(blaze::sum(v), m.columns());

                auto it = v.begin();    // iterator on repetition vector
                for (auto i = 0, c1 = 0, c2 = 0; i != result.rows(); i++, c2++)
                {
                    if (*it == c2) {
                        ++it; ++c1;
                        c2 = 0;
                    }
                    if (*it == 0) {
                        ++it; ++c1;
                        c2 = 0;
                    }
                    blaze::row(result, i) = blaze::row(m, c1);
                }
                return primitive_argument_type{std::move(result)};
            }
        }
        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "repeat_operation::repeat2d1d_axis0",
            generate_error_message(
                "for matrices, the repetition along axis 0 should be a scalar, "
                "a unit-size vector or a vector with the size of a's number of "
                "rows."));
    }

    template <typename T>
    primitive_argument_type repeat_operation::repeat2d_axis0(
        ir::node_data<T>&& arg,
        ir::node_data<val_type>&& rep) const
    {
        switch (rep.num_dimensions())
        {

        case 0:
            return repeat2d0d_axis0(std::move(arg), std::move(rep.scalar()));

        case 1:
            return repeat2d1d_axis0(std::move(arg), std::move(rep));

        default:
            break;
        }
        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "repeat_operation::repeat2d_axis0",
            generate_error_message(
                "the repetition should be a scalar or a vector for matrices."));
    }

    template <typename T>
    primitive_argument_type repeat_operation::repeat2d0d_axis1(
        ir::node_data<T>&& arg,
        val_type&& rep) const
    {
        auto m = arg.matrix();
        blaze::DynamicMatrix<T> result(m.rows(), m.columns() * rep);

        for (std::size_t i = 0; i != result.columns(); ++i)
        {
            blaze::column(result, i) =
                blaze::column(m, static_cast<std::int64_t>(i / rep));
        }

        return primitive_argument_type{std::move(result)};
    }

    template <typename T>
    primitive_argument_type repeat_operation::repeat2d1d_axis1(
        ir::node_data<T>&& arg,
        ir::node_data<val_type>&& rep) const
    {
        auto v = rep.vector();
        if (v.size() == 1)
        {
            return repeat2d0d_axis1(std::move(arg), std::move(v[0]));
        }
        else
        {
            auto m = arg.matrix();
            if (v.size() == m.columns())
            {
                blaze::DynamicMatrix<T> result(m.rows(), blaze::sum(v));

                auto it = v.begin();    // iterator on repetition vector
                for (auto i = 0, c1 = 0, c2 = 0; i != result.columns();
                     i++, c2++)
                {
                    if (*it == c2) {
                        ++it; ++c1;
                        c2 = 0;
                    }
                    if (*it == 0) {
                        ++it; ++c1;
                        c2 = 0;
                    }
                    blaze::column(result, i) = blaze::column(m, c1);
                }
                return primitive_argument_type{std::move(result)};
            }
        }
        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "repeat_operation::repeat2d1d_axis1",
            generate_error_message(
                "for matrices, the repetition along axis 1 should be a scalar, "
                "a unit-size vector or a vector with the size of a's number of "
                "columns."));
    }

    template <typename T>
    primitive_argument_type repeat_operation::repeat2d_axis1(
        ir::node_data<T>&& arg,
        ir::node_data<val_type>&& rep) const
    {
        switch (rep.num_dimensions())
        {

        case 0:
            return repeat2d0d_axis1(std::move(arg), std::move(rep.scalar()));

        case 1:
            return repeat2d1d_axis1(std::move(arg), std::move(rep));

        default:
            break;
        }
        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "repeat_operation::repeat2d_axis1",
            generate_error_message(
                "the repetition should be a scalar or a vector for matrices."));
    }

    template <typename T>
    primitive_argument_type repeat_operation::repeat2d0d_flatten(
        ir::node_data<T>&& arg,
        val_type&& rep) const
    {
        auto m = arg.matrix();
        blaze::DynamicVector<T> result(m.rows() * m.columns() * rep);
        using phylanx::util::matrix_iterator;
        matrix_iterator<decltype(m)> begin(m, 0);
        matrix_iterator<decltype(m)> end(m, m.rows());

        std::size_t c = 0;
        for (auto it = begin; it != end; ++it, ++c)
        {
            blaze::subvector(result, c * rep, rep) = *it;
        }

        return primitive_argument_type{std::move(result)};
    }

    template <typename T>
    primitive_argument_type repeat_operation::repeat2d1d_flatten(
        ir::node_data<T>&& arg,
        ir::node_data<val_type>&& rep) const
    {
        auto v = rep.vector();
        if (v.size() == 1)
        {
            return repeat2d0d_flatten(std::move(arg), std::move(v[0]));
        }
        else
        {
            auto m = arg.matrix();
            if (v.size() == m.rows() * m.columns())
            {
                blaze::DynamicVector<T> result(blaze::sum(v));
                using phylanx::util::matrix_iterator;
                matrix_iterator<decltype(m)> begin(m, 0);
                matrix_iterator<decltype(m)> end(m, m.rows());

                std::size_t c = 0;       // counter on result's elements
                auto it2 = v.begin();    // iterator over repetition vector
                for (auto it1 = begin; it1 != end; ++it1, ++it2)
                {
                    blaze::subvector(result, c, *it2) = *it1;
                    c += *it2;
                }
                return primitive_argument_type{std::move(result)};
            }
        }
        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "repeat_operation::repeat2d1d_flatten",
            generate_error_message("the repetition should be a unit-size "
                                   "vector or a vector which size is the "
                                   "number of a's elements."));
    }

    template <typename T>
    primitive_argument_type repeat_operation::repeat2d(ir::node_data<T>&& arg,
        ir::node_data<val_type>&& rep,
        hpx::util::optional<val_type> axis) const
    {
        if (axis)
        {
            switch (axis.value())
            {
            case -2:
                HPX_FALLTHROUGH;
            case 0:
                return repeat2d_axis0(std::move(arg), std::move(rep));

            case -1:
                HPX_FALLTHROUGH;
            case 1:
                return repeat2d_axis1(std::move(arg), std::move(rep));

            default:
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "repeat_operation::repeat2d",
                    generate_error_message(
                        "the repeat_operation primitive requires operand axis "
                        "to be between -2 and 1 for matrix values."));
            }
        }
        else
        {
            switch (rep.num_dimensions())
            {
            case 0:
                return repeat2d0d_flatten(
                    std::move(arg), std::move(rep.scalar()));

            case 1:
                return repeat2d1d_flatten(std::move(arg), std::move(rep));

            default:
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "repeat_operation::repeat2d",
                    generate_error_message("the repetition should be a scalar "
                                           "or a vector for matrix values"));
            }
        }
    }

    ///////////////////////////////////////////////////////////////////////////
#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
    template <typename T>
    primitive_argument_type repeat_operation::repeat3d0d_axis0(
        ir::node_data<T>&& arg,
        val_type&& rep) const
    {
        auto t = arg.tensor();
        blaze::DynamicTensor<T> result(t.pages() * rep, t.rows(), t.columns());

        for (std::size_t i = 0; i != result.pages(); ++i)
        {
            blaze::pageslice(result, i) =
                blaze::pageslice(t, static_cast<std::int64_t>(i / rep));
        }

        return primitive_argument_type{std::move(result)};
    }

    template <typename T>
    primitive_argument_type repeat_operation::repeat3d1d_axis0(
        ir::node_data<T>&& arg,
        ir::node_data<val_type>&& rep) const
    {
        auto v = rep.vector();
        if (v.size() == 1)
        {
            return repeat3d0d_axis0(std::move(arg), std::move(v[0]));
        }
        else
        {
            auto t = arg.tensor();
            if (v.size() == t.pages())
            {
                blaze::DynamicTensor<T> result(
                    blaze::sum(v), t.rows(), t.columns());

                auto it = v.begin();    // iterator on repetition vector
                for (auto i = 0, c1 = 0, c2 = 0; i != result.pages(); i++, c2++)
                {
                    if (*it == c2) {
                        ++it; ++c1;
                        c2 = 0;
                    }
                    if (*it == 0) {
                        ++it; ++c1;
                        c2 = 0;
                    }
                    blaze::pageslice(result, i) = blaze::pageslice(t, c1);
                }
                return primitive_argument_type{std::move(result)};
            }
        }
        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "repeat_operation::repeat3d1d_axis0",
            generate_error_message(
                "for tensors, the repetition along axis 0 should be a scalar, "
                "a unit-size vector or a vector with the size of a's number of "
                "pages."));
    }

    template <typename T>
    primitive_argument_type repeat_operation::repeat3d_axis0(
        ir::node_data<T>&& arg,
        ir::node_data<val_type>&& rep) const
    {
        switch (rep.num_dimensions())
        {

        case 0:
            return repeat3d0d_axis0(std::move(arg), std::move(rep.scalar()));

        case 1:
            return repeat3d1d_axis0(std::move(arg), std::move(rep));

        default:
            break;
        }
        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "repeat_operation::repeat3d_axis0",
            generate_error_message(
                "the repetition should be a scalar or a vector for tensors."));
    }

    template <typename T>
    primitive_argument_type repeat_operation::repeat3d0d_axis1(
        ir::node_data<T>&& arg,
        val_type&& rep) const
    {
        auto t = arg.tensor();
        blaze::DynamicTensor<T> result(t.pages(), t.rows() * rep, t.columns());

        for (std::size_t i = 0; i != result.rows(); ++i)
        {
            blaze::rowslice(result, i) =
                blaze::rowslice(t, static_cast<std::int64_t>(i / rep));
        }

        return primitive_argument_type{std::move(result)};
    }

    template <typename T>
    primitive_argument_type repeat_operation::repeat3d1d_axis1(
        ir::node_data<T>&& arg,
        ir::node_data<val_type>&& rep) const
    {
        auto v = rep.vector();
        if (v.size() == 1)
        {
            return repeat3d0d_axis1(std::move(arg), std::move(v[0]));
        }
        else
        {
            auto t = arg.tensor();
            if (v.size() == t.rows())
            {
                blaze::DynamicTensor<T> result(
                    t.pages(), blaze::sum(v), t.columns());

                auto it = v.begin();    // iterator on repetition vector
                for (auto i = 0, c1 = 0, c2 = 0; i != result.rows();
                     i++, c2++)
                {
                    if (*it == c2) {
                        ++it; ++c1;
                        c2 = 0;
                    }
                    if (*it == 0) {
                        ++it; ++c1;
                        c2 = 0;
                    }
                    blaze::rowslice(result, i) = blaze::rowslice(t, c1);
                }
                return primitive_argument_type{std::move(result)};
            }
        }
        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "repeat_operation::repeat3d1d_axis1",
            generate_error_message(
                "for tensors, the repetition along axis 1 should be a scalar, "
                "a unit-size vector or a vector with the size of a's number of "
                "rows."));
    }

    template <typename T>
    primitive_argument_type repeat_operation::repeat3d_axis1(
        ir::node_data<T>&& arg,
        ir::node_data<val_type>&& rep) const
    {
        switch (rep.num_dimensions())
        {

        case 0:
            return repeat3d0d_axis1(std::move(arg), std::move(rep.scalar()));

        case 1:
            return repeat3d1d_axis1(std::move(arg), std::move(rep));

        default:
            break;
        }
        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "repeat_operation::repeat3d_axis1",
            generate_error_message(
                "the repetition should be a scalar or a vector for tensors."));
    }

    template <typename T>
    primitive_argument_type repeat_operation::repeat3d0d_axis2(
        ir::node_data<T>&& arg,
        val_type&& rep) const
    {
        auto t = arg.tensor();
        blaze::DynamicTensor<T> result(t.pages(), t.rows(), t.columns() * rep);

        for (std::size_t i = 0; i != result.columns(); ++i)
        {
            blaze::columnslice(result, i) =
                blaze::columnslice(t, static_cast<std::int64_t>(i / rep));
        }

        return primitive_argument_type{std::move(result)};
    }

    template <typename T>
    primitive_argument_type repeat_operation::repeat3d1d_axis2(
        ir::node_data<T>&& arg,
        ir::node_data<val_type>&& rep) const
    {
        auto v = rep.vector();
        if (v.size() == 1)
        {
            return repeat3d0d_axis2(std::move(arg), std::move(v[0]));
        }
        else
        {
            auto t = arg.tensor();
            if (v.size() == t.columns())
            {
                blaze::DynamicTensor<T> result(
                    t.pages(), t.rows(), blaze::sum(v));

                auto it = v.begin();    // iterator on repetition vector
                for (auto i = 0, c1 = 0, c2 = 0; i != result.columns();
                     i++, c2++)
                {
                    if (*it == c2) {
                        ++it; ++c1;
                        c2 = 0;
                    }
                    if (*it == 0) {
                        ++it; ++c1;
                        c2 = 0;
                    }
                    blaze::columnslice(result, i) = blaze::columnslice(t, c1);
                }
                return primitive_argument_type{std::move(result)};
            }
        }
        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "repeat_operation::repeat3d1d_axis2",
            generate_error_message(
                "for tensors, the repetition along axis 1 should be a scalar, "
                "a unit-size vector or a vector with the size of a's number of "
                "columns."));
    }

    template <typename T>
    primitive_argument_type repeat_operation::repeat3d_axis2(
        ir::node_data<T>&& arg,
        ir::node_data<val_type>&& rep) const
    {
        switch (rep.num_dimensions())
        {

        case 0:
            return repeat3d0d_axis2(std::move(arg), std::move(rep.scalar()));

        case 1:
            return repeat3d1d_axis2(std::move(arg), std::move(rep));

        default:
            break;
        }
        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "repeat_operation::repeat3d_axis2",
            generate_error_message(
                "the repetition should be a scalar or a vector for tensors."));
    }

    template <typename T>
    primitive_argument_type repeat_operation::repeat3d0d_flatten(
        ir::node_data<T>&& arg,
        val_type&& rep) const
    {
        auto t = arg.tensor();
        blaze::DynamicVector<T> result(
            t.pages() * t.rows() * t.columns() * rep);
        using phylanx::util::tensor_iterator;
        tensor_iterator<decltype(t)> begin(t, 0);
        tensor_iterator<decltype(t)> end(t, t.pages());

        std::size_t c = 0;
        for (auto it = begin; it != end; ++it, ++c)
        {
            blaze::subvector(result, c * rep, rep) = *it;
        }

        return primitive_argument_type{std::move(result)};
    }

    template <typename T>
    primitive_argument_type repeat_operation::repeat3d1d_flatten(
        ir::node_data<T>&& arg,
        ir::node_data<val_type>&& rep) const
    {
        auto v = rep.vector();
        if (v.size() == 1)
        {
            return repeat3d0d_flatten(std::move(arg), std::move(v[0]));
        }
        else
        {
            auto t = arg.tensor();
            if (v.size() ==t.pages()* t.rows() * t.columns())
            {
                blaze::DynamicVector<T> result(blaze::sum(v));
                using phylanx::util::tensor_iterator;
                tensor_iterator<decltype(t)> begin(t, 0);
                tensor_iterator<decltype(t)> end(t, t.pages());

                std::size_t c = 0;       // counter on result's elements
                auto it2 = v.begin();    // iterator over repetition vector
                for (auto it1 = begin; it1 != end; ++it1, ++it2)
                {
                    blaze::subvector(result, c, *it2) = *it1;
                    c += *it2;
                }
                return primitive_argument_type{std::move(result)};
            }
        }
        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "repeat_operation::repeat3d1d_flatten",
            generate_error_message("the repetition should be a unit-size "
                                   "vector or a vector which size is the "
                                   "number of a's elements."));
    }

    template <typename T>
    primitive_argument_type repeat_operation::repeat3d(ir::node_data<T>&& arg,
        ir::node_data<val_type>&& rep,
        hpx::util::optional<val_type> axis) const
    {
        if (axis)
        {
            switch (axis.value())
            {
            case -3:
                HPX_FALLTHROUGH;
            case 0:
                return repeat3d_axis0(std::move(arg), std::move(rep));

            case -2:
                HPX_FALLTHROUGH;
            case 1:
                return repeat3d_axis1(std::move(arg), std::move(rep));

            case -1:
                HPX_FALLTHROUGH;
            case 2:
                return repeat3d_axis2(std::move(arg), std::move(rep));

            default:
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "repeat_operation::repeat3d",
                    generate_error_message(
                        "the repeat_operation primitive requires operand axis "
                        "to be between -3 and 2 for tensor values."));
            }
        }
        else
        {
            switch (rep.num_dimensions())
            {
            case 0:
                return repeat3d0d_flatten(
                    std::move(arg), std::move(rep.scalar()));

            case 1:
                return repeat3d1d_flatten(std::move(arg), std::move(rep));

            default:
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "repeat_operation::repeat3d",
                    generate_error_message("the repetition should be a scalar "
                                           "or a vector for tensor values"));
            }
        }
    }
#endif

    ///////////////////////////////////////////////////////////////////////////
    template <typename T>
    primitive_argument_type repeat_operation::repeatnd(ir::node_data<T>&& arg,
        ir::node_data<val_type>&& rep,
        hpx::util::optional<val_type> axis) const
    {
        std::size_t a_dims = arg.num_dimensions();
        switch (a_dims)
        {
        case 0:
            return repeat0d(std::move(arg), std::move(rep), axis);

        case 1:
            return repeat1d(std::move(arg), std::move(rep), axis);

        case 2:
            return repeat2d(std::move(arg), std::move(rep), axis);

#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
        case 3:
            return repeat3d(std::move(arg), std::move(rep), axis);
#endif

        default:
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "repeat_operation::repeatnd",
                generate_error_message(
                    "operand a has an invalid number of dimensions"));
        }
    }

    ///////////////////////////////////////////////////////////////////////////
    hpx::future<primitive_argument_type> repeat_operation::eval(
        primitive_arguments_type const& operands,
        primitive_arguments_type const& args, eval_context ctx) const
    {
        if (operands.size() != 2 && operands.size() != 3)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "repeat_operation::eval",
                generate_error_message("the repeat_operation primitive requires "
                                       "exactly two, or three operands"));
        }

        for (auto const& i : operands)
        {
            if (!valid(i))
            {
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "repeat_operation::eval",
                    generate_error_message(
                        "the repeat_operation primitive requires that the "
                        "arguments given by the operands array are valid"));
            }
        }

        auto this_ = this->shared_from_this();
        return hpx::dataflow(hpx::launch::sync, hpx::util::unwrapping(
            [this_ = std::move(this_)](primitive_arguments_type&& args)
            -> primitive_argument_type
            {
                hpx::util::optional<std::int64_t> axis;

                // axis is argument #3
                if (args.size() == 3)
                {
                    axis = extract_scalar_integer_value_strict(
                        args[2], this_->name_, this_->codename_);
                }

                if (this_->validate_repetition(
                        extract_integer_value_strict(args[1])))
                {
                    switch (extract_common_type(args[0]))
                    {
                    case node_data_type_bool:
                        return this_->repeatnd(
                            extract_boolean_value(std::move(args[0]),
                                this_->name_, this_->codename_),
                            extract_integer_value_strict(std::move(args[1])),
                            axis);

                    case node_data_type_int64:
                        return this_->repeatnd(
                            extract_integer_value(std::move(args[0]),
                                this_->name_, this_->codename_),
                            extract_integer_value_strict(std::move(args[1])),
                            axis);

                    case node_data_type_double:
                        return this_->repeatnd(
                            extract_numeric_value(std::move(args[0]),
                                this_->name_, this_->codename_),
                            extract_integer_value_strict(std::move(args[1])),
                            axis);

                    default:
                        HPX_THROW_EXCEPTION(hpx::bad_parameter,
                            "repeat::eval",
                            this_->generate_error_message(
                                "the repeat primitive requires for all arguments "
                                "to be numeric data types"));
                    }
                }

                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "repeat_operation::eval",
                    this_->generate_error_message(
                        "the repetition cannot be/contain a negative number."));
            }),
            detail::map_operands(operands, functional::value_operand{},
                args, name_, codename_, std::move(ctx)));
    }
}}}
