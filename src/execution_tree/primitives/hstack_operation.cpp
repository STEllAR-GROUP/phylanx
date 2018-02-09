//  Copyright (c) 2018 Bibek Wagle
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)


#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/hstack_operation.hpp>
#include <phylanx/ir/node_data.hpp>

#include <hpx/include/components.hpp>
#include <hpx/include/lcos.hpp>
#include <hpx/include/util.hpp>

#include <cstddef>
#include <memory>
#include <numeric>
#include <string>
#include <utility>
#include <vector>

#include <blaze/Math.h>

//////////////////////////////////////////////////////////////////////////////
typedef hpx::components::component<
    phylanx::execution_tree::primitives::hstack_operation>
    hstack_operation_type;
HPX_REGISTER_DERIVED_COMPONENT_FACTORY(hstack_operation_type,
    phylanx_hstack_operation_component, "phylanx_primitive_component",
    hpx::components::factory_enabled)
HPX_DEFINE_GET_COMPONENT_TYPE(hstack_operation_type::wrapped_type)

///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace execution_tree { namespace primitives
{
    ///////////////////////////////////////////////////////////////////////////
    match_pattern_type const hstack_operation::match_data =
        {
        hpx::util::make_tuple("hstack",
            std::vector<std::string>{"hstack(_1, __2)"},
            &create<hstack_operation>)
        };

    ///////////////////////////////////////////////////////////////////////////
    hstack_operation::hstack_operation(
            std::vector<primitive_argument_type>&& operands)
      : base_primitive(std::move(operands))
    {}

    ///////////////////////////////////////////////////////////////////////////
    namespace detail
    {
        struct hstack : std::enable_shared_from_this<hstack>
        {
            hstack() = default;

        protected:
            using arg_type = ir::node_data<double>;
            using args_type = std::vector<arg_type>;
            using storage1d_type = typename arg_type::storage1d_type;
            using storage2d_type = typename arg_type::storage2d_type;

            primitive_argument_type hstack0d(args_type&& args) const
            {
                auto vec_size = args.size();
                blaze::DynamicVector<double> temp(vec_size);

                for (std::size_t i = 0; i < vec_size; ++i)
                {
                    temp[i] = args[i].scalar();
                }

                return primitive_argument_type{
                    ir::node_data<double>{storage1d_type{std::move(temp)}}};
            }

            primitive_argument_type hstack1d(args_type&& args) const
            {
                auto args_size = args.size();
                auto total_elements = 0;

                for (std::size_t i = 0; i < args_size; ++i)
                {
                    total_elements += args[i].size();
                }

                blaze::DynamicVector<double> temp(total_elements);
                auto iter = temp.begin();

                for (std::size_t i = 0; i < args.size(); ++i)
                {
                    std::copy(args[i].vector().begin(), args[i].vector().end(), iter);
                    iter += args[i].size();
                }

                return primitive_argument_type{
                    ir::node_data<double>{storage1d_type{std::move(temp)}}};
            }

            primitive_argument_type hstack2d(args_type&& args) const
            {
                auto args_size = args.size();
                auto total_cols = args[0].dimension(1);

                for (std::size_t i = 1; i < args_size; ++i)
                {
                    if (args[i - 1].dimension(0) != args[i].dimension(0))
                    {
                        HPX_THROW_EXCEPTION(hpx::bad_parameter,
                            "phylanx::execution_tree::primitives::"
                            "hstack_operation::hstack_operation",
                            "the hstack_operation primitive requires the "
                            "number of rows be equal for all matrix being "
                            "stacked");
                    }

                    total_cols += args[i].dimension(1);
                }

                blaze::DynamicMatrix<double> temp(
                    args[0].dimension(0), total_cols);

                auto step = 0;
                for (std::size_t i = 0; i < args_size; ++i)
                {
                    auto num_cols = args[i].dimension(1);
                    for (std::size_t j = 0; j < num_cols; ++j)
                    {
                        blaze::column(temp, j + step) =
                            blaze::column(args[i].matrix(), j);
                    }
                    step = step + num_cols;
                }

                return primitive_argument_type{
                    ir::node_data<double>{storage2d_type{std::move(temp)}}};
            }

        public:
            hpx::future<primitive_argument_type> eval(
                std::vector<primitive_argument_type> const& operands,
                std::vector<primitive_argument_type> const& args) const
            {
                if (operands.size() != 2)
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "phylanx::execution_tree::primitives::"
                        "hstack_operation::hstack_operation",
                        "the hstack_operation primitive requires exactly "
                        "two arguments");
                }

                bool arguments_valid = true;
                for (std::size_t i = 0; i != operands.size(); ++i)
                {
                    if (!valid(operands[i]))
                    {
                        arguments_valid = false;
                    }
                }

                if (!arguments_valid)
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "hstack_operation::eval",
                        "the hstack_operation primitive requires that the "
                        "arguments given by the operands array are valid");
                }

                auto this_ = this->shared_from_this();
                return hpx::dataflow(hpx::util::unwrapping(
                    [this_](args_type&& args) -> primitive_argument_type
                    {
                        std::size_t matrix_dims = args[0].num_dimensions();
                        switch (matrix_dims)
                        {
                        case 0:
                            return this_->hstack0d(std::move(args));

                        case 1:
                            return this_->hstack1d(std::move(args));

                        case 2:
                            return this_->hstack2d(std::move(args));

                        default:
                            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                                "hstack_operation::eval",
                                "left hand side operand has unsupported "
                                "number of dimensions");
                        }
                    }),
                    detail::map_operands(
                        operands, functional::numeric_operand{}, args));
            }
        };
    }

    hpx::future<primitive_argument_type> hstack_operation::eval(
        std::vector<primitive_argument_type> const& args) const
    {
        if (operands_.empty())
        {
            return std::make_shared<detail::hstack>()->eval(args, noargs);
        }

        return std::make_shared<detail::hstack>()->eval(operands_, args);
    }
}}}
