//   Copyright (c) 2017-2019 Hartmut Kaiser
//
//   Distributed under the Boost Software License, Version 1.0. (See accompanying
//   file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/plugins/arithmetics/cumulative_impl.hpp>
#include <phylanx/plugins/arithmetics/cumsum.hpp>

#include <hpx/include/parallel_scan.hpp>

#include <functional>
#include <string>
#include <utility>
#include <vector>

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace execution_tree { namespace primitives
{
    ///////////////////////////////////////////////////////////////////////////
    match_pattern_type const cumsum::match_data =
    {
        match_pattern_type{
            "cumsum",
            std::vector<std::string>{
                "cumsum(_1)", "cumsum(_1, _2)"
            },
            &create_cumsum, &create_primitive<cumsum>, R"(
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
    namespace detail
    {
        struct cumsum_op
        {
            template <typename T>
            static T initial()
            {
                return T(0);
            }

            template <typename InIter, typename OutIter, typename T>
            OutIter operator()(
                InIter begin, InIter end, OutIter dest, T init) const
            {
                return hpx::parallel::inclusive_scan(
                    hpx::parallel::execution::seq, begin, end, dest,
                    std::plus<>{}, init);
            }
        };
    }

    ///////////////////////////////////////////////////////////////////////////
    cumsum::cumsum(primitive_arguments_type&& operands,
            std::string const& name, std::string const& codename)
      : base_type(std::move(operands), name, codename)
    {}
}}}
