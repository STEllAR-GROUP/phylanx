// Copyright (c) 2017-2018 Bibek Wagle
// Copyright (c) 2017-2018 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/row_slicing.hpp>
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
#include <blaze/math/Elements.h>

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace execution_tree { namespace primitives
{
    ///////////////////////////////////////////////////////////////////////////
    primitive create_row_slicing_operation(hpx::id_type const& locality,
        std::vector<primitive_argument_type>&& operands,
            std::string const& name, std::string const& codename)
    {
        static std::string type("slice_row");
        return create_primitive_component(
            locality, type, std::move(operands), name, codename);
    }

    match_pattern_type const row_slicing_operation::match_data =
    {
        hpx::util::make_tuple("slice_row",
            std::vector<std::string>{"slice_row(_1, _2, _3)",
            "slice_row(_1,_2,_3,_4)"},
            &create_row_slicing_operation,
            &create_primitive<row_slicing_operation>)
    };

    ///////////////////////////////////////////////////////////////////////////
    row_slicing_operation::row_slicing_operation(
            std::vector<primitive_argument_type>&& operands,
            std::string const& name, std::string const& codename)
      : primitive_component_base(std::move(operands), name, codename)
    {}

    ///////////////////////////////////////////////////////////////////////////
    std::vector<int> row_slicing_operation::create_list_row(
        int start, int stop, int step, int array_length) const
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
                    "row_slicing_operation::create_list_row",
                execution_tree::generate_error_message(
                    "slicing will produce empty result, please "
                        "check your parameters",
                    name_, codename_));
        }
        return result;
    }

    primitive_argument_type row_slicing_operation::row_slicing0d(args_type&& args) const
    {
        return primitive_argument_type{std::move(args[0])};
    }

    primitive_argument_type row_slicing_operation::row_slicing1d(args_type&& args) const
    {
        auto row_start = args[1].scalar();
        auto row_stop = args[2].scalar();
        int step = 1;

        if (args.size() == 4)
        {
            step = args[3].scalar();

            if (step == 0)
            {
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "phylanx::execution_tree::primitives::"
                        "row_slicing_operation::row_slicing_operation",
                    execution_tree::generate_error_message(
                        "step can not be zero",
                        name_, codename_));
            }
        }

        auto init_list =
            create_list_row(row_start, row_stop, step, args[0].size());

        auto sv = blaze::elements(args[0].vector(), init_list);

        if (sv.size() == 1)
        {
            return primitive_argument_type{sv[0]};
        }

        storage1d_type v{sv};
        return primitive_argument_type{
            ir::node_data<double>(std::move(v))};
    }

    primitive_argument_type row_slicing_operation::row_slicing2d(args_type&& args) const
    {
        auto row_start = args[1].scalar();
        auto row_stop = args[2].scalar();
        auto num_matrix_rows = args[0].dimensions()[0];
        auto num_matrix_cols = args[0].dimensions()[1];

        int step = 1;

        if (args.size() == 4)
        {
            step = args[3].scalar();
            if (step == 0)
            {
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "phylanx::execution_tree::primitives::"
                        "row_slicing_operation::row_slicing_operation",
                    execution_tree::generate_error_message(
                        "step can not be zero",
                        name_, codename_));
            }
        }

        auto init_list =
            create_list_row(row_start, row_stop, step, num_matrix_rows);

        auto sm = blaze::rows(args[0].matrix(), init_list);

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

        storage2d_type m{sm};

        return primitive_argument_type{
            ir::node_data<double>(std::move(m))};
    }

    hpx::future<primitive_argument_type> row_slicing_operation::eval(
        std::vector<primitive_argument_type> const& operands,
        std::vector<primitive_argument_type> const& args) const
    {
        if (operands.size() != 3 && operands.size() !=4)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "phylanx::execution_tree::primitives::"
                    "row_slicing_operation::row_slicing_operation",
                execution_tree::generate_error_message(
                    "the row_slicing_operation primitive requires "
                        "either three or four arguments",
                    name_, codename_));
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
                "row_slicing_operation::eval",
                execution_tree::generate_error_message(
                    "the row_slicing_operation primitive requires "
                        "that the arguments given by the operands "
                        "array are valid",
                    name_, codename_));
        }

        auto this_ = this->shared_from_this();
        return hpx::dataflow(hpx::util::unwrapping(
            [this_](args_type&& args) -> primitive_argument_type
            {
                std::size_t matrix_dims = args[0].num_dimensions();
                switch (matrix_dims)
                {
                case 0:
                    return this_->row_slicing0d(std::move(args));

                case 1:
                    return this_->row_slicing1d(std::move(args));

                case 2:
                    return this_->row_slicing2d(std::move(args));

                default:
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "row_slicing_operation::eval",
                        execution_tree::generate_error_message(
                            "left hand side operand has unsupported "
                                "number of dimensions",
                            this_->name_, this_->codename_));
                }
            }),
            detail::map_operands(
                operands, functional::numeric_operand{}, args,
                name_, codename_));
    }

    //////////////////////////////////////////////////////////////////////////
    hpx::future<primitive_argument_type> row_slicing_operation::eval(
        std::vector<primitive_argument_type> const& args) const
    {
        if (operands_.empty())
        {
            return eval(args, noargs);
        }
        return eval(operands_, args);
    }
}}}
