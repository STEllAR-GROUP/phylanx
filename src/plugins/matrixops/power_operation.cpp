// Copyright (c) 2017-2018 Parsa Amini
// Copyright (c) 2017-2018 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/ir/node_data.hpp>
#include <phylanx/plugins/matrixops/power_operation.hpp>

#include <hpx/include/lcos.hpp>
#include <hpx/include/naming.hpp>
#include <hpx/include/util.hpp>
#include <hpx/throw_exception.hpp>

#include <cmath>
#include <cstddef>
#include <memory>
#include <string>
#include <utility>
#include <vector>

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace execution_tree { namespace primitives
{
    ///////////////////////////////////////////////////////////////////////////
    match_pattern_type const power_operation::match_data =
    {
        match_pattern_type{
            "power",
            std::vector<std::string>{"power(_1, _2)"},
            &create_power_operation, &create_primitive<power_operation>, R"(
            base, pow
            Args:

                base (float) : the base of the exponent
                pow (float) : the power of the exponent

            Returns:

            The value `base`**`pow`.)",
            true
        }
    };

    ///////////////////////////////////////////////////////////////////////////
    power_operation::power_operation(
            primitive_arguments_type&& operands,
            std::string const& name, std::string const& codename)
      : primitive_component_base(std::move(operands), name, codename)
      , dtype_(extract_dtype(name_))
    {}

    ///////////////////////////////////////////////////////////////////////////
    template <typename T>
    primitive_argument_type power_operation::power0d(
        ir::node_data<T>&& lhs, ir::node_data<T>&& rhs) const
    {
        if (lhs.is_ref())
        {
            lhs = T(std::pow(lhs.scalar(), rhs[0]));
        }
        else
        {
            lhs.scalar() = T(std::pow(lhs.scalar(), rhs[0]));
        }
        return primitive_argument_type{std::move(lhs)};
    }

    template <typename T>
    primitive_argument_type power_operation::power1d(
        ir::node_data<T>&& lhs, ir::node_data<T>&& rhs) const
    {
        if (lhs.is_ref())
        {
            lhs = blaze::pow(lhs.vector(), rhs[0]);
        }
        else
        {
            lhs.vector() = blaze::pow(lhs.vector(), rhs[0]);
        }
        return primitive_argument_type{std::move(lhs)};
    }

    template <typename T>
    primitive_argument_type power_operation::power2d(
        ir::node_data<T>&& lhs, ir::node_data<T>&& rhs) const
    {
        if (lhs.is_ref())
        {
            lhs = blaze::pow(lhs.matrix(), rhs[0]);
        }
        else
        {
            lhs.matrix() = blaze::pow(lhs.matrix(), rhs[0]);
        }
        return primitive_argument_type{std::move(lhs)};
    }

#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
    template <typename T>
    primitive_argument_type power_operation::power3d(
        ir::node_data<T>&& lhs, ir::node_data<T>&& rhs) const
    {
        if (lhs.is_ref())
        {
            lhs = blaze::pow(lhs.tensor(), rhs[0]);
        }
        else
        {
            lhs.tensor() = blaze::pow(lhs.tensor(), rhs[0]);
        }
        return primitive_argument_type{std::move(lhs)};
    }
#endif

    primitive_argument_type power_operation::power0d(
        primitive_argument_type&& lhs, primitive_argument_type&& rhs) const
    {
        node_data_type t = dtype_;
        if (t == node_data_type_unknown)
        {
            t = extract_common_type(lhs, rhs);
        }

        // #FIXME: for now, map all power operations on the double dtype (Blaze
        // throws compilation error for int64_t)
        switch (t)
        {
        case node_data_type_bool:    HPX_FALLTHROUGH;
        case node_data_type_int64:   HPX_FALLTHROUGH;
        case node_data_type_unknown: HPX_FALLTHROUGH;
        case node_data_type_double:
            return power0d(extract_numeric_value(std::move(lhs)),
                extract_numeric_value(std::move(rhs)));

        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "power_operation::power0d",
            generate_error_message(
                "the power primitive requires for its argument to "
                    "be numeric data type"));
    }

    primitive_argument_type power_operation::power1d(
        primitive_argument_type&& lhs, primitive_argument_type&& rhs) const
    {
        node_data_type t = dtype_;
        if (t == node_data_type_unknown)
        {
            t = extract_common_type(lhs, rhs);
        }

        // #FIXME: for now, map all power operations on the double dtype (Blaze
        // throws compilation error for int64_t)
        switch (t)
        {
        case node_data_type_bool:    HPX_FALLTHROUGH;
        case node_data_type_int64:   HPX_FALLTHROUGH;
        case node_data_type_unknown: HPX_FALLTHROUGH;
        case node_data_type_double:
            return power1d(extract_numeric_value(std::move(lhs)),
                extract_numeric_value(std::move(rhs)));

        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "power_operation::power1d",
            generate_error_message(
                "the power primitive requires for its argument to "
                    "be numeric data type"));
    }

    primitive_argument_type power_operation::power2d(
        primitive_argument_type&& lhs, primitive_argument_type&& rhs) const
    {
        node_data_type t = dtype_;
        if (t == node_data_type_unknown)
        {
            t = extract_common_type(lhs, rhs);
        }

        // #FIXME: for now, map all power operations on the double dtype (Blaze
        // throws compilation error for int64_t)
        switch (t)
        {
        case node_data_type_bool:    HPX_FALLTHROUGH;
        case node_data_type_int64:   HPX_FALLTHROUGH;
        case node_data_type_unknown: HPX_FALLTHROUGH;
        case node_data_type_double:
            return power2d(extract_numeric_value(std::move(lhs)),
                extract_numeric_value(std::move(rhs)));

        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "power_operation::power2d",
            generate_error_message(
                "the power primitive requires for its argument to "
                    "be numeric data type"));
    }

#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
    primitive_argument_type power_operation::power3d(
        primitive_argument_type&& lhs, primitive_argument_type&& rhs) const
    {
        node_data_type t = dtype_;
        if (t == node_data_type_unknown)
        {
            t = extract_common_type(lhs, rhs);
        }

        // #FIXME: for now, map all power operations on the double dtype (Blaze
        // throws compilation error for int64_t)
        switch (t)
        {
        case node_data_type_bool:    HPX_FALLTHROUGH;
        case node_data_type_int64:   HPX_FALLTHROUGH;
        case node_data_type_unknown: HPX_FALLTHROUGH;
        case node_data_type_double:
            return power3d(extract_numeric_value(std::move(lhs)),
                extract_numeric_value(std::move(rhs)));

        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "power_operation::power3d",
            generate_error_message(
                "the power primitive requires for its argument to "
                    "be numeric data type"));
    }
#endif

    ///////////////////////////////////////////////////////////////////////////
    hpx::future<primitive_argument_type> power_operation::eval(
        primitive_arguments_type const& operands,
        primitive_arguments_type const& args, eval_context ctx) const
    {
        if (operands.size() != 2)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "power_operation::eval",
                generate_error_message(
                    "the power_operation primitive requires "
                        "exactly two operands"));
        }

        if (!valid(operands[0]) || !valid(operands[1]))
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "power_operation::eval",
                generate_error_message(
                    "the power_operation primitive requires "
                        "that the arguments given by the operands "
                        "array are valid"));
        }

        auto&& op0 = value_operand(operands[0], args, name_, codename_, ctx);

        auto this_ = this->shared_from_this();
        return hpx::dataflow(hpx::launch::sync,
            [this_ = std::move(this_)](
                    hpx::future<primitive_argument_type>&& f1,
                    hpx::future<primitive_argument_type>&& f2)
            ->  primitive_argument_type
            {
                auto&& op1 = f1.get();
                auto&& op2 = f2.get();

                if (extract_numeric_value_dimension(
                        op2, this_->name_, this_->codename_) != 0)
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "power_operation::eval",
                        this_->generate_error_message(
                            "right hand side operand has to be a scalar value"));
                }

                switch (extract_numeric_value_dimension(
                    op1, this_->name_, this_->codename_))
                {
                case 0:
                    return this_->power0d(std::move(op1), std::move(op2));

                case 1:
                    return this_->power1d(std::move(op1), std::move(op2));

                case 2:
                    return this_->power2d(std::move(op1), std::move(op2));

#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
                case 3:
                    return this_->power3d(std::move(op1), std::move(op2));
#endif
                default:
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "power_operation::eval",
                        this_->generate_error_message(
                            "left hand side operand has unsupported "
                                "number of dimensions"));
                }
            },
            std::move(op0),
            value_operand(operands[1], args, name_, codename_, std::move(ctx)));
    }
}}}
