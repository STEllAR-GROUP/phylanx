//   Copyright (c) 2017 Bibek Wagle
//
//   Distributed under the Boost Software License, Version 1.0. (See accompanying
//   file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef PHYLANX_EXPONENTIAL_OPERATION_HPP_OCT031241PM
#define PHYLANX_EXPONENTIAL_OPERATION_HPP_OCT031241PM

#include <phylanx/config.hpp>
#include <phylanx/ast/node.hpp>
#include <phylanx/ir/node_data.hpp>
#include <phylanx/execution_tree/primitives/base_primitive.hpp>

#include <hpx/include/components.hpp>

#include <utility>

namespace phylanx { namespace execution_tree { namespace primitives
    {
      class HPX_COMPONENT_EXPORT exponential_operation
          : public base_primitive
          , public hpx::components::component_base<exponential_operation>
      {
      private:
        using operands_type = std::vector<ir::node_data<double>>;
        primitive_argument_type operands_;

      public:
        exponential_operation() = default;

        exponential_operation(primitive_argument_type &&operands);

        hpx::future<ir::node_data<double>> eval() const override;

      protected:
        ir::node_data<double> exponential0d(operands_type const& ops) const;
        ir::node_data<double> exponentialxd(operands_type const& ops) const;
      };
    }}}
#endif //PHYLANX_EXPONENTIAL_OPERATION_HPP_OCT031241PM