//  Copyright (c) 2017-2018 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/access_variable.hpp>
#include <phylanx/execution_tree/primitives/primitive_component.hpp>
#include <phylanx/util/future_or_value.hpp>

#include <hpx/include/lcos.hpp>
#include <hpx/include/naming.hpp>
#include <hpx/include/util.hpp>
#include <hpx/throw_exception.hpp>

#include <algorithm>
#include <set>
#include <string>
#include <utility>
#include <vector>

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace execution_tree { namespace primitives
{
    ///////////////////////////////////////////////////////////////////////////
    match_pattern_type const access_variable::match_data =
    {
        hpx::util::make_tuple("access-variable",
            std::vector<std::string>{},
            nullptr, &create_primitive<access_variable>)
    };

    ///////////////////////////////////////////////////////////////////////////
    access_variable::access_variable(
        std::vector<primitive_argument_type>&& operands,
        std::string const& name, std::string const& codename)
        : primitive_component_base(std::move(operands), name, codename, true)
    {
        // operands_[0] is expected to be the actual variable, operands_[1] and
        // operands_[2] are optional slicing arguments

        if (operands_.empty() || operands_.size() > 3)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "access_variable::access_variable",
                generate_error_message(
                    "the access_variable primitive requires at least one "
                    "and at most three operands"));
        }

        if (valid(operands_[0]))
        {
            operands_[0] = extract_copy_value(std::move(operands_[0]));
        }

        // try to bind to the variable object locally
        primitive* p = util::get_if<primitive>(&operands_[0]);
        if (p != nullptr)
        {
            hpx::error_code ec(hpx::lightweight);
            target_ = hpx::get_ptr<primitive_component>(
                hpx::launch::sync, p->get_id(), ec);
        }
    }

    ///////////////////////////////////////////////////////////////////////////
    hpx::future<primitive_argument_type> access_variable::eval(
        std::vector<primitive_argument_type> const& params,
        eval_mode mode) const
    {
        // handle slicing, we can replace the params with our slicing
        // parameter as variable evaluation can't depend on those anyways
        mode = eval_mode(mode | eval_dont_wrap_functions);
        switch (operands_.size())
        {
        case 2:
            {
                // one slicing parameter
                auto this_ = this->shared_from_this();
                return value_operand(operands_[1], params, name_, codename_)
                    .then(hpx::launch::sync,
                        [this_, mode](hpx::future<primitive_argument_type>&& rows)
                        -> hpx::future<primitive_argument_type>
                        {
                            if (this_->target_)
                            {
                                return this_->target_->eval_single(
                                    rows.get(), mode);
                            }
                            return value_operand(this_->operands_[0],
                                rows.get(), this_->name_, this_->codename_,
                                mode);
                        });
            }

        case 3:
            {
                // two slicing parameters
                auto this_ = this->shared_from_this();
                return hpx::dataflow(hpx::launch::sync,
                    [this_, mode](
                        util::future_or_value<primitive_argument_type>&& rows,
                        util::future_or_value<primitive_argument_type>&& cols)
                    -> hpx::future<primitive_argument_type>
                    {
                        std::vector<primitive_argument_type> args;
                        args.reserve(2);
                        args.emplace_back(rows.get());
                        args.emplace_back(cols.get());
                        if (this_->target_)
                        {
                            return this_->target_->eval(std::move(args), mode);
                        }
                        return value_operand(this_->operands_[0],
                            std::move(args), this_->name_, this_->codename_, mode);
                    },
                    value_operand_fov(operands_[1], params, name_, codename_),
                    value_operand_fov(operands_[2], params, name_, codename_));
            }

        default:
            break;
        }

        // no slicing parameters given, access variable directly
        if (target_)
        {
            return target_->eval(noargs, mode);
        }
        return value_operand(operands_[0], noargs, name_, codename_, mode);
    }

    void access_variable::store(std::vector<primitive_argument_type>&& vals)
    {
        if (vals.size() != 1)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "access_variable::store",
                generate_error_message(
                    "invoking store with slicing parameters is not supported"));
        }

        // handle slicing, simply append the slicing parameters to the end of
        // the argument list
        for (auto it = operands_.begin() + 1; it != operands_.end(); ++it)
        {
            vals.emplace_back(extract_ref_value(*it));
        }

        if (target_)
        {
            target_->store(std::move(vals));
        }
        else
        {
            primitive* p = util::get_if<primitive>(&operands_[0]);
            if (p != nullptr)
            {
                p->store(hpx::launch::sync, std::move(vals));
            }
        }
    }

    void access_variable::store(primitive_argument_type&& val)
    {
        if (operands_.size() > 1)
        {
            // handle slicing, simply append the slicing parameters to the end of
            // the argument list
            std::vector<primitive_argument_type> vals;
            vals.reserve(operands_.size());
            vals.emplace_back(std::move(val));

            for (auto it = operands_.begin() + 1; it != operands_.end(); ++it)
            {
                vals.emplace_back(extract_ref_value(*it));
            }

            if (target_)
            {
                target_->store(std::move(vals));
            }
            else
            {
                primitive* p = util::get_if<primitive>(&operands_[0]);
                if (p != nullptr)
                {
                    p->store(hpx::launch::sync, std::move(vals));
                }
            }
        }
        else
        {
            // no slicing parameters given, simply dispatch request
            if (target_)
            {
                target_->store_single(std::move(val));
            }
            else
            {
                primitive* p = util::get_if<primitive>(&operands_[0]);
                if (p != nullptr)
                {
                    p->store(hpx::launch::sync, std::move(val));
                }
            }
        }
    }

    topology access_variable::expression_topology(
        std::set<std::string>&& functions,
        std::set<std::string>&& resolve_children) const
    {
        primitive const* p = util::get_if<primitive>(&operands_[0]);
        if (p != nullptr)
        {
            std::string name = p->registered_name();
            if (resolve_children.find(name) != resolve_children.end())
            {
                // recurse into function, if asked to do that
                return p->expression_topology(hpx::launch::sync,
                    std::move(functions), std::move(resolve_children));
            }

            // add only the name of the direct dependent node (no recursion)
            return topology{std::move(name)};
        }
        return {};
    }
}}}

