// Copyright (c) 2017 Alireza Kheirkhahan
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/ast/detail/is_literal_value.hpp>
#include <phylanx/execution_tree/primitives/store_operation.hpp>
#include <phylanx/ir/node_data.hpp>

#include <hpx/include/lcos.hpp>
#include <hpx/include/naming.hpp>
#include <hpx/include/util.hpp>
#include <hpx/throw_exception.hpp>

#include <cstddef>
#include <memory>
#include <string>
#include <utility>
#include <vector>

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace execution_tree { namespace primitives
{
    ///////////////////////////////////////////////////////////////////////////
    primitive create_store_operation(hpx::id_type const& locality,
        std::vector<primitive_argument_type>&& operands,
        std::string const& name, std::string const& codename)
    {
        static std::string type("store");
        return create_primitive_component(
            locality, type, std::move(operands), name, codename);
    }

    match_pattern_type const store_operation::match_data =
    {
        hpx::util::make_tuple("store",
            std::vector<std::string>{"store(_1, _2)"},
            &create_store_operation, &create_primitive<store_operation>)
    };

    ///////////////////////////////////////////////////////////////////////////
    store_operation::store_operation(
            std::vector<primitive_argument_type> && operands,
            std::string const& name, std::string const& codename)
      : primitive_component_base(std::move(operands), name, codename)
    {}

    ///////////////////////////////////////////////////////////////////////////
    namespace detail
    {
        struct store : std::enable_shared_from_this<store>
        {
            store(std::vector<primitive_argument_type> const& operands,
                    std::vector<primitive_argument_type> const& args,
                    std::string const& name, std::string const& codename)
              : operands_(operands)
              , args_(args)
              , name_(name)
              , codename_(codename)
            {
                if (operands_.size() != 2)
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "store_operation::eval",
                        generate_error_message(
                            "the store_operation primitive requires exactly "
                                "two operands",
                            name_, codename_));

                    if (!valid(operands_[0]) || !valid(operands_[1]))
                    {
                        HPX_THROW_EXCEPTION(hpx::bad_parameter,
                            "store_operation::store_operation",
                            generate_error_message(
                                "the store_operation primitive requires that "
                                    "the arguments given by the operands array "
                                    "is valid",
                                name_, codename_));
                    }

                    if (!is_primitive_operand(operands_[0]))
                    {
                        HPX_THROW_EXCEPTION(hpx::bad_parameter,
                            "store_operation::store_operation",
                            generate_error_message(
                                "the first argument of the store primitive must "
                                    "refer to a another primitive and can't be a "
                                    "literal value",
                                name_, codename_));
                    }
                }
            }

            hpx::future<primitive_argument_type> eval()
            {
                auto this_ = this->shared_from_this();
                return literal_operand(operands_[1], args_, name_, codename_)
                    .then(hpx::util::unwrapping(
                        [this_](primitive_argument_type&& val)
                        {
                            primitive_operand(this_->operands_[0], this_->name_,
                                    this_->codename_)
                                .store(hpx::launch::sync, std::move(val));
                            return primitive_argument_type{};
                        }));
            }

            std::vector<primitive_argument_type> operands_;
            std::vector<primitive_argument_type> args_;
            std::string name_;
            std::string codename_;
        };
    }

    hpx::future<primitive_argument_type> store_operation::eval(
        std::vector<primitive_argument_type> const& args) const
    {
        if (operands_.empty())
        {
            return std::make_shared<detail::store>(
                args, noargs, name_, codename_)->eval();
        }

        return std::make_shared<detail::store>(
            operands_, args, name_, codename_)->eval();
    }
}}}

