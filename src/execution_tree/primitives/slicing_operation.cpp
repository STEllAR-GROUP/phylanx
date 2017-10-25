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
                  "slice", "slice(_1, _2, _3, _4, _5)", &create<slicing_operation>)
          };

      ///////////////////////////////////////////////////////////////////////////
      slicing_operation::slicing_operation(
          std::vector<primitive_argument_type>&& operands)
          : operands_(std::move(operands))
      {
        if (operands_.size() != 5)
        {
          HPX_THROW_EXCEPTION(hpx::bad_parameter,
                              "phylanx::execution_tree::primitives::slicing_operation::"
                                  "slicing_operation",
                              "the slicing_operation primitive requires exactly five "
                                  "arguments");
        }

        if (!valid(operands_[0]) || !valid(operands_[1])
            || !valid(operands_[2]) || !valid(operands_[3])
            || !valid(operands_[4]))
        {
          HPX_THROW_EXCEPTION(hpx::bad_parameter,
                              "phylanx::execution_tree::primitives::slicing_operation::"
                                  "slicing_operation",
                              "the slicing_operation primitive "
                                  "requires that the arguments "
                                  "given by the operands array are valid");
        }
      }

      namespace detail
      {
        struct slice : std::enable_shared_from_this<slice>
        {
          slice(std::vector<primitive_argument_type> const& operands)
          : operands_(operands)
              {}

        protected:
          using operand_type = ir::node_data<double>;
          using operands_type = std::vector<operand_type>;

          primitive_result_type slice0d(operands_type && ops) const
          {
            HPX_THROW_EXCEPTION(hpx::not_implemented, "slice0d",
            "slice0d not implemented yet");
          }

          primitive_result_type slice1d(operands_type && ops) const
          {
            HPX_THROW_EXCEPTION(hpx::not_implemented,"slice1d",
                                "slice1d not implemented yet");
          }

          primitive_result_type slice2d(operands_type && ops) const
          {
            HPX_THROW_EXCEPTION(hpx::not_implemented,"slice2d",
                                "slice2d not implemented yet");
          }

        public:
          hpx::future<primitive_result_type> eval() const
          {
            auto this_ = this->shared_from_this();
            std::cout<<"HI"<<std::endl;
            return hpx::dataflow(hpx::util::unwrapping(
                [this_](operands_type && ops) -> primitive_result_type
                {
                  std::size_t input_dims = ops[0].num_dimensions();
                  std::cout<<"Innput_dims"<<input_dims<<std::endl;
                  switch (input_dims)
                  {
                    case 0:
                      return this_->slice0d(std::move(ops));

                    case 1:
                      return this_->slice1d(std::move(ops));

                    case 2:
                      return this_->slice2d(std::move(ops));

                    default:
                      HPX_THROW_EXCEPTION(hpx::bad_parameter,
                                          "slice_operation::eval",
                                          "input data has unsupported "
                                              "number of dimensions");
                  }
                }), detail::map_operands(operands_, numeric_operand)
            );
          }
        private:
          std::vector<primitive_argument_type> operands_;
        };
      }

      hpx::future<primitive_result_type> slicing_operation::eval() const
      {
        return std::make_shared<detail::slice>(operands_)->eval();
      }
}}}
