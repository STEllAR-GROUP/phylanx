//  Copyright (c) 2018 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/ast/detail/is_literal_value.hpp>
#include <phylanx/execution_tree/primitives/apply.hpp>

#include <hpx/include/lcos.hpp>
#include <hpx/include/naming.hpp>
#include <hpx/include/util.hpp>
#include <hpx/runtime/launch_policy.hpp>
#include <hpx/throw_exception.hpp>
#include <hpx/util/assert.hpp>

#include <cstddef>
#include <string>
#include <utility>
#include <vector>

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace execution_tree { namespace primitives
{
    namespace apply_detail
    {
        ///////////////////////////////////////////////////////////////////////
        char const* const primitive_argument_type_names[] = {
            "phylanx::ast::nil",
            "phylanx::ir::node_data<std::uint8_t>",
            "std::int64_t",
            "std::string",
            "phylanx::ir::node_data<double>",
            "phylanx::execution_tree::primitive",
            "std::vector<phylanx::ast::expression>",
            "phylanx::ir::range"
        };

        static char const* const get_primitive_argument_type_name(std::size_t index)
        {
            if (index > sizeof(primitive_argument_type_names) /
                sizeof(primitive_argument_type_names[0]))
            {
                return "unknown";
            }
            return primitive_argument_type_names[index];
        }

        ///////////////////////////////////////////////////////////////////////
        primitive_argument_type trace(char const* const func,
            primitive const& this_, primitive_argument_type&& data)
        {
            if (!primitive::enable_tracing)
            {
                return std::move(data);
            }

            primitive const* p = util::get_if<primitive>(&data);
            if (p != nullptr)
            {
                LAPP_(debug) << this_.registered_name() << "::" << func << ": "
                    << p->registered_name();
            }
            else
            {
                LAPP_(debug)
                    << this_.registered_name() << "::" << func << ": " << data;
            }

            return std::move(data);
        }

        hpx::future<primitive_argument_type> lazy_trace(char const* const func,
            primitive const& this_, hpx::future<primitive_argument_type>&& f)
        {
            if (!primitive::enable_tracing)
            {
                return std::move(f);
            }

            return f.then(
                [=](hpx::future<primitive_argument_type>&& f)
            {
                return trace(func, this_, f.get());
            });
        }

        ///////////////////////////////////////////////////////////////////////////
        std::vector<primitive_argument_type> get_list_value(
            primitive_argument_type const& val,
            std::string const& name, std::string const& codename)
        {
            switch (val.index())
            {
            case 7:     // std::vector<primitive_argument_type>
                return util::get<7>(val).args();

            default:
                break;
            }

            std::string type(apply_detail::get_primitive_argument_type_name(val.index()));
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "phylanx::execution_tree::get_list_value",
                generate_error_message(
                    "primitive_argument_type does not hold a boolean "
                    "value type (type held: '" + type + "')",
                    name, codename));
        }

        std::vector<primitive_argument_type> get_list_value(
            primitive_argument_type && val,
            std::string const& name, std::string const& codename)
        {
            switch (val.index())
            {
            case 0:     // nil
                return std::vector<primitive_argument_type>{
                    primitive_argument_type{ util::get<0>(std::move(val)) }};

            case 1:    // phylanx::ir::node_data<std::uint8_t>
                return std::vector<primitive_argument_type>{
                    primitive_argument_type{ util::get<1>(std::move(val)) }};

            case 2:     // std::uint64_t
                return std::vector<primitive_argument_type>{
                    primitive_argument_type{ util::get<2>(std::move(val)) }};

            case 3:     // string
                return std::vector<primitive_argument_type>{
                    primitive_argument_type{ util::get<3>(std::move(val)) }};

            case 4:     // phylanx::ir::node_data<double>
                return std::vector<primitive_argument_type>{
                    primitive_argument_type{ util::get<4>(std::move(val)) }};

            case 5:     // primitive
                return std::vector<primitive_argument_type>{
                    primitive_argument_type{ util::get<5>(std::move(val)) }};

            case 6:     // std::vector<ast::expression>
                return std::vector<primitive_argument_type>{
                    primitive_argument_type{ util::get<6>(std::move(val)) }};

            case 7:     // std::vector<primitive_argument_type>
                return util::get<7>(std::move(val)).args();

            default:
                break;
            }

            std::string type(apply_detail::get_primitive_argument_type_name(val.index()));
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "phylanx::execution_tree::get_list_value",
                generate_error_message(
                    "primitive_argument_type does not hold a boolean "
                    "value type (type held: '" + type + "')",
                    name, codename));
        }

        ///////////////////////////////////////////////////////////////////////////
        std::vector<primitive_argument_type> get_list_value_strict(
            primitive_argument_type const& val,
            std::string const& name, std::string const& codename)
        {
            switch (val.index())
            {
            case 6:     // std::vector<ast::expression>
            {
                std::vector<primitive_argument_type> result;
                result.reserve(util::get<6>(val).size());
                for (auto const& v : util::get<6>(val))
                {
                    if (ast::detail::is_literal_value(v))
                    {
                        result.push_back(to_primitive_value_type(
                            ast::detail::literal_value(v)));
                    }
                    else
                    {
                        break;
                    }
                }
                return result;
            }

            case 7:     // std::vector<primitive_argument_type>
                return util::get<7>(val).args();

            case 0: HPX_FALLTHROUGH;    // nil
            case 1: HPX_FALLTHROUGH;    // phylanx::ir::node_data<std::uint8_t>
            case 2: HPX_FALLTHROUGH;    // std::uint64_t
            case 3: HPX_FALLTHROUGH;    // string
            case 4: HPX_FALLTHROUGH;    // phylanx::ir::node_data<double>
            case 5: HPX_FALLTHROUGH;    // primitive
            default:
                break;
            }

            std::string type(apply_detail::get_primitive_argument_type_name(val.index()));
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "phylanx::execution_tree::get_list_value_strict",
                generate_error_message(
                    "primitive_argument_type does not hold a list "
                    "value type (type held: '" + type + "')",
                    name, codename));
        }

        std::vector<primitive_argument_type> get_list_value_strict(
            primitive_argument_type && val,
            std::string const& name, std::string const& codename)
        {
            switch (val.index())
            {
            case 6:     // std::vector<ast::expression>
            {
                std::vector<primitive_argument_type> result;
                result.reserve(util::get<6>(val).size());
                for (auto && v : util::get<6>(std::move(val)))
                {
                    if (ast::detail::is_literal_value(v))
                    {
                        result.push_back(to_primitive_value_type(
                            ast::detail::literal_value(v)));
                    }
                    else
                    {
                        break;
                    }
                }
                return result;
            }

            case 7:     // std::vector<primitive_argument_type>
                return util::get<7>(std::move(val)).args();

            case 0: HPX_FALLTHROUGH;    // nil
            case 1: HPX_FALLTHROUGH;    // phylanx::ir::node_data<std::uint8_t>
            case 2: HPX_FALLTHROUGH;    // std::uint64_t
            case 3: HPX_FALLTHROUGH;    // string
            case 4: HPX_FALLTHROUGH;    // phylanx::ir::node_data<double>
            case 5: HPX_FALLTHROUGH;    // primitive
            default:
                break;
            }

            std::string type(apply_detail::get_primitive_argument_type_name(val.index()));
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "phylanx::execution_tree::get_list_value_strict",
                generate_error_message(
                    "primitive_argument_type does not hold a list "
                    "value type (type held: '" + type + "')",
                    name, codename));
        }

        ///////////////////////////////////////////////////////////////////////////
        hpx::future<std::vector<primitive_argument_type>> list_operand(
            primitive_argument_type const& val,
            std::vector<primitive_argument_type> const& args,
            std::string const& name, std::string const& codename)
        {
            primitive const* p = util::get_if<primitive>(&val);
            if (p != nullptr)
            {
                return p->eval(args).then(
                    [&](hpx::future<primitive_argument_type> && f)
                {
                    return get_list_value(f.get(), name, codename);
                });
            }

            HPX_ASSERT(valid(val));
            return hpx::make_ready_future(get_list_value(val, name, codename));
        }

        hpx::future<std::vector<primitive_argument_type>> list_operand(
            primitive_argument_type const& val,
            std::vector<primitive_argument_type> && args,
            std::string const& name, std::string const& codename)
        {
            primitive const* p = util::get_if<primitive>(&val);
            if (p != nullptr)
            {
                return p->eval(std::move(args)).then(
                    [&](hpx::future<primitive_argument_type> && f)
                {
                    return get_list_value(f.get(), name, codename);
                });
            }

            HPX_ASSERT(valid(val));
            return hpx::make_ready_future(get_list_value(val, name, codename));
        }

        hpx::future<std::vector<primitive_argument_type>> list_operand(
            primitive_argument_type const& val,
            std::vector<primitive_argument_type> const& args,
            std::string const& name, std::string && codename)
        {
            primitive const* p = util::get_if<primitive>(&val);
            if (p != nullptr)
            {
                return p->eval(args).then(
                    [&](hpx::future<primitive_argument_type> && f)
                {
                    return get_list_value(f.get(), name, codename);
                });
            }

            HPX_ASSERT(valid(val));
            return hpx::make_ready_future(
                get_list_value(std::move(val), name, codename));
        }

        hpx::future<std::vector<primitive_argument_type>> list_operand(
            primitive_argument_type const& val,
            std::vector<primitive_argument_type> && args,
            std::string const& name, std::string && codename)
        {
            primitive const* p = util::get_if<primitive>(&val);
            if (p != nullptr)
            {
                return p->eval(std::move(args)).then(
                    [&](hpx::future<primitive_argument_type> && f)
                {
                    return get_list_value(f.get(), name, codename);
                });
            }

            HPX_ASSERT(valid(val));
            return hpx::make_ready_future(
                get_list_value(std::move(val), name, codename));
        }

        std::vector<primitive_argument_type> list_operand_sync(
            primitive_argument_type const& val,
            std::vector<primitive_argument_type> const& args,
            std::string const& name, std::string const& codename)
        {
            primitive const* p = util::get_if<primitive>(&val);
            if (p != nullptr)
            {
                return get_list_value(
                    p->eval(hpx::launch::sync, args), name, codename);
            }

            HPX_ASSERT(valid(val));
            return get_list_value(val, name, codename);
        }
    }

    ///////////////////////////////////////////////////////////////////////////
    primitive create_apply(hpx::id_type const& locality,
        std::vector<primitive_argument_type>&& operands, std::string const& name,
        std::string const& codename)
    {
        static std::string type("apply");
        return create_primitive_component(
            locality, type, std::move(operands), name, codename);
    }

    match_pattern_type const apply::match_data =
    {
        hpx::util::make_tuple("apply",
            std::vector<std::string>{"apply(_1, _2)"},
            &create_apply, &create_primitive<apply>)
    };

    ///////////////////////////////////////////////////////////////////////////
    apply::apply(std::vector<primitive_argument_type>&& operands,
            std::string const& name, std::string const& codename)
      : primitive_component_base(std::move(operands), name, codename)
    {}

    hpx::future<primitive_argument_type> apply::eval(
        std::vector<primitive_argument_type> const& params) const
    {
        if (operands_.size() != 2)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "apply::eval",
                generate_error_message(
                    "the apply primitive requires exactly two operands"));
        }

        primitive const* p = util::get_if<primitive>(&operands_[0]);
        if (p == nullptr)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "apply::eval",
                generate_error_message(
                    "the first argument to apply must be an invocable "
                    "object"));
        }

        auto this_ = this->shared_from_this();
        return apply_detail::list_operand(operands_[1], params, name_, codename_).then(
            hpx::launch::sync,
            [this_](hpx::future<std::vector<primitive_argument_type>>&& f)
            {
                primitive const* p =
                    util::get_if<primitive>(&this_->operands_[0]);

                if (p == nullptr)
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "apply::eval",
                        this_->generate_error_message(
                            "the first argument to apply must be an invocable "
                            "object"));
                }

                return p->eval(hpx::launch::sync, f.get());
            });
    }
}}}
