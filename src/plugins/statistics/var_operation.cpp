// Copyright (c) 2018 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/plugins/statistics/var_operation.hpp>
#include <phylanx/plugins/statistics/statistics_base_impl.hpp>
#include <phylanx/util/blaze_traits.hpp>

#include <hpx/assert.hpp>

#include <cstddef>
#include <functional>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace execution_tree { namespace primitives
{
    ///////////////////////////////////////////////////////////////////////////
    namespace detail
    {
        template <typename T>
        HPX_FORCEINLINE T sqr(T v)
        {
            return v * v;
        }

        template <typename T>
        struct statistics_var_op
        {
            using result_type = double;

            statistics_var_op(std::string const& name,
                    std::string const& codename)
              : name_(name), codename_(codename)
              , count_(0), mean_(0), m2_(0)
            {}

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
            typename std::enable_if<traits::is_scalar<Scalar>::value,
                double>::type
            operator()(Scalar s, double initial)
            {
                process_value(s);
                return initial;
            }

            template <typename Vector>
            typename std::enable_if<!traits::is_scalar<Vector>::value,
                double>::type
            operator()(Vector& v, double initial)
            {
                for (auto && elem : v)
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
                            "empty sequences are not supported", name_,
                            codename_));
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
    }

    ///////////////////////////////////////////////////////////////////////////
    match_pattern_type const var_operation::match_data =
    {
        match_pattern_type{
            "var",
            std::vector<std::string>{
                "var(_1)", "var(_1, _2)", "var(_1, _2, _3)"
            },
            &create_var_operation, &create_primitive<var_operation>, R"(
            v, axis, keepdims
            Args:

                v (vector or matrix) : a vector or matrix
                axis (optional, integer): a axis to sum along
                keepdims (optional, boolean): keep dimension of input

            Returns:

            The statistical variance of all values along the specified axis.)",
            true
        }
    };

    ///////////////////////////////////////////////////////////////////////////
    var_operation::var_operation(primitive_arguments_type&& operands,
            std::string const& name, std::string const& codename)
      : base_type(std::move(operands), name, codename)
    {
    }
}}}
