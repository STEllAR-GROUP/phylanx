//  Copyright (c) 2017-2018 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/access_argument.hpp>
#include <phylanx/execution_tree/primitives/slice.hpp>

#include <hpx/include/lcos.hpp>
#include <hpx/include/util.hpp>
#include <hpx/throw_exception.hpp>
#include <hpx/util/assert.hpp>

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
        // operands_[0] is expected to be the actual variable, operands_[1] and
        // operands_[2] are optional slicing arguments

        if (operands_.empty() || operands_.size() > 3)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "access_argument::access_argument",
                generate_error_message(
                    "the access_argument primitive requires at least one "
                    "and at most three operands"));
        }

        argnum_ = extract_integer_value(operands_[0])[0];
    }

    hpx::future<primitive_argument_type> access_argument::eval(
        primitive_arguments_type const& params, eval_context ctx) const
    {
        if (argnum_ >= params.size())
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "phylanx::execution_tree::primitives::access_argument::eval",
                generate_error_message(hpx::util::format(
                    "argument count out of bounds, expected at least "
                        PHYLANX_FORMAT_SPEC(1) " argument(s) while only "
                        PHYLANX_FORMAT_SPEC(2) " argument(s) were supplied",
                        argnum_ + 1, params.size())));
        }

        // handle slicing, we can replace the params with our slicing
        // parameters as argument evaluation can't depend on those anyways
        switch (operands_.size())
        {
        case 2:
        case 3:
            {
                // one slicing parameter
                if (is_primitive_operand(params[argnum_]))
                {
                    primitive_arguments_type fargs;
                    fargs.reserve(operands_.size() - 1);
                    for (auto it = operands_.begin() + 1; it != operands_.end();
                         ++it)
                    {
                        fargs.emplace_back(
                            extract_ref_value(*it, name_, codename_));
                    }

                    ctx.add_mode(eval_dont_wrap_functions);
                    return value_operand(params[argnum_], std::move(fargs),
                        name_, codename_, std::move(ctx));
                }

                if (operands_.size() > 2)
                {
                    // handle row/column-slicing
                    auto op1 = value_operand(
                        operands_[1], params, name_, codename_, ctx);
                    auto this_ = this->shared_from_this();
                    return hpx::dataflow(
                        hpx::launch::sync,
                        [this_ = std::move(this_), target = params[argnum_]](
                                hpx::future<primitive_argument_type>&& rows,
                                hpx::future<primitive_argument_type>&& cols)
                        ->  primitive_argument_type
                        {
                            return slice(target, rows.get(), cols.get(),
                                this_->name_, this_->codename_);
                        },
                        op1,
                        value_operand(operands_[2], params, name_, codename_,
                            std::move(ctx)));
                }

                // handle row-slicing
                auto this_ = this->shared_from_this();
                return value_operand(
                        operands_[1], params, name_, codename_, std::move(ctx))
                    .then(hpx::launch::sync,
                        [this_ = std::move(this_), target = params[argnum_]](
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

        return hpx::make_ready_future(
            extract_value(params[argnum_], name_, codename_));
    }

    void access_argument::store(primitive_arguments_type&& data,
        primitive_arguments_type&& params)
    {
        if (data.size() != 1)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "access_argument::store",
                generate_error_message(
                    "invoking store with slicing parameters is not supported"));
        }

        if (argnum_ >= params.size())
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "phylanx::execution_tree::primitives::access_argument::store",
                generate_error_message(hpx::util::format(
                    "argument count out of bounds, expected at least "
                    PHYLANX_FORMAT_SPEC(1) " argument(s) while only "
                    PHYLANX_FORMAT_SPEC(2) " argument(s) were supplied",
                    argnum_ + 1, params.size())));
        }

        if (is_primitive_operand(params[argnum_]))
        {
            // handle slicing, simply append the slicing parameters to the end
            // of the argument list
            data.reserve(data.size() + operands_.size() - 1);
            for (auto it = operands_.begin() + 1; it != operands_.end(); ++it)
            {
                data.emplace_back(extract_ref_value(*it, name_, codename_));
            }

            primitive* p = util::get_if<primitive>(&operands_[0]);
            HPX_ASSERT(p != nullptr);

            p->store(hpx::launch::sync, std::move(data), std::move(params));
        }
        else if (is_ref_value(params[argnum_]))
        {
            switch (operands_.size())
            {
            case 1:
                {
                    HPX_THROW_EXCEPTION(hpx::not_implemented,
                        "phylanx::execution_tree::primitives::access_argument::"
                            "store",
                        generate_error_message(
                            "assignment to (non-sliced) function argument is "
                            "not supported (yet)"));
                }
                break;

            case 2:
                slice(std::move(params[argnum_]),
                    value_operand_sync(std::move(operands_[1]),
                        std::move(params), name_, codename_),
                    std::move(data[0]), name_, codename_);
                return;

            case 3:
                {
                    auto data1 = value_operand_sync(
                        operands_[1], params, name_, codename_);
                    slice(std::move(params[argnum_]), std::move(data1),
                        value_operand_sync(
                            operands_[2], std::move(params), name_, codename_),
                        std::move(data[0]), name_, codename_);
                }
                return;

#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
            case 4:
                {
                    auto data1 = value_operand_sync(
                        operands_[1], params, name_, codename_);
                    auto data2 = value_operand_sync(
                        operands_[2], params, name_, codename_);
                    slice(std::move(params[argnum_]),
                        std::move(data1), std::move(data2),
                        value_operand_sync(
                            operands_[3], std::move(params), name_, codename_),
                        std::move(data[0]), name_, codename_);
                }
                return;
#endif

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
        primitive_arguments_type&& params)
    {
        if (argnum_ >= params.size())
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "phylanx::execution_tree::primitives::access_argument::store",
                generate_error_message(hpx::util::format(
                    "argument count out of bounds, expected at least "
                        PHYLANX_FORMAT_SPEC(1) " argument(s) while only "
                        PHYLANX_FORMAT_SPEC(2) " argument(s) were supplied",
                        argnum_ + 1, params.size())));
        }

        if (is_primitive_operand(params[argnum_]))
        {
            if (operands_.size() > 1)
            {
                // handle slicing, simply append the slicing parameters to the
                // end of the argument list
                primitive_arguments_type vals;
                vals.reserve(operands_.size());
                vals.emplace_back(std::move(data));

                for (auto it = operands_.begin() + 1; it != operands_.end();
                     ++it)
                {
                    vals.emplace_back(extract_ref_value(*it, name_, codename_));
                }

                primitive* p = util::get_if<primitive>(&params[argnum_]);
                HPX_ASSERT(p != nullptr);

                p->store(hpx::launch::sync, std::move(vals), std::move(params));
            }
            else
            {
                // no slicing parameters given, simply dispatch request
                primitive* p = util::get_if<primitive>(&operands_[0]);
                HPX_ASSERT(p != nullptr);

                p->store(hpx::launch::sync, std::move(data), std::move(params));
            }
        }
        else if (is_ref_value(params[argnum_]))
        {
            switch (operands_.size())
            {
            case 2:
                slice(std::move(params[argnum_]),
                    value_operand_sync(std::move(operands_[1]),
                        std::move(params), name_, codename_),
                    std::move(data), name_, codename_);
                return;

            case 3:
                {
                    auto data1 = value_operand_sync(
                        operands_[1], params, name_, codename_);
                    slice(std::move(params[argnum_]), std::move(data1),
                        value_operand_sync(operands_[2], std::move(params),
                            name_, codename_),
                        std::move(data), name_, codename_);
                }
                return;

            case 1:
                {
                    HPX_THROW_EXCEPTION(hpx::not_implemented,
                        "phylanx::execution_tree::primitives::access_argument::"
                            "store",
                        generate_error_message(
                            "assignment to (non-sliced) function argument is "
                            "not supported (yet)"));
                }

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
