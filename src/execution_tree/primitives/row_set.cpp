//  Copyright (c) 2018 Bibek Wagle
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/row_set.hpp>
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
    primitive create_row_set_operation(hpx::id_type const& locality,
        std::vector<primitive_argument_type>&& operands, std::string const& name)
    {
        static std::string type("set_row");
        return create_primitive_component(
            locality, type, std::move(operands), name);
    }

    match_pattern_type const row_set_operation::match_data =
    {
        hpx::util::make_tuple("set_row",
            std::vector<std::string>{"set_row(_1, _2, _3, _4, _5)"},
            &create_row_set_operation,
            &create_primitive<row_set_operation>)
    };

    ///////////////////////////////////////////////////////////////////////////
    row_set_operation::row_set_operation(
            std::vector<primitive_argument_type>&& operands)
      : primitive_component_base(std::move(operands))
    {}

    ///////////////////////////////////////////////////////////////////////////
    namespace detail
    {
        std::vector<int> create_list_row_set(
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

            if (result.empty())
            {
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "phylanx::execution_tree::primitives::"
                    "row_set_operation::create_list_row_set",
                    "Set will produce empty result, please check your "
                    "parameters");
            }

            return result;
        }

        struct set_row : std::enable_shared_from_this<set_row>
        {
            set_row() = default;

        protected:
            using arg_type = ir::node_data<double>;
            using args_type = std::vector<arg_type>;

            using storage0d_type = typename arg_type::storage0d_type;
            using storage1d_type = typename arg_type::storage1d_type;
            using storage2d_type = typename arg_type::storage2d_type;

            primitive_argument_type row_set0d(args_type&& args) const
            {
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "phylanx::execution_tree::primitives::"
                    "row_set_operation::row_set_0d",
                    "use store operation for setting value to a variable");
            }

            primitive_argument_type row_set1d(args_type&& args) const
            {
                auto row_start = args[1].scalar();
                auto row_stop = args[2].scalar();
                int step = args[3].scalar();
                auto value_dimnum = args[4].num_dimensions();

                if (step == 0)
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "phylanx::execution_tree::primitives::"
                        "row_set_operation::row_set1d",
                        "step can not be zero");
                }

                if (value_dimnum == 2)
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "phylanx::execution_tree::primitives::"
                        "row_set_operation::row_set1d",
                        "cant store matrix in a vector");
                }

                auto init_list = create_list_row_set(
                    row_start, row_stop, step, args[0].size());

                auto sv = blaze::elements(args[0].vector(), init_list);

                if (value_dimnum == 0)
                {
                    blaze::DynamicVector<double> temp(
                        sv.size(), args[4].scalar());
                    sv = temp;
                    return primitive_argument_type{ir::node_data<double>(0)};
                }

                auto temp = args[4].vector();

                if (sv.size() != temp.size())
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "phylanx::execution_tree::primitives::"
                        "row_set_operation::row_set1d",
                        "size mismatch");
                }

                sv = temp;
                return primitive_argument_type{ir::node_data<double>(0)};
            }

            primitive_argument_type row_set2d(args_type&& args) const
            {
                auto row_start = args[1].scalar();
                auto row_stop = args[2].scalar();
                auto step_row = args[3].scalar();
                auto col_start = 0;
                auto col_stop = args[0].dimensions()[1];
                auto step_col = 1;
                auto num_matrix_rows = args[0].dimensions()[0];
                auto num_matrix_cols = args[0].dimensions()[1];
                auto value_dimnum = args[4].num_dimensions();

                if (step_col == 0)
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "phylanx::execution_tree::primitives::"
                        "set_operation::set_operation",
                        "step can not be zero");
                }

                auto init_list_row = create_list_row_set(
                    row_start, row_stop, step_row, num_matrix_rows);

                auto sm = blaze::rows(args[0].matrix(), init_list_row);

                if (value_dimnum == 0)
                {
                    blaze::DynamicMatrix<double> data(
                        sm.rows(), sm.columns(), args[4].scalar());
                    sm = data;
                    return primitive_argument_type{ir::node_data<double>(0)};
                }

                if (value_dimnum == 1)
                {
                    auto data = blaze::trans(args[4].vector());
                    std::size_t data_size = data.size();
                    std::size_t num_cols = sm.columns();
                    std::size_t num_rows = sm.rows();
                    blaze::DynamicMatrix<double> temp(sm.rows(), sm.columns());

                    if (data_size != num_cols)
                    {
                        HPX_THROW_EXCEPTION(hpx::bad_parameter,
                            "phylanx::execution_tree::primitives::"
                            "row_set_operation::row_set2d",
                            "matrix/vector size mismatch");
                    }

                    for (std::size_t j = 0; j < num_rows; j++)
                    {
                        blaze::row(temp, j) = data;
                    }

                    sm = temp;

                    return primitive_argument_type{ir::node_data<double>(0)};
                }

                auto data = args[4].matrix();
                auto data_rows = data.rows();
                auto data_cols = data.columns();
                auto num_cols = sm.columns();
                auto num_rows = sm.rows();
                if (data_rows != num_rows || data_cols != num_cols)
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "phylanx::execution_tree::primitives::"
                        "set_operation::set_operation",
                        "matrix size mismatch");
                }
                blaze::DynamicMatrix<double> temp(data);
                sm = temp;
                return primitive_argument_type{ir::node_data<double>(0)};
            }

        public:
            hpx::future<primitive_argument_type> eval(
                std::vector<primitive_argument_type> const& operands,
                std::vector<primitive_argument_type> const& args) const
            {
                if (operands.size() != 5)
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "phylanx::execution_tree::primitives::"
                        "row_set_operation::row_set_operation",
                        "the row_set_operation primitive requires five "
                        "arguments");
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
                        "row_set_operation::eval",
                        "the row_set_operation primitive requires that the "
                        "arguments given by the operands array are valid");
                }

                auto this_ = this->shared_from_this();
                return hpx::dataflow(
                    hpx::util::unwrapping(
                        [this_](args_type&& args) -> primitive_argument_type {
                            std::size_t matrix_dims = args[0].num_dimensions();
                            switch (matrix_dims)
                            {
                            case 0:
                                return this_->row_set0d(std::move(args));

                            case 1:
                                return this_->row_set1d(std::move(args));

                            case 2:
                                return this_->row_set2d(std::move(args));

                            default:
                                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                                    "row_set_operation::eval",
                                    "left hand side operand has unsupported "
                                    "number of dimensions");
                            }
                        }),
                    detail::map_operands(
                        operands, functional::numeric_operand{}, args));
            }
        };
    }

    hpx::future<primitive_argument_type> row_set_operation::eval(
        std::vector<primitive_argument_type> const& args) const
    {
        if (operands_.empty())
        {
            return std::make_shared<detail::set_row>()->eval(args, noargs);
        }

        return std::make_shared<detail::set_row>()->eval(operands_, args);
    }
}}}
