// Copyright (c) 2018 Monil, Mohammad Alaul Haque
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/plugins/statistics/mean_operation.hpp>
#include <phylanx/plugins/statistics/statistics_base_impl.hpp>
#include <phylanx/util/blaze_traits.hpp>

#include <cstddef>
#include <functional>
#include <string>
#include <type_traits>
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
        struct statistics_mean_op
        {
            using result_type = double;

            statistics_mean_op(std::string const& name,
                    std::string const& codename)
              : name_(name), codename_(codename)
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
            operator()(Vector const& v, double initial) const
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
                            "empty sequences are not supported", name_,
                            codename_));
                }

                return value / size;
            }

            std::string const& name_;
            std::string const& codename_;
        };
    }

    ///////////////////////////////////////////////////////////////////////////
    match_pattern_type const mean_operation::match_data =
    {
        match_pattern_type{
            "mean",
            std::vector<std::string>{
                "mean(_1, _2, _3)", "mean(_1, _2)", "mean(_1)"
            },
            &create_mean_operation,
            &create_primitive<mean_operation>, R"(
            ar, axis, keepdims
            Args:

                ar (array) : an array of values
                axis (optional, int) : the axis along which to calculate the mean
                keepdims (optional, boolean): keep dimension of input

            Returns:

            The mean of the array. If an axis is specified, the result is the
            vector created when the mean is taken along the specified axis.)",
            true
        }
    };

    ///////////////////////////////////////////////////////////////////////////
    mean_operation::mean_operation(
            primitive_arguments_type&& operands,
            std::string const& name, std::string const& codename)
      : base_type(std::move(operands), name, codename)
    {
    }
}}}
