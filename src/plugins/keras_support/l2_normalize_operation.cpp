// Copyright (c) 2019 Bita Hasheminezhad
// Copyright (c) 2019 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/ir/node_data.hpp>
#include <phylanx/plugins/keras_support/l2_normalize_operation.hpp>

#include <hpx/include/lcos.hpp>
#include <hpx/include/naming.hpp>
#include <hpx/include/util.hpp>
#include <hpx/throw_exception.hpp>

#include <algorithm>
#include <cstddef>
#include <cstdint>
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
    match_pattern_type const l2_normalize_operation::match_data =
    {
        hpx::util::make_tuple("l2_normalize",
        std::vector<std::string>{"l2_normalize(_1)","l2_normalize(_1,_2)"},
        &create_l2_normalize_operation, &create_primitive<l2_normalize_operation>,
        R"(a, axis
        Args:

            a (array_like) : input array
            axis (optional, integer): axis along which to perform normalization
                The default value is None.

        Returns:

        Normalizes an array with respect to the L2 norm alongside the specified
        axis.)")
    };

    ///////////////////////////////////////////////////////////////////////////
    l2_normalize_operation::l2_normalize_operation(primitive_arguments_type&& operands,
        std::string const& name, std::string const& codename)
      : primitive_component_base(std::move(operands), name, codename)
    {}

    ///////////////////////////////////////////////////////////////////////////
    primitive_argument_type l2_normalize_operation::l2_normalize0d() const
    {
        return primitive_argument_type{static_cast<double>(1.)};
    }

    ///////////////////////////////////////////////////////////////////////////
    primitive_argument_type l2_normalize_operation::l2_normalize1d(
        arg_type&& arg) const
    {
        auto v = arg.vector();
        if (!arg.is_ref())
        {
            v = v/blaze::l2Norm(v);
            return primitive_argument_type{std::move(arg)};
        }

        blaze::DynamicVector<val_type> result = v / blaze::l2Norm(v);
        return primitive_argument_type{std::move(result)};
    }

    ///////////////////////////////////////////////////////////////////////////
    primitive_argument_type l2_normalize_operation::l2_normalize2d_axis0(
        arg_type&& arg) const
    {
        auto m = arg.matrix();
        if (!arg.is_ref())
        {
            for (std::size_t i = 0; i != m.columns(); ++i)
                blaze::column(m, i) =
                    blaze::column(m, i) / blaze::l2Norm(blaze::column(m, i));

            return primitive_argument_type{std::move(arg)};
        }

        blaze::DynamicMatrix<val_type> result(m.rows(), m.columns());
        for (std::size_t i = 0; i != m.columns(); ++i)
            blaze::column(result, i) =
                blaze::column(m, i) / blaze::l2Norm(blaze::column(m, i));

        return primitive_argument_type{std::move(result)};
    }

    primitive_argument_type l2_normalize_operation::l2_normalize2d_axis1(
        arg_type&& arg) const
    {
        auto m = arg.matrix();
        if (!arg.is_ref())
        {
            for (std::size_t i = 0; i != m.rows(); ++i)
                blaze::row(m, i) =
                    blaze::row(m, i) / blaze::l2Norm(blaze::row(m, i));

            return primitive_argument_type{std::move(arg)};
        }

        blaze::DynamicMatrix<val_type> result(m.rows(), m.columns());
        for (std::size_t i = 0; i != m.rows(); ++i)
            blaze::row(result, i) =
                blaze::row(m, i) / blaze::l2Norm(blaze::row(m, i));

        return primitive_argument_type{std::move(result)};
    }

    primitive_argument_type l2_normalize_operation::l2_normalize2d_flatten(
        arg_type&& arg) const
    {
        auto m = arg.matrix();
        if (!arg.is_ref())
        {
            m = m / blaze::l2Norm(m);
            return primitive_argument_type{std::move(arg)};
        }

        blaze::DynamicMatrix<val_type> result = m / blaze::l2Norm(m);
        return primitive_argument_type{std::move(result)};
    }

    primitive_argument_type l2_normalize_operation::l2_normalize2d(
        arg_type&& arg, std::int64_t axis) const
    {
        switch (axis)
        {
        case -2: HPX_FALLTHROUGH;
        case 0:
            return l2_normalize2d_axis0(std::move(arg));

        case -1: HPX_FALLTHROUGH;
        case 1:
            return l2_normalize2d_axis1(std::move(arg));

        default:
            break;
        }
        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "l2_normalize_operation::l2_normalize2d",
            generate_error_message(
                "the l2_normalize_operation primitive requires operand axis "
                "to be between -2 and 1 for matrices."));
    }

    ///////////////////////////////////////////////////////////////////////////
#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
    primitive_argument_type l2_normalize_operation::l2_normalize3d_axis0(
        arg_type&& arg) const
    {
        auto t = arg.tensor();
        if (!arg.is_ref())
        {
            for (std::size_t j = 0; j != t.rows(); ++j)
            {
                auto slice = blaze::rowslice(t, j);
                for (std::size_t i = 0; i != t.columns(); ++i)
                    blaze::row(slice, i) = blaze::row(slice, i) /
                        blaze::l2Norm(blaze::row(slice, i));
            }
            return primitive_argument_type{std::move(arg)};
        }

        blaze::DynamicTensor<val_type> result(t.pages(), t.rows(), t.columns());
        for (std::size_t j = 0; j != t.rows(); ++j)
        {
            auto slice = blaze::rowslice(result, j);
            auto t_slice = blaze::rowslice(t, j);

            for (std::size_t i = 0; i != t.columns(); ++i)
                blaze::row(slice, i) = blaze::row(t_slice, i) /
                    blaze::l2Norm(blaze::row(t_slice, i));
        }
        return primitive_argument_type{std::move(result)};
    }

    primitive_argument_type l2_normalize_operation::l2_normalize3d_axis1(
        arg_type&& arg) const
    {
        auto t = arg.tensor();
        if (!arg.is_ref())
        {
            for (std::size_t j = 0; j != t.pages(); ++j)
            {
                auto slice = blaze::pageslice(t, j);
                for (std::size_t i = 0; i != t.columns(); ++i)
                    blaze::column(slice, i) = blaze::column(slice, i) /
                        blaze::l2Norm(blaze::column(slice, i));
            }
            return primitive_argument_type{std::move(arg)};
        }

        blaze::DynamicTensor<val_type> result(t.pages(), t.rows(), t.columns());
        for (std::size_t j = 0; j != t.pages(); ++j)
        {
            auto slice = blaze::pageslice(result, j);
            auto t_slice = blaze::pageslice(t, j);
            for (std::size_t i = 0; i != t.columns(); ++i)
                blaze::column(slice, i) = blaze::column(t_slice, i) /
                    blaze::l2Norm(blaze::column(t_slice, i));
        }
        return primitive_argument_type{std::move(result)};
    }

    primitive_argument_type l2_normalize_operation::l2_normalize3d_axis2(
        arg_type&& arg) const
    {
        auto t = arg.tensor();
        if (!arg.is_ref())
        {
            for (std::size_t j = 0; j != t.pages(); ++j)
            {
                auto slice = blaze::pageslice(t, j);
                for (std::size_t i = 0; i != t.rows(); ++i)
                    blaze::row(slice, i) = blaze::row(slice, i) /
                        blaze::l2Norm(blaze::row(slice, i));
            }
            return primitive_argument_type{std::move(arg)};
        }

        blaze::DynamicTensor<val_type> result(t.pages(), t.rows(), t.columns());
        for (std::size_t j = 0; j != t.pages(); ++j)
        {
            auto slice = blaze::pageslice(result, j);
            auto t_slice = blaze::pageslice(t, j);
            for (std::size_t i = 0; i != t.rows(); ++i)
                blaze::row(slice, i) = blaze::row(t_slice, i) /
                    blaze::l2Norm(blaze::row(t_slice, i));
        }

        return primitive_argument_type{std::move(result)};
    }

    primitive_argument_type l2_normalize_operation::l2_normalize3d_flatten(
        arg_type&& arg) const
    {
        auto t = arg.tensor();
    //    if (!arg.is_ref())
    //    {
    //        t = t / blaze::l2Norm(t);
            return primitive_argument_type{std::move(arg)};
    //    }

    //    blaze::DynamicMatrix<val_type> result = t / blaze::l2Norm(t);
    //    return primitive_argument_type{std::move(result)};
    }

    primitive_argument_type l2_normalize_operation::l2_normalize3d(
        arg_type&& arg, std::int64_t axis) const
    {
        switch (axis)
        {
        case -3: HPX_FALLTHROUGH;
        case 0:
            return l2_normalize3d_axis0(std::move(arg));

        case -2: HPX_FALLTHROUGH;
        case 1:
            return l2_normalize3d_axis1(std::move(arg));

        case -1: HPX_FALLTHROUGH;
        case 2:
            return l2_normalize3d_axis2(std::move(arg));

        default:
            break;
        }
        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "l2_normalize_operation::l2_normalize3d",
            generate_error_message(
                "the l2_normalize_operation primitive requires operand axis "
                "to be between -3 and 2 for tensors."));
    }
#endif

    ///////////////////////////////////////////////////////////////////////////
    hpx::future<primitive_argument_type> l2_normalize_operation::eval(
        primitive_arguments_type const& operands,
        primitive_arguments_type const& args, eval_context ctx) const
    {
        if (operands.empty() || operands.size() > 2)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "l2_normalize_operation::eval",
                util::generate_error_message(
                    "the l2_normalize_operation primitive requires one, or "
                    "two operands",
                    name_, codename_));
        }

        if (!valid(operands[0]))
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "l2_normalize_operation::eval",
                util::generate_error_message(
                    "the l2_normalize_operation primitive requires that the "
                    "arguments given by the operands array are valid",
                    name_, codename_));
        }

        auto this_ = this->shared_from_this();
        return hpx::dataflow(hpx::launch::sync, hpx::util::unwrapping(
            [this_ = std::move(this_)](primitive_arguments_type&& args)
            -> primitive_argument_type
            {
                // Extract the array, the result should always be double
                arg_type a = extract_numeric_value(
                    std::move(args[0]), this_->name_, this_->codename_);

                std::size_t a_dims = a.num_dimensions();

                if (args.size() == 2 && valid(args[1]))
                {
                    std::int64_t axis = extract_scalar_integer_value_strict(
                        std::move(args[1]), this_->name_, this_->codename_);

                    switch (a_dims)
                    {
                    case 0:
                        return this_->l2_normalize0d();

                    case 1:
                        return this_->l2_normalize1d(std::move(a));

                    case 2:
                        return this_->l2_normalize2d(std::move(a), axis);

    #if defined(PHYLANX_HAVE_BLAZE_TENSOR)
                    case 3:
                        return this_->l2_normalize3d(std::move(a), axis);
    #endif
                    default:
                        HPX_THROW_EXCEPTION(hpx::bad_parameter,
                            "l2_normalize_operation::eval",
                            this_->generate_error_message(
                                "operand a has an invalid number of dimensions"));
                    }
                }

                // no axis is given or axis=None
                switch (a_dims)
                {
                case 0:
                    return this_->l2_normalize0d();

                case 1:
                    return this_->l2_normalize1d(std::move(a));

                case 2:
                    return this_->l2_normalize2d_flatten(std::move(a));

    #if defined(PHYLANX_HAVE_BLAZE_TENSOR)
                case 3:
                    return this_->l2_normalize3d_flatten(std::move(a));
    #endif
                default:
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "l2_normalize_operation::eval",
                        this_->generate_error_message(
                            "operand a has an invalid number of dimensions"));
                }
            }),
            detail::map_operands(operands, functional::value_operand{},
                args, name_, codename_, std::move(ctx)));
    }
}}}

