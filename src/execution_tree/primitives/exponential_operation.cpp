//   Copyright (c) 2017 Bibek Wagle
//
//   Distributed under the Boost Software License, Version 1.0. (See accompanying
//   file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/ast/detail/is_literal_value.hpp>
#include <phylanx/execution_tree/primitives/exponential_operation.hpp>
#include <phylanx/ir/node_data.hpp>
#include <phylanx/util/serialization/eigen.hpp>

#include <hpx/include/components.hpp>
#include <hpx/include/lcos.hpp>
#include <hpx/include/util.hpp>

#include <cstddef>
#include <numeric>
#include <utility>
#include <vector>

////////////////////////////////////////////////////////////////////////////////////////
typedef hpx::components::component<
    phylanx::execution_tree::primitives::exponential_operation>
    exponential_operation_type;
HPX_REGISTER_DERIVED_COMPONENT_FACTORY(
    exponential_operation_type, phylanx_exponential_operation_component,
    "phylanx_primitive_component", hpx::components::factory_enabled)
HPX_DEFINE_GET_COMPONENT_TYPE(exponential_operation_type::wrapped_type)
///////////////////////////////////////////////////////////////////////////////////////

namespace phylanx { namespace execution_tree { namespace primitives
    {
      ///////////////////////////////////////////////////////////////////////////
      exponential_operation::exponential_operation(primitive_argument_type &&operands)
          : operands_(std::move(operands))
      {
        std::cout<<"Exponential Constructor"<<std::endl;
      }

      ///////////////////////////////////////////////////////////////////////////
      ir::node_data<double> exponential_operation::exponential0d(operands_type const& ops) const
      {
        return ir::node_data<double>(42);
      }

      ///////////////////////////////////////////////////////////////////////////
      ir::node_data<double> exponential_operation::exponentialxd(operands_type const&
      ops) const
      {
        return ir::node_data<double>(42);
      }

      hpx::future<ir::node_data<double>> exponential_operation::eval() const
      {
        return hpx::make_ready_future(ir::node_data<double>(42));
      }

    }}}