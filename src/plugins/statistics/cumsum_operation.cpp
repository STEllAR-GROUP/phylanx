//   Copyright (c) 2017-2018 Hartmut Kaiser
//   Copyright (c) 2019 Bibek Wagle
//
//   Distributed under the Boost Software License, Version 1.0. (See accompanying
//   file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/plugins/statistics/cumsum_operation.hpp>
#include <phylanx/plugins/statistics/statistics_base_impl.hpp>
#include <phylanx/util/blaze_traits.hpp>

#include <hpx/include/lcos.hpp>
#include <hpx/include/naming.hpp>
#include <hpx/include/parallel_scan.hpp>
#include <hpx/include/util.hpp>
#include <hpx/throw_exception.hpp>
#include <hpx/util/format.hpp>

#include <array>
#include <cstddef>
#include <cstdint>
#include <functional>
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
    namespace detail
    {
        template <typename T>
        struct statistics_cumsum_op
        {
            statistics_cumsum_op(std::string const& name,
                std::string const& codename)
            {}

            static constexpr T initial()
            {
                return T(0);
            }

            template <typename Scalar>
            typename std::enable_if<traits::is_scalar<Scalar>::value, T>::type
            operator()(Scalar s, T initial) const
            {
               return s;
            }

            // template <typename Vector>
            // typename std::enable_if<traits::is_vector<Vector>::value, Vector>::type
            // operator()(Vector const& v, T initial) const
            // {
            //     return v;
            // }

            // template <typename Matrix>
            // typename std::enable_if<traits::is_matrix<Matrix>::value, Matrix>::type
            // operator()(Matrix const& m, T initial) const
            // {
            //     return m;
            // }

            static T finalize(T value, std::size_t size)
            {
                return value;
            }
        };
    }

    ///////////////////////////////////////////////////////////////////////////
    match_pattern_type const cumsum_operation::match_data =
    {
        match_pattern_type{
            "cumsum",
            std::vector<std::string>{
                "cumsum(_1)", "cumsum(_1, _2)"
            },
            &create_cumsum_operation, &create_primitive<cumsum_operation>, R"(
            a, axis
            Args:

                a (array_like) : input array
                axis (int, optional) : Axis along which the cumulative sum is
                    computed. The default (None) is to compute the cumsum over
                    the flattened array.

            Returns:

            Return the cumulative sum of the elements along a given axis.)",
            true
        }
    };

    ///////////////////////////////////////////////////////////////////////////
    cumsum_operation::cumsum_operation(primitive_arguments_type&& operands,
            std::string const& name, std::string const& codename)
      : base_type(std::move(operands), name, codename)
    {
    }

    template <typename T>
    primitive_argument_type cumsum_operation::statistics0d(arg_type<T>&& arg,
        hpx::util::optional<std::int64_t> const& axis, bool keepdims,
        hpx::util::optional<T> const& initial) const
    {
        auto val = extract_numeric_value(this->base_type::statistics0d(std::move(arg), axis, keepdims, initial));
        blaze::DynamicVector<T> result(1, val.scalar());
        return primitive_argument_type{std::move(result)};
    }

    template <typename T>
    primitive_argument_type cumsum_operation::statistics1d(arg_type<T>&& arg,
        hpx::util::optional<std::int64_t> const& axis, bool keepdims,
        hpx::util::optional<T> const& initial) const
    {
        if (axis && axis.value() != 0 && axis.value() != -1)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "statistics::statistics1d",
                generate_error_message(
                    "the statistics_operation primitive requires operand axis "
                    "to be either 0 or -1 for vectors."));
        }

        auto v = arg.vector();
        blaze::DynamicVector<T> result(v.size());
        hpx::parallel::inclusive_scan(
            hpx::parallel::execution::par, v.begin(), v.end(), result.begin());
        return primitive_argument_type{std::move(result)};
    }

    template <typename T>
    primitive_argument_type cumsum_operation::statistics2d_flat(
        arg_type<T>&& arg, bool keepdims,
        hpx::util::optional<T> const& initial) const
    {
        auto m = arg.matrix();
        blaze::DynamicVector<T> result(m.rows() * m.columns());

        T init = T(0);
        auto last = result.begin();

        for (std::size_t row = 0; row != m.rows(); ++row)
        {
            last = hpx::parallel::inclusive_scan(hpx::parallel::execution::par,
                m.begin(row), m.end(row), last, std::plus<T>{}, init);

            init = *(last - 1);
        }

        return primitive_argument_type{std::move(result)};
    }

    template <typename T>
    primitive_argument_type cumsum_operation::statistics2d_axis0(
        arg_type<T>&& arg, bool keepdims,
        hpx::util::optional<T> const& initial) const
    {
        auto m = arg.matrix();
        blaze::DynamicMatrix<T> result(m.rows(), m.columns());

        for (std::size_t col = 0; col != m.columns(); ++col)
        {
            auto column = blaze::column(m, col);
            auto result_column = blaze::column(result, col);

            hpx::parallel::inclusive_scan(hpx::parallel::execution::par,
                column.begin(), column.end(), result_column.begin());
        }

        return primitive_argument_type{std::move(result)};
    }

    template <typename T>
    primitive_argument_type cumsum_operation::statistics2d_axis1(
        arg_type<T>&& arg, bool keepdims,
        hpx::util::optional<T> const& initial) const
    {
        auto m = arg.matrix();
        blaze::DynamicMatrix<T> result(m.rows(), m.columns());

        for (std::size_t row = 0; row != m.rows(); ++row)
        {
            hpx::parallel::inclusive_scan(hpx::parallel::execution::par,
                m.begin(row), m.end(row), result.begin(row));
        }

        return primitive_argument_type{std::move(result)};
    }
}}}
