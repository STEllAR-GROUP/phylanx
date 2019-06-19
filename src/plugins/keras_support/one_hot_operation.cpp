// Copyright (c) 2019 Bita Hasheminezhad
// Copyright (c) 2019 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/ir/node_data.hpp>
#include <phylanx/plugins/keras_support/one_hot_operation.hpp>

#include <hpx/include/lcos.hpp>
#include <hpx/include/naming.hpp>
#include <hpx/include/util.hpp>
#include <hpx/throw_exception.hpp>

#include <cstddef>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include <blaze/Math.h>
#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
#include <blaze_tensor/Math.h>
#endif

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace execution_tree { namespace primitives
{
    ///////////////////////////////////////////////////////////////////////////
    match_pattern_type const one_hot_operation::match_data =
    {
        hpx::util::make_tuple("one_hot",
        std::vector<std::string>{"one_hot(_1,_2)"},
        &create_one_hot_operation, &create_primitive<one_hot_operation>,
        R"(indices, num_classes
        Args:

            indices (array_like) : input array of integers. nD integer tensor
                of shape  (batch_size, dim1, dim2, ... dim(n-1))
            num_classes (integer): number of classes to consider

        Returns:

        (n + 1)D one hot representation of the input with shape (batch_size,
        dim1, dim2, ... dim(n-1), num_classes).)")
    };

    ///////////////////////////////////////////////////////////////////////////
    one_hot_operation::one_hot_operation(primitive_arguments_type&& operands,
        std::string const& name, std::string const& codename)
      : primitive_component_base(std::move(operands), name, codename)
    {}

    ///////////////////////////////////////////////////////////////////////////
    primitive_argument_type one_hot_operation::one_hot0d(
        arg_type&& arg, val_type num_classes) const
    {
        auto a = arg.scalar();
        blaze::DynamicVector<double> result(num_classes, 0.);

        if (a < num_classes)
            result[a] = 1.;

        return primitive_argument_type{std::move(result)};
    }

    ///////////////////////////////////////////////////////////////////////////
    primitive_argument_type one_hot_operation::one_hot1d(
        arg_type&& arg, val_type num_classes) const
    {
        auto v = arg.vector();
        blaze::DynamicMatrix<double> result(v.size(), num_classes, 0.);

        for (std::size_t i = 0; i != v.size(); ++i)
        {
            if (v[i] < num_classes)
                result(i, v[i]) = 1.;
        }

        return primitive_argument_type{std::move(result)};
    }

    ///////////////////////////////////////////////////////////////////////////
#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
    primitive_argument_type one_hot_operation::one_hot2d(
        arg_type&& arg, val_type num_classes) const
    {
        auto m = arg.matrix();
        blaze::DynamicTensor<double> result(
            m.rows(), m.columns(), num_classes, 0.);

        for (std::size_t r = 0; r != m.rows(); ++r)
        {
            auto slice = blaze::pageslice(result, r);
            for (std::size_t c = 0; c != m.columns(); ++c)
            {
                if (m(r, c) < num_classes)
                    slice(c, m(r, c)) = 1.;
            }
        }

        return primitive_argument_type{std::move(result)};
    }
#endif

    ///////////////////////////////////////////////////////////////////////////
    hpx::future<primitive_argument_type> one_hot_operation::eval(
        primitive_arguments_type const& operands,
        primitive_arguments_type const& args, eval_context ctx) const
    {
        if (operands.size() != 2)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "one_hot_operation::eval",
                generate_error_message(
                    "the one_hot_operation primitive requires exactly "
                    "two operands"));
        }

        for (auto const& i : operands)
        {
            if (!valid(i))
            {
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "one_hot_operation::eval",
                    generate_error_message(
                        "the one_hot_operation primitive requires that the "
                        "arguments given by the operands array are valid"));
            }
        }

        auto this_ = this->shared_from_this();
        return hpx::dataflow(hpx::launch::sync, hpx::util::unwrapping(
            [this_ = std::move(this_)](primitive_arguments_type&& args)
            -> primitive_argument_type
            {
                arg_type a = extract_integer_value_strict(
                    std::move(args[0]), this_->name_, this_->codename_);

                std::size_t a_dims = a.num_dimensions();

                val_type num_classes = extract_scalar_integer_value_strict(
                    std::move(args[1]), this_->name_, this_->codename_);

                if (num_classes < 0)
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "one_hot_operation::eval",
                        this_->generate_error_message(
                            "the one_hot operation requires num_classes to be "
                            "non-negative"));
                }

                switch (a_dims)
                {
                case 0:
                    return this_->one_hot0d(std::move(a), num_classes);

                case 1:
                    return this_->one_hot1d(std::move(a), num_classes);

#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
                case 2:
                    return this_->one_hot2d(std::move(a), num_classes);
#endif
                default:
                    break;
                }
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "one_hot_operation::eval",
                    this_->generate_error_message(
                        "operand a has an invalid number of dimensions"));
            }),
            detail::map_operands(operands, functional::value_operand{},
                args, name_, codename_, std::move(ctx)));
    }
}}}

