//  Copyright (c) 2017 Alireza Kheirkhahan
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/ast/detail/is_literal_value.hpp>
#include <phylanx/execution_tree/primitives/store_operation.hpp>
#include <phylanx/ir/node_data.hpp>
#include <phylanx/util/optional.hpp>
#include <phylanx/util/serialization/eigen.hpp>
#include <phylanx/util/serialization/optional.hpp>

#include <hpx/include/components.hpp>
#include <hpx/include/lcos.hpp>
#include <hpx/include/util.hpp>

#include <cstddef>
#include <memory>
#include <utility>
#include <vector>

///////////////////////////////////////////////////////////////////////////////
typedef hpx::components::component<
    phylanx::execution_tree::primitives::store_operation>
    store_operation_type;
HPX_REGISTER_DERIVED_COMPONENT_FACTORY(store_operation_type,
    phylanx_store_operation_component, "phylanx_primitive_component",
    hpx::components::factory_enabled)
HPX_DEFINE_GET_COMPONENT_TYPE(store_operation_type::wrapped_type)

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace execution_tree { namespace primitives
{
    ///////////////////////////////////////////////////////////////////////////
    std::vector<match_pattern_type> const store_operation::match_data =
    {
        hpx::util::make_tuple(
            "store", "store(_1, _2)", &create<store_operation>)
    };

    ///////////////////////////////////////////////////////////////////////////
    store_operation::store_operation(
            std::vector<primitive_argument_type> && operands)
      : operands_(std::move(operands))
    {
        if (operands_.size() != 2)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "store_operation::store_operation",
                "the store_operation primitive requires exactly two "
                    "operands");
        }

        if (!valid(operands_[0]) || !valid(operands_[1]))
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "store_operation::store_operation",
                "the store_operation primitive requires that the arguments "
                    "given by the operands array is valid");
        }

        if (!is_primitive_operand(operands_[0]))
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "store_operation::store_operation",
                "the first argument of the store primitive must refer to a "
                    "another primitive and can't be a literal value");
        }
    }

    ///////////////////////////////////////////////////////////////////////////
    namespace detail
    {
        struct store : std::enable_shared_from_this<store>
        {
            store(std::vector<primitive_argument_type> const& operands)
              : operands_(operands)
            {}

            hpx::future<primitive_result_type> eval()
            {
                auto this_ = this->shared_from_this();
                return literal_operand(operands_[1])
                    .then(hpx::util::unwrapping(
                        [this_](primitive_result_type&& val)
                        {
                            primitive_operand(this_->operands_[0])
                                .store(hpx::launch::sync, val);
                            return std::move(val);
                        }));
            }

            std::vector<primitive_argument_type> operands_;
        };
    }

    hpx::future<primitive_result_type> store_operation::eval() const
    {
        return std::make_shared<detail::store>(operands_)->eval();
    }
}}}

