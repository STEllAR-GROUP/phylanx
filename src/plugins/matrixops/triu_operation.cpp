// Copyright (c) 2021 Karame M. Shokooh
// Copyright (c) 2021 Hartmut kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/ir/node_data.hpp>
#include <phylanx/plugins/matrixops/triu_operation.hpp>

#include <hpx/errors/throw_exception.hpp>
#include <hpx/include/lcos.hpp>
#include <hpx/include/naming.hpp>
#include <hpx/include/util.hpp>

#include <cmath>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include <blaze/Math.h>

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace execution_tree { namespace primitives
{
    ///////////////////////////////////////////////////////////////////////////
    std::vector<match_pattern_type> const triu_operation::match_data =
    {
        match_pattern_type{"triu",
            std::vector<std::string>{"triu(_1,_2)", "triu(_1)"},
            &create_triu_operation, &create_primitive<triu_operation>, R"(
            a, k
            Args:

                a (array) : a matrix or a tensor
                k (optional, integer) : index of the diagonal: 0 (the default)
                  refers to the main diagonal, a positive value refers to an
                  upper diagonal, and a negative value to a lower diagonal.
                
            Returns:

            Return a copy of an array with the elements below the k-th diagonal zeroed.)"
        },

        match_pattern_type{"tril",
            std::vector<std::string>{"tril(_1,_2)", "tril(_1)"},
            &create_triu_operation, &create_primitive<triu_operation>, R"(
            a, k
            Args:

                a (array) : a matrix or a tensor
                k (optional, integer) : index of the diagonal: 0 (the default)
                  refers to the main diagonal, a positive value refers to an
                  upper diagonal, and a negative value to a lower diagonal.
                
            Returns:

            Return a copy of an array with the elements above the k-th diagonal zeroed.)"
        },     
    };

    ///////////////////////////////////////////////////////////////////////////
    triu_operation::tri_mode extract_tri_mode(std::string const& name)
    {
        triu_operation::tri_mode result = triu_operation::tri_mode_up;

        if (name.find("tril") != std::string::npos)
        {
            result = triu_operation::tri_mode_low;
        }
        return result;
    }

    triu_operation::triu_operation(primitive_arguments_type&& operands,
        std::string const& name, std::string const& codename)
      : primitive_component_base(std::move(operands), name, codename)
      , mode_(extract_tri_mode(name_))
    {
    }
    ///////////////////////////////////////////////////////////////////////////
    template <typename T>
    primitive_argument_type triu_operation::triu2d(
        ir::node_data<T>&& arg, std::int64_t k) const

    {
        auto m = arg.matrix();

        std::int64_t columns = m.columns();
        std::int64_t rows = m.rows();

        if (!arg.is_ref())
        {
            if (k >= columns)
            {
                m = static_cast<T>(0);
                return primitive_argument_type{
                    ir::node_data<T>{std::move(arg)}};
            }
            else if (k <= 1 - rows)
            {
                return primitive_argument_type{
                    ir::node_data<T>{std::move(arg)}};
            }
            for (std::int64_t i = 1 - rows; i != k; ++i)
            {
                blaze::band(m, i) = static_cast<T>(0);
            }
            return primitive_argument_type{ir::node_data<T>{std::move(arg)}};
        }

        blaze::DynamicMatrix<T> result(rows, columns, static_cast<T>(0));

        if (k >= columns)
        {
            return primitive_argument_type{ir::node_data<T>{std::move(result)}};
        }
        else if (k <= 1 - rows)
        {
            result = m;
            return primitive_argument_type{ir::node_data<T>{std::move(result)}};
        }
        for (std::int64_t i = k; i != columns; ++i)
        {
            blaze::band(result, i) = blaze::band(m, i);
        }
        return primitive_argument_type{ir::node_data<T>{std::move(result)}};
    }

    ///////////////////////////////////////////////////////////////////////////
    primitive_argument_type triu_operation::triu2d(
        primitive_argument_type&& arg, std::int64_t k) const
    {
        switch (extract_common_type(arg))
        {
        case node_data_type_bool:
            return triu2d(
                extract_boolean_value_strict(std::move(arg), name_, codename_),
                k);

        case node_data_type_int64:
            return triu2d(
                extract_integer_value_strict(std::move(arg), name_, codename_),
                k);

        case node_data_type_double:
            return triu2d(
                extract_numeric_value_strict(std::move(arg), name_, codename_),
                k);

        case node_data_type_unknown:
            return triu2d(
                extract_numeric_value(std::move(arg), name_, codename_), k);

        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter, "triu_operation::triu2d",
            generate_error_message(
                "the triu primitive requires for all arguments to "
                "be numeric data types"));
    }

    ///////////////////////////////////////////////////////////////////////////
    template <typename T>
    primitive_argument_type triu_operation::triu3d(
        ir::node_data<T>&& arg, std::int64_t k) const

    {
        auto t = arg.tensor();

        std::int64_t columns = t.columns();
        std::int64_t rows = t.rows();
        std::size_t pages = t.pages();

        if (!arg.is_ref())
        {
            if (k >= columns)
            {
                t = static_cast<T>(0);
                return primitive_argument_type{
                    ir::node_data<T>{std::move(arg)}};
            }
            else if (k <= 1 - rows)
            {
                return primitive_argument_type{
                    ir::node_data<T>{std::move(arg)}};
            }
            for (std::size_t p = 0; p != pages; ++p)
            {
                auto slice = blaze::pageslice(t, p);

                for (std::int64_t i = 1 - rows; i != k; ++i)
                {
                    blaze::band(slice, i) = static_cast<T>(0);
                }
            }
            return primitive_argument_type{ir::node_data<T>{std::move(arg)}};
        }

        blaze::DynamicTensor<T> result(pages, rows, columns, static_cast<T>(0));

        if (k >= columns)
        {
            return primitive_argument_type{ir::node_data<T>{std::move(result)}};
        }
        else if (k <= 1 - rows)
        {
            result = t;
            return primitive_argument_type{ir::node_data<T>{std::move(result)}};
        }
        for (std::size_t p = 0; p != pages; ++p)
        {
            auto slice = blaze::pageslice(t, p);
            auto result_slice = blaze::pageslice(result, p);

            for (std::int64_t i = k; i != columns; ++i)
            {
                blaze::band(result_slice, i) = blaze::band(slice, i);
            }
        }
        return primitive_argument_type{ir::node_data<T>{std::move(result)}};
    }

    ///////////////////////////////////////////////////////////////////////////
    primitive_argument_type triu_operation::triu3d(
        primitive_argument_type&& arg, std::int64_t k) const
    {
        switch (extract_common_type(arg))
        {
        case node_data_type_bool:
            return triu3d(
                extract_boolean_value_strict(std::move(arg), name_, codename_),
                k);

        case node_data_type_int64:
            return triu3d(
                extract_integer_value_strict(std::move(arg), name_, codename_),
                k);

        case node_data_type_double:
            return triu3d(
                extract_numeric_value_strict(std::move(arg), name_, codename_),
                k);

        case node_data_type_unknown:
            return triu3d(
                extract_numeric_value(std::move(arg), name_, codename_), k);

        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter, "triu_operation::triu3d",
            generate_error_message(
                "the triu primitive requires for all arguments to "
                "be numeric data types"));
    }

    ///////////////////////////////////////////////////////////////////////////
    template <typename T>
    primitive_argument_type triu_operation::tril2d(
        ir::node_data<T>&& arg, std::int64_t k) const

    {
        auto m = arg.matrix();

        std::int64_t columns = m.columns();
        std::int64_t rows = m.rows();

        if (!arg.is_ref())
        {
            if (k <= -rows)
            {
                m = static_cast<T>(0);
                return primitive_argument_type{
                    ir::node_data<T>{std::move(arg)}};
            }
            else if (k >= columns - 1)
            {
                return primitive_argument_type{
                    ir::node_data<T>{std::move(arg)}};
            }
            for (std::int64_t i = k + 1; i != columns; ++i)
            {
                blaze::band(m, i) = static_cast<T>(0);
            }
            return primitive_argument_type{ir::node_data<T>{std::move(arg)}};
        }

        blaze::DynamicMatrix<T> result(rows, columns, static_cast<T>(0));

        if (k <= -rows)
        {
            return primitive_argument_type{ir::node_data<T>{std::move(result)}};
        }
        else if (k >= columns - 1)
        {
            result = m;
            return primitive_argument_type{ir::node_data<T>{std::move(result)}};
        }
        for (std::int64_t i = k; i != -rows; --i)
        {
            blaze::band(result, i) = blaze::band(m, i);
        }
        return primitive_argument_type{ir::node_data<T>{std::move(result)}};
    }

    ///////////////////////////////////////////////////////////////////////////
    primitive_argument_type triu_operation::tril2d(
        primitive_argument_type&& arg, std::int64_t k) const
    {
        switch (extract_common_type(arg))
        {
        case node_data_type_bool:
            return tril2d(
                extract_boolean_value_strict(std::move(arg), name_, codename_),
                k);

        case node_data_type_int64:
            return tril2d(
                extract_integer_value_strict(std::move(arg), name_, codename_),
                k);

        case node_data_type_double:
            return tril2d(
                extract_numeric_value_strict(std::move(arg), name_, codename_),
                k);

        case node_data_type_unknown:
            return tril2d(
                extract_numeric_value(std::move(arg), name_, codename_), k);

        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter, "triu_operation::tril2d",
            generate_error_message(
                "the tri primitive requires for all arguments to "
                "be numeric data types"));
    }

    ///////////////////////////////////////////////////////////////////////////
    template <typename T>
    primitive_argument_type triu_operation::tril3d(
        ir::node_data<T>&& arg, std::int64_t k) const
    {
        auto t = arg.tensor();

        std::int64_t columns = t.columns();
        std::int64_t rows = t.rows();
        std::size_t pages = t.pages();

        if (!arg.is_ref())
        {
            if (k <= -rows)
            {
                t = static_cast<T>(0);
                return primitive_argument_type{
                    ir::node_data<T>{std::move(arg)}};
            }
            else if (k >= columns - 1)
            {
                return primitive_argument_type{
                    ir::node_data<T>{std::move(arg)}};
            }
            for (std::size_t p = 0; p != pages; ++p)
            {
                auto slice = blaze::pageslice(t, p);

                for (std::int64_t i = k + 1; i != columns; ++i)
                {
                    blaze::band(slice, i) = static_cast<T>(0);
                }
            }
            return primitive_argument_type{ir::node_data<T>{std::move(arg)}};
        }

        blaze::DynamicTensor<T> result(pages, rows, columns, static_cast<T>(0));

        if (k <= -rows)
        {
            return primitive_argument_type{ir::node_data<T>{std::move(result)}};
        }
        else if (k >= columns - 1)
        {
            result = t;
            return primitive_argument_type{ir::node_data<T>{std::move(result)}};
        }
        for (std::size_t p = 0; p != pages; ++p)
        {
            auto slice = blaze::pageslice(t, p);
            auto result_slice = blaze::pageslice(result, p);

            for (std::int64_t i = k; i != -rows; --i)
            {
                blaze::band(result_slice, i) = blaze::band(slice, i);
            }
        }
        return primitive_argument_type{ir::node_data<T>{std::move(result)}};
    }

    ///////////////////////////////////////////////////////////////////////////
    primitive_argument_type triu_operation::tril3d(
        primitive_argument_type&& arg, std::int64_t k) const
    {
        switch (extract_common_type(arg))
        {
        case node_data_type_bool:
            return tril3d(
                extract_boolean_value_strict(std::move(arg), name_, codename_),
                k);

        case node_data_type_int64:
            return tril3d(
                extract_integer_value_strict(std::move(arg), name_, codename_),
                k);

        case node_data_type_double:
            return tril3d(
                extract_numeric_value_strict(std::move(arg), name_, codename_),
                k);

        case node_data_type_unknown:
            return tril3d(
                extract_numeric_value(std::move(arg), name_, codename_), k);

        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter, "triu_operation::tril3d",
            generate_error_message(
                "the tril primitive requires for all arguments to "
                "be numeric data types"));
    }

    ///////////////////////////////////////////////////////////////////////////
    hpx::future<primitive_argument_type> triu_operation::eval(
        primitive_arguments_type const& operands,
        primitive_arguments_type const& args, eval_context ctx) const
    {
        if (operands.empty() || operands.size() > 2)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter, "triu_operation::eval",
                util::generate_error_message(
                    hpx::util::format("the triu_operation primitive can have "
                                      "one or two operands "
                                      "got {}",
                        operands.size()),
                    name_, codename_));
        }

        if (!valid(operands[0]))
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter, "triu_operation::eval",
                util::generate_error_message(
                    "the triu_operation primitive requires that the "
                    "arguments given by the operands array are "
                    "valid",
                    name_, codename_));
        }

        if (operands.size() == 2 && !valid(operands[1]))
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter, "triu_operation::eval",
                util::generate_error_message(
                    "the triu_operation primitive requires that the "
                    "arguments given by the operands array are valid",
                    name_, codename_));
        }

         auto this_ = this->shared_from_this();
         return hpx::dataflow(hpx::launch::sync,
             hpx::util::unwrapping([this_ = std::move(this_)](
                                       primitive_arguments_type&& args)
                                       -> primitive_argument_type {
                 std::size_t ndim = extract_numeric_value_dimension(
                     args[0], this_->name_, this_->codename_);

                 if (ndim != 2 && ndim != 3)
                 {
                     HPX_THROW_EXCEPTION(hpx::bad_parameter,
                         "triu_operation::eval",
                         this_->generate_error_message(
                             "triu operation only accepts matrices or tensors."
                             "given number of dimensions is invalid "));
                 }

                 std::int64_t k = 0;
                 if (args.size() > 1)
                 {
                     k = extract_scalar_integer_value(
                         std::move(args[1]), this_->name_, this_->codename_);
                 }

                 if (this_->mode_ == tri_mode_up)
                 {
                     switch (extract_numeric_value_dimension(
                         args[0], this_->name_, this_->codename_))
                     {
                     case 2:
                         return this_->triu2d(std::move(args[0]), k);

                     case 3:
                         return this_->triu3d(std::move(args[0]), k);

                     default:
                         HPX_THROW_EXCEPTION(hpx::bad_parameter,
                             "triu_operation::eval",
                             this_->generate_error_message(
                                 "This operand has unsupported "
                                 "number of dimensions"));
                     }
                 }

                 switch (extract_numeric_value_dimension(
                     args[0], this_->name_, this_->codename_))
                 {
                 case 2:
                     return this_->tril2d(std::move(args[0]), k);

                 case 3:
                     return this_->tril3d(std::move(args[0]), k);

                 default:
                     HPX_THROW_EXCEPTION(hpx::bad_parameter,
                         "triu_operation::eval",
                         this_->generate_error_message(
                             "This operand has unsupported "
                             "number of dimensions"));
                 }
             }),
             detail::map_operands(std::move(operands),
                 functional::value_operand{}, args, name_, codename_,
                 std::move(ctx)));
    }
}}}
