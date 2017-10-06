//  Copyright (c) 2017 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_PRIMITIVES_FILE_READ_SEP_17_2017_1016AM)
#define PHYLANX_PRIMITIVES_FILE_READ_SEP_17_2017_1016AM

#include <phylanx/config.hpp>
#include <phylanx/ast/node.hpp>
#include <phylanx/ir/node_data.hpp>
#include <phylanx/execution_tree/primitives/base_primitive.hpp>
#include <phylanx/util/optional.hpp>
#include <phylanx/util/serialization/optional.hpp>

#include <hpx/include/components.hpp>

#include <array>
#include <fstream>
#include <utility>
#include <string>

namespace phylanx { namespace execution_tree { namespace primitives
{
    class HPX_COMPONENT_EXPORT file_read
      : public base_primitive
      , public hpx::components::component_base<file_read>
    {
        using operand_type = util::optional<ir::node_data<double>>;
        using operands_type = std::vector<operand_type>;

    public:
        static match_pattern_type const match_data;

        file_read() = default;

        file_read(std::vector<primitive_argument_type>&& operands);

        hpx::future<operand_type> eval() const override;

    private:
        std::string filename_;
    };
}}}

#endif


