//  Copyright (c) 2017-2018 Bibek Wagle
//  Copyright (c) 2017-2018 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/slicing_operation.hpp>
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
    primitive create_slicing_operation(hpx::id_type const& locality,
        std::vector<primitive_argument_type>&& operands, std::string const& name)
    {
        static std::string type("slice");
        return create_primitive_component(
            locality, type, std::move(operands), name);
    }

    match_pattern_type const slicing_operation::match_data =
    {
        hpx::util::make_tuple("slice",
            std::vector<std::string>{"slice(_1, _2, _3, _4, _5)",
        "slice(_1,_2,_3,_4,_5,_6,_7)"},
            &create_slicing_operation, &create_primitive<slicing_operation>)
    };

    ///////////////////////////////////////////////////////////////////////////
    slicing_operation::slicing_operation(
            std::vector<primitive_argument_type>&& operands)
      : primitive_component_base(std::move(operands))
    {}

    ///////////////////////////////////////////////////////////////////////////
    namespace detail
    {
        std::vector<int> create_list_slice(
            int start, int stop, int step, int array_length)
        {
            auto actual_start = 0;
            auto actual_stop = 0;

            if (start >= 0)
            {
                actual_start = start;
            }

            if (start < 0)
            {
                actual_start = array_length + start;
            }

            if (stop >= 0)
            {
                actual_stop = stop;
            }

            if (stop < 0)
            {
                actual_stop = array_length + stop;
            }

            std::vector<int> result;

            if (step > 0)
            {
                for (int i = actual_start; i < actual_stop; i += step)
                {
                    result.push_back(i);
                }
            }

            if (step < 0)
            {
                for (int i = actual_start; i > actual_stop; i += step)
                {
                    result.push_back(i);
                }
            }

            //Note: std::vector will throw Invalid element access index if
            //bad parameters are passed to start, stop
            return result;
        }

        struct slicing : std::enable_shared_from_this<slicing>
        {
            slicing() = default;

        protected:
            using arg_type = ir::node_data<double>;
            using args_type = std::vector<arg_type>;
            using storage0d_type = typename arg_type::storage0d_type;
            using storage1d_type = typename arg_type::storage1d_type;
            using storage2d_type = typename arg_type::storage2d_type;

            primitive_argument_type slicing0d(args_type&& args) const
            {
                return primitive_argument_type(std::move(args[0]));
            }

            primitive_argument_type slicing1d(args_type&& args) const
            {
                // return elements starting from row_start to row_stop.
                // the values passed to col_stat and col_stop does not have an
                // effect on the result.

                auto row_start = args[1].scalar();
                auto row_stop = args[2].scalar();
                int step = 1;

                if (args.size() == 7)
                {
                    step = args[3].scalar();

                    if (step == 0)
                    {
                        HPX_THROW_EXCEPTION(hpx::bad_parameter,
                            "phylanx::execution_tree::primitives::"
                            "slicing_operation::slicing_"
                            "operation",
                            "step can not be zero");
                    }
                }

                auto init_list = create_list_slice(
                    row_start, row_stop, step, args[0].size());

                auto sv = blaze::elements(args[0].vector(), init_list);

                if (sv.size() == 1)
                {
                    return primitive_argument_type{sv[0]};
                }

                storage1d_type v{sv};
                return primitive_argument_type{
                    ir::node_data<double>(std::move(v))};
            }

            primitive_argument_type slicing2d(args_type&& args) const
            {
                auto row_start = args[1].scalar();
                auto row_stop = args[2].scalar();
                auto col_start = args[3].scalar();
                auto col_stop = args[4].scalar();
                auto num_matrix_rows = args[0].dimensions()[0];
                auto num_matrix_cols = args[0].dimensions()[1];

                int step_row = 1;
                int step_col = 1;

                if (args.size() == 7)
                {
                    step_row = args[3].scalar();
                    step_col = args[6].scalar();
                    col_start = args[4].scalar();
                    col_stop = args[5].scalar();
                    
                    if (step_row == 0 || step_col == 0)
                    {
                        HPX_THROW_EXCEPTION(hpx::bad_parameter,
                            "phylanx::execution_tree::primitives::"
                            "row_slicing_operation::row_slicing_"
                            "operation",
                            "step can not be zero");
                    }
                }

                auto init_list_row = create_list_slice(
                    row_start, row_stop, step_row, num_matrix_rows);
                auto init_list_col = create_list_slice(
                    col_start, col_stop, step_col, num_matrix_cols);

                blaze::DynamicMatrix<double> sm_row =
                    blaze::rows(args[0].matrix(), init_list_row);
                auto sm = blaze::columns(sm_row, init_list_col);

                if (sm.rows() == 1)
                {
                    auto sv = blaze::trans(blaze::row(sm, 0));
                    if (sv.size() == 1)
                    {
                        return primitive_argument_type{sv[0]};
                    }

                    storage1d_type v{sv};
                    return primitive_argument_type{
                        ir::node_data<double>{std::move(v)}};
                }

                if (sm.columns() == 1)
                {
                    auto sv = blaze::column(sm, 0);
                    if (sv.size() == 1)
                    {
                        return primitive_argument_type{sv[0]};
                    }

                    storage1d_type v{sv};
                    return primitive_argument_type{
                        ir::node_data<double>{std::move(v)}};
                }

                storage2d_type m{sm};

                return primitive_argument_type{
                    ir::node_data<double>(std::move(m))};
            }

        public:
            hpx::future<primitive_argument_type> eval(
                std::vector<primitive_argument_type> const& operands,
                std::vector<primitive_argument_type> const& args) const
            {
                if (operands.size() != 5 && operands.size() != 7)
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "phylanx::execution_tree::primitives::"
                        "slicing_operation::slicing_operation",
                        "the slicing_operation primitive requires either "
                        "five or seven arguments");
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
                        "slicing_operation::eval",
                        "the slicing_operation primitive requires that the "
                        "arguments given by the operands array are valid");
                }

                auto this_ = this->shared_from_this();
                return hpx::dataflow(
                    hpx::util::unwrapping(
                        [this_](args_type&& args) -> primitive_argument_type {
                            std::size_t lhs_dims = args[0].num_dimensions();
                            switch (lhs_dims)
                            {
                            case 0:
                                return this_->slicing0d(std::move(args));

                            case 1:
                                return this_->slicing1d(std::move(args));

                            case 2:
                                return this_->slicing2d(std::move(args));

                            default:
                                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                                    "slicing_operation::eval",
                                    "left hand side operand has unsupported "
                                    "number of dimensions");
                            }
                        }),
                    detail::map_operands(
                        operands, functional::numeric_operand{}, args));
            }
        };
    }

    hpx::future<primitive_argument_type> slicing_operation::eval(
        std::vector<primitive_argument_type> const& args) const
    {
        if (operands_.empty())
        {
            return std::make_shared<detail::slicing>()->eval(args, noargs);
        }

        return std::make_shared<detail::slicing>()->eval(operands_, args);
    }
}}}
