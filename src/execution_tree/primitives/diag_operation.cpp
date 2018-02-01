// Copyright (c) 2018 Bibek Wagle
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/diag_operation.hpp>
#include <phylanx/ir/node_data.hpp>

#include <hpx/include/components.hpp>
#include <hpx/include/lcos.hpp>
#include <hpx/include/util.hpp>

#include <cstddef>
#include <cmath>
#include <memory>
#include <numeric>
#include <string>
#include <utility>
#include <vector>

#include <blaze/Math.h>

//////////////////////////////////////////////////////////////////////////////
typedef hpx::components::component<
    phylanx::execution_tree::primitives::diag_operation>
    diag_operation_type;
HPX_REGISTER_DERIVED_COMPONENT_FACTORY(diag_operation_type,
    phylanx_diag_operation_component, "phylanx_primitive_component",
    hpx::components::factory_enabled)
HPX_DEFINE_GET_COMPONENT_TYPE(diag_operation_type::wrapped_type)

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace execution_tree { namespace primitives {
        ///////////////////////////////////////////////////////////////////////////
        match_pattern_type const diag_operation::match_data =
            {
            hpx::util::make_tuple("diag",
                std::vector<std::string>{"diag(_1, _2)"},
                &create<diag_operation>)
            };

        ///////////////////////////////////////////////////////////////////////////
        diag_operation::diag_operation(
            std::vector<primitive_argument_type>&& operands)
          : base_primitive(std::move(operands))
        { }

        ///////////////////////////////////////////////////////////////////////////
        namespace detail {
            struct diag_primitive : std::enable_shared_from_this<diag_primitive>
            {
                diag_primitive() = default;

            protected:
                using arg_type = ir::node_data<double>;
                using args_type = std::vector<arg_type>;
                using storage1d_type = typename arg_type::storage1d_type;
                using storage2d_type = typename arg_type::storage2d_type;

                primitive_result_type diag0d(args_type&& args) const
                {
                    return primitive_result_type(std::move(args[0]));
                }

                primitive_result_type diag1d(args_type&& args) const
                {
                    auto vecsize = args[0].dimension(0);

                    auto k = 0;
                    auto incr = 0;

                    if (args.size() == 2)
                    {
                        k = args[1].scalar();
                        incr = std::abs(k);
                    }

                    blaze::DynamicMatrix<double> result(
                        vecsize + incr, vecsize + incr, 0);
                    blaze::Band<blaze::DynamicMatrix<double>> diag =
                        blaze::band(result, k);
                    diag = args[0].vector();

                    return ir::node_data<double>{
                        storage2d_type{std::move(result)}};
                }

                primitive_result_type diag2d(args_type&& args) const
                {
                    auto row_size = args[0].dimension(0);
                    auto col_size = args[0].dimension(0);

                    auto k = 0;

                    if (args.size() == 2)
                    {
                        k = args[1].scalar();
                    }

                    blaze::Band<blaze::CustomMatrix<double, true, true>> diag =
                        blaze::band(args[0].matrix(), k);

                    blaze::DynamicVector<double> result(diag);

                    return ir::node_data<double>{
                        storage1d_type{std::move(result)}};
                }

            public:
                hpx::future<primitive_result_type> eval(
                    std::vector<primitive_argument_type> const& operands,
                    std::vector<primitive_argument_type> const& args) const
                {
                    if (operands.size() > 2)
                    {
                        HPX_THROW_EXCEPTION(hpx::bad_parameter,
                            "phylanx::execution_tree::primitives::"
                            "diag_operation::diag_operation",
                            "the diag_operation primitive requires "
                            "either one or two arguments");
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
                            "diag_operation::eval",
                            "the diag_operation primitive requires "
                            "that the "
                            "arguments given by the operands array are valid");
                    }

                    auto this_ = this->shared_from_this();
                    return hpx::dataflow(
                        hpx::util::unwrapping([this_](args_type&& args)
                                                  -> primitive_result_type {
                            std::size_t matrix_dims = args[0].num_dimensions();
                            switch (matrix_dims)
                            {
                            case 0:
                                return this_->diag0d(std::move(args));

                            case 1:
                                return this_->diag1d(std::move(args));

                            case 2:
                                return this_->diag2d(std::move(args));

                            default:
                                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                                    "diag_operation::eval",
                                    "left hand side operand has unsupported "
                                    "number of dimensions");
                            }
                        }),
                        detail::map_operands(operands, functional::numeric_operand{}, args));
                }
            };
        }

        hpx::future<primitive_result_type> diag_operation::eval(
            std::vector<primitive_argument_type> const& args) const
        {
            if (operands_.empty())
            {
                return std::make_shared<detail::diag_primitive>()->eval(
                    args, noargs);
            }

            return std::make_shared<detail::diag_primitive>()->eval(
                operands_, args);
        }
}}}
