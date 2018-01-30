//   Copyright (c) 2018 R. Tohid
//
//   Distributed under the Boost Software License, Version 1.0. (See accompanying
//   file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_PRIMITIVES_LINSPACE_JAN_22_2018_0931AM)
#define PHYLANX_PRIMITIVES_LINSPACE_JAN_22_2018_0931AM

#include <phylanx/execution_tree/primitives/base_primitive.hpp>

namespace phylanx { namespace execution_tree { namespace primitives
{
    /**
     * @brief Linear space of evenly distributed numbers over the given interval.
     *
     * @author R. Tohid
     * @version 0.0.1
     * @date 2018
     */
    class HPX_COMPONENT_EXPORT linspace
      : public base_primitive
      , public hpx::components::component_base<linspace>
    {
    public:
        static match_pattern_type const match_data;

        /**
         * @brief Default constructor.
         */
        linspace() = default;

        /**
         * @brief Creates a linear space of evenly spaced numbers over the given interval.
         *
         * @param args Is a vector with exactly three elements (in order):
         *
         * start: the first value of the sequence.\n
         * stop: the last value of the sequence. It will be ignored if the number of
         * samples is less than 2.\n
         * num_samples: number of samples in the sequence.
         */
        linspace(std::vector<primitive_argument_type>&& args);

        hpx::future<primitive_result_type> eval(
            std::vector<primitive_argument_type> const& args) const override;
    };
}}}

#endif
