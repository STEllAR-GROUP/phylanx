//  Copyright (c) 2017-2020 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_COMMON_STATISTICS_OPERATIONS_2020_MAY_21_0352PM)
#define PHYLANX_COMMON_STATISTICS_OPERATIONS_2020_MAY_21_0352PM

#include <phylanx/config.hpp>
#include <phylanx/plugins/common/export_definitions.hpp>
#include <phylanx/util/blaze_traits.hpp>
#include <phylanx/util/detail/numeric_limits_min.hpp>

#include <hpx/assert.hpp>

#include <algorithm>
#include <limits>
#include <string>
#include <type_traits>
#include <utility>

#include <blaze/Math.h>
#include <blaze_tensor/Math.h>

///////////////////////////////////////////////////////////////////////////////
// explicitly instantiate the required functions
namespace phylanx { namespace common {

    ///////////////////////////////////////////////////////////////////////////
    template <typename T>
    struct statistics_all_op
    {
        using result_type = std::uint8_t;

        statistics_all_op(std::string const& name, std::string const& codename)
        {
        }

        static constexpr std::uint8_t initial()
        {
            return 1;
        }

        template <typename Scalar>
        typename std::enable_if<traits::is_scalar<Scalar>::value,
            std::uint8_t>::type
        operator()(Scalar s, std::uint8_t initial) const
        {
            return (s && initial) ? 1 : 0;
        }

        template <typename Vector>
        typename std::enable_if<!traits::is_scalar<Vector>::value,
            std::uint8_t>::type
        operator()(Vector& v, std::uint8_t initial) const
        {
            return initial &&
                std::all_of(v.begin(), v.end(),
                    [](T val) -> std::uint8_t { return (val != 0) ? 1 : 0; });
        }

        static constexpr std::uint8_t finalize(
            std::uint8_t value, std::size_t size)
        {
            return value ? 1 : 0;
        }
    };

    ///////////////////////////////////////////////////////////////////////////
    template <typename T>
    struct statistics_any_op
    {
        using result_type = std::uint8_t;

        statistics_any_op(std::string const& name, std::string const& codename)
        {
        }

        static constexpr bool initial()
        {
            return false;
        }

        template <typename Scalar>
        typename std::enable_if<traits::is_scalar<Scalar>::value, bool>::type
        operator()(Scalar s, bool initial) const
        {
            return s || initial;
        }

        template <typename Vector>
        typename std::enable_if<!traits::is_scalar<Vector>::value, T>::type
        operator()(Vector& v, bool initial) const
        {
            return initial ||
                std::any_of(
                    v.begin(), v.end(), [](T val) -> bool { return val != 0; });
        }

        static constexpr bool finalize(bool value, std::size_t size)
        {
            return value;
        }
    };

    ///////////////////////////////////////////////////////////////////////////
    template <typename T>
    struct statistics_min_op
    {
        using result_type = T;

        statistics_min_op(std::string const& name, std::string const& codename)
        {
        }

        static constexpr T initial()
        {
            return (std::numeric_limits<T>::max)();
        }

        template <typename Scalar>
        typename std::enable_if<traits::is_scalar<Scalar>::value, T>::type
        operator()(Scalar s, T initial) const
        {
            return (std::min)(s, initial);
        }

        template <typename Vector>
        typename std::enable_if<!traits::is_scalar<Vector>::value, T>::type
        operator()(Vector& v, T initial) const
        {
            return (std::min)((blaze::min)(v), initial);
        }

        static T finalize(T value, std::size_t size)
        {
            return value;
        }
    };

    ///////////////////////////////////////////////////////////////////////////
    template <typename T>
    struct statistics_max_op
    {
        using result_type = T;

        statistics_max_op(std::string const& name, std::string const& codename)
        {
        }

        static constexpr T initial()
        {
            return phylanx::util::detail::numeric_limits_min<T>();
        }

        template <typename Scalar>
        typename std::enable_if<traits::is_scalar<Scalar>::value, T>::type
        operator()(Scalar s, T initial) const
        {
            return (std::max)(s, initial);
        }

        template <typename Vector>
        typename std::enable_if<!traits::is_scalar<Vector>::value, T>::type
        operator()(Vector& v, T initial) const
        {
            return (std::max)((blaze::max)(v), initial);
        }

        static T finalize(T value, std::size_t size)
        {
            return value;
        }
    };

    ///////////////////////////////////////////////////////////////////////////
    template <typename T>
    struct statistics_sum_op
    {
        using result_type = T;

        statistics_sum_op(std::string const& name, std::string const& codename)
        {
        }

        static constexpr T initial()
        {
            return T(0);
        }

        template <typename Scalar>
        typename std::enable_if<traits::is_scalar<Scalar>::value, T>::type
        operator()(Scalar s, T initial) const
        {
            return s + initial;
        }

        template <typename Vector>
        typename std::enable_if<!traits::is_scalar<Vector>::value, T>::type
        operator()(Vector& v, T initial) const
        {
            return blaze::sum(v) + initial;
        }

        static T finalize(T value, std::size_t size)
        {
            return value;
        }
    };

    ///////////////////////////////////////////////////////////////////////////
    template <typename T>
    struct statistics_logsumexp_op
    {
        using result_type = double;

        statistics_logsumexp_op(
            std::string const& name, std::string const& codename)
        {
        }

        static constexpr double initial()
        {
            return 0.0;
        }

        template <typename Scalar>
        typename std::enable_if<traits::is_scalar<Scalar>::value, double>::type
        operator()(Scalar s, double initial) const
        {
            return s;
        }

        template <typename Vector>
        typename std::enable_if<!traits::is_scalar<Vector>::value, double>::type
        operator()(Vector& v, double initial) const
        {
            return blaze::sum(blaze::exp(v)) + initial;
        }

        static double finalize(double value, std::size_t size)
        {
            return blaze::log(value);
        }
    };

    ///////////////////////////////////////////////////////////////////////////
    template <typename T>
    struct statistics_prod_op
    {
        using result_type = T;

        statistics_prod_op(std::string const& name, std::string const& codename)
        {
        }

        static constexpr T initial()
        {
            return T(1);
        }

        template <typename Scalar>
        typename std::enable_if<traits::is_scalar<Scalar>::value, T>::type
        operator()(Scalar s, T initial) const
        {
            return s * initial;
        }

        template <typename Vector>
        typename std::enable_if<!traits::is_scalar<Vector>::value, T>::type
        operator()(Vector& v, T initial) const
        {
            return blaze::prod(v) * initial;
        }

        static T finalize(T value, std::size_t size)
        {
            return value;
        }
    };

    ///////////////////////////////////////////////////////////////////////////
    template <typename T>
    struct statistics_mean_op
    {
        using result_type = double;

        statistics_mean_op(std::string const& name, std::string const& codename)
          : name_(name)
          , codename_(codename)
        {
        }

        static constexpr double initial()
        {
            return 0.0;
        }

        template <typename Scalar>
        typename std::enable_if<traits::is_scalar<Scalar>::value, T>::type
        operator()(Scalar s, double initial) const
        {
            return s + initial;
        }

        template <typename Vector>
        typename std::enable_if<!traits::is_scalar<Vector>::value, T>::type
        operator()(Vector& v, double initial) const
        {
            return blaze::sum(v) + initial;
        }

        double finalize(double value, std::size_t size) const
        {
            if (size == 0)
            {
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "statistics_mean_op::finalize",
                    util::generate_error_message(
                        "empty sequences are not supported", name_, codename_));
            }

            return value / size;
        }

        std::string const& name_;
        std::string const& codename_;
    };

    ///////////////////////////////////////////////////////////////////////////
    template <typename T>
    struct statistics_stddev_op
    {
        using result_type = double;

        statistics_stddev_op(
            std::string const& name, std::string const& codename)
          : name_(name)
          , codename_(codename)
          , count_(0)
          , mean_(0)
          , m2_(0)
        {
        }

        static constexpr double initial()
        {
            return 0.0;
        }

        // Use Welford's online algorithm, see
        // https://en.wikipedia.org/wiki/Algorithms_for_calculating_variance
        void process_value(double val)
        {
            ++count_;
            double delta = val - mean_;
            mean_ += delta / count_;
            double delta2 = val - mean_;
            m2_ += delta * delta2;
        }

        template <typename Scalar>
        typename std::enable_if<traits::is_scalar<Scalar>::value, double>::type
        operator()(Scalar s, double initial)
        {
            process_value(s);
            return initial;
        }

        template <typename Vector>
        typename std::enable_if<!traits::is_scalar<Vector>::value, double>::type
        operator()(Vector& v, double initial)
        {
            for (auto&& elem : v)
            {
                process_value(elem);
            }
            return initial;
        }

        double finalize(double value, std::size_t size) const
        {
            HPX_ASSERT(count_ == size);
            if (size == 0)
            {
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "statistics_std_op::finalize",
                    util::generate_error_message(
                        "empty sequences are not supported", name_, codename_));
            }
            if (size == 1)
            {
                return 0.0;
            }

            return std::sqrt(m2_ / size);
        }

        std::string const& name_;
        std::string const& codename_;

        std::size_t count_;
        double mean_;
        double m2_;
    };

    ///////////////////////////////////////////////////////////////////////////
    template <typename T>
    struct statistics_var_op
    {
        using result_type = double;

        statistics_var_op(std::string const& name, std::string const& codename)
          : name_(name)
          , codename_(codename)
          , count_(0)
          , mean_(0)
          , m2_(0)
        {
        }

        static constexpr double initial()
        {
            return 0.0;
        }

        // Use Welford's online algorithm, see
        // https://en.wikipedia.org/wiki/Algorithms_for_calculating_variance
        void process_value(double val)
        {
            ++count_;
            double delta = val - mean_;
            mean_ += delta / count_;
            double delta2 = val - mean_;
            m2_ += delta * delta2;
        }

        template <typename Scalar>
        typename std::enable_if<traits::is_scalar<Scalar>::value, double>::type
        operator()(Scalar s, double initial)
        {
            process_value(s);
            return initial;
        }

        template <typename Vector>
        typename std::enable_if<!traits::is_scalar<Vector>::value, double>::type
        operator()(Vector& v, double initial)
        {
            for (auto&& elem : v)
            {
                process_value(elem);
            }
            return initial;
        }

        double finalize(double value, std::size_t size) const
        {
            HPX_ASSERT(count_ == size);
            if (size == 0)
            {
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "statistics_var_op::finalize",
                    util::generate_error_message(
                        "empty sequences are not supported", name_, codename_));
            }
            if (size == 1)
            {
                return 0.0;
            }

            return m2_ / size;
        }

        std::string const& name_;
        std::string const& codename_;

        std::size_t count_;
        double mean_;
        double m2_;
    };

}}    // namespace phylanx::common

#endif
