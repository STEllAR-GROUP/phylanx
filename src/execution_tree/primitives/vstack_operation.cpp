//  Copyright (c) 2018 Bibek Wagle
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/vstack_operation.hpp>
#include <phylanx/ir/node_data.hpp>

#include <hpx/include/lcos.hpp>
#include <hpx/include/naming.hpp>
#include <hpx/include/util.hpp>
#include <hpx/throw_exception.hpp>

#include <cstddef>
#include <memory>
#include <numeric>
#include <string>
#include <utility>
#include <vector>

#include <blaze/Math.h>

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace execution_tree { namespace primitives
{
    ///////////////////////////////////////////////////////////////////////////
    primitive create_vstack_operation(hpx::id_type const& locality,
        std::vector<primitive_argument_type>&& operands, std::string const& name)
    {
        static std::string type("vstack");
        return create_primitive_component(
            locality, type, std::move(operands), name);
    }

    match_pattern_type const vstack_operation::match_data =
    {
        hpx::util::make_tuple("vstack",
            std::vector<std::string>{"vstack(_1, __2)"},
            &create_vstack_operation, &create_primitive<vstack_operation>)
    };

    ///////////////////////////////////////////////////////////////////////////
    vstack_operation::vstack_operation(
            std::vector<primitive_argument_type>&& operands)
      : primitive_component_base(std::move(operands))
    {}

    ///////////////////////////////////////////////////////////////////////////
    namespace detail
    {
        struct vstack : std::enable_shared_from_this<vstack>
        {
            vstack() = default;

        protected:
            using arg_type = ir::node_data<double>;
            using args_type = std::vector<arg_type>;
            using storage1d_type = typename arg_type::storage1d_type;
            using storage2d_type = typename arg_type::storage2d_type;

            primitive_argument_type vstack0d(args_type&& args) const
            {
                auto vec_size = args.size();
                blaze::DynamicVector<double> temp(vec_size);

                for (std::size_t i = 0; i < vec_size; ++i)
                {
                    temp[i] = args[i].scalar();
                }

                blaze::DynamicMatrix<double> result(vec_size, 1);
                blaze::column(result, 0) = temp;

                return primitive_argument_type{
                    ir::node_data<double>{storage2d_type{std::move(result)}}};
            }

            primitive_argument_type vstack1d(args_type&& args) const
            {
                auto args_size = args.size();
                arg_type& first = args[0];
                std::size_t first_size = first.dimension(0);

                for (std::size_t i = 1; i < args_size; ++i)
                {
                    if (args[i - 1].dimension(0) != args[i].dimension(0))
                    {
                        HPX_THROW_EXCEPTION(hpx::bad_parameter,
                            "phylanx::execution_tree::primitives::"
                            "vstack_operation::vstack_operation",
                            "the vstack_operation primitive requires the fist "
                            "dimension be equal for all vectors to be stacked");
                    }
                }

                blaze::DynamicMatrix<double> temp(args_size, first_size);
                for (std::size_t i = 0; i < args_size; ++i)
                {
                    blaze::row(temp, i) = blaze::trans(args[i].vector());
                }

                return primitive_argument_type{
                    ir::node_data<double>{storage2d_type{std::move(temp)}}};
            }

            primitive_argument_type vstack2d(args_type&& args) const
            {
                auto args_size = args.size();
                auto total_rows = args[0].dimension(0);

                for (std::size_t i = 1; i < args_size; ++i)
                {
                    if (args[i - 1].dimension(1) != args[i].dimension(1))
                    {
                        HPX_THROW_EXCEPTION(hpx::bad_parameter,
                            "phylanx::execution_tree::primitives::"
                            "vstack_operation::vstack_operation",
                            "the vstack_operation primitive requires the "
                            "number of  "
                            "columns be equal for all matrix being stacked");
                    }

                    total_rows += args[i].dimension(0);
                }

                blaze::DynamicMatrix<double> temp(
                    total_rows, args[0].dimension(1));

                auto step = 0;
                for (std::size_t i = 0; i < args_size; ++i)
                {
                    auto num_rows = args[i].dimension(0);
                    for (std::size_t j = 0; j < num_rows; ++j)
                    {
                        blaze::row(temp, j + step) =
                            blaze::row(args[i].matrix(), j);
                    }
                    step = step + num_rows;
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
                        "vstack_operation::vstack_operation",
                        "the vstack_operation primitive requires exactly "
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
                        "vstack_operation::eval",
                        "the vstack_operation primitive requires that the "
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
                            return this_->vstack0d(std::move(args));

                        case 1:
                            return this_->vstack1d(std::move(args));

                        case 2:
                            return this_->vstack2d(std::move(args));

                        default:
                            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                                "vstack_operation::eval",
                                "left hand side operand has unsupported "
                                "number of dimensions");
                        }
                    }),
                    detail::map_operands(
                        operands, functional::numeric_operand{}, args));
            }
        };
    }

    hpx::future<primitive_argument_type> vstack_operation::eval(
        std::vector<primitive_argument_type> const& args) const
    {
        if (operands_.empty())
        {
            return std::make_shared<detail::vstack>()->eval(args, noargs);
        }

        return std::make_shared<detail::vstack>()->eval(operands_, args);
    }
}}}
