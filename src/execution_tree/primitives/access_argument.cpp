//  Copyright (c) 2017-2019 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/access_argument.hpp>
#include <phylanx/execution_tree/primitives/slice.hpp>

#include <hpx/assertion.hpp>
#include <hpx/include/lcos.hpp>
#include <hpx/include/util.hpp>
#include <hpx/errors/throw_exception.hpp>

#include <memory>
#include <string>
#include <utility>
#include <vector>

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace execution_tree { namespace primitives
{
    ///////////////////////////////////////////////////////////////////////////
    match_pattern_type const access_argument::match_data =
    {
        hpx::util::make_tuple("access-argument",
            std::vector<std::string>{},
            nullptr, &create_primitive<access_argument>,
            "Internal")
    };

    ///////////////////////////////////////////////////////////////////////////
    access_argument::access_argument(
            primitive_arguments_type&& args,
            std::string const& name, std::string const& codename)
      : primitive_component_base(std::move(args), name, codename, true)
    {
        // operands_[0] is expected to be the actual variable,
        // operands_[1] is the (optional) default value
        // operands_[2], operands_[3], and operands_[4] are optional slicing
        // arguments

        if (operands_.empty() || operands_.size() > 5)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "access_argument::access_argument",
                generate_error_message(
                    "the access_argument primitive requires at least one "
                    "and at most four operands"));
        }
        argnum_ = extract_integer_value(operands_[0])[0];
    }

    hpx::future<primitive_argument_type> access_argument::eval(
        primitive_arguments_type const& params, eval_context ctx) const
    {
        primitive_argument_type target;
        if (argnum_ >= params.size())
        {
            if (operands_.size() < 2 ||
                (!valid(operands_[1]) && !is_explicit_nil(operands_[1])))
            {
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "phylanx::execution_tree::primitives::access_argument::eval",
                    generate_error_message(hpx::util::format(
                        "argument count out of bounds, expected at least "
                        PHYLANX_FORMAT_SPEC(1) " argument(s) while only "
                        PHYLANX_FORMAT_SPEC(2) " argument(s) were supplied",
                        argnum_ + 1, params.size())));
            }
            target = extract_ref_value(operands_[1], name_, codename_);
        }
        else if (valid(params[argnum_]) || is_explicit_nil(params[argnum_]))
        {
            target = extract_ref_value(params[argnum_], name_, codename_);
        }
        else
        {
            HPX_ASSERT(operands_.size() > 1 &&
                (valid(operands_[1]) || is_explicit_nil(operands_[1])));
            target = extract_ref_value(operands_[1], name_, codename_);
        }

        // handle slicing, we can replace the params with our slicing
        // parameters as argument evaluation can't depend on those anyways
        switch (operands_.size())
        {
        case 5: HPX_FALLTHROUGH;
        case 3: HPX_FALLTHROUGH;
        case 4:
            {
                // one slicing parameter
                if (is_primitive_operand(target))
                {
                    primitive_arguments_type fargs;
                    fargs.reserve(operands_.size() - 2);
                    for (auto it = operands_.begin() + 2; it != operands_.end();
                         ++it)
                    {
                        fargs.emplace_back(
                            extract_ref_value(*it, name_, codename_));
                    }

                    ctx.add_mode(
                        eval_mode(eval_dont_wrap_functions | eval_slicing));
                    return value_operand(target, std::move(fargs),
                        name_, codename_, std::move(ctx));
                }

                auto this_ = this->shared_from_this();
                if (operands_.size() == 4)
                {
                    // handle row/column-slicing
                    auto op1 = value_operand(
                        operands_[2], params, name_, codename_, ctx);
                    return hpx::dataflow(
                        hpx::launch::sync,
                        [this_ = std::move(this_), target = std::move(target)](
                                hpx::future<primitive_argument_type>&& rows,
                                hpx::future<primitive_argument_type>&& cols)
                        ->  primitive_argument_type
                        {
                            return slice(target, rows.get(), cols.get(),
                                this_->name_, this_->codename_);
                        },
                        op1,
                        value_operand(operands_[3], params, name_, codename_,
                            std::move(ctx)));
                }

                if (operands_.size() > 4)
                {
                    // handle page/row/column-slicing
                    auto op1 = value_operand(
                        operands_[2], params, name_, codename_, ctx);
                    auto op2 = value_operand(
                        operands_[3], params, name_, codename_, ctx);
                    return hpx::dataflow(
                        hpx::launch::sync,
                        [this_ = std::move(this_), target = std::move(target)](
                                hpx::future<primitive_argument_type>&& pages,
                                hpx::future<primitive_argument_type>&& rows,
                                hpx::future<primitive_argument_type>&& cols)
                        ->  primitive_argument_type
                        {
                            return slice(target, pages.get(), rows.get(), cols.get(),
                                this_->name_, this_->codename_);
                        },
                        op1, op2,
                        value_operand(operands_[4], params, name_, codename_,
                            std::move(ctx)));
                }

                // handle row-slicing
                return value_operand(
                        operands_[2], params, name_, codename_, std::move(ctx))
                    .then(hpx::launch::sync,
                        [this_ = std::move(this_), target = std::move(target)](
                            hpx::future<primitive_argument_type>&& rows)
                        -> primitive_argument_type
                        {
                            return slice(target, rows.get(), this_->name_,
                                this_->codename_);
                        });
            }

        default:
            break;
        }

        // evaluate the argument
        ctx.add_mode(eval_dont_evaluate_partials);
        return value_operand(
            target, this->noargs, name_, codename_, std::move(ctx));
    }

    void access_argument::store(primitive_arguments_type&& data,
        primitive_arguments_type&& params, eval_context ctx)
    {
        if (data.size() != 1)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "access_argument::store",
                generate_error_message(
                    "invoking store with slicing parameters is not supported"));
        }

        primitive_argument_type target;
        if (argnum_ >= params.size())
        {
            if (operands_.size() < 2 ||
                (!valid(operands_[1]) && !is_explicit_nil(operands_[1])))
            {
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "phylanx::execution_tree::primitives::access_argument::store",
                    generate_error_message(hpx::util::format(
                        "argument count out of bounds, expected at least "
                        PHYLANX_FORMAT_SPEC(1) " argument(s) while only "
                        PHYLANX_FORMAT_SPEC(2) " argument(s) were supplied",
                        argnum_ + 1, params.size())));
            }
            target = extract_ref_value(operands_[1], name_, codename_);
        }
        else if (valid(params[argnum_]) || is_explicit_nil(params[argnum_]))
        {
            target = std::move(params[argnum_]);
        }
        else
        {
            HPX_ASSERT(operands_.size() > 1 &&
                (valid(operands_[1]) || is_explicit_nil(operands_[1])));
            target = extract_ref_value(operands_[1], name_, codename_);
        }

        if (is_primitive_operand(target))
        {
            // handle slicing, simply append the slicing parameters to the end
            // of the argument list
            data.reserve(data.size() + operands_.size() - 2);
            for (auto it = operands_.begin() + 2; it != operands_.end(); ++it)
            {
                data.emplace_back(extract_ref_value(*it, name_, codename_));
            }

            primitive* p = util::get_if<primitive>(&operands_[0]);
            HPX_ASSERT(p != nullptr);

            p->store(hpx::launch::sync, std::move(data), std::move(params),
                ctx.add_mode(eval_slicing));
        }
        else if (is_ref_value(target))
        {
            switch (operands_.size())
            {
            case 1:
            case 2:
                {
                    HPX_THROW_EXCEPTION(hpx::not_implemented,
                        "phylanx::execution_tree::primitives::access_argument::"
                            "store",
                        generate_error_message(
                            "assignment to (non-sliced) function argument is "
                            "not supported (yet)"));
                }
                break;

            case 3:
                slice(std::move(target),
                    value_operand_sync(std::move(operands_[2]),
                        std::move(params), name_, codename_),
                    std::move(data[0]), name_, codename_);
                return;

            case 4:
                {
                    auto data1 = value_operand_sync(
                        operands_[2], params, name_, codename_);
                    slice(std::move(target), std::move(data1),
                        value_operand_sync(
                            operands_[3], std::move(params), name_, codename_),
                        std::move(data[0]), name_, codename_);
                }
                return;

            case 5:
                {
                    auto data1 = value_operand_sync(
                        operands_[2], params, name_, codename_);
                    auto data2 = value_operand_sync(
                        operands_[3], params, name_, codename_);
                    slice(std::move(target),
                        std::move(data1), std::move(data2),
                        value_operand_sync(
                            operands_[4], std::move(params), name_, codename_),
                        std::move(data[0]), name_, codename_);
                }
                return;

            default:
                break;
            }
        }
        else
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "phylanx::execution_tree::primitives::access_argument::store",
                generate_error_message(
                    "storing a value to a local value has no effect"));
        }
    }

    void access_argument::store(primitive_argument_type&& data,
        primitive_arguments_type&& params, eval_context ctx)
    {
        primitive_argument_type target;
        if (argnum_ >= params.size())
        {
            if (operands_.size() < 2 ||
                (!valid(operands_[1]) && !is_explicit_nil(operands_[1])))
            {
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "phylanx::execution_tree::primitives::access_argument::store",
                    generate_error_message(hpx::util::format(
                        "argument count out of bounds, expected at least "
                        PHYLANX_FORMAT_SPEC(1) " argument(s) while only "
                        PHYLANX_FORMAT_SPEC(2) " argument(s) were supplied",
                        argnum_ + 1, params.size())));
            }
            target = extract_ref_value(operands_[1], name_, codename_);
        }
        else if (valid(params[argnum_]) || is_explicit_nil(params[argnum_]))
        {
            target = std::move(params[argnum_]);
        }
        else
        {
            HPX_ASSERT(operands_.size() > 1 &&
                (valid(operands_[1]) || is_explicit_nil(operands_[1])));
            target = extract_ref_value(operands_[1], name_, codename_);
        }

        if (is_primitive_operand(target))
        {
            if (operands_.size() > 2)
            {
                // handle slicing, simply append the slicing parameters to the
                // end of the argument list
                primitive_arguments_type vals;
                vals.reserve(operands_.size() - 1);
                vals.emplace_back(std::move(data));

                for (auto it = operands_.begin() + 2; it != operands_.end();
                     ++it)
                {
                    vals.emplace_back(extract_ref_value(*it, name_, codename_));
                }

                primitive* p = util::get_if<primitive>(&target);
                HPX_ASSERT(p != nullptr);

                p->store(hpx::launch::sync, std::move(vals), std::move(params),
                    ctx.add_mode(eval_slicing));
            }
            else
            {
                // no slicing parameters given, simply dispatch request
                primitive* p = util::get_if<primitive>(&operands_[0]);
                HPX_ASSERT(p != nullptr);

                p->store(hpx::launch::sync, std::move(data), std::move(params),
                    std::move(ctx));
            }
        }
        else if (is_ref_value(target))
        {
            switch (operands_.size())
            {
            case 1:
            case 2:
                {
                    HPX_THROW_EXCEPTION(hpx::not_implemented,
                        "phylanx::execution_tree::primitives::access_argument::"
                            "store",
                        generate_error_message(
                            "assignment to (non-sliced) function argument is "
                            "not supported (yet)"));
                }

            case 3:
                slice(std::move(target),
                    value_operand_sync(std::move(operands_[2]),
                        std::move(params), name_, codename_),
                    std::move(data), name_, codename_);
                return;

            case 4:
                {
                    auto data1 = value_operand_sync(
                        operands_[2], params, name_, codename_);
                    slice(std::move(target), std::move(data1),
                        value_operand_sync(operands_[3], std::move(params),
                            name_, codename_),
                        std::move(data), name_, codename_);
                }
                return;

            case 5:
                {
                    auto data1 = value_operand_sync(
                        operands_[2], params, name_, codename_);
                    auto data2 = value_operand_sync(
                        operands_[3], params, name_, codename_);
                    slice(std::move(target),
                        std::move(data1), std::move(data2),
                        value_operand_sync(operands_[4], std::move(params),
                            name_, codename_),
                        std::move(data), name_, codename_);
                }
                return;

            default:
                break;
            }
        }
        else
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "phylanx::execution_tree::primitives::access_argument::store",
                generate_error_message(
                    "storing a value to a local value has no effect"));
        }
    }
}}}
