//   Copyright (c) 2018 R. Tohid
//
//   Distributed under the Boost Software License, Version 1.0. (See accompanying
//   file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_PRIMITIVES_LINEARMATRIX_FEB_04_2018_0331PM)
#define PHYLANX_PRIMITIVES_LINEARMATRIX_FEB_04_2018_0331PM

#include <phylanx/execution_tree/primitives/base_primitive.hpp>

#include <vector>

namespace phylanx { namespace execution_tree { namespace primitives
{
    class linearmatrix
      : public base_primitive
      , public hpx::components::component_base<linearmatrix>
    {
    public:
        static match_pattern_type const match_data;

        linearmatrix() = default;

        linearmatrix(std::vector<primitive_argument_type>&& args);

        hpx::future<primitive_result_type> eval(
            std::vector<primitive_argument_type> const& args) const override;
    };
}}}

#endif

