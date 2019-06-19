// Copyright (c) 2019 Shahrzad Shirzad
// Copyright (c) 2019 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/node_data_helpers.hpp>
#include <phylanx/ir/node_data.hpp>
#include <phylanx/plugins/matrixops/pad.hpp>
#include <phylanx/util/detail/numeric_limits_min.hpp>
#include <phylanx/util/matrix_iterators.hpp>

#include <hpx/include/lcos.hpp>
#include <hpx/include/naming.hpp>
#include <hpx/include/util.hpp>
#include <hpx/throw_exception.hpp>
#include <hpx/util/iterator_facade.hpp>

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
namespace phylanx { namespace execution_tree { namespace primitives {
    ///////////////////////////////////////////////////////////////////////////
    match_pattern_type const pad::match_data = {hpx::util::make_tuple("pad",
        std::vector<std::string>{"pad(_1, _2, _3)", "pad(_1, _2, _3, _4)"},
        &create_pad, &create_primitive<pad>, R"(
            a, pad_width, mode, constant_values
            Args:

                a (array_like of rank N) : array containing elements to pad
                pad_width(sequence, array_like, int) : number of values padded to the
                                                       edges of each axis

                mode(str) : 'constant' pads with a constant value
                            (other modes have not been implemented yet)
                constant_values(sequence or int) :  Used in 'constant'.
                                                    The values to set the padded
                                                    values for each axis.
                                                    Default is 0

            Returns:

            Padded array of rank equal to array with shape increased according to
            pad_width."
            )")};

    ///////////////////////////////////////////////////////////////////////////
    pad::pad(primitive_arguments_type&& operands, std::string const& name,
        std::string const& codename)
      : primitive_component_base(std::move(operands), name, codename)
    {
    }

    ///////////////////////////////////////////////////////////////////////////
    template <typename T>
    primitive_argument_type pad::pad_1d(ir::node_data<T>&& arg,
        ir::node_data<std::int64_t>&& width, T&& value) const
    {
        auto v = arg.vector();
        auto w = width.vector();

        blaze::DynamicVector<T> result(w[0] + v.size() + w[1], 0);
        blaze::subvector(result, 0, w[0]) = value;
        blaze::subvector(result, w[0], v.size()) = v;
        blaze::subvector(result, w[0] + v.size(), w[1]) = value;

        return primitive_argument_type{result};
    }

    template <typename T>
    primitive_argument_type pad::pad_2d(ir::node_data<T>&& arg,
        ir::node_data<std::int64_t>&& width, T&& value) const
    {
        auto m = arg.matrix();
        auto w = width.matrix();

        blaze::DynamicMatrix<T> result(m.rows() + w(0, 0) + w(0, 1),
            m.columns() + w(1, 0) + w(1, 1), value);

        blaze::submatrix(result, w(0, 0), w(1, 0), m.rows(), m.columns()) = m;

        return primitive_argument_type{result};
    }

#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
    template <typename T>
    primitive_argument_type pad::pad_3d(ir::node_data<T>&& arg,
        ir::node_data<std::int64_t>&& width, T&& value) const
    {
        auto m = arg.tensor();
        auto w = width.matrix();

        blaze::DynamicTensor<T> result(m.pages() + w(0, 0) + w(0, 1),
            m.rows() + w(1, 0) + w(1, 1), m.columns() + w(2, 0) + w(2, 1),
            value);

        blaze::subtensor(result, w(0, 0), w(1, 0), w(2, 0), m.pages(), m.rows(),
            m.columns()) = m;

        return primitive_argument_type{result};
    }
#endif

    template <typename T>
    primitive_argument_type pad::pad_helper(ir::node_data<T>&& arg,
        ir::node_data<std::int64_t>&& width, ir::node_data<T>&& values) const
    {
        if (values.num_dimensions() != 0)
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "pad::pad_helper",
                generate_error_message(
                    "the current implementation of the pad primitive "
                    "requires the constant_values argument to be a scalar"));

        std::size_t ndim = arg.num_dimensions();
        switch (ndim)
        {
        case 1:
            return pad_1d<T>(
                std::move(arg), std::move(width), std::move(values.scalar()));
        case 2:
            return pad_2d<T>(
                std::move(arg), std::move(width), std::move(values.scalar()));
#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
        case 3:
            return pad_3d<T>(
                std::move(arg), std::move(width), std::move(values.scalar()));
#endif
        default:
            break;
        }
        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "pad::pad_helper",
            generate_error_message("unsupported number of dimensions"));
    }

    primitive_argument_type pad::get_array(
        primitive_arguments_type& args, std::size_t&& ndim) const
    {
        if (!valid(args[0]))
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "pad::pad_get_array",
                generate_error_message("syntax error"));

        if (args.size() == 1 && !is_list_operand_strict(std::move(args[0])))
            return std::move(args[0]);

        if (!valid(args[1]))
        {
            if (args.size() == 2)
            {
                //(constant, ) is equivalent to (constant, constant),...

                if (is_integer_operand_strict(args[0]))
                {
                    blaze::DynamicVector<std::int64_t> v(2,
                        extract_scalar_integer_value_strict(
                            std::move(args[0]), name_, codename_));
                    return primitive_argument_type{v};
                }
                else
                {
                    //((before_1, after_1), ) is equivalent to
                    // (before_1, after_1),...,(before_1, after_1)
                    if (is_list_operand_strict(args[0]))
                    {
                        auto w = extract_list_value_strict(
                            std::move(args[0]), name_, codename_);
                        if (w.size() != 2)
                            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                                "pad::pad_get_array",
                                generate_error_message(
                                    "Unable to create correctly shaped array "
                                    "from pad width argument"));

                        else
                        {
                            blaze::DynamicVector<std::int64_t> v(2, 0);
                            auto it = w.begin();
                            //(((before_1, after_1), )), ((((before_1, after_1), ))), ...
                            // has not been implemented yet
                            v[0] = extract_scalar_integer_value_strict(*it++);
                            v[1] = extract_scalar_integer_value_strict(*it);
                            return primitive_argument_type{std::move(v)};
                        }
                    }
                    else
                        HPX_THROW_EXCEPTION(hpx::bad_parameter,
                            "pad::pad_get_array",
                            generate_error_message("syntax error"));
                }
            }
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "pad::pad_get_array",
                generate_error_message("syntax error"));
        }

        if (args.size() == 2 &&
            (!is_list_operand_strict(std::move(args[0])) &&
                !is_list_operand_strict(std::move(args[1]))))
        {
            blaze::DynamicVector<std::int64_t> v(2, 0);
            v[0] = extract_scalar_integer_value_strict(
                std::move(args[0]), name_, codename_);
            v[1] = extract_scalar_integer_value_strict(
                std::move(args[1]), name_, codename_);
            return primitive_argument_type{std::move(v)};
        }

        if (args.size() != ndim)
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "pad::pad_get_array",
                generate_error_message("syntax error"));

        blaze::DynamicMatrix<std::int64_t> m(ndim, 2);
        std::size_t i = 0;
        for (auto&& w : args)
        {
            if (!is_list_operand_strict(w))
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "pad::pad_get_array",
                    generate_error_message("syntax error"));
            auto d = extract_list_value_strict(std::move(w), name_, codename_);
            if (d.size() != 2)
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "pad::pad_get_array",
                    generate_error_message("syntax error"));
            auto it = d.begin();
            if (is_list_operand_strict(*it))
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "pad::pad_get_array",
                    generate_error_message("syntax error"));
            m(i, 0) = extract_scalar_integer_value_strict(std::move(*it));
            if (is_list_operand_strict(*++it))
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "pad::pad_get_array",
                    generate_error_message("syntax error"));
            m(i, 1) = extract_scalar_integer_value_strict(std::move(*it));
            i++;
        }
        return primitive_argument_type{std::move(m)};
    }

    primitive_argument_type pad::get_array(
        primitive_arguments_type&& args, std::size_t&& ndim) const
    {
        if (!valid(args[0]))
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "pad::pad_get_array",
                generate_error_message("syntax error"));

        if (args.size() == 1 && !is_list_operand_strict(std::move(args[0])))
            return std::move(args[0]);

        if (!valid(args[1]))
        {
            if (args.size() == 2)
            {
                //(constant, ) is equivalent to (constant, constant),...
                if (is_integer_operand_strict(args[0]))
                {
                    blaze::DynamicVector<std::int64_t> v(2,
                        extract_scalar_integer_value_strict(
                            std::move(args[0]), name_, codename_));
                    return primitive_argument_type{v};
                }
                else
                {
                    //((before_1, after_1), ) is equivalent to
                    // (before_1, after_1),...,(before_1, after_1)
                    if (is_list_operand_strict(args[0]))
                    {
                        auto w = extract_list_value_strict(
                            std::move(args[0]), name_, codename_);
                        if (w.size() != 2)
                            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                                "pad::pad_get_array",
                                generate_error_message(
                                    "Unable to create correctly shaped array "
                                    "from pad width argument"));

                        else
                        {
                            blaze::DynamicVector<std::int64_t> v(2, 0);
                            auto it = w.begin();
                            //(((before_1, after_1), )), ((((before_1, after_1), ))), ...
                            // has not been implemented yet
                            v[0] = extract_scalar_integer_value_strict(*it++);
                            v[1] = extract_scalar_integer_value_strict(*it);
                            return primitive_argument_type{std::move(v)};
                        }
                    }
                    else
                        HPX_THROW_EXCEPTION(hpx::bad_parameter,
                            "pad::pad_get_array",
                            generate_error_message("syntax error"));
                }
            }
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "pad::pad_get_array",
                generate_error_message("syntax error"));
        }

        if (args.size() == 2 &&
            (!is_list_operand_strict(std::move(args[0])) &&
                !is_list_operand_strict(std::move(args[1]))))
        {
            blaze::DynamicVector<std::int64_t> v(2, 0);
            v[0] = extract_scalar_integer_value_strict(
                std::move(args[0]), name_, codename_);
            v[1] = extract_scalar_integer_value_strict(
                std::move(args[1]), name_, codename_);
            return primitive_argument_type{std::move(v)};
        }

        if (args.size() != ndim)
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "pad::pad_get_array",
                generate_error_message("syntax error"));

        blaze::DynamicMatrix<std::int64_t> m(ndim, 2);
        std::size_t i = 0;
        for (auto&& w : args)
        {
            if (!is_list_operand_strict(w))
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "pad::pad_get_array",
                    generate_error_message("syntax error"));
            auto d = extract_list_value_strict(std::move(w), name_, codename_);
            if (d.size() != 2)
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "pad::pad_get_array",
                    generate_error_message("syntax error"));
            auto it = d.begin();
            if (is_list_operand_strict(*it))
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "pad::pad_get_array",
                    generate_error_message("syntax error"));
            m(i, 0) = extract_scalar_integer_value_strict(std::move(*it++));
            if (is_list_operand_strict(*it))
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "pad::pad_get_array",
                    generate_error_message("syntax error"));
            m(i, 1) = extract_scalar_integer_value_strict(std::move(*it));
            i++;
        }
        return primitive_argument_type{std::move(m)};
    }
    ///////////////////////////////////////////////////////////////////////////
    hpx::future<primitive_argument_type> pad::eval(
        primitive_arguments_type const& operands,
        primitive_arguments_type const& args, eval_context ctx) const
    {
        if (operands.empty() || operands.size() < 3 || operands.size() > 4)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "pad::eval",
                generate_error_message(
                    "the pad primitive requires three or four operands"));
        }

        for (std::size_t i = 0; i != operands.size(); ++i)
        {
            if (!valid(operands[i]))
            {
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "pad::eval",
                    generate_error_message(
                        "the pad primitive requires that all the "
                        "arguments to be valid"));
            }
        }

        auto this_ = this->shared_from_this();
        return hpx::dataflow(hpx::launch::sync, hpx::util::unwrapping(
            [this_ = std::move(this_)](primitive_arguments_type&& args)
            -> primitive_argument_type
            {
                if (!is_string_operand(args[2]))
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "pad::eval",
                        this_->generate_error_message(
                            "the padding mode should be a string"));
                }

                if (extract_string_value(args[2]) != "constant")
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "pad::eval",
                        this_->generate_error_message(
                            "the pad primitive has only been implemented "
                            "for constant mode"));
                }

                std::size_t ndim = extract_numeric_value_dimension(
                    args[0], this_->name_, this_->codename_);

                if (is_list_operand_strict(args[1]))
                {
                    auto w = extract_list_value_strict(
                        std::move(args[1]), this_->name_, this_->codename_);

                    if (w.is_ref())
                    {
                        primitive_arguments_type width_result;
                        width_result = w.copy();
                        args[1] = this_->get_array(
                            std::move(width_result), std::move(ndim));
                    }
                    else
                    {
                        args[1] = this_->get_array(w.args(), std::move(ndim));
                    }
                }

                ir::node_data<std::int64_t> width{0};

                switch (ndim)
                {
                case 0:
                    return std::move(args[0]);

                case 1:
                    width = extract_value_vector<std::int64_t>(
                        std::move(args[1]), 2, this_->name_, this_->codename_);
                    break;

                case 2:
                    width =
                        extract_value_matrix<std::int64_t>(std::move(args[1]),
                            ndim, 2, this_->name_, this_->codename_);
                    break;

#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
                case 3:
                    width =
                        extract_value_matrix<std::int64_t>(std::move(args[1]),
                            ndim, 2, this_->name_, this_->codename_);
                    break;
#endif
                default:
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "pad::pad_helper",
                        this_->generate_error_message(
                            "unsupported number of dimensions"));
                }

                if (args.size() > 3)
                {
                    switch (extract_common_type(args[0]))
                    {
                    case node_data_type_int64:
                        return this_->pad_helper(
                            extract_integer_value_strict(std::move(args[0]),
                                this_->name_, this_->codename_),
                            std::move(width),
                            extract_integer_value_strict(std::move(args[3]),
                                this_->name_,
                                this_->codename_));

                    case node_data_type_bool:
                        return this_->pad_helper(
                            extract_boolean_value_strict(std::move(args[0]),
                                this_->name_, this_->codename_),
                            std::move(width),
                            extract_boolean_value_strict(std::move(args[3]),
                                this_->name_,
                                this_->codename_));

                    case node_data_type_unknown: HPX_FALLTHROUGH;
                    case node_data_type_double:
                        return this_->pad_helper(
                            extract_numeric_value(std::move(args[0]),
                                this_->name_, this_->codename_),
                            std::move(width),
                            extract_numeric_value(
                                std::move(args[3]), this_->codename_));

                    default:
                        break;
                    }
                }
                else
                {
                    switch (extract_common_type(args[0]))
                    {
                    case node_data_type_int64:
                        return this_->pad_helper(
                            extract_integer_value_strict(std::move(args[0]),
                                this_->name_, this_->codename_),
                            std::move(width),
                            ir::node_data<std::int64_t>{0});

                    case node_data_type_bool:
                        return this_->pad_helper(
                            extract_boolean_value_strict(std::move(args[0]),
                                this_->name_, this_->codename_),
                            std::move(width),
                            ir::node_data<std::uint8_t>{0});

                    case node_data_type_unknown: HPX_FALLTHROUGH;
                    case node_data_type_double:
                        return this_->pad_helper(
                            extract_numeric_value(std::move(args[0]),
                                this_->name_, this_->codename_),
                            std::move(width),
                            ir::node_data<double>{0.0});

                    default:
                        break;
                    }
                }
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "pad::eval",
                    this_->generate_error_message(
                        "the pad primitive requires for all arguments to "
                        "be numeric data types"));
            }),
            detail::map_operands(operands, functional::value_operand{},
                args, name_, codename_, std::move(ctx)));
    }
}}}
