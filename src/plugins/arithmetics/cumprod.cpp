//   Copyright (c) 2017-2019 Hartmut Kaiser
//   Copyright (c) 2019 Bibek Wagle
//
//   Distributed under the Boost Software License, Version 1.0. (See accompanying
//   file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/plugins/arithmetics/cumulative_impl.hpp>
#include <phylanx/plugins/arithmetics/cumprod.hpp>

#include <hpx/include/parallel_scan.hpp>

#include <functional>
#include <string>
#include <utility>
#include <vector>

#include <blaze/Math.h>

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace execution_tree { namespace primitives
{
    ///////////////////////////////////////////////////////////////////////////
    match_pattern_type const cumprod::match_data =
    {
        match_pattern_type{
            "cumprod",
            std::vector<std::string>{
                "cumprod(_1_a, __arg(_2_axis, nil), __arg(_3_dtype, nil))"
            },
            &create_cumprod, &create_primitive<cumprod>, R"(
            a, axis
            Args:

                a (array_like) : input array
                axis (int, optional) : Axis along which the cumulative product is
                    computed. The default (None) is to compute the cumprod over
                    the flattened array.
                dtype (nil, optional) : the data-type of the returned array,
                  defaults to dtype of input arrays.

            Returns:

            Return the cumulative product of the elements along a given axis.)"
        }
    };

    ///////////////////////////////////////////////////////////////////////////
    namespace detail
    {
        struct cumprod_op
        {
            template <typename T>
            static T initial()
            {
                return T(1);
            }

            template <typename InIter, typename OutIter, typename T>
            OutIter operator()(
                InIter begin, InIter end, OutIter dest, T init) const
            {
                return hpx::parallel::inclusive_scan(
                    hpx::parallel::execution::seq, begin, end, dest,
                    std::multiplies<>{}, init);
            }
        };
    }

    ///////////////////////////////////////////////////////////////////////////
    cumprod::cumprod(primitive_arguments_type&& operands,
            std::string const& name, std::string const& codename)
      : base_type(std::move(operands), name, codename)
    {}
}}}
