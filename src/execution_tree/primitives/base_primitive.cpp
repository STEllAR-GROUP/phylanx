// Copyright (c) 2017-2018 Hartmut Kaiser
// Copyright (c) 2018 Tianyi Zhang
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// phylanxinspect:noinclude:HPX_ASSERT

#include <phylanx/config.hpp>
#include <phylanx/ast/detail/is_literal_value.hpp>
#include <phylanx/ast/node.hpp>
#include <phylanx/execution_tree/compiler/primitive_name.hpp>
#include <phylanx/execution_tree/primitives/base_primitive.hpp>
#include <phylanx/execution_tree/primitives/primitive_component.hpp>
#include <phylanx/ir/node_data.hpp>
#include <phylanx/util/repr_manip.hpp>

#include <hpx/include/actions.hpp>
#include <hpx/include/components.hpp>
#include <hpx/runtime/launch_policy.hpp>
#include <hpx/util/logging.hpp>

#include <array>
#include <cstdint>
#include <iosfwd>
#include <set>
#include <sstream>
#include <string>
#include <vector>
#include <utility>

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace execution_tree
{
    ///////////////////////////////////////////////////////////////////////////
#if defined(PHYLANX_DEBUG)
    bool primitive::enable_tracing = true;
#else
    bool primitive::enable_tracing = false;
#endif

    namespace detail
    {
        ///////////////////////////////////////////////////////////////////////
        char const* const primitive_argument_type_names[] = {
            "phylanx::ast::nil",
            "phylanx::ir::node_data<std::uint8_t>",
            "phylanx::ir::node_data<std::int64_t>",
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

        bool trace(char const* const func, primitive const& this_, bool data)
        {
            if (!primitive::enable_tracing)
            {
                return std::move(data);
            }

            LAPP_(debug) << this_.registered_name() << "::" << func << ": "
                         << std::boolalpha << data;

            return std::move(data);
        }

        template <typename T>
        hpx::future<T> lazy_trace(char const* const func,
            primitive const& this_, hpx::future<T>&& f)
        {
            if (!primitive::enable_tracing)
            {
                return std::move(f);
            }

            return f.then(hpx::launch::sync,
                [=](hpx::future<T>&& f)
                {
                    return trace(func, this_, f.get());
                });
        }
    }

    ///////////////////////////////////////////////////////////////////////////
    primitive::primitive(hpx::future<hpx::id_type>&& fid, std::string const& name)
      : base_type(std::move(fid))
    {
        if (!name.empty())
        {
            this->base_type::register_as(name).get();
        }
    }

    hpx::future<primitive_argument_type> primitive::eval(
        std::vector<primitive_argument_type> const& params,
        eval_mode mode) const
    {
        using action_type = primitives::primitive_component::eval_action;
        hpx::future<primitive_argument_type> f =
            hpx::async(action_type(), this->base_type::get_id(), params, mode);
        return detail::lazy_trace("eval", *this, std::move(f));
    }
    hpx::future<primitive_argument_type> primitive::eval(
        std::vector<primitive_argument_type>&& params,
        eval_mode mode) const
    {
        using action_type = primitives::primitive_component::eval_action;
        hpx::future<primitive_argument_type> f = hpx::async(
            action_type(), this->base_type::get_id(), std::move(params), mode);
        return detail::lazy_trace("eval", *this, std::move(f));
    }

    hpx::future<primitive_argument_type> primitive::eval(
        eval_mode mode) const
    {
        static std::vector<primitive_argument_type> params;
        return eval(params, mode);
    }

    primitive_argument_type primitive::eval(hpx::launch::sync_policy,
        std::vector<primitive_argument_type> const& params,
        eval_mode mode) const
    {
        using action_type = primitives::primitive_component::eval_action;
        hpx::future<primitive_argument_type> f =
            action_type()(this->base_type::get_id(), params, mode);
        return detail::trace("eval", *this, f.get());
    }
    primitive_argument_type primitive::eval(hpx::launch::sync_policy,
        std::vector<primitive_argument_type>&& params,
        eval_mode mode) const
    {
        using action_type = primitives::primitive_component::eval_action;
        hpx::future<primitive_argument_type> f =
            action_type()(this->base_type::get_id(), std::move(params), mode);
        return detail::trace("eval", *this, f.get());
    }

    primitive_argument_type primitive::eval(hpx::launch::sync_policy,
        eval_mode mode) const
    {
        return eval(mode).get();
    }

    hpx::future<void> primitive::store(primitive_argument_type data)
    {
        using action_type = primitives::primitive_component::store_action;
        return hpx::async(
            action_type(), this->base_type::get_id(), std::move(data));
    }

    void primitive::store(hpx::launch::sync_policy,
        primitive_argument_type data)
    {
        using action_type = primitives::primitive_component::store_action;
        action_type()(this->base_type::get_id(), std::move(data));
    }

    ///////////////////////////////////////////////////////////////////////
    hpx::future<void> primitive::store_set_1d(
        ir::node_data<double> data, std::vector<int64_t> list)
    {
        using action_type =
            primitives::primitive_component::store_set_1d_action;
        return hpx::async(action_type(), this->base_type::get_id(),
            std::move(data), std::move(list));
    }

    void primitive::store_set_1d(hpx::launch::sync_policy,
        ir::node_data<double> data, std::vector<int64_t> list)
    {
        using action_type =
            primitives::primitive_component::store_set_1d_action;
        action_type()(
            this->base_type::get_id(), std::move(data), std::move(list));
    }

    hpx::future<void> primitive::store_set_2d(ir::node_data<double> data,
        std::vector<int64_t> list_row, std::vector<int64_t> list_col)
    {
        using action_type =
            primitives::primitive_component::store_set_2d_action;
        return hpx::async(action_type(), this->base_type::get_id(),
            std::move(data), std::move(list_row), std::move(list_col));
    }

    void primitive::store_set_2d(hpx::launch::sync_policy,
        ir::node_data<double> data, std::vector<int64_t> list_row,
        std::vector<int64_t> list_col)
    {
        using action_type =
            primitives::primitive_component::store_set_2d_action;
        action_type()(this->base_type::get_id(), std::move(data),
            std::move(list_row), std::move(list_col));
    }
    /////////////////////////////////////////////////////////////////////////

    hpx::future<topology> primitive::expression_topology(
        std::set<std::string>&& functions) const
    {
        return expression_topology(
            std::move(functions), std::set<std::string>{});
    }

    hpx::future<topology> primitive::expression_topology(
        std::set<std::string>&& functions,
        std::set<std::string>&& resolve_children) const
    {
        // retrieve name of this node (the component can only retrieve
        // names of dependent nodes)
        std::string this_name = this->base_type::registered_name();

        // retrieve name of component instance
        using action_type = primitives::primitive_component::
            expression_topology_action;

        hpx::future<topology> f =
            hpx::async(action_type(), this->base_type::get_id(),
                std::move(functions), std::move(resolve_children));

        return f.then(hpx::launch::sync,
            [this_name](hpx::future<topology> && f)
            {
                topology && t = f.get();
                if (t.name_.empty())
                {
                    t.name_ = this_name;
                    return t;
                }
                std::vector<topology> children;
                children.emplace_back(std::move(t));
                return topology{std::move(children), this_name};
            });
    }

    topology primitive::expression_topology(
        hpx::launch::sync_policy, std::set<std::string>&& functions) const
    {
        return expression_topology(std::move(functions)).get();
    }

    topology primitive::expression_topology(hpx::launch::sync_policy,
        std::set<std::string>&& functions,
        std::set<std::string>&& resolve_children) const
    {
        return expression_topology(
            std::move(functions), std::move(resolve_children)).get();
    }

    bool primitive::bind(
        std::vector<primitive_argument_type> const& params) const
    {
        using action_type = primitives::primitive_component::bind_action;
        return detail::trace("bind", *this,
            action_type()(this->base_type::get_id(), params));
    }
    bool primitive::bind(std::vector<primitive_argument_type>&& params) const
    {
        using action_type = primitives::primitive_component::bind_action;
        return detail::trace("bind", *this,
            action_type()(this->base_type::get_id(), std::move(params)));
    }

    ///////////////////////////////////////////////////////////////////////////
    // traverse expression-tree topology and generate Newick representation
    namespace detail
    {
        std::string newick_tree_helper(
            topology const& t, std::set<std::string>& handled_nodes)
        {
            std::string result;

            // handle each node only once
            if (handled_nodes.find(t.name_) == handled_nodes.end())
            {
                handled_nodes.insert(t.name_);

                if (!t.children_.empty())
                {
                    bool first = true;
                    for (auto const& child : t.children_)
                    {
                        std::string name =
                            newick_tree_helper(child, handled_nodes);
                        if (!first && !name.empty())
                        {
                            result += ',';
                        }
                        first = false;
                        if (!name.empty())
                        {
                            result += std::move(name);
                        }
                    }

                    if (!result.empty() &&
                        !(result[0] == '(' && result[result.size() - 1] == ')'))
                    {
                        result = "(" + result + ")";
                    }
                }

                if (!t.name_.empty())
                {
                    if (!result.empty())
                    {
                        result += " ";
                    }
                    result += t.name_;
                }
            }

            return result;
        }
    }

    std::string newick_tree(std::string const& name, topology const& t)
    {
        std::set<std::string> handled_nodes;
        return "(" + detail::newick_tree_helper(t, handled_nodes) + ") " +
            name + ";";
    }

    ///////////////////////////////////////////////////////////////////////////
    namespace detail
    {
        std::string dot_tree_helper(
            topology const& t, std::set<std::string>& handled_nodes)
        {
            std::string result;

            // handle each node only once
            if (handled_nodes.find(t.name_) == handled_nodes.end())
            {
                handled_nodes.insert(t.name_);

                for (auto const& child : t.children_)
                {
                    result +=
                        "    \"" + t.name_ + "\" -- \"" + child.name_ + "\";\n";
                    if (!child.children_.empty())
                    {
                        result += dot_tree_helper(child, handled_nodes);
                    }
                    else if (!child.name_.empty())
                    {
                        result += "    \"" + child.name_ + "\";\n";
                    }
                }
            }

            return result;
        }
    }

    std::string dot_tree(std::string const& name, topology const& t)
    {
        std::set<std::string> handled_nodes;
        std::string result = "graph \"" + name + "\" {\n";

        if (!t.children_.empty())
        {
            result += detail::dot_tree_helper(t, handled_nodes);
        }
        else if (!t.name_.empty())
        {
            result += "    \"" + t.name_ + "\";\n";
        }

        return result + "}\n";
    }

    ///////////////////////////////////////////////////////////////////////////
    primitive_argument_type extract_value(primitive_argument_type const& val,
        std::string const& name, std::string const& codename)
    {
        switch (val.index())
        {
        case 0: HPX_FALLTHROUGH;    // nil
        case 1: HPX_FALLTHROUGH;    // phylanx::ir::node_data<std::uint8_t>
        case 2: HPX_FALLTHROUGH;    // ir::node_data<std::int64_t>
        case 3: HPX_FALLTHROUGH;    // std::string
        case 4: HPX_FALLTHROUGH;    // phylanx::ir::node_data<double>
        case 5: HPX_FALLTHROUGH;    // primitive
        case 6: HPX_FALLTHROUGH;    // std::vector<ast::expression>
        case 7:                     // phylanx::ir::range
            return val;

        default:
            break;
        }

        std::string type(detail::get_primitive_argument_type_name(val.index()));
        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "phylanx::execution_tree::extract_value",
            generate_error_message(
                "primitive_argument_type does not hold a value type "
                    "(type held: '" + type + "')",
                name, codename));
    }

    primitive_argument_type extract_copy_value(primitive_argument_type const& val,
        std::string const& name, std::string const& codename)
    {
        switch (val.index())
        {
        case 0: HPX_FALLTHROUGH;    // nil
        case 2: HPX_FALLTHROUGH;    // ir::node_data<std::int64_t>
        case 3: HPX_FALLTHROUGH;    // std::string
        case 5: HPX_FALLTHROUGH;    // primitive
        case 6: HPX_FALLTHROUGH;    // std::vector<ast::expression>
        case 7:                     // phylanx::ir::range
            return val;

        case 1:    // phylanx::ir::node_data<std::uint8_t>
            {
                auto const& v = util::get<1>(val);
                if (v.is_ref())
                {
                    return primitive_argument_type{v.copy()};
                }
                return primitive_argument_type{v};
            }
            break;

        case 4:     // phylanx::ir::node_data<double>
            {
                auto const& v = util::get<4>(val);
                if (v.is_ref())
                {
                    return primitive_argument_type{v.copy()};
                }
                return primitive_argument_type{v};
            }
            break;

        default:
            break;
        }

        std::string type(detail::get_primitive_argument_type_name(val.index()));
        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "phylanx::execution_tree::extract_copy_value",
            generate_error_message(
                "primitive_argument_type does not hold a value type "
                    "(type held: '" + type + "')",
                name, codename));
    }

    primitive_argument_type extract_ref_value(primitive_argument_type const& val,
        std::string const& name, std::string const& codename)
    {
        switch (val.index())
        {
        case 0: HPX_FALLTHROUGH;    // nil
        case 3: HPX_FALLTHROUGH;    // std::string
        case 5: HPX_FALLTHROUGH;    // primitive
        case 6: HPX_FALLTHROUGH;    // std::vector<ast::expression>
        case 7:                     // phylanx::ir::range
            return val;

        case 1:    // phylanx::ir::node_data<std::uint8_t>
            {
                auto const& v = util::get<1>(val);
                if (v.is_ref())
                {
                    return primitive_argument_type{v};
                }
                return primitive_argument_type{v.ref()};
            }
            break;

        case 2:    // phylanx::ir::node_data<std::int64_t>
            {
                auto const& v = util::get<2>(val);
                if (v.is_ref())
                {
                    return primitive_argument_type{v};
                }
                return primitive_argument_type{v.ref()};
            }
            break;

            case 4:    // phylanx::ir::node_data<double>
            {
                auto const& v = util::get<4>(val);
                if (v.is_ref())
                {
                    return primitive_argument_type{v};
                }
                return primitive_argument_type{v.ref()};
            }
            break;

        default:
            break;
        }

        std::string type(detail::get_primitive_argument_type_name(val.index()));
        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "phylanx::execution_tree::extract_ref_value",
            generate_error_message(
                "primitive_argument_type does not hold a value type "
                    "(type held: '" + type + "')",
                name, codename));
    }

    primitive_argument_type extract_value(primitive_argument_type&& val,
        std::string const& name, std::string const& codename)
    {
        switch (val.index())
        {
        case 0: HPX_FALLTHROUGH;    // nil
        case 1: HPX_FALLTHROUGH;    // phylanx::ir::node_data<std::uint8_t>
        case 2: HPX_FALLTHROUGH;    // ir::node_data<std::int64_t>
        case 3: HPX_FALLTHROUGH;    // std::string
        case 4: HPX_FALLTHROUGH;    // phylanx::ir::node_data<double>
        case 5: HPX_FALLTHROUGH;    // primitive
        case 6: HPX_FALLTHROUGH;    // std::vector<ast::expression>
        case 7:                     // phylanx::ir::range
            return std::move(val);

        default:
            break;
        }

        std::string type(detail::get_primitive_argument_type_name(val.index()));
        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "phylanx::execution_tree::extract_value",
            generate_error_message(
                "primitive_argument_type does not hold a value type "
                    "(type held: '" + type + "')",
                name, codename));
    }

    primitive_argument_type extract_copy_value(primitive_argument_type&& val,
        std::string const& name, std::string const& codename)
    {
        switch (val.index())
        {
        case 0: HPX_FALLTHROUGH;    // nil
        case 3: HPX_FALLTHROUGH;    // std::string
        case 5: HPX_FALLTHROUGH;    // primitive
        case 6: HPX_FALLTHROUGH;    // std::vector<ast::expression>
        case 7:                     // phylanx::ir::range
            return std::move(val);

        case 1:    // phylanx::ir::node_data<std::uint8_t>
            {
                auto && v = util::get<1>(std::move(val));
                if (v.is_ref())
                {
                    return primitive_argument_type{v.copy()};
                }
                return primitive_argument_type{std::move(v)};
            }
            break;

        case 2:    // phylanx::ir::node_data<std::int64_t>
            {
                auto&& v = util::get<2>(std::move(val));
                if (v.is_ref())
                {
                    return primitive_argument_type{v.copy()};
                }
                return primitive_argument_type{std::move(v)};
            }
            break;

            case 4:    // phylanx::ir::node_data<double>
            {
                auto && v = util::get<4>(std::move(val));
                if (v.is_ref())
                {
                    return primitive_argument_type{v.copy()};
                }
                return primitive_argument_type{std::move(v)};
            }
            break;

        default:
            break;
        }

        std::string type(detail::get_primitive_argument_type_name(val.index()));
        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "phylanx::execution_tree::extract_value",
            generate_error_message(
                "primitive_argument_type does not hold a value type "
                    "(type held: '" + type + "')",
                name, codename));
    }

    primitive_argument_type extract_ref_value(primitive_argument_type&& val,
        std::string const& name, std::string const& codename)
    {
        switch (val.index())
        {
        case 0: HPX_FALLTHROUGH;    // nil
        case 1: HPX_FALLTHROUGH;    // phylanx::ir::node_data<std::uint8_t>
        case 2: HPX_FALLTHROUGH;    // ir::node_data<std::int64_t>
        case 3: HPX_FALLTHROUGH;    // std::string
        case 4: HPX_FALLTHROUGH;    // phylanx::ir::node_data<double>
        case 5: HPX_FALLTHROUGH;    // primitive
        case 6: HPX_FALLTHROUGH;    // std::vector<ast::expression>
        case 7:                     // phylanx::ir::range
            return std::move(val);

        default:
            break;
        }

        std::string type(detail::get_primitive_argument_type_name(val.index()));
        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "phylanx::execution_tree::extract_value",
            generate_error_message(
                "primitive_argument_type does not hold a value type "
                    "(type held: '" + type + "')",
                name, codename));
    }

    ///////////////////////////////////////////////////////////////////////////
    primitive_argument_type extract_literal_value(
        primitive_argument_type const& val,
        std::string const& name, std::string const& codename)
    {
        switch (val.index())
        {
        case 0: HPX_FALLTHROUGH;    // nil
        case 1: HPX_FALLTHROUGH;    // phylanx::ir::node_data<std::uint8_t>
        case 2: HPX_FALLTHROUGH;    // ir::node_data<std::int64_t>
        case 3: HPX_FALLTHROUGH;    // std::string
        case 4: HPX_FALLTHROUGH;    // phylanx::ir::node_data<double>
        case 7:                     // phylanx::ir::range
            return val;

        case 6:                     // std::vector<ast::expression>
            {
                auto const& exprs = util::get<6>(val);
                if (exprs.size() == 1)
                {
                    if (ast::detail::is_literal_value(exprs[0]))
                    {
                        return to_primitive_value_type(
                            ast::detail::literal_value(exprs[0]));
                    }
                }
            }
            break;

        case 5: HPX_FALLTHROUGH;    // primitive
        default:
            break;
        }

        std::string type(detail::get_primitive_argument_type_name(val.index()));
        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "phylanx::execution_tree::extract_literal_value",
            generate_error_message(
                "primitive_argument_type does not hold a literal value type "
                    "(type held: '" + type + "')",
                name, codename));
    }

    primitive_argument_type extract_literal_ref_value(
        primitive_argument_type const& val,
        std::string const& name, std::string const& codename)
    {
        switch (val.index())
        {
        case 0: HPX_FALLTHROUGH;    // nil
        case 1:                     // phylanx::ir::node_data<std::uint8_t>
            return primitive_argument_type{util::get<1>(val).ref()};

        case 2:                     // ir::node_data<std::int64_t>
            {
                auto const& v = util::get<2>(val);
                if (v.is_ref())
                {
                    return primitive_argument_type{v};
                }
                return primitive_argument_type{v.ref()};
            }
                    break;

        case 3:                     // std::string
            return val;

        case 4:     // phylanx::ir::node_data<double>
            {
                auto const& v = util::get<4>(val);
                if (v.is_ref())
                {
                    return primitive_argument_type{v};
                }
                return primitive_argument_type{v.ref()};
            }
            break;

        case 6:                     // std::vector<ast::expression>
            {
                auto const& exprs = util::get<6>(val);
                if (exprs.size() == 1)
                {
                    if (ast::detail::is_literal_value(exprs[0]))
                    {
                        return to_primitive_value_type(
                            ast::detail::literal_value(exprs[0]));
                    }
                }
            }
            break;

        case 7:                     // phylanx::ir::range
            {
                auto const& r = util::get<7>(val);
                if (r.is_ref())
                {
                    return primitive_argument_type{r};
                }
                return primitive_argument_type{r.ref()};
            }
            break;

        case 5: HPX_FALLTHROUGH;    // primitive
        default:
            break;
        }

        std::string type(detail::get_primitive_argument_type_name(val.index()));
        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "phylanx::execution_tree::extract_literal_value",
            generate_error_message(
                "primitive_argument_type does not hold a literal value type "
                    "(type held: '" + type + "')",
                name, codename));
    }

    primitive_argument_type extract_literal_value(primitive_argument_type&& val,
        std::string const& name, std::string const& codename)
    {
        switch (val.index())
        {
        case 0: HPX_FALLTHROUGH;    // nil
        case 1: HPX_FALLTHROUGH;    // phylanx::ir::node_data<std::uint8_t>
        case 2: HPX_FALLTHROUGH;    // ir::node_data<std::int64_t>
        case 3: HPX_FALLTHROUGH;    // std::string
        case 4: HPX_FALLTHROUGH;    // phylanx::ir::node_data<double>
        case 7:                     // phylanx::ir::range
            return std::move(val);

        case 6:                     // std::vector<ast::expression>
            {
                auto && exprs = util::get<6>(std::move(val));
                if (exprs.size() == 1)
                {
                    if (ast::detail::is_literal_value(exprs[0]))
                    {
                        return to_primitive_value_type(
                            ast::detail::literal_value(std::move(exprs[0])));
                    }
                }
            }
            break;

        case 5: HPX_FALLTHROUGH;    // primitive
        default:
            break;
        }

        std::string type(detail::get_primitive_argument_type_name(val.index()));
        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "phylanx::execution_tree::extract_literal_value",
            generate_error_message(
                "primitive_argument_type does not hold a literal value type "
                    "(type held: '" + type + "')",
                name, codename));
    }

    bool is_literal_operand(primitive_argument_type const& val)
    {
        switch (val.index())
        {
        case 0: HPX_FALLTHROUGH;    // nil
        case 1: HPX_FALLTHROUGH;    // phylanx::ir::node_data<std::uint8_t>
        case 2: HPX_FALLTHROUGH;    // ir::node_data<std::int64_t>
        case 3: HPX_FALLTHROUGH;    // std::string
        case 4: HPX_FALLTHROUGH;    // phylanx::ir::node_data<double>
        case 6:                     // std::vector<ast::expression>
            return true;

        case 5: HPX_FALLTHROUGH;    // primitive
        case 7: HPX_FALLTHROUGH;    // phylanx::ir::range
        default:
            break;
        }
        return false;
    }

    ///////////////////////////////////////////////////////////////////////////
    ir::node_data<double> extract_numeric_value(
        primitive_argument_type const& val,
        std::string const& name, std::string const& codename)
    {
        switch (val.index())
        {
        case 1:    // phylanx::ir::node_data<std::uint8_t>
            return ir::node_data<double>{util::get<1>(val).ref()};

        case 2:     // ir::node_data<std::int64_t>
            return ir::node_data<double>{util::get<2>(val).ref()};

        case 4:     // phylanx::ir::node_data<double>
            return util::get<4>(val).ref();

        case 6:     // std::vector<ast::expression>
            {
                auto const& exprs = util::get<6>(val);
                if (exprs.size() == 1)
                {
                    if (ast::detail::is_literal_value(exprs[0]))
                    {
                        return to_primitive_numeric_type(
                            ast::detail::literal_value(exprs[0]));
                    }
                }
            }
            break;

        case 0: HPX_FALLTHROUGH;    // nil
        case 3: HPX_FALLTHROUGH;    // string
        case 5: HPX_FALLTHROUGH;    // primitive
        case 7: HPX_FALLTHROUGH;    // phylanx::ir::range
        default:
            break;
        }

        std::string type(detail::get_primitive_argument_type_name(val.index()));
        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "phylanx::execution_tree::extract_numeric_value",
            generate_error_message(
                "primitive_argument_type does not hold a numeric "
                    "value type (type held: '" + type + "')",
                name, codename));
    }

    ir::node_data<double> extract_numeric_value_strict(
        primitive_argument_type const& val,
        std::string const& name, std::string const& codename)
    {
        switch (val.index())
        {
        case 4:     // phylanx::ir::node_data<double>
            return util::get<4>(val).ref();

        case 0: HPX_FALLTHROUGH;    // nil
        case 1: HPX_FALLTHROUGH;    // phylanx::ir::node_data<std::uint8_t>
        case 2: HPX_FALLTHROUGH;    // phylanx::ir::node_data<std::int64_t>
        case 3: HPX_FALLTHROUGH;    // string
        case 5: HPX_FALLTHROUGH;    // primitive
        case 6: HPX_FALLTHROUGH;    // std::vector<ast::expression>
        case 7: HPX_FALLTHROUGH;    // phylanx::ir::range
        default:
            break;
        }

        std::string type(detail::get_primitive_argument_type_name(val.index()));
        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "phylanx::execution_tree::extract_numeric_value_strict",
            generate_error_message(
                "primitive_argument_type does not hold a numeric "
                    "value type (type held: '" + type + "')",
                name, codename));
    }

    ir::node_data<double> extract_numeric_value(primitive_argument_type&& val,
        std::string const& name, std::string const& codename)
    {
        switch (val.index())
        {
        case 1:    // phylanx::ir::node_data<std::uint8_t>
            return ir::node_data<double>{util::get<1>(std::move(val))};

        case 2:     // ir::node_data<std::int64_t>
            return ir::node_data<double>{util::get<2>(std::move(val))};

        case 4:     // phylanx::ir::node_data<double>
            return util::get<4>(std::move(val));

        case 6:     // std::vector<ast::expression>
            {
                auto && exprs = util::get<6>(std::move(val));
                if (exprs.size() == 1)
                {
                    if (ast::detail::is_literal_value(exprs[0]))
                    {
                        return to_primitive_numeric_type(
                            ast::detail::literal_value(std::move(exprs[0])));
                    }
                }
            }
            break;

        case 0: HPX_FALLTHROUGH;    // nil
        case 3: HPX_FALLTHROUGH;    // string
        case 5: HPX_FALLTHROUGH;    // primitive
        case 7: HPX_FALLTHROUGH;    // phylanx::ir::range
        default:
            break;
        }

        std::string type(detail::get_primitive_argument_type_name(val.index()));
        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "phylanx::execution_tree::extract_numeric_value",
            generate_error_message(
                "primitive_argument_type does not hold a numeric "
                    "value type (type held: '" + type + "')",
                name, codename));
    }

    ir::node_data<double> extract_numeric_value_strict(
        primitive_argument_type&& val, std::string const& name,
        std::string const& codename)
    {
        switch (val.index())
        {
        case 4:     // phylanx::ir::node_data<double>
            return util::get<4>(std::move(val));

        case 0: HPX_FALLTHROUGH;    // nil
        case 1: HPX_FALLTHROUGH;    // phylanx::ir::node_data<std::uint8_t>
        case 2: HPX_FALLTHROUGH;    // phylanx::ir::node_data<std::int64_t>
        case 3: HPX_FALLTHROUGH;    // string
        case 5: HPX_FALLTHROUGH;    // primitive
        case 6: HPX_FALLTHROUGH;    // std::vector<ast::expression>
        case 7: HPX_FALLTHROUGH;    // phylanx::ir::range
        default:
            break;
        }

        std::string type(detail::get_primitive_argument_type_name(val.index()));
        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "phylanx::execution_tree::extract_numeric_value_strict",
            generate_error_message(
                "primitive_argument_type does not hold a numeric "
                    "value type (type held: '" + type + "')",
                name, codename));
    }

    bool is_numeric_operand(primitive_argument_type const& val)
    {
        switch (val.index())
        {
        case 1: HPX_FALLTHROUGH;    // phylanx::ir::node_data<std::uint8_t>
        case 2: HPX_FALLTHROUGH;    // ir::node_data<std::int64_t>
        case 4: HPX_FALLTHROUGH;    // phylanx::ir::node_data<double>
        case 6:                     // std::vector<ast::expression>
            return true;

        case 0: HPX_FALLTHROUGH;    // nil
        case 3: HPX_FALLTHROUGH;    // string
        case 5: HPX_FALLTHROUGH;    // primitive
        case 7: HPX_FALLTHROUGH;    // phylanx::ir::range
        default:
            break;
        }
        return false;
    }

    bool is_numeric_operand_strict(primitive_argument_type const& val)
    {
        switch (val.index())
        {
        case 4:     // phylanx::ir::node_data<double>
            return true;

        case 0: HPX_FALLTHROUGH;    // nil
        case 1: HPX_FALLTHROUGH;    // phylanx::ir::node_data<std::uint8_t>
        case 2: HPX_FALLTHROUGH;    // phylanx::ir::node_data<std::int64_t>
        case 3: HPX_FALLTHROUGH;    // string
        case 5: HPX_FALLTHROUGH;    // primitive
        case 6: HPX_FALLTHROUGH;    // std::vector<ast::expression>
        case 7: HPX_FALLTHROUGH;    // phylanx::ir::range
        default:
            break;
        }
        return false;
    }

    std::size_t extract_numeric_value_dimension(
        primitive_argument_type const& val, std::string const& name,
        std::string const& codename)
    {
        switch (val.index())
        {
        case 1:    // phylanx::ir::node_data<std::uint8_t>
            return util::get<1>(val).num_dimensions();

        case 2:     // ir::node_data<std::int64_t>
            return util::get<2>(val).num_dimensions();

        case 4:     // phylanx::ir::node_data<double>
            return util::get<4>(val).num_dimensions();

        case 6:     // std::vector<ast::expression>
            {
                auto const& exprs = util::get<6>(val);
                if (exprs.size() == 1)
                {
                    if (ast::detail::is_literal_value(exprs[0]))
                    {
                        return to_primitive_numeric_type(
                            ast::detail::literal_value(exprs[0]))
                                .num_dimensions();
                    }
                }
            }
            break;

        case 0: HPX_FALLTHROUGH;    // nil
        case 3: HPX_FALLTHROUGH;    // string
        case 5: HPX_FALLTHROUGH;    // primitive
        case 7: HPX_FALLTHROUGH;    // phylanx::ir::range
        default:
            break;
        }

        std::string type(detail::get_primitive_argument_type_name(val.index()));
        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "phylanx::execution_tree::extract_numeric_value_dimension",
            generate_error_message(
                "primitive_argument_type does not hold a numeric "
                    "value type (type held: '" + type + "')",
                name, codename));
    }

    std::array<std::size_t, 2> extract_numeric_value_dimensions(
        primitive_argument_type const& val, std::string const& name,
        std::string const& codename)
    {
        switch (val.index())
        {
        case 1:    // phylanx::ir::node_data<std::uint8_t>
            return util::get<1>(val).dimensions();

        case 2:     // ir::node_data<std::int64_t>
            return util::get<2>(val).dimensions();

        case 4:     // phylanx::ir::node_data<double>
            return util::get<4>(val).dimensions();

        case 6:     // std::vector<ast::expression>
            {
                auto const& exprs = util::get<6>(val);
                if (exprs.size() == 1)
                {
                    if (ast::detail::is_literal_value(exprs[0]))
                    {
                        return to_primitive_numeric_type(
                            ast::detail::literal_value(exprs[0])).dimensions();
                    }
                }
            }
            break;

        case 0: HPX_FALLTHROUGH;    // nil
        case 3: HPX_FALLTHROUGH;    // string
        case 5: HPX_FALLTHROUGH;    // primitive
        case 7: HPX_FALLTHROUGH;    // phylanx::ir::range
        default:
            break;
        }

        std::string type(detail::get_primitive_argument_type_name(val.index()));
        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "phylanx::execution_tree::extract_numeric_value_dimensions",
            generate_error_message(
                "primitive_argument_type does not hold a numeric "
                    "value type (type held: '" + type + "')",
                name, codename));
    }

    ///////////////////////////////////////////////////////////////////////////
    ir::node_data<std::uint8_t>
    extract_boolean_data(primitive_argument_type const& val,
        std::string const& name, std::string const& codename)
    {
        switch (val.index())
        {
        case 1:                     // phylanx::ir::node_data<std::uint8_t>
            return util::get<1>(val).ref();

        case 0: HPX_FALLTHROUGH;    // nil
        case 2: HPX_FALLTHROUGH;    // ir::node_data<std::int64_t>
        case 3: HPX_FALLTHROUGH;    // string
        case 4: HPX_FALLTHROUGH;    // phylanx::ir::node_data<double>
        case 5: HPX_FALLTHROUGH;    // primitive
        case 6: HPX_FALLTHROUGH;    // std::vector<ast::expression>
        case 7: HPX_FALLTHROUGH;    // phylanx::ir::range
        default:
            break;
        }

        std::string type(detail::get_primitive_argument_type_name(val.index()));
        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "phylanx::execution_tree::extract_boolean_data",
            generate_error_message(
                "primitive_argument_type does not hold a boolean "
                    "value type (type held: '" + type + "')",
                name, codename));
    }

    ir::node_data<std::uint8_t> extract_boolean_data(primitive_argument_type&& val,
        std::string const& name, std::string const& codename)
    {
        switch (val.index())
        {
        case 1:                     // phylanx::ir::node_data<std::uint8_t>
            return util::get<1>(std::move(val));

        case 0: HPX_FALLTHROUGH;    // nil
        case 2: HPX_FALLTHROUGH;    // ir::node_data<std::int64_t>
        case 3: HPX_FALLTHROUGH;    // string
        case 4: HPX_FALLTHROUGH;    // phylanx::ir::node_data<double>
        case 5: HPX_FALLTHROUGH;    // primitive
        case 6: HPX_FALLTHROUGH;    // std::vector<ast::expression>
        case 7: HPX_FALLTHROUGH;    // phylanx::ir::range
        default:
            break;
        }

        std::string type(detail::get_primitive_argument_type_name(val.index()));
        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "phylanx::execution_tree::extract_boolean_data",
            generate_error_message(
                "primitive_argument_type does not hold a boolean "
                    "value type (type held: '" + type + "')",
                name, codename));
    }

    bool is_boolean_data_operand(primitive_argument_type const& val)
    {
        switch (val.index())
        {
        case 1:                     // phylanx::ir::node_data<std::uint8_t>
            return true;

        case 0: HPX_FALLTHROUGH;    // nil
        case 2: HPX_FALLTHROUGH;    // ir::node_data<std::int64_t>
        case 3: HPX_FALLTHROUGH;    // string
        case 4: HPX_FALLTHROUGH;    // phylanx::ir::node_data<double>
        case 5: HPX_FALLTHROUGH;    // primitive
        case 6: HPX_FALLTHROUGH;    // std::vector<ast::expression>
        case 7: HPX_FALLTHROUGH;    // phylanx::ir::range
        default:
            break;
        }
        return false;
    }

    ///////////////////////////////////////////////////////////////////////////
    ir::node_data<std::int64_t> extract_integer_value(
        primitive_argument_type const& val, std::string const& name,
        std::string const& codename)
    {
        switch (val.index())
        {
        case 1:    // phylanx::ir::node_data<std::uint8_t>
            return ir::node_data<std::int64_t>(util::get<1>(val));

        case 2:     // ir::node_data<std::int64_t>
            return util::get<2>(val);

        case 4:     // phylanx::ir::node_data<double>
            return ir::node_data<std::int64_t>(util::get<4>(val));

        case 6:     // std::vector<ast::expression>
            {
                auto const& exprs = util::get<6>(val);
                if (exprs.size() == 1)
                {
                    if (ast::detail::is_literal_value(exprs[0]))
                    {
                        return to_primitive_int_type(
                            ast::detail::literal_value(exprs[0]));
                    }
                }
            }
            break;

        case 0: HPX_FALLTHROUGH;    // nil
        case 3: HPX_FALLTHROUGH;    // string
        case 5: HPX_FALLTHROUGH;    // primitive
        case 7: HPX_FALLTHROUGH;    // phylanx::ir::range
        default:
            break;
        }

        std::string type(detail::get_primitive_argument_type_name(val.index()));
        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "phylanx::execution_tree::extract_integer_value",
            generate_error_message(
                "primitive_argument_type does not hold an integer "
                    "value type (type held: '" + type + "')",
                name, codename));
    }

    ir::node_data<std::int64_t> extract_integer_value(
        primitive_argument_type&& val, std::string const& name,
        std::string const& codename)
    {
        switch (val.index())
        {
        case 1:    // phylanx::ir::node_data<std::uint8_t>
            return ir::node_data<std::int64_t>(util::get<1>(std::move(val)));

        case 2:     // ir::node_data<std::int64_t>
            return util::get<2>(std::move(val));

        case 4:     // phylanx::ir::node_data<double>
            return ir::node_data<std::int64_t>(util::get<4>(std::move(val)));

        case 6:     // std::vector<ast::expression>
            {
                auto && exprs = util::get<6>(std::move(val));
                if (exprs.size() == 1)
                {
                    if (ast::detail::is_literal_value(exprs[0]))
                    {
                        return to_primitive_int_type(
                            ast::detail::literal_value(std::move(exprs[0])));
                    }
                }
            }
            break;

        case 0: HPX_FALLTHROUGH;    // nil
        case 3: HPX_FALLTHROUGH;    // string
        case 5: HPX_FALLTHROUGH;    // primitive
        case 7: HPX_FALLTHROUGH;    // phylanx::ir::range
        default:
            break;
        }

        std::string type(detail::get_primitive_argument_type_name(val.index()));
        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "phylanx::execution_tree::extract_integer_value",
            generate_error_message(
                "primitive_argument_type does not hold an integer "
                    "value type (type held: '" + type + "')",
                name, codename));
    }

    std::int64_t extract_scalar_integer_value(
        primitive_argument_type const& val, std::string const& name,
        std::string const& codename)
    {
        switch (val.index())
        {
        case 1:    // phylanx::ir::node_data<std::uint8_t>
            if (util::get<1>(val).num_dimensions() == 0)
                return std::int64_t(util::get<1>(val)[0]);
            break;

        case 2:    // ir::node_data<std::int64_t>
            if (util::get<2>(val).num_dimensions() == 0)
                return util::get<2>(val)[0];
            break;

        case 4:    // phylanx::ir::node_data<double>
            if (util::get<4>(val).num_dimensions() == 0)
                return std::int64_t(util::get<4>(val)[0]);
            break;

        case 6:    // std::vector<ast::expression>
        {
            auto const& exprs = util::get<6>(val);
            if (exprs.size() == 1)
            {
                if (ast::detail::is_literal_value(exprs[0]))
                {
                    return to_primitive_int_type(
                        ast::detail::literal_value(exprs[0]))[0];
                }
            }
        }
        break;

        case 0:HPX_FALLTHROUGH;    // nil
        case 3:HPX_FALLTHROUGH;    // string
        case 5:HPX_FALLTHROUGH;    // primitive
        case 7:HPX_FALLTHROUGH;    // phylanx::ir::range
        default:
            break;
        }

        std::string type(detail::get_primitive_argument_type_name(val.index()));
        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "phylanx::execution_tree::extract_scalar_integer_value",
            generate_error_message(
                "primitive_argument_type does not hold an integer "
                "value type (type held: '" +
                    type + "')",
                name, codename));
    }

    std::int64_t extract_scalar_integer_value(
        primitive_argument_type&& val, std::string const& name,
        std::string const& codename)
    {
        switch (val.index())
        {
        case 1:    // phylanx::ir::node_data<std::uint8_t>
            if (util::get<1>(val).num_dimensions() == 0)
                return std::int64_t(util::get<1>(std::move(val))[0]);
            break;

        case 2:    // ir::node_data<std::int64_t>
            if (util::get<2>(val).num_dimensions() == 0)
                return util::get<2>(std::move(val))[0];
            break;

        case 4:    // phylanx::ir::node_data<double>
            if (util::get<4>(val).num_dimensions() == 0)
                return std::int64_t(util::get<4>(std::move(val))[0]);
            break;

        case 6:    // std::vector<ast::expression>
        {
            auto&& exprs = util::get<6>(std::move(val));
            if (exprs.size() == 1)
            {
                if (ast::detail::is_literal_value(exprs[0]))
                {
                    return to_primitive_int_type(
                        ast::detail::literal_value(std::move(exprs[0])))[0];
                }
            }
        }
        break;

        case 0:HPX_FALLTHROUGH;    // nil
        case 3:HPX_FALLTHROUGH;    // string
        case 5:HPX_FALLTHROUGH;    // primitive
        case 7:HPX_FALLTHROUGH;    // phylanx::ir::range
        default:
            break;
        }

        std::string type(detail::get_primitive_argument_type_name(val.index()));
        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "phylanx::execution_tree::extract_scalar_integer_value",
            generate_error_message(
                "primitive_argument_type does not hold an integer "
                "value type (type held: '" +
                    type + "')",
                name, codename));
    }

    bool is_integer_operand(primitive_argument_type const& val)
    {
        switch (val.index())
        {
        case 1: HPX_FALLTHROUGH;    // phylanx::ir::node_data<std::uint8_t>
        case 2: HPX_FALLTHROUGH;    // ir::node_data<std::int64_t>
        case 4: HPX_FALLTHROUGH;    // phylanx::ir::node_data<double>
        case 6:     // std::vector<ast::expression>
            return true;

        case 0: HPX_FALLTHROUGH;    // nil
        case 3: HPX_FALLTHROUGH;    // string
        case 5: HPX_FALLTHROUGH;    // primitive
        case 7: HPX_FALLTHROUGH;    // phylanx::ir::range
        default:
            break;
        }
        return false;
    }

    ///////////////////////////////////////////////////////////////////////////
    std::int64_t extract_scalar_integer_value_strict(
        primitive_argument_type const& val, std::string const& name,
        std::string const& codename)
    {
        switch (val.index())
        {
        case 2:     // ir::node_data<std::int64_t>
            if (util::get<2>(val).num_dimensions() == 0)
                return util::get<2>(val)[0];
            break;

        case 6:     // std::vector<ast::expression>
            {
                auto && exprs = util::get<6>(std::move(val));
                if (exprs.size() == 1)
                {
                    if (ast::detail::is_literal_value(exprs[0]))
                    {
                        return to_primitive_int_type(
                            ast::detail::literal_value(std::move(exprs[0])))[0];
                    }
                }
            }
            break;

        case 0: HPX_FALLTHROUGH;    // nil
        case 1: HPX_FALLTHROUGH;    // phylanx::ir::node_data<std::uint8_t>
        case 3: HPX_FALLTHROUGH;    // string
        case 4: HPX_FALLTHROUGH;    // phylanx::ir::node_data<double>
        case 5: HPX_FALLTHROUGH;    // primitive
        case 7: HPX_FALLTHROUGH;    // phylanx::ir::range
        default:
            break;
        }

        std::string type(detail::get_primitive_argument_type_name(val.index()));
        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "phylanx::execution_tree::extract_scalar_integer_value_strict",
            generate_error_message(
                "primitive_argument_type does not hold an integer "
                    "value type (type held: '" + type + "')",
                name, codename));
    }

    std::int64_t extract_scalar_integer_value_strict(
        primitive_argument_type&& val, std::string const& name,
        std::string const& codename)
    {
        switch (val.index())
        {
        case 2:    // ir::node_data<std::int64_t>
            if(util::get<2>(val).num_dimensions()==0)
                return util::get<2>(std::move(val))[0];
            break;

        case 6:    // std::vector<ast::expression>
        {
            auto&& exprs = util::get<6>(std::move(val));
            if (exprs.size() == 1)
            {
                if (ast::detail::is_literal_value(exprs[0]))
                {
                    return to_primitive_int_type(
                        ast::detail::literal_value(std::move(exprs[0])))[0];
                }
            }
        }
        break;

        case 0:HPX_FALLTHROUGH;    // nil
        case 1:HPX_FALLTHROUGH;    // phylanx::ir::node_data<std::uint8_t>
        case 3:HPX_FALLTHROUGH;    // string
        case 4:HPX_FALLTHROUGH;    // phylanx::ir::node_data<double>
        case 5:HPX_FALLTHROUGH;    // primitive
        case 7:HPX_FALLTHROUGH;    // phylanx::ir::range
        default:
            break;
        }

        std::string type(detail::get_primitive_argument_type_name(val.index()));
        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "phylanx::execution_tree::extract_scalar_integer_value_strict",
            generate_error_message(
                "primitive_argument_type does not hold an integer "
                "value type (type held: '" +
                    type + "')",
                name, codename));
    }

    ir::node_data<std::int64_t> extract_integer_value_strict(
        primitive_argument_type const& val, std::string const& name,
        std::string const& codename)
    {
        switch (val.index())
        {
        case 2:     // ir::node_data<std::int64_t>
            return util::get<2>(val);

        case 6:     // std::vector<ast::expression>
        {
            auto && exprs = util::get<6>(std::move(val));
            if (exprs.size() == 1)
            {
                if (ast::detail::is_literal_value(exprs[0]))
                {
                    return to_primitive_int_type(
                            ast::detail::literal_value(std::move(exprs[0])));
                }
            }
        }
            break;

        case 0: HPX_FALLTHROUGH;    // nil
        case 1: HPX_FALLTHROUGH;    // phylanx::ir::node_data<std::uint8_t>
        case 3: HPX_FALLTHROUGH;    // string
        case 4: HPX_FALLTHROUGH;    // phylanx::ir::node_data<double>
        case 5: HPX_FALLTHROUGH;    // primitive
        case 7: HPX_FALLTHROUGH;    // phylanx::ir::range
        default:
            break;
        }

        std::string type(detail::get_primitive_argument_type_name(val.index()));
        HPX_THROW_EXCEPTION(hpx::bad_parameter,
                            "phylanx::execution_tree::extract_integer_value_strict",
                            generate_error_message(
                                    "primitive_argument_type does not hold an integer "
                                    "value type (type held: '" + type + "')",
                                    name, codename));
    }

    ir::node_data<std::int64_t> extract_integer_value_strict(
            primitive_argument_type&& val, std::string const& name,
            std::string const& codename)
    {
        switch (val.index())
        {
            case 2:    // ir::node_data<std::int64_t>
                return util::get<2>(std::move(val));

            case 6:    // std::vector<ast::expression>
            {
                auto&& exprs = util::get<6>(std::move(val));
                if (exprs.size() == 1)
                {
                    if (ast::detail::is_literal_value(exprs[0]))
                    {
                        return to_primitive_int_type(
                                ast::detail::literal_value(std::move(exprs[0])));
                    }
                }
            }
                break;

            case 0:HPX_FALLTHROUGH;    // nil
            case 1:HPX_FALLTHROUGH;    // phylanx::ir::node_data<std::uint8_t>
            case 3:HPX_FALLTHROUGH;    // string
            case 4:HPX_FALLTHROUGH;    // phylanx::ir::node_data<double>
            case 5:HPX_FALLTHROUGH;    // primitive
            case 7:HPX_FALLTHROUGH;    // phylanx::ir::range
            default:
                break;
        }

        std::string type(detail::get_primitive_argument_type_name(val.index()));
        HPX_THROW_EXCEPTION(hpx::bad_parameter,
                            "phylanx::execution_tree::extract_integer_value_strict",
                            generate_error_message(
                                    "primitive_argument_type does not hold an integer "
                                    "value type (type held: '" +
                                    type + "')",
                                    name, codename));
    }

    bool is_integer_operand_strict(primitive_argument_type const& val)
    {
        switch (val.index())
        {
        case 2:     // ir::node_data<std::int64_t>
            return true;

        case 0: HPX_FALLTHROUGH;    // nil
        case 1: HPX_FALLTHROUGH;    // phylanx::ir::node_data<std::uint8_t>
        case 3: HPX_FALLTHROUGH;    // string
        case 4: HPX_FALLTHROUGH;    // phylanx::ir::node_data<double>
        case 5: HPX_FALLTHROUGH;    // primitive
        case 6: HPX_FALLTHROUGH;    // std::vector<ast::expression>
        case 7: HPX_FALLTHROUGH;    // phylanx::ir::range
        default:
            break;
        }
        return false;
    }

    ///////////////////////////////////////////////////////////////////////////
    ir::node_data<std::uint8_t> extract_boolean_value(
        primitive_argument_type const& val, std::string const& name,
        std::string const& codename)
    {
        switch (val.index())
        {
        case 1:    // phylanx::ir::node_data<std::uint8_t>
            return util::get<1>(val);

        case 2:     // ir::node_data<std::int64_t>
            return ir::node_data<std::uint8_t>{util::get<2>(val)};

        case 4:     // phylanx::ir::node_data<double>
            return ir::node_data<std::uint8_t>{util::get<4>(val)};

        case 6:     // std::vector<ast::expression>
            {
                auto const& exprs = util::get<6>(val);
                if (exprs.size() == 1)
                {
                    if (ast::detail::is_literal_value(exprs[0]))
                    {
                        return to_primitive_bool_type(
                            ast::detail::literal_value(std::move(exprs[0])));
                    }
                }
            }
            break;

        case 0: HPX_FALLTHROUGH;    // nil
        case 3: HPX_FALLTHROUGH;    // string
        case 5: HPX_FALLTHROUGH;    // primitive
        case 7: HPX_FALLTHROUGH;    // phylanx::ir::range
        default:
            break;
        }

        std::string type(detail::get_primitive_argument_type_name(val.index()));
        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "phylanx::execution_tree::extract_boolean_value",
            generate_error_message(
                "primitive_argument_type does not hold a boolean "
                    "value type (type held: '" + type + "')",
                name, codename));
    }

    ir::node_data<std::uint8_t> extract_boolean_value_strict(
        primitive_argument_type const& val, std::string const& name,
        std::string const& codename)
    {
        switch (val.index())
        {
        case 1:    // phylanx::ir::node_data<std::uint8_t>
            return util::get<1>(val);

        case 0: HPX_FALLTHROUGH;    // nil
        case 2: HPX_FALLTHROUGH;    // ir::node_data<std::int64_t>
        case 3: HPX_FALLTHROUGH;    // string
        case 4: HPX_FALLTHROUGH;    // phylanx::ir::node_data<double>
        case 5: HPX_FALLTHROUGH;    // primitive
        case 6: HPX_FALLTHROUGH;    // std::vector<ast::expression>
        case 7: HPX_FALLTHROUGH;    // phylanx::ir::range
        default:
            break;
        }

        std::string type(detail::get_primitive_argument_type_name(val.index()));
        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "phylanx::execution_tree::extract_boolean_value_strict",
            generate_error_message(
                "primitive_argument_type does not hold a boolean "
                    "value type (type held: '" + type + "')",
                name, codename));
    }

    ir::node_data<std::uint8_t> extract_boolean_value(
        primitive_argument_type&& val, std::string const& name,
        std::string const& codename)
    {
        switch (val.index())
        {
        case 1:    // phylanx::ir::node_data<std::uint8_t>
            return util::get<1>(std::move(val));

        case 2:     // ir::node_data<std::int64_t>
            return ir::node_data<std::uint8_t>{util::get<2>(std::move(val))};

        case 4:     // phylanx::ir::node_data<double>
            return ir::node_data<std::uint8_t>{util::get<4>(std::move(val))};

        case 6:     // std::vector<ast::expression>
            {
                auto const& exprs = util::get<6>(val);
                if (exprs.size() == 1)
                {
                    if (ast::detail::is_literal_value(exprs[0]))
                    {
                        return to_primitive_bool_type(
                            ast::detail::literal_value(std::move(exprs[0])));
                    }
                }
            }
            break;

        case 0: HPX_FALLTHROUGH;    // nil
        case 3: HPX_FALLTHROUGH;    // string
        case 5: HPX_FALLTHROUGH;    // primitive
        case 7: HPX_FALLTHROUGH;    // phylanx::ir::range
        default:
            break;
        }

        std::string type(detail::get_primitive_argument_type_name(val.index()));
        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "phylanx::execution_tree::extract_boolean_value",
            generate_error_message(
                "primitive_argument_type does not hold a boolean "
                    "value type (type held: '" + type + "')",
                name, codename));
    }

    ir::node_data<std::uint8_t> extract_boolean_value_strict(
        primitive_argument_type&& val, std::string const& name,
        std::string const& codename)
    {
        switch (val.index())
        {
        case 1:    // phylanx::ir::node_data<std::uint8_t>
            return util::get<1>(std::move(val));

        case 0: HPX_FALLTHROUGH;    // nil
        case 2: HPX_FALLTHROUGH;    // ir::node_data<std::int64_t>
        case 3: HPX_FALLTHROUGH;    // string
        case 4: HPX_FALLTHROUGH;    // phylanx::ir::node_data<double>
        case 5: HPX_FALLTHROUGH;    // primitive
        case 6: HPX_FALLTHROUGH;    // std::vector<ast::expression>
        case 7: HPX_FALLTHROUGH;    // phylanx::ir::range
        default:
            break;
        }

        std::string type(detail::get_primitive_argument_type_name(val.index()));
        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "phylanx::execution_tree::extract_boolean_value_strict",
            generate_error_message(
                "primitive_argument_type does not hold a boolean "
                    "value type (type held: '" + type + "')",
                name, codename));
    }

    ///////////////////////////////////////////////////////////////////////////
    std::uint8_t extract_scalar_boolean_value(
        primitive_argument_type const& val, std::string const& name,
        std::string const& codename)
    {
        switch (val.index())
        {
        case 0:     // nil
            return false;

        case 1:    // phylanx::ir::node_data<std::uint8_t>
            return bool(util::get<1>(val));

        case 2:     // ir::node_data<std::int64_t>
            return bool(util::get<2>(val));

        case 4:     // phylanx::ir::node_data<double>
            return bool(util::get<4>(val));

        case 6:     // std::vector<ast::expression>
            {
                auto const& exprs = util::get<6>(val);
                if (exprs.size() == 1)
                {
                    if (ast::detail::is_literal_value(exprs[0]))
                    {
                        return to_primitive_bool_type(
                            ast::detail::literal_value(std::move(exprs[0])))[0];
                    }
                }
            }
            break;

        case 7:     // phylanx::ir::range
            return !(util::get<7>(val).empty());

        case 3: HPX_FALLTHROUGH;    // string
        case 5: HPX_FALLTHROUGH;    // primitive
        default:
            break;
        }

        std::string type(detail::get_primitive_argument_type_name(val.index()));
        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "phylanx::execution_tree::extract_scalar_boolean_value",
            generate_error_message(
                "primitive_argument_type does not hold a boolean "
                    "value type (type held: '" + type + "')",
                name, codename));
    }

    std::uint8_t extract_scalar_boolean_value(primitive_argument_type && val,
        std::string const& name, std::string const& codename)
    {
        switch (val.index())
        {
        case 0:     // nil
            return false;

        case 1:    // phylanx::ir::node_data<std::uint8_t>
            return bool(util::get<1>(std::move(val)));

        case 2:     // ir::node_data<std::int64_t>
            return bool(util::get<2>(std::move(val)));

        case 4:     // phylanx::ir::node_data<double>
            return bool(util::get<4>(std::move(val)));

        case 6:     // std::vector<ast::expression>
            {
                auto && exprs = util::get<6>(std::move(val));
                if (exprs.size() == 1)
                {
                    if (ast::detail::is_literal_value(exprs[0]))
                    {
                        return to_primitive_bool_type(
                            ast::detail::literal_value(std::move(exprs[0])))[0];
                    }
                }
            }
            break;

        case 7:     // phylanx::ir::range
            return !(util::get<7>(std::move(val)).empty());

        case 3: HPX_FALLTHROUGH;    // string
        case 5: HPX_FALLTHROUGH;    // primitive
        default:
            break;
        }

        std::string type(detail::get_primitive_argument_type_name(val.index()));
        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "phylanx::execution_tree::extract_scalar_boolean_value",
            generate_error_message(
                "primitive_argument_type does not hold a boolean "
                    "value type (type held: '" + type + "')",
                name, codename));
    }

    bool is_boolean_operand(primitive_argument_type const& val)
    {
        switch (val.index())
        {
        case 0: HPX_FALLTHROUGH;    // nil
        case 1: HPX_FALLTHROUGH;    // phylanx::ir::node_data<std::uint8_t>
        case 2: HPX_FALLTHROUGH;    // ir::node_data<std::int64_t>
        case 4: HPX_FALLTHROUGH;    // phylanx::ir::node_data<double>
        case 6: HPX_FALLTHROUGH;    // std::vector<ast::expression>
        case 7:                     // phylanx::ir::range
            return true;

        case 3: HPX_FALLTHROUGH;    // string
        case 5: HPX_FALLTHROUGH;    // primitive
        default:
            break;
        }
        return false;
    }

    bool is_boolean_operand_strict(primitive_argument_type const& val)
    {
        switch (val.index())
        {
        case 1:    // phylanx::ir::node_data<std::uint8_t>
            return true;

        case 0: HPX_FALLTHROUGH;    // nil
        case 2: HPX_FALLTHROUGH;    // ir::node_data<std::int64_t>
        case 3: HPX_FALLTHROUGH;    // string
        case 4: HPX_FALLTHROUGH;    // phylanx::ir::node_data<double>
        case 5: HPX_FALLTHROUGH;    // primitive
        case 6: HPX_FALLTHROUGH;    // std::vector<ast::expression>
        case 7:                     // phylanx::ir::range
        default:
            break;
        }
        return false;
    }

    ///////////////////////////////////////////////////////////////////////////
    std::string extract_string_value(primitive_argument_type const& val,
        std::string const& name, std::string const& codename)
    {
        switch (val.index())
        {
        case 3:     // string
            return util::get<3>(val);

        case 6:     // std::vector<ast::expression>
            {
                auto && exprs = util::get<6>(val);
                if (exprs.size() == 1)
                {
                    if (ast::detail::is_literal_value(exprs[0]))
                    {
                        return to_primitive_string_type(
                            ast::detail::literal_value(exprs[0]));
                    }
                }
            }
            break;

        case 0: HPX_FALLTHROUGH;    // nil
        case 1: HPX_FALLTHROUGH;    // phylanx::ir::node_data<std::uint8_t>
        case 2: HPX_FALLTHROUGH;    // ir::node_data<std::int64_t>
        case 4: HPX_FALLTHROUGH;    // phylanx::ir::node_data<double>
        case 7: HPX_FALLTHROUGH;    // phylanx::ir::range
        case 5: HPX_FALLTHROUGH;    // primitive
        default:
            break;
        }

        std::string type(detail::get_primitive_argument_type_name(val.index()));
        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "phylanx::execution_tree::extract_string_value",
            generate_error_message(
                "primitive_argument_type does not hold a string "
                    "value type (type held: '" + type + "')",
                name, codename));
    }

    std::string extract_string_value(primitive_argument_type && val,
        std::string const& name, std::string const& codename)
    {
        switch (val.index())
        {
        case 3:     // string
            return util::get<3>(std::move(val));

        case 6:     // std::vector<ast::expression>
            {
                auto && exprs = util::get<6>(std::move(val));
                if (exprs.size() == 1)
                {
                    if (ast::detail::is_literal_value(exprs[0]))
                    {
                        return to_primitive_string_type(
                            ast::detail::literal_value(std::move(exprs[0])));
                    }
                }
            }
            break;

        case 0: HPX_FALLTHROUGH;    // nil
        case 1: HPX_FALLTHROUGH;    // phylanx::ir::node_data<std::uint8_t>
        case 2: HPX_FALLTHROUGH;    // ir::node_data<std::int64_t>
        case 4: HPX_FALLTHROUGH;    // phylanx::ir::node_data<double>
        case 7: HPX_FALLTHROUGH;    // phylanx::ir::range
        case 5: HPX_FALLTHROUGH;    // primitive
        default:
            break;
        }

        std::string type(detail::get_primitive_argument_type_name(val.index()));
        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "phylanx::execution_tree::extract_string_value",
            generate_error_message(
                "primitive_argument_type does not hold a string "
                    "value type (type held: '" + type + "')",
                name, codename));
    }

    bool is_string_operand(primitive_argument_type const& val)
    {
        switch (val.index())
        {
        case 3: HPX_FALLTHROUGH;    // string
        case 6:     // std::vector<ast::expression>
            return true;

        case 0: HPX_FALLTHROUGH;    // nil
        case 1: HPX_FALLTHROUGH;    // phylanx::ir::node_data<std::uint8_t>
        case 2: HPX_FALLTHROUGH;    // ir::node_data<std::int64_t>
        case 4: HPX_FALLTHROUGH;    // phylanx::ir::node_data<double>
        case 7: HPX_FALLTHROUGH;    // phylanx::ir::range
        case 5: HPX_FALLTHROUGH;    // primitive
        default:
            break;
        }
        return false;
    }

    ///////////////////////////////////////////////////////////////////////////
    std::vector<ast::expression> extract_ast_value(
        primitive_argument_type const& val,
        std::string const& name, std::string const& codename)
    {
        switch (val.index())
        {
        case 2:     // ir::node_data<std::int64_t>
            return {ast::expression(util::get<2>(val)[0])};

        case 3:     // string
            return {ast::expression(util::get<3>(val))};

        case 4:     // phylanx::ir::node_data<double>
            return {ast::expression(util::get<4>(val))};

        case 6:     // std::vector<ast::expression>
            return util::get<6>(val);

        case 0: HPX_FALLTHROUGH;    // nil
        case 1: HPX_FALLTHROUGH;    // phylanx::ir::node_data<std::uint8_t>
        case 5: HPX_FALLTHROUGH;    // primitive
        case 7: HPX_FALLTHROUGH;    // phylanx::ir::range
        default:
            break;
        }

        std::string type(detail::get_primitive_argument_type_name(val.index()));
        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "phylanx::execution_tree::extract_ast_value",
            generate_error_message(
                "primitive_argument_type does not hold a boolean "
                    "value type (type held: '" + type + "')",
                name, codename));
    }

    std::vector<ast::expression> extract_ast_value(
        primitive_argument_type && val,
        std::string const& name, std::string const& codename)
    {
        switch (val.index())
        {
        case 2:     // ir::node_data<std::int64_t>
            return {ast::expression(util::get<2>(std::move(val))[0])};

        case 3:     // string
            return {ast::expression(util::get<3>(std::move(val)))};

        case 4:     // phylanx::ir::node_data<double>
            return {ast::expression(util::get<4>(std::move(val)))};

        case 6:     // std::vector<ast::expression>
            return util::get<6>(std::move(val));

        case 0: HPX_FALLTHROUGH;    // nil
        case 1: HPX_FALLTHROUGH;    // phylanx::ir::node_data<std::uint8_t>
        case 5: HPX_FALLTHROUGH;    // primitive
        case 7: HPX_FALLTHROUGH;    // phylanx::ir::range
        default:
            break;
        }

        std::string type(detail::get_primitive_argument_type_name(val.index()));
        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "phylanx::execution_tree::extract_ast_value",
            generate_error_message(
                "primitive_argument_type does not hold a boolean "
                    "value type (type held: '" + type + "')",
                name, codename));
    }

    bool is_ast_operand(primitive_argument_type const& val)
    {
        switch (val.index())
        {
        case 2: HPX_FALLTHROUGH;    // ir::node_data<std::int64_t>
        case 3: HPX_FALLTHROUGH;    // string
        case 4: HPX_FALLTHROUGH;    // phylanx::ir::node_data<double>
        case 6:     // std::vector<ast::expression>
            return true;

        case 0: HPX_FALLTHROUGH;    // nil
        case 1: HPX_FALLTHROUGH;        // phylanx::ir::node_data<std::uint8_t>
        case 5: HPX_FALLTHROUGH;    // primitive
        case 7: HPX_FALLTHROUGH;    // phylanx::ir::range
        default:
            break;
        }
        return false;
    }

    ///////////////////////////////////////////////////////////////////////////
    ir::range extract_list_value(
        primitive_argument_type const& val,
        std::string const& name, std::string const& codename)
    {
        switch (val.index())
        {
        case 0:     // nil
            return std::vector<primitive_argument_type>{
                primitive_argument_type{util::get<0>(val)}};

        case 1:    // phylanx::ir::node_data<std::uint8_t>
            return std::vector<primitive_argument_type>{
                primitive_argument_type{util::get<1>(val)}};

        case 2:     // ir::node_data<std::int64_t>
            return std::vector<primitive_argument_type>{
                primitive_argument_type{util::get<2>(val)}};

        case 3:     // string
            return std::vector<primitive_argument_type>{
                primitive_argument_type{util::get<3>(val)}};

        case 4:     // phylanx::ir::node_data<double>
            return std::vector<primitive_argument_type>{
                primitive_argument_type{util::get<4>(val)}};

        case 5:     // primitive
            return std::vector<primitive_argument_type>{
                primitive_argument_type{util::get<5>(val)}};

        case 6:     // std::vector<ast::expression>
            return std::vector<primitive_argument_type>{
                primitive_argument_type{util::get<6>(val)}};

        case 7:     // phylanx::ir::range
            return util::get<7>(val);

        default:
            break;
        }

        std::string type(detail::get_primitive_argument_type_name(val.index()));
        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "phylanx::execution_tree::extract_list_value",
            generate_error_message(
                "primitive_argument_type does not hold a boolean "
                    "value type (type held: '" + type + "')",
                name, codename));
    }

    ir::range extract_list_value(
        primitive_argument_type && val,
        std::string const& name, std::string const& codename)
    {
        switch (val.index())
        {
        case 0:     // nil
            return std::vector<primitive_argument_type>{
                primitive_argument_type{util::get<0>(std::move(val))}};

        case 1:    // phylanx::ir::node_data<std::uint8_t>
            return std::vector<primitive_argument_type>{
                primitive_argument_type{util::get<1>(std::move(val))}};

        case 2:     // ir::node_data<std::int64_t>
            return std::vector<primitive_argument_type>{
                primitive_argument_type{util::get<2>(std::move(val))}};

        case 3:     // string
            return std::vector<primitive_argument_type>{
                primitive_argument_type{util::get<3>(std::move(val))}};

        case 4:     // phylanx::ir::node_data<double>
            return std::vector<primitive_argument_type>{
                primitive_argument_type{util::get<4>(std::move(val))}};

        case 5:     // primitive
            return std::vector<primitive_argument_type>{
                primitive_argument_type{util::get<5>(std::move(val))}};

        case 6:     // std::vector<ast::expression>
            return std::vector<primitive_argument_type>{
                primitive_argument_type{util::get<6>(std::move(val))}};

        case 7:     // phylanx::ir::range
            return util::get<7>(std::move(val));

        default:
            break;
        }

        std::string type(detail::get_primitive_argument_type_name(val.index()));
        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "phylanx::execution_tree::extract_list_value",
            generate_error_message(
                "primitive_argument_type does not hold a boolean "
                    "value type (type held: '" + type + "')",
                name, codename));
    }

    bool is_list_operand(primitive_argument_type const& val)
    {
        switch (val.index())
        {
        case 0: HPX_FALLTHROUGH;    // nil
        case 1: HPX_FALLTHROUGH;    // phylanx::ir::node_data<std::uint8_t>
        case 2: HPX_FALLTHROUGH;    // ir::node_data<std::int64_t>
        case 3: HPX_FALLTHROUGH;    // string
        case 4: HPX_FALLTHROUGH;    // phylanx::ir::node_data<double>
        case 5: HPX_FALLTHROUGH;    // primitive
        case 6: HPX_FALLTHROUGH;    // std::vector<ast::expression>
        case 7:                     // phylanx::ir::range
            return true;

        default:
            break;
        }
        return false;
    }

    ///////////////////////////////////////////////////////////////////////////
    ir::range extract_list_value_strict(
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

        case 7:     // phylanx::ir:range
            return util::get<7>(val);

        case 0: HPX_FALLTHROUGH;    // nil
        case 1: HPX_FALLTHROUGH;    // phylanx::ir::node_data<std::uint8_t>
        case 2: HPX_FALLTHROUGH;    // ir::node_data<std::int64_t>
        case 3: HPX_FALLTHROUGH;    // string
        case 4: HPX_FALLTHROUGH;    // phylanx::ir::node_data<double>
        case 5: HPX_FALLTHROUGH;    // primitive
        default:
            break;
        }

        std::string type(detail::get_primitive_argument_type_name(val.index()));
        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "phylanx::execution_tree::extract_list_value_strict",
            generate_error_message(
                "primitive_argument_type does not hold a list "
                    "value type (type held: '" + type + "')",
                name, codename));
    }

    ir::range extract_list_value_strict(
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

        case 7:     // phylanx::ir::range
            return util::get<7>(std::move(val));

        case 0: HPX_FALLTHROUGH;    // nil
        case 1: HPX_FALLTHROUGH;    // phylanx::ir::node_data<std::uint8_t>
        case 2: HPX_FALLTHROUGH;    // ir::node_data<std::int64_t>
        case 3: HPX_FALLTHROUGH;    // string
        case 4: HPX_FALLTHROUGH;    // phylanx::ir::node_data<double>
        case 5: HPX_FALLTHROUGH;    // primitive
        default:
            break;
        }

        std::string type(detail::get_primitive_argument_type_name(val.index()));
        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "phylanx::execution_tree::extract_list_value_strict",
            generate_error_message(
                "primitive_argument_type does not hold a list "
                    "value type (type held: '" + type + "')",
                name, codename));
    }

    bool is_list_operand_strict(primitive_argument_type const& val)
    {
        switch (val.index())
        {
        case 6: HPX_FALLTHROUGH;    // std::vector<ast::expression>
        case 7:                     // phylanx::ir::range
            return true;

        case 0: HPX_FALLTHROUGH;    // nil
        case 1: HPX_FALLTHROUGH;    // phylanx::ir::node_data<std::uint8_t>
        case 2: HPX_FALLTHROUGH;    // ir::node_data<std::int64_t>
        case 3: HPX_FALLTHROUGH;    // string
        case 4: HPX_FALLTHROUGH;    // phylanx::ir::node_data<double>
        case 5: HPX_FALLTHROUGH;    // primitive
        default:
            break;
        }
        return false;
    }

    ///////////////////////////////////////////////////////////////////////////
    primitive primitive_operand(primitive_argument_type const& val,
        std::string const& name, std::string const& codename)
    {
        primitive const* p = util::get_if<primitive>(&val);
        if (p != nullptr)
            return *p;

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "phylanx::execution_tree::extract_primitive",
            generate_error_message(
                "primitive_value_type does not hold a primitive",
                name, codename));
    }

    primitive primitive_operand(primitive_argument_type const& val,
        compiler::primitive_name_parts const& parts,
        std::string const& codename)
    {
        primitive const* p = util::get_if<primitive>(&val);
        if (p != nullptr)
            return *p;

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "phylanx::execution_tree::extract_primitive",
            generate_error_message(
                "primitive_value_type does not hold a primitive",
                parts, codename));
    }

    bool is_primitive_operand(primitive_argument_type const& val)
    {
        return util::get_if<primitive>(&val) != nullptr;
    }

    primitive_argument_type primitive_argument_type::operator()() const
    {
        if (is_primitive_operand(*this))
        {
            return extract_copy_value(value_operand_sync(*this, {}));
        }
        return extract_ref_value(*this);
    }

    primitive_argument_type primitive_argument_type::operator()(
        std::vector<primitive_argument_type> const& args) const
    {
        if (is_primitive_operand(*this))
        {
            return extract_copy_value(value_operand_sync(*this, args));
        }
        return extract_ref_value(*this);
    }

    primitive_argument_type primitive_argument_type::operator()(
        std::vector<primitive_argument_type> && args) const
    {
        if (is_primitive_operand(*this))
        {
            // evaluate the function itself
            return extract_copy_value(
                value_operand_sync(*this, std::move(args)));
        }
        return extract_ref_value(*this);
    }

    ///////////////////////////////////////////////////////////////////////////
    hpx::future<primitive_argument_type> value_operand(
        primitive_argument_type const& val,
        std::vector<primitive_argument_type> const& args,
        std::string const& name, std::string const& codename, eval_mode mode)
    {
        primitive const* p = util::get_if<primitive>(&val);
        if (p != nullptr)
        {
            hpx::future<primitive_argument_type> f = p->eval(args, mode);
            if (f.is_ready())
            {
                return f;
            }

            return f.then(hpx::launch::sync,
                [&](hpx::future<primitive_argument_type> && f)
                {
                    return extract_value(f.get(), name, codename);
                });
        }

        if (valid(val))
        {
            return hpx::make_ready_future(
                extract_ref_value(val, name, codename));
        }
        return hpx::make_ready_future(val);
    }

    hpx::future<primitive_argument_type> value_operand(
        primitive_argument_type const& val,
        std::vector<primitive_argument_type>&& args, std::string const& name,
        std::string const& codename, eval_mode mode)
    {
        primitive const* p = util::get_if<primitive>(&val);
        if (p != nullptr)
        {
            hpx::future<primitive_argument_type> f =
                p->eval(std::move(args), mode);
            if (f.is_ready())
            {
                return f;
            }

            return f.then(hpx::launch::sync,
                [&](hpx::future<primitive_argument_type> && f)
                {
                    return extract_value(f.get(), name, codename);
                });
        }

        if (valid(val))
        {
            return hpx::make_ready_future(
                extract_ref_value(val, name, codename));
        }
        return hpx::make_ready_future(val);
    }

    hpx::future<primitive_argument_type> value_operand(
        primitive_argument_type&& val,
        std::vector<primitive_argument_type> const& args,
        std::string const& name, std::string const& codename,
        eval_mode mode)
    {
        primitive* p = util::get_if<primitive>(&val);
        if (p != nullptr)
        {
            hpx::future<primitive_argument_type> f = p->eval(args, mode);
            if (f.is_ready())
            {
                return f;
            }

            return f.then(hpx::launch::sync,
                [&](hpx::future<primitive_argument_type> && f)
                {
                    return extract_value(f.get(), name, codename);
                });
        }

        if (valid(val))
        {
            return hpx::make_ready_future(
                extract_ref_value(std::move(val), name, codename));
        }
        return hpx::make_ready_future(std::move(val));
    }

    hpx::future<primitive_argument_type> value_operand(
        primitive_argument_type&& val,
        std::vector<primitive_argument_type>&& args, std::string const& name,
        std::string const& codename, eval_mode mode)
    {
        primitive* p = util::get_if<primitive>(&val);
        if (p != nullptr)
        {
            hpx::future<primitive_argument_type> f =
                p->eval(std::move(args), mode);
            if (f.is_ready())
            {
                return f;
            }

            return f.then(hpx::launch::sync,
                [&](hpx::future<primitive_argument_type> && f)
                {
                    return extract_value(f.get(), name, codename);
                });
        }

        if (valid(val))
        {
            return hpx::make_ready_future(
                extract_ref_value(std::move(val), name, codename));
        }
        return hpx::make_ready_future(std::move(val));
    }

    ///////////////////////////////////////////////////////////////////////////
    primitive_argument_type value_operand_sync(
        primitive_argument_type const& val,
        std::vector<primitive_argument_type> const& args,
        std::string const& name, std::string const& codename,
        eval_mode mode)
    {
        primitive const* p = util::get_if<primitive>(&val);
        if (p != nullptr)
        {
            return extract_value(
                p->eval(hpx::launch::sync, args, mode), name, codename);
        }

        if (valid(val))
        {
            return extract_value(val, name, codename);
        }
        return val;
    }

    primitive_argument_type value_operand_sync(
        primitive_argument_type const& val,
        std::vector<primitive_argument_type>&& args, std::string const& name,
        std::string const& codename, eval_mode mode)
    {
        primitive const* p = util::get_if<primitive>(&val);
        if (p != nullptr)
        {
            return extract_value(
                p->eval(hpx::launch::sync, std::move(args), mode), name,
                codename);
        }

        if (valid(val))
        {
            return extract_value(val, name, codename);
        }
        return val;
    }

    primitive_argument_type value_operand_sync(primitive_argument_type&& val,
        std::vector<primitive_argument_type> const& args,
        std::string const& name, std::string const& codename,
        eval_mode mode)
    {
        primitive* p = util::get_if<primitive>(&val);
        if (p != nullptr)
        {
            return extract_value(
                p->eval(hpx::launch::sync, args, mode), name, codename);
        }

        if (valid(val))
        {
            return extract_value(std::move(val), name, codename);
        }
        return std::move(val);
    }

    primitive_argument_type value_operand_sync(primitive_argument_type&& val,
        std::vector<primitive_argument_type>&& args, std::string const& name,
        std::string const& codename, eval_mode mode)
    {
        primitive const* p = util::get_if<primitive>(&val);
        if (p != nullptr)
        {
            return extract_value(
                p->eval(hpx::launch::sync, std::move(args), mode), name,
                codename);
        }

        if (valid(val))
        {
            return extract_value(std::move(val), name, codename);
        }
        return std::move(val);
    }

    ///////////////////////////////////////////////////////////////////////////
    primitive_argument_type value_operand_ref_sync(
        primitive_argument_type const& val,
        std::vector<primitive_argument_type> const& args,
        std::string const& name, std::string const& codename)
    {
        primitive const* p = util::get_if<primitive>(&val);
        if (p != nullptr)
        {
            return extract_value(
                p->eval(hpx::launch::sync, args), name, codename);
        }

        if (valid(val))
        {
            return extract_ref_value(val, name, codename);
        }
        return val;
    }

    ///////////////////////////////////////////////////////////////////////////
    hpx::future<primitive_argument_type> literal_operand(
        primitive_argument_type const& val,
        std::vector<primitive_argument_type> const& args,
        std::string const& name, std::string const& codename)
    {
        primitive const* p = util::get_if<primitive>(&val);
        if (p != nullptr)
        {
            hpx::future<primitive_argument_type> f = p->eval(args);
            if (f.is_ready())
            {
                return hpx::make_ready_future(
                    extract_literal_value(f.get(), name, codename));
            }

            return f.then(hpx::launch::sync,
                [&](hpx::future<primitive_argument_type> && f)
                {
                    return extract_literal_value(f.get(), name, codename);
                });
        }

        HPX_ASSERT(valid(val));
        return hpx::make_ready_future(
            extract_literal_ref_value(val, name, codename));
    }

    hpx::future<primitive_argument_type> literal_operand(
        primitive_argument_type const& val,
        std::vector<primitive_argument_type> && args,
        std::string const& name, std::string const& codename)
    {
        primitive const* p = util::get_if<primitive>(&val);
        if (p != nullptr)
        {
            hpx::future<primitive_argument_type> f = p->eval(std::move(args));
            if (f.is_ready())
            {
                return hpx::make_ready_future(
                    extract_literal_value(f.get(), name, codename));
            }

            return f.then(hpx::launch::sync,
                [&](hpx::future<primitive_argument_type>&& f)
                {
                    return extract_literal_value(f.get(), name, codename);
                });
        }

        HPX_ASSERT(valid(val));
        return hpx::make_ready_future(
            extract_literal_ref_value(val, name, codename));
    }

    hpx::future<primitive_argument_type> literal_operand(
        primitive_argument_type && val,
        std::vector<primitive_argument_type> const& args,
        std::string const& name, std::string const& codename)
    {
        primitive* p = util::get_if<primitive>(&val);
        if (p != nullptr)
        {
            hpx::future<primitive_argument_type> f = p->eval(args);
            if (f.is_ready())
            {
                return hpx::make_ready_future(
                    extract_literal_value(f.get(), name, codename));
            }

            return f.then(hpx::launch::sync,
                [&](hpx::future<primitive_argument_type> && f)
                {
                    return extract_literal_value(f.get(), name, codename);
                });
        }

        HPX_ASSERT(valid(val));
        return hpx::make_ready_future(
            extract_literal_ref_value(std::move(val), name, codename));
    }

    hpx::future<primitive_argument_type> literal_operand(
        primitive_argument_type && val,
        std::vector<primitive_argument_type> && args,
        std::string const& name, std::string const& codename)
    {
        primitive const* p = util::get_if<primitive>(&val);
        if (p != nullptr)
        {
            hpx::future<primitive_argument_type> f = p->eval(std::move(args));
            if (f.is_ready())
            {
                return hpx::make_ready_future(
                    extract_literal_value(f.get(), name, codename));
            }

            return f.then(hpx::launch::sync,
                [&](hpx::future<primitive_argument_type> && f)
                {
                    return extract_literal_value(f.get(), name, codename);
                });
        }

        HPX_ASSERT(valid(val));
        return hpx::make_ready_future(
            extract_literal_ref_value(std::move(val), name, codename));
    }

    ///////////////////////////////////////////////////////////////////////////
    primitive_argument_type literal_operand_sync(
        primitive_argument_type const& val,
        std::vector<primitive_argument_type> const& args,
        std::string const& name, std::string const& codename)
    {
        primitive const* p = util::get_if<primitive>(&val);
        if (p != nullptr)
        {
            return extract_literal_value(
                p->eval(hpx::launch::sync, args), name, codename);
        }

        HPX_ASSERT(valid(val));
        return extract_literal_ref_value(val, name, codename);
    }

    // Extract an integer value from a primitive_argument_type
    hpx::future<ir::node_data<std::int64_t>> integer_operand(
        primitive_argument_type const& val,
        std::vector<primitive_argument_type> const& args,
        std::string const& name, std::string const& codename)
    {
        primitive const* p = util::get_if<primitive>(&val);
        if (p != nullptr)
        {
            hpx::future<primitive_argument_type> f = p->eval(args);
            if (f.is_ready())
            {
                return hpx::make_ready_future(
                    extract_integer_value(f.get(), name, codename));
            }

            return f.then(hpx::launch::sync,
                [&](hpx::future<primitive_argument_type> && f)
                {
                    return extract_integer_value(f.get(), name, codename);
                });
        }

        HPX_ASSERT(valid(val));
        return hpx::make_ready_future(
            extract_integer_value(val, name, codename));
    }

    hpx::future<ir::node_data<std::int64_t>> integer_operand(
        primitive_argument_type const& val,
        std::vector<primitive_argument_type>&& args, std::string const& name,
        std::string const& codename)
    {
        primitive const* p = util::get_if<primitive>(&val);
        if (p != nullptr)
        {
            hpx::future<primitive_argument_type> f = p->eval(std::move(args));
            if (f.is_ready())
            {
                return hpx::make_ready_future(
                    extract_integer_value(f.get(), name, codename));
            }

            return f.then(hpx::launch::sync,
                [&](hpx::future<primitive_argument_type>&& f)
                {
                    return extract_integer_value(f.get(), name, codename);
                });
        }

        HPX_ASSERT(valid(val));
        return hpx::make_ready_future(
            extract_integer_value(val, name, codename));
    }

    hpx::future<ir::node_data<std::int64_t>> integer_operand(
        primitive_argument_type&& val,
        std::vector<primitive_argument_type> const& args,
        std::string const& name, std::string const& codename)
    {
        primitive const* p = util::get_if<primitive>(&val);
        if (p != nullptr)
        {
            hpx::future<primitive_argument_type> f = p->eval(args);
            if (f.is_ready())
            {
                return hpx::make_ready_future(
                    extract_integer_value(f.get(), name, codename));
            }

            return f.then(hpx::launch::sync,
                [&](hpx::future<primitive_argument_type>&& f)
                {
                    return extract_integer_value(f.get(), name, codename);
                });
        }

        HPX_ASSERT(valid(val));
        return hpx::make_ready_future(
            extract_integer_value(std::move(val), name, codename));
    }

    hpx::future<ir::node_data<std::int64_t>> integer_operand(
        primitive_argument_type&& val,
        std::vector<primitive_argument_type>&& args, std::string const& name,
        std::string const& codename)
    {
        primitive const* p = util::get_if<primitive>(&val);
        if (p != nullptr)
        {
            hpx::future<primitive_argument_type> f = p->eval(std::move(args));
            if (f.is_ready())
            {
                return hpx::make_ready_future(
                    extract_integer_value(f.get(), name, codename));
            }

            return f.then(hpx::launch::sync,
                [&](hpx::future<primitive_argument_type> && f)
                {
                    return extract_integer_value(f.get(), name, codename);
                });
        }

        HPX_ASSERT(valid(val));
        return hpx::make_ready_future(
            extract_integer_value(std::move(val), name, codename));
    }

    // Extract an integer value from a primitive_argument_type
    hpx::future<ir::node_data<std::int64_t>> integer_operand_strict(
        primitive_argument_type const& val,
        std::vector<primitive_argument_type> const& args,
        std::string const& name, std::string const& codename)
    {
        primitive const* p = util::get_if<primitive>(&val);
        if (p != nullptr)
        {
            hpx::future<primitive_argument_type> f = p->eval(args);
            if (f.is_ready())
            {
                return hpx::make_ready_future(
                    extract_integer_value_strict(f.get(), name, codename));
            }

            return f.then(hpx::launch::sync,
                [&](hpx::future<primitive_argument_type> && f)
                {
                    return extract_integer_value_strict(
                        f.get(), name, codename);
                });
        }

        HPX_ASSERT(valid(val));
        return hpx::make_ready_future(
            extract_integer_value_strict(val, name, codename));
    }

    //make extract_shape.cpp error happy
    hpx::future<std::int64_t> scalar_integer_operand_strict(
        primitive_argument_type const& val,
        std::vector<primitive_argument_type> const& args,
        std::string const& name, std::string const& codename)
    {
        primitive const* p = util::get_if<primitive>(&val);
        if (p != nullptr)
        {
            hpx::future<primitive_argument_type> f = p->eval(args);
            if (f.is_ready())
            {
                return hpx::make_ready_future(
                    extract_scalar_integer_value_strict(f.get(), name, codename));
            }

            return f.then(hpx::launch::sync,
                [&](hpx::future<primitive_argument_type> && f)
                {
                    return extract_scalar_integer_value_strict(
                        f.get(), name, codename);
                });
        }

        HPX_ASSERT(valid(val));
        return hpx::make_ready_future(
            extract_integer_value_strict(val, name, codename)[0]);
    }

    hpx::future<ir::node_data<std::int64_t>> integer_operand_strict(
        primitive_argument_type const& val,
        std::vector<primitive_argument_type>&& args, std::string const& name,
        std::string const& codename)
    {
        primitive const* p = util::get_if<primitive>(&val);
        if (p != nullptr)
        {
            hpx::future<primitive_argument_type> f = p->eval(std::move(args));
            if (f.is_ready())
            {
                return hpx::make_ready_future(
                    extract_integer_value_strict(f.get(), name, codename));
            }

            return f.then(hpx::launch::sync,
                [&](hpx::future<primitive_argument_type> && f)
                {
                    return extract_integer_value_strict(
                        f.get(), name, codename);
                });
        }

        HPX_ASSERT(valid(val));
        return hpx::make_ready_future(
            extract_integer_value_strict(val, name, codename));
    }

    hpx::future<ir::node_data<std::int64_t>> integer_operand_strict(
        primitive_argument_type&& val,
        std::vector<primitive_argument_type> const& args,
        std::string const& name, std::string const& codename)
    {
        primitive const* p = util::get_if<primitive>(&val);
        if (p != nullptr)
        {
            hpx::future<primitive_argument_type> f = p->eval(args);
            if (f.is_ready())
            {
                return hpx::make_ready_future(
                    extract_integer_value_strict(f.get(), name, codename));
            }

            return f.then(hpx::launch::sync,
                [&](hpx::future<primitive_argument_type> && f)
                {
                    return extract_integer_value_strict(f.get(), name, codename);
                });
        }

        HPX_ASSERT(valid(val));
        return hpx::make_ready_future(
            extract_integer_value_strict(std::move(val), name, codename));
    }

    hpx::future<ir::node_data<std::int64_t>> integer_operand_strict(
        primitive_argument_type&& val,
        std::vector<primitive_argument_type>&& args, std::string const& name,
        std::string const& codename)
    {
        primitive const* p = util::get_if<primitive>(&val);
        if (p != nullptr)
        {
            hpx::future<primitive_argument_type> f = p->eval(std::move(args));
            if (f.is_ready())
            {
                return hpx::make_ready_future(
                    extract_integer_value_strict(f.get(), name, codename));
            }

            return f.then(hpx::launch::sync,
                [&](hpx::future<primitive_argument_type> && f)
                {
                    return extract_integer_value_strict(f.get(), name, codename);
                });
        }

        HPX_ASSERT(valid(val));
        return hpx::make_ready_future(
            extract_integer_value_strict(std::move(val), name, codename));
    }

    ///////////////////////////////////////////////////////////////////////////
    hpx::future<ir::node_data<double>> numeric_operand(
        primitive_argument_type const& val,
        std::vector<primitive_argument_type> const& args,
        std::string const& name, std::string const& codename)
    {
        primitive const* p = util::get_if<primitive>(&val);
        if (p != nullptr)
        {
            hpx::future<primitive_argument_type> f = p->eval(args);
            if (f.is_ready())
            {
                return hpx::make_ready_future(
                    extract_numeric_value(f.get(), name, codename));
            }

            return f.then(hpx::launch::sync,
                [&](hpx::future<primitive_argument_type> && f)
                {
                    return extract_numeric_value(f.get(), name, codename);
                });
        }

        HPX_ASSERT(valid(val));
        return hpx::make_ready_future(
            extract_numeric_value(val, name, codename));
    }

    hpx::future<ir::node_data<double>> numeric_operand(
        primitive_argument_type const& val,
        std::vector<primitive_argument_type> && args,
        std::string const& name, std::string const& codename)
    {
        primitive const* p = util::get_if<primitive>(&val);
        if (p != nullptr)
        {
            hpx::future<primitive_argument_type> f = p->eval(std::move(args));
            if (f.is_ready())
            {
                return hpx::make_ready_future(
                    extract_numeric_value(f.get(), name, codename));
            }

            return f.then(hpx::launch::sync,
                [&](hpx::future<primitive_argument_type> && f)
                {
                    return extract_numeric_value(f.get(), name, codename);
                });
        }

        HPX_ASSERT(valid(val));
        return hpx::make_ready_future(
            extract_numeric_value(val, name, codename));
    }

    hpx::future<ir::node_data<double>> numeric_operand(
        primitive_argument_type && val,
        std::vector<primitive_argument_type> const& args,
        std::string const& name, std::string const& codename)
    {
        primitive* p = util::get_if<primitive>(&val);
        if (p != nullptr)
        {
            hpx::future<primitive_argument_type> f = p->eval(args);
            if (f.is_ready())
            {
                return hpx::make_ready_future(
                    extract_numeric_value(f.get(), name, codename));
            }

            return f.then(hpx::launch::sync,
                [&](hpx::future<primitive_argument_type> && f)
                {
                    return extract_numeric_value(f.get(), name, codename);
                });
        }

        HPX_ASSERT(valid(val));
        return hpx::make_ready_future(
            extract_numeric_value(std::move(val), name, codename));
    }

    hpx::future<ir::node_data<double>> numeric_operand(
        primitive_argument_type && val,
        std::vector<primitive_argument_type> && args,
        std::string const& name, std::string const& codename)
    {
        primitive* p = util::get_if<primitive>(&val);
        if (p != nullptr)
        {
            hpx::future<primitive_argument_type> f = p->eval(std::move(args));
            if (f.is_ready())
            {
                return hpx::make_ready_future(
                    extract_numeric_value(f.get(), name, codename));
            }

            return f.then(hpx::launch::sync,
                [&](hpx::future<primitive_argument_type> && f)
                {
                    return extract_numeric_value(f.get(), name, codename);
                });
        }

        HPX_ASSERT(valid(val));
        return hpx::make_ready_future(
            extract_numeric_value(std::move(val), name, codename));
    }

    ///////////////////////////////////////////////////////////////////////////
    ir::node_data<double> numeric_operand_sync(
        primitive_argument_type const& val,
        std::vector<primitive_argument_type> const& args,
        std::string const& name, std::string const& codename)
    {
        primitive const* p = util::get_if<primitive>(&val);
        if (p != nullptr)
        {
            return extract_numeric_value(
                p->eval(hpx::launch::sync, args), name, codename);
        }

        HPX_ASSERT(valid(val));
        return extract_numeric_value(val, name, codename);
    }

    ///////////////////////////////////////////////////////////////////////////
    hpx::future<std::uint8_t> boolean_operand(
        primitive_argument_type const& val,
        std::vector<primitive_argument_type> const& args,
        std::string const& name, std::string const& codename)
    {
        primitive const* p = util::get_if<primitive>(&val);
        if (p != nullptr)
        {
            hpx::future<primitive_argument_type> f = p->eval(args);
            if (f.is_ready())
            {
                return hpx::make_ready_future(
                    extract_scalar_boolean_value(f.get(), name, codename));
            }

            return f.then(hpx::launch::sync,
                [&](hpx::future<primitive_argument_type> && f)
                {
                    return extract_scalar_boolean_value(f.get(), name, codename);
                });
        }

        HPX_ASSERT(valid(val));
        return hpx::make_ready_future(
            extract_scalar_boolean_value(val, name, codename));
    }

    std::uint8_t boolean_operand_sync(primitive_argument_type const& val,
        std::vector<primitive_argument_type> const& args,
        std::string const& name, std::string const& codename)
    {
        primitive const* p = util::get_if<primitive>(&val);
        if (p != nullptr)
        {
            return extract_scalar_boolean_value(
                p->eval(hpx::launch::sync, args), name, codename);
        }

        HPX_ASSERT(valid(val));
        return extract_scalar_boolean_value(val, name, codename);
    }

    ///////////////////////////////////////////////////////////////////////////
    hpx::future<std::string> string_operand(
        primitive_argument_type const& val,
        std::vector<primitive_argument_type> const& args,
        std::string const& name, std::string const& codename)
    {
        primitive const* p = util::get_if<primitive>(&val);
        if (p != nullptr)
        {
            hpx::future<primitive_argument_type> f = p->eval(args);
            if (f.is_ready())
            {
                return hpx::make_ready_future(
                    extract_string_value(f.get(), name, codename));
            }

            return f.then(hpx::launch::sync,
                [&](hpx::future<primitive_argument_type> && f)
                {
                    return extract_string_value(f.get(), name, codename);
                });
        }

        HPX_ASSERT(valid(val));
        return hpx::make_ready_future(
            extract_string_value(val, name, codename));
    }

    std::string string_operand_sync(primitive_argument_type const& val,
        std::vector<primitive_argument_type> const& args,
        std::string const& name, std::string const& codename)
    {
        primitive const* p = util::get_if<primitive>(&val);
        if (p != nullptr)
        {
            return extract_string_value(
                p->eval(hpx::launch::sync, args), name, codename);
        }

        HPX_ASSERT(valid(val));
        return extract_string_value(val, name, codename);
    }

    ///////////////////////////////////////////////////////////////////////////
    hpx::future<std::vector<ast::expression>> ast_operand(
        primitive_argument_type const& val,
        std::vector<primitive_argument_type> const& args,
        std::string const& name, std::string const& codename)
    {
        primitive const* p = util::get_if<primitive>(&val);
        if (p != nullptr)
        {
            hpx::future<primitive_argument_type> f = p->eval(args);
            if (f.is_ready())
            {
                return hpx::make_ready_future(
                    extract_ast_value(f.get(), name, codename));
            }

            return f.then(hpx::launch::sync,
                [&](hpx::future<primitive_argument_type> && f)
                {
                    return extract_ast_value(f.get(), name, codename);
                });
        }

        HPX_ASSERT(valid(val));
        return hpx::make_ready_future(extract_ast_value(val, name, codename));
    }

    std::vector<ast::expression> ast_operand_sync(
        primitive_argument_type const& val,
        std::vector<primitive_argument_type> const& args,
        std::string const& name, std::string const& codename)
    {
        primitive const* p = util::get_if<primitive>(&val);
        if (p != nullptr)
        {
            return extract_ast_value(
                p->eval(hpx::launch::sync, args), name, codename);
        }

        HPX_ASSERT(valid(val));
        return extract_ast_value(val, name, codename);
    }

    ///////////////////////////////////////////////////////////////////////////
    hpx::future<ir::range> list_operand(
        primitive_argument_type const& val,
        std::vector<primitive_argument_type> const& args,
        std::string const& name, std::string const& codename)
    {
        primitive const* p = util::get_if<primitive>(&val);
        if (p != nullptr)
        {
            hpx::future<primitive_argument_type> f = p->eval(args);
            if (f.is_ready())
            {
                return hpx::make_ready_future(
                    extract_list_value(f.get(), name, codename));
            }

            return f.then(hpx::launch::sync,
                [&](hpx::future<primitive_argument_type> && f)
                {
                    return extract_list_value(f.get(), name, codename);
                });
        }

        HPX_ASSERT(valid(val));
        return hpx::make_ready_future(extract_list_value(val, name, codename));
    }

    hpx::future<ir::range> list_operand(
        primitive_argument_type const& val,
        std::vector<primitive_argument_type> && args,
        std::string const& name, std::string const& codename)
    {
        primitive const* p = util::get_if<primitive>(&val);
        if (p != nullptr)
        {
            hpx::future<primitive_argument_type> f = p->eval(std::move(args));
            if (f.is_ready())
            {
                return hpx::make_ready_future(
                    extract_list_value(f.get(), name, codename));
            }

            return f.then(hpx::launch::sync,
                [&](hpx::future<primitive_argument_type> && f)
                {
                    return extract_list_value(f.get(), name, codename);
                });
        }

        HPX_ASSERT(valid(val));
        return hpx::make_ready_future(extract_list_value(val, name, codename));
    }

    hpx::future<ir::range> list_operand(
        primitive_argument_type const& val,
        std::vector<primitive_argument_type> const& args,
        std::string const& name, std::string && codename)
    {
        primitive const* p = util::get_if<primitive>(&val);
        if (p != nullptr)
        {
            hpx::future<primitive_argument_type> f = p->eval(args);
            if (f.is_ready())
            {
                return hpx::make_ready_future(
                    extract_list_value(f.get(), name, codename));
            }

            return f.then(hpx::launch::sync,
                [&](hpx::future<primitive_argument_type> && f)
                {
                    return extract_list_value(f.get(), name, codename);
                });
        }

        HPX_ASSERT(valid(val));
        return hpx::make_ready_future(
            extract_list_value(std::move(val), name, codename));
    }

    hpx::future<ir::range> list_operand(
        primitive_argument_type const& val,
        std::vector<primitive_argument_type> && args,
        std::string const& name, std::string && codename)
    {
        primitive const* p = util::get_if<primitive>(&val);
        if (p != nullptr)
        {
            hpx::future<primitive_argument_type> f = p->eval(std::move(args));
            if (f.is_ready())
            {
                return hpx::make_ready_future(
                    extract_list_value(f.get(), name, codename));
            }

            return f.then(hpx::launch::sync,
                [&](hpx::future<primitive_argument_type> && f)
                {
                    return extract_list_value(f.get(), name, codename);
                });
        }

        HPX_ASSERT(valid(val));
        return hpx::make_ready_future(
            extract_list_value(std::move(val), name, codename));
    }

    ir::range list_operand_sync(
        primitive_argument_type const& val,
        std::vector<primitive_argument_type> const& args,
        std::string const& name, std::string const& codename)
    {
        primitive const* p = util::get_if<primitive>(&val);
        if (p != nullptr)
        {
            return extract_list_value(
                p->eval(hpx::launch::sync, args), name, codename);
        }

        HPX_ASSERT(valid(val));
        return extract_list_value(val, name, codename);
    }

    ///////////////////////////////////////////////////////////////////////////
    hpx::future<ir::range> list_operand_strict(
        primitive_argument_type const& val,
        std::vector<primitive_argument_type> const& args,
        std::string const& name, std::string const& codename)
    {
        primitive const* p = util::get_if<primitive>(&val);
        if (p != nullptr)
        {
            hpx::future<primitive_argument_type> f = p->eval(args);
            if (f.is_ready())
            {
                return hpx::make_ready_future(
                    extract_list_value_strict(f.get(), name, codename));
            }

            return f.then(hpx::launch::sync,
                [&](hpx::future<primitive_argument_type> && f)
                {
                    return extract_list_value_strict(f.get(), name, codename);
                });
        }

        HPX_ASSERT(valid(val));
        return hpx::make_ready_future(
            extract_list_value_strict(val, name, codename));
    }

    hpx::future<ir::range> list_operand_strict(
        primitive_argument_type const& val,
        std::vector<primitive_argument_type> && args,
        std::string const& name, std::string const& codename)
    {
        primitive const* p = util::get_if<primitive>(&val);
        if (p != nullptr)
        {
            hpx::future<primitive_argument_type> f = p->eval(std::move(args));
            if (f.is_ready())
            {
                return hpx::make_ready_future(
                    extract_list_value_strict(f.get(), name, codename));
            }

            return f.then(hpx::launch::sync,
                [&](hpx::future<primitive_argument_type> && f)
                {
                    return extract_list_value_strict(f.get(), name, codename);
                });
        }

        HPX_ASSERT(valid(val));
        return hpx::make_ready_future(
            extract_list_value_strict(val, name, codename));
    }

    hpx::future<ir::range> list_operand_strict(
        primitive_argument_type const& val,
        std::vector<primitive_argument_type> const& args,
        std::string const& name, std::string && codename)
    {
        primitive const* p = util::get_if<primitive>(&val);
        if (p != nullptr)
        {
            hpx::future<primitive_argument_type> f = p->eval(args);
            if (f.is_ready())
            {
                return hpx::make_ready_future(
                    extract_list_value_strict(f.get(), name, codename));
            }

            return f.then(hpx::launch::sync,
                [&](hpx::future<primitive_argument_type> && f)
                {
                    return extract_list_value_strict(f.get(), name, codename);
                });
        }

        HPX_ASSERT(valid(val));
        return hpx::make_ready_future(
            extract_list_value_strict(std::move(val), name, codename));
    }

    hpx::future<ir::range> list_operand_strict(
        primitive_argument_type const& val,
        std::vector<primitive_argument_type> && args,
        std::string const& name, std::string && codename)
    {
        primitive const* p = util::get_if<primitive>(&val);
        if (p != nullptr)
        {
            hpx::future<primitive_argument_type> f = p->eval(std::move(args));
            if (f.is_ready())
            {
                return hpx::make_ready_future(
                    extract_list_value_strict(f.get(), name, codename));
            }

            return f.then(hpx::launch::sync,
                [&](hpx::future<primitive_argument_type> && f)
                {
                    return extract_list_value_strict(f.get(), name, codename);
                });
        }

        HPX_ASSERT(valid(val));
        return hpx::make_ready_future(
            extract_list_value_strict(std::move(val), name, codename));
    }

    ///////////////////////////////////////////////////////////////////////////
    primitive_argument_type to_primitive_value_type(
        ast::literal_value_type&& val)
    {
        switch (val.index())
        {
        case 0:
            return primitive_argument_type{};

        case 1:    // bool
            return primitive_argument_type{util::get<1>(std::move(val))};

        case 2:     // ir::node_data<std::int64_t>
            return primitive_argument_type{util::get<2>(std::move(val))};

        case 3:     // std::string
            return primitive_argument_type{util::get<3>(std::move(val))};

        case 4:     // phylanx::ir::node_data<double>
            return primitive_argument_type{util::get<4>(std::move(val))};

        // phylanx::util::recursive_wrapper<std::vector<literal_argument_type>>
        case 5:
            {
                auto && v = util::get<5>(std::move(val)).get();
                std::vector<primitive_argument_type> data;
                data.reserve(v.size());
                for (auto && value : std::move(v))
                {
                    data.push_back(to_primitive_value_type(std::move(value)));
                }
                return primitive_argument_type{data};
            }
            break;

        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "phylanx::execution_tree::primitives::to_primitive_value_type",
            "unsupported primitive_argument_type");
    }

    ir::node_data<double> to_primitive_numeric_type(
        ast::literal_value_type&& val)
    {
        switch (val.index())
        {
        case 1:                     // bool
            return ir::node_data<double>{
                util::get<1>(std::move(val)) ? 1.0 : 0.0};

        case 2:                     // ir::node_data<std::int64_t>
            return ir::node_data<double>{double(util::get<2>(std::move(val)))};

        case 4:                     // phylanx::ir::node_data<double>
            return util::get<4>(std::move(val));

        case 0: HPX_FALLTHROUGH;    // ast::nil
        case 3: HPX_FALLTHROUGH;    // std::string
        // phylanx::util::recursive_wrapper<std::vector<literal_argument_type>>
        case 5: HPX_FALLTHROUGH;
        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "phylanx::execution_tree::primitives::to_primitive_numeric_type",
            "unsupported primitive_argument_type");
    }

    std::string to_primitive_string_type(ast::literal_value_type&& val)
    {
        switch (val.index())
        {
        case 3:     // std::string
            return util::get<3>(std::move(val));

        case 0: HPX_FALLTHROUGH;    // ast::nil
        case 1: HPX_FALLTHROUGH;    // bool
        case 2: HPX_FALLTHROUGH;    // ir::node_data<std::int64_t>
        case 4: HPX_FALLTHROUGH;    // phylanx::ir::node_data<double>
        // phylanx::util::recursive_wrapper<std::vector<literal_argument_type>>
        case 5: HPX_FALLTHROUGH;
        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "phylanx::execution_tree::primitives::to_primitive_string_type",
            "unsupported primitive_argument_type");
    }

    ir::node_data<std::int64_t> to_primitive_int_type(
        ast::literal_value_type&& val)
    {
        switch (val.index())
        {
        case 1:     // bool
            return ir::node_data<std::int64_t>{util::get<1>(std::move(val)) ? 1 : 0};

        case 2:     // ir::node_data<std::int64_t>
            return ir::node_data<std::int64_t>{util::get<2>(std::move(val))};

        case 4:     // phylanx::ir::node_data<double>
            return ir::node_data<std::int64_t>{util::get<4>(std::move(val))};

        case 0: HPX_FALLTHROUGH;    // ast::nil
        case 3: HPX_FALLTHROUGH;    // std::string
        // phylanx::util::recursive_wrapper<std::vector<literal_argument_type>>
        case 5: HPX_FALLTHROUGH;
        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "phylanx::execution_tree::primitives::to_primitive_int_type",
            "unsupported primitive_argument_type");
    }

    ir::node_data<std::uint8_t> to_primitive_bool_type(
        ast::literal_value_type&& val)
    {
        switch (val.index())
        {
        case 1:     // bool
            return ir::node_data<std::uint8_t>{
                util::get<1>(std::move(val)) ? std::uint8_t(1) : std::uint8_t(0)
            };

        case 2:     // ir::node_data<std::int64_t>
            return ir::node_data<std::uint8_t>{
                util::get<1>(std::move(val)) ? std::uint8_t(1) : std::uint8_t(0)
            };

        case 4:     // phylanx::ir::node_data<double>
            return ir::node_data<std::uint8_t>{
                util::get<1>(std::move(val)) ? std::uint8_t(1) : std::uint8_t(0)
            };


        case 0: HPX_FALLTHROUGH;    // ast::nil
        case 3: HPX_FALLTHROUGH;    // std::string
        // phylanx::util::recursive_wrapper<std::vector<literal_argument_type>>
        case 5: HPX_FALLTHROUGH;
        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "phylanx::execution_tree::primitives::to_primitive_bool_type",
            "unsupported primitive_argument_type");
    }

    ///////////////////////////////////////////////////////////////////////////
    std::ostream& operator<<(std::ostream& os, primitive const& val)
    {
        os << value_operand_sync(primitive_argument_type{val}, {});
        return os;
    }

    ///////////////////////////////////////////////////////////////////////////
    std::ostream& operator<<(std::ostream& os,
        primitive_argument_type const& val)
    {
        switch (val.index())
        {
        case 0:     // nil
            ast::detail::to_string{os}(util::get<0>(val));
            return os;

        case 1:    // phylanx::ir::node_data<std::uint8_t>
            ast::detail::to_string{os}(util::get<1>(val));
            return os;

        case 2:     // ir::node_data<std::int64_t>
            ast::detail::to_string{os}(util::get<2>(val));
            return os;

        case 3:     // std::string
            ast::detail::to_string{os}(util::get<3>(val));
            return os;

        case 4:     // phylanx::ir::node_data<double>
            ast::detail::to_string{os}(util::get<4>(val));
            return os;

        case 5:
            ast::detail::to_string{os}(util::get<5>(val));
            return os;

        case 6:     // std::vector<ast::expression>
            for (auto const& ast : util::get<6>(val))
            {
                ast::detail::to_string{os}(ast);
            }
            return os;

        case 7:     // phylanx::ir::range
            {
                os << "list(";
                bool first = true;
                for (auto const& elem : util::get<7>(val))
                {
                    if (!first)
                    {
                        os << ", ";
                    }
                    first = false;
                    ast::detail::to_string{os}(elem);
                }
                os << ")";
            }
            return os;

        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "phylanx::execution_tree::operator<<(primitive_argument_type)",
            "primitive_argument_type does not hold a value type");

        return os;
    }

    std::string to_string(primitive_argument_type const& value)
    {
        std::stringstream str;
        str << value;
        return str.str();
    }
}}

