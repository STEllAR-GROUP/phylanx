// Copyright (c) 2018-2019 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/ir/node_data.hpp>
#include <phylanx/plugins/keras_support/hard_sigmoid_operation.hpp>

#include <hpx/include/lcos.hpp>
#include <hpx/include/naming.hpp>
#include <hpx/include/util.hpp>
#include <hpx/throw_exception.hpp>

#include <algorithm>
#include <cmath>
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
    match_pattern_type const hard_sigmoid_operation::match_data =
    {
        hpx::util::make_tuple("hard_sigmoid",
            std::vector<std::string>{"hard_sigmoid(_1)"},
            &create_hard_sigmoid_operation, &create_primitive<hard_sigmoid_operation>,
            R"(a
            Args:

                a (array_like) : input array

            Returns:

            Returns an array of the same shape which is the Segment-wise linear
            approximation of the sigmoid function.)")
    };

    ///////////////////////////////////////////////////////////////////////////
    hard_sigmoid_operation::hard_sigmoid_operation(
        primitive_arguments_type&& operands, std::string const& name,
        std::string const& codename)
      : primitive_component_base(std::move(operands), name, codename)
    {}

    ///////////////////////////////////////////////////////////////////////////
    namespace detail
    {
        template <typename T>
        T make_uniform(T val, double)
        {
            return val;
        }

        template <typename T, bool AF, bool PF, bool TF, typename RT>
        blaze::UniformVector<T> make_uniform(
            T val, blaze::CustomVector<T, AF, PF, TF, RT> const& v)
        {
            return blaze::UniformVector<T>(v.size(), val);
        }

        template <typename T, bool AF, bool PF, bool SO, typename RT>
        blaze::UniformMatrix<T> make_uniform(
            T val, blaze::CustomMatrix<T, AF, PF, SO, RT> const& m)
        {
            return blaze::UniformMatrix<T>(m.rows(), m.columns(), val);
        }

#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
        template <typename T, bool AF, bool PF, typename RT>
        blaze::UniformTensor<T> make_uniform(
            T val, blaze::CustomTensor<T, AF, PF, RT> const& t)
        {
            return blaze::UniformTensor<T>(
                t.pages(), t.rows(), t.columns(), val);
        }
#endif

        template <typename Ones, typename Zeros, typename Fifth, typename Halfs,
            typename Data>
        decltype(auto) hard_sigmoid(Ones const& ones, Zeros const& zeros,
            Fifth const& fifth, Halfs const& halfs, Data const& d)
        {
            return (blaze::max)(zeros, (blaze::min)(ones, d * fifth + halfs));
        }

        template <typename Ones, typename Zeros, typename Fifth, typename Halfs,
            typename Data>
        decltype(auto) hard_sigmoid_nd(Ones const& ones, Zeros const& zeros,
            Fifth const& fifth, Halfs const& halfs, Data const& d)
        {
            return (blaze::max)(zeros, (blaze::min)(ones, d % fifth + halfs));
        }
    }

    ///////////////////////////////////////////////////////////////////////////
    primitive_argument_type hard_sigmoid_operation::hard_sigmoid0d(arg_type&& arg) const
    {
        return primitive_argument_type{
            detail::hard_sigmoid(1.0, 0.0, 0.2, 0.5, arg.scalar())};
    }

    ///////////////////////////////////////////////////////////////////////////
    primitive_argument_type hard_sigmoid_operation::hard_sigmoid1d(arg_type&& arg) const
    {
        auto v = arg.vector();

        auto ones = detail::make_uniform(1.0, v);
        auto zeros = detail::make_uniform(0.0, v);
        auto fifth = detail::make_uniform(0.2, v);
        auto halfs = detail::make_uniform(0.5, v);

        if (!arg.is_ref())
        {
            arg.vector() = detail::hard_sigmoid(ones, zeros, fifth, halfs, v);
        }
        else
        {
            arg = detail::hard_sigmoid(ones, zeros, fifth, halfs, v);
        }

        return primitive_argument_type{std::move(arg)};
    }

    ///////////////////////////////////////////////////////////////////////////
    primitive_argument_type hard_sigmoid_operation::hard_sigmoid2d(arg_type&& arg) const
    {
        auto m = arg.matrix();

        auto ones = detail::make_uniform(1.0, m);
        auto zeros = detail::make_uniform(0.0, m);
        auto fifth = detail::make_uniform(0.2, m);
        auto halfs = detail::make_uniform(0.5, m);

        if (!arg.is_ref())
        {
            arg.matrix() = detail::hard_sigmoid_nd(ones, zeros, fifth, halfs, m);
        }
        else
        {
            arg = detail::hard_sigmoid_nd(ones, zeros, fifth, halfs, m);
        }

        return primitive_argument_type{std::move(arg)};
    }

#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
    ///////////////////////////////////////////////////////////////////////////
    primitive_argument_type hard_sigmoid_operation::hard_sigmoid3d(arg_type&& arg) const
    {
        auto t = arg.tensor();

        auto ones = detail::make_uniform(1.0, t);
        auto zeros = detail::make_uniform(0.0, t);
        auto fifth = detail::make_uniform(0.2, t);
        auto halfs = detail::make_uniform(0.5, t);

        if (!arg.is_ref())
        {
            arg.tensor() = detail::hard_sigmoid_nd(ones, zeros, fifth, halfs, t);
        }
        else
        {
            arg = detail::hard_sigmoid_nd(ones, zeros, fifth, halfs, t);
        }

        return primitive_argument_type{std::move(arg)};
    }
#endif

    ///////////////////////////////////////////////////////////////////////////
    hpx::future<primitive_argument_type> hard_sigmoid_operation::eval(
        primitive_arguments_type const& operands,
        primitive_arguments_type const& args, eval_context ctx) const
    {
        if (operands.size() != 1)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "hard_sigmoid_operation::eval",
                generate_error_message(
                    "the hard_sigmoid_operation primitive requires exactly "
                    "one operand"));
        }

        if (!valid(operands[0]))
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "hard_sigmoid_operation::eval",
                generate_error_message(
                    "the hard_sigmoid_operation primitive requires that the "
                    "argument given by the operands array is valid"));
        }

        auto this_ = this->shared_from_this();
        return value_operand(operands[0], args, name_, codename_, std::move(ctx))
            .then(hpx::launch::sync,
                [this_ = std::move(this_)](
                        hpx::future<primitive_argument_type>&& f)
                -> primitive_argument_type
                {
                    // Extract the argument, the result should always be double
                    arg_type a = extract_numeric_value(
                        f.get(), this_->name_, this_->codename_);

                    std::size_t a_dims = a.num_dimensions();
                    switch (a_dims)
                    {
                    case 0:
                        return this_->hard_sigmoid0d(std::move(a));

                    case 1:
                        return this_->hard_sigmoid1d(std::move(a));

                    case 2:
                        return this_->hard_sigmoid2d(std::move(a));

    #if defined(PHYLANX_HAVE_BLAZE_TENSOR)
                    case 3:
                        return this_->hard_sigmoid3d(std::move(a));
    #endif
                    default:
                        HPX_THROW_EXCEPTION(hpx::bad_parameter,
                            "hard_sigmoid_operation::eval",
                            this_->generate_error_message(
                                "operand a has an invalid number of "
                                "dimensions"));
                    }
                });
    }
}}}

