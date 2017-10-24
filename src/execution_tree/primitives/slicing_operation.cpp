// Copyright (c) 2017 Bibek Wagle
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)


#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/slicing_operation.hpp>
#include <phylanx/ir/node_data.hpp>
#include <phylanx/util/optional.hpp>
#include <phylanx/util/serialization/ast.hpp>
#include <phylanx/util/serialization/optional.hpp>

#include <hpx/include/components.hpp>
#include <hpx/include/lcos.hpp>

#include <cstddef>
#include <memory>
#include <utility>
#include <vector>

///////////////////////////////////////////////////////////////////////////////
typedef hpx::components::component<
    phylanx::execution_tree::primitives::slicing_operation>
    slicing_operation_type;
HPX_REGISTER_DERIVED_COMPONENT_FACTORY(
    slicing_operation_type, phylanx_slicing_operation_component,
    "phylanx_primitive_component", hpx::components::factory_enabled)
HPX_DEFINE_GET_COMPONENT_TYPE(slicing_operation_type::wrapped_type)

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace execution_tree { namespace primitives
    {
      ///////////////////////////////////////////////////////////////////////////
      std::vector<match_pattern_type> const slicing_operation::match_data =
          {
              hpx::util::make_tuple(
                  "slice", "slice(_1, _2, _3, _4)", &create<slicing_operation>)
          };

      ///////////////////////////////////////////////////////////////////////////
      slicing_operation::slicing_operation(
          std::vector<primitive_argument_type>&& operands)
          : operands_(std::move(operands))
      {
        if (operands_.size() != 4)
        {
          HPX_THROW_EXCEPTION(hpx::bad_parameter,
                              "phylanx::execution_tree::primitives::slicing_operation::"
                                  "slicing_operation",
                              "the slicing_operation primitive requires exactly four arguments");
        }

        if (!valid(operands_[0]) || !valid(operands_[1])
            || !valid(operands_[2]) || !valid(operands_[3]))
        {
          HPX_THROW_EXCEPTION(hpx::bad_parameter,
                              "phylanx::execution_tree::primitives::slicing_operation::"
                                  "slicing_operation",
                              "the slicing_operation primitive requires that the arguments "
                                  "given by the operands array are valid");
        }
      }

      hpx::future<primitive_result_type> slicing_operation::eval() const
      {
        return hpx::make_ready_future(
            primitive_result_type{});
      }
    }}}
