//  Copyright (c) 2017-2018 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_PRIMITIVES_FILE_READ_SEP_17_2017_1016AM)
#define PHYLANX_PRIMITIVES_FILE_READ_SEP_17_2017_1016AM

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/base_primitive.hpp>

#include <hpx/include/components.hpp>

#include <string>
#include <vector>

namespace phylanx { namespace execution_tree { namespace primitives
{
    class file_read
      : public base_primitive
      , public hpx::components::component_base<file_read>
    {
    public:
        static match_pattern_type const match_data;

        file_read() = default;

        PHYLANX_EXPORT file_read(
            std::vector<primitive_argument_type>&& operands);

        PHYLANX_EXPORT hpx::future<primitive_argument_type> eval(
            std::vector<primitive_argument_type> const& args) const override;
    };
}}}

#endif


