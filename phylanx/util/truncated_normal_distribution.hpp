// Copyright (c) 2019 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_UTIL_TRUNCATED_NORMAL_DISTRIBUTION_MAR_11_2019_0621PM)
#define PHYLANX_UTIL_TRUNCATED_NORMAL_DISTRIBUTION_MAR_11_2019_0621PM

#include <phylanx/config.hpp>

#include <hpx/util/assert.hpp>

#include <cstddef>
#include <type_traits>

namespace phylanx { namespace util
{
    // A truncated normal has values generated follow a normal distribution with
    // specified mean and standard deviation, except that values whose magnitude
    // is more than two standard deviations from the mean are dropped and re-picked.
    template <typename T = double>
    class truncated_normal_distribution
    {
    public:
        static_assert(std::is_same<T, float>::value ||
                std::is_same<T, double>::value ||
                std::is_same<T, long double>::value,
            "invalid template argument for truncated_normal_distribution, "
            "requires one of: float, double, or long double");

        typedef T result_type;

        struct param_type
        {
            typedef truncated_normal_distribution distribution_type;

            explicit param_type(T mean = 0.0, T sigma = 1.0)
              : mean_(mean), sigma_(sigma)
            {
                HPX_ASSERT(0.0 < sigma_);
            }

            // test for equality
            bool operator==(param_type const& right) const
            {
                return mean_ == right.mean_ && sigma_ == right.sigma_;
            }

            // test for inequality
            bool operator!=(param_type const& right) const
            {
                return !(*this == right);
            }

            // return mean value
            T mean() const
            {
                return mean_;
            }

            // return sigma value
            T sigma() const
            {
                return sigma_;
            }

            // return sigma value
            T stddev() const
            {
                return sigma_;
            }

            T mean_;
            T sigma_;
        };

        explicit truncated_normal_distribution(T mean = 0.0, T sigma = 1.0)
          : params_(mean, sigma)
          , value2_(0.0)
          , value2_valid_(false)
        {
        }

        explicit truncated_normal_distribution(param_type const& params)
          : params_(params)
          , value2_(0.0)
          , value2_valid_(false)
        {
        }

        // return mean value
        T mean() const
        {
            return params_.mean();
        }

        // return sigma value
        T sigma() const
        {
            return params_.sigma();
        }

        // return sigma value
        T stddev() const
        {
            return params_.sigma();
        }

        // return parameter package
        param_type param() const
        {
            return params_;
        }

        // set parameter package
        void param(param_type const& params)
        {
            params_ = params;
            reset();
        }

        // get smallest possible result
        result_type (min)() const
        {
            return params_.mean_ - 2 * params_.sigma_;
        }

        // get largest possible result
        result_type (max)() const
        {
            return params_.mean_ + 2 * params_.sigma_;
        }

        // clear internal state
        void reset()
        {
            value2_valid_ = false;
        }

        // return next value
        template <typename Engine>
        result_type operator()(Engine& eng)
        {
            return generate(eng, params_);
        }

        // return next value, given parameter package
        template <typename Engine>
        result_type operator()(Engine& eng, param_type const& params)
        {
            reset();
            return generate(eng, params, true);
        }

    private:
        // compute next value, see Knuth, vol. 2, p. 122, algorithm P
        template <typename Engine>
        result_type generate(Engine& eng, param_type const& params,
            bool force = false)
        {
            T result;
            while (true)
            {
                if (!force && value2_valid_)
                {
                    result = value2_;
                    value2_valid_ = false;
                }
                else
                {
                    constexpr std::size_t neg1 = static_cast<std::size_t>(-1);

                    // generate two values, store one, return one
                    T val1, val2, s;
                    while (true)
                    {
                        // reject bad values
                        val1 = 2 * std::generate_canonical<T, neg1>(eng) - 1;
                        val2 = 2 * std::generate_canonical<T, neg1>(eng) - 1;
                        s = val1 * val1 + val2 * val2;
                        if (s < 1)
                            break;
                    }

                    T const x =
                        static_cast<T>(std::sqrt(-2.0 * std::log(s) / s));

                    result = x * val1;

                    if (!force)
                    {    // save second value for next call
                        value2_ = x * val2;
                        value2_valid_ = true;
                    }
                }

                // exit if number is in valid range
                if (result >= static_cast<T>(-2) && result <= static_cast<T>(2))
                    break;
            }

            return result * params.sigma_ + params.mean_;
        }

        param_type params_;
        T value2_;
        bool value2_valid_;
    };

    // test for equality
    template <typename T>
    bool operator==(truncated_normal_distribution<T> const& lhs,
        truncated_normal_distribution<T> const& rhs)
    {
        return lhs.param() == rhs.param();
    }

    // test for inequality
    template <typename T>
    bool operator!=(truncated_normal_distribution<T> const& lhs,
        truncated_normal_distribution<T> const& rhs)
    {
        return !(lhs == rhs);
    }
}}

#endif
