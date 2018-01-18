//  Copyright (c) 2017-2018 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/base_primitive.hpp>
#include <phylanx/ir/node_data.hpp>

#include <hpx/include/actions.hpp>
#include <hpx/include/components.hpp>

#include <iosfwd>
#include <string>
#include <vector>
#include <utility>

///////////////////////////////////////////////////////////////////////////////
// Add factory registration functionality
HPX_REGISTER_COMPONENT_MODULE()

///////////////////////////////////////////////////////////////////////////////
// Serialization support for the base_file actions
typedef phylanx::execution_tree::primitives::base_primitive base_primitive_type;

HPX_REGISTER_ACTION(base_primitive_type::eval_action,
    phylanx_primitive_eval_action)
HPX_REGISTER_ACTION(base_primitive_type::eval_direct_action,
    phylanx_primitive_eval_direct_action)
HPX_REGISTER_ACTION(base_primitive_type::store_action,
    phylanx_primitive_store_action)
HPX_REGISTER_ACTION(base_primitive_type::bind_action,
    phylanx_primitive_bind_action)
HPX_REGISTER_ACTION(base_primitive_type::expression_topology_action,
    phylanx_primitive_expression_topology_action)
HPX_DEFINE_GET_COMPONENT_TYPE(base_primitive_type)

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace execution_tree { namespace primitives
{
    std::vector<primitive_argument_type> base_primitive::noargs{};

    bool base_primitive::bind(std::vector<primitive_argument_type> const& args)
    {
        if (operands_.empty())
        {
            return true;
        }

        // by default simply call bind on all dependents
        std::vector<hpx::future<bool>> results;
        results.reserve(operands_.size());

        for (auto& operand : operands_)
        {
            primitive* p = util::get_if<primitive>(&operand);
            if (p != nullptr)
            {
                results.push_back(p->bind(args));
            }
        }

        bool result = true;
        if (!results.empty())
        {
            hpx::wait_all(results);
            for (auto& r : results)
            {
                result = r.get() && result;
            }
        }
        return result;
    }

    topology base_primitive::expression_topology() const
    {
        std::vector<hpx::future<topology>> results;
        results.reserve(operands_.size());

        for (auto& operand : operands_)
        {
            primitive const* p = util::get_if<primitive>(&operand);
            if (p != nullptr)
            {
                results.push_back(p->expression_topology());
            }
        }

        std::vector<topology> children;
        if (!results.empty())
        {
            hpx::wait_all(results);

            for (auto& r : results)
            {
                children.emplace_back(r.get());
            }
        }

        return topology{std::move(children)};
    }
}}}

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace execution_tree
{
    ///////////////////////////////////////////////////////////////////////////
    primitive::primitive(hpx::future<hpx::id_type>&& fid, std::string const& name)
      : base_type(std::move(fid))
    {
        if (!name.empty())
        {
            this->base_type::register_as("/phylanx/" + name).get();
        }
    }

    hpx::future<primitive_argument_type> primitive::eval(
        std::vector<primitive_argument_type> const& params) const
    {
        using action_type = primitives::base_primitive::eval_action;
        return hpx::async(action_type(), this->base_type::get_id(), params);
    }

    hpx::future<primitive_argument_type> primitive::eval() const
    {
        static std::vector<primitive_argument_type> params;
        return eval(params);
    }

    primitive_argument_type primitive::eval_direct(
        std::vector<primitive_argument_type> const& params) const
    {
        using action_type = primitives::base_primitive::eval_direct_action;
        return action_type()(this->base_type::get_id(), params);
    }

    primitive_argument_type primitive::eval_direct() const
    {
        static std::vector<primitive_argument_type> params;
        return eval_direct(params);
    }

    hpx::future<void> primitive::store(primitive_argument_type data)
    {
        using action_type = primitives::base_primitive::store_action;
        return hpx::async(
            action_type(), this->base_type::get_id(), std::move(data));
    }

    void primitive::store(hpx::launch::sync_policy,
        primitive_argument_type data)
    {
        return store(std::move(data)).get();
    }

    hpx::future<bool> primitive::bind(
        std::vector<primitive_argument_type> const& args)
    {
        using action_type = primitives::base_primitive::bind_action;
        return hpx::async(action_type(), this->base_type::get_id(), args);
    }

    bool primitive::bind(hpx::launch::sync_policy,
        std::vector<primitive_argument_type> const& args)
    {
        return bind(args).get();
    }

    hpx::future<topology> primitive::expression_topology() const
    {
        // retrieve name of component instance
        using action_type = primitives::base_primitive::
            expression_topology_action;

        hpx::future<topology> f =
            hpx::async(action_type(), this->base_type::get_id());

        // retrieve name of this node (the component can only retrieve
        // names of dependent nodes)
        std::string this_name = this->base_type::registered_name();

        return f.then(
            [this_name](hpx::future<topology> && f)
            {
                topology t = f.get();
                t.name_ = this_name;
                return t;
            });
    }

    topology primitive::expression_topology(hpx::launch::sync_policy) const
    {
        return expression_topology().get();
    }

    ///////////////////////////////////////////////////////////////////////////
    // traverse expression-tree topology and generate Newick representation
    namespace detail
    {
        std::string newick_tree_helper(topology const& t)
        {
            std::string result;

            if (!t.children_.empty())
            {
                bool first = true;
                for (auto const& child : t.children_)
                {
                    std::string name = newick_tree_helper(child);
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

                if (!result.empty())
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

            return result;
        }
    }

    std::string newick_tree(std::string const& name, topology const& t)
    {
        return "(" + detail::newick_tree_helper(t) + ") " + name + ";";
    }

    ///////////////////////////////////////////////////////////////////////////
    namespace detail
    {
        std::string dot_tree_helper(topology const& t)
        {
            std::string result;

            for (auto const& child : t.children_)
            {
                result += "    \"" + t.name_ + "\" -- \"" + child.name_ + "\";\n";
                if (!child.children_.empty())
                {
                    result += dot_tree_helper(child);
                }
            }

            return result;
        }
    }

    std::string dot_tree(std::string const& name, topology const& t)
    {
        std::string result = "graph " + name + " {\n";

        if (!t.children_.empty())
        {
            result += detail::dot_tree_helper(t);
        }

        return result + "}\n";
    }

    ///////////////////////////////////////////////////////////////////////////
    primitive_argument_type extract_value(primitive_argument_type const& val)
    {
        switch (val.index())
        {
        case 0:     // nil
        case 1:     // bool
        case 2:     // std::uint64_t
        case 3:     // std::string
        case 4:     // phylanx::ir::node_data<double>
        case 5:     // primitive
        case 6:     // std::vector<ast::expression>
        case 7:     // std::vector<primitive_argument_type>
            return val;

        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "phylanx::execution_tree::extract_value",
            "primitive_argument_type does not hold a value type");
    }

    primitive_argument_type extract_copy_value(primitive_argument_type const& val)
    {
        switch (val.index())
        {
        case 0:     // nil
        case 1:     // bool
        case 2:     // std::uint64_t
        case 3:     // std::string
        case 5:     // primitive
        case 6:     // std::vector<ast::expression>
        case 7:     // std::vector<primitive_argument_type>
            return val;

        case 4:     // phylanx::ir::node_data<double>
            {
                auto const& v = util::get<4>(val);
                if (v.is_ref())
                {
                    return v.copy();
                }
                return v;
            }
            break;

        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "phylanx::execution_tree::extract_copy_value",
            "primitive_argument_type does not hold a value type");
    }

    primitive_argument_type extract_ref_value(primitive_argument_type const& val)
    {
        switch (val.index())
        {
        case 0:     // nil
        case 1:     // bool
        case 2:     // std::uint64_t
        case 3:     // std::string
        case 5:     // primitive
        case 6:     // std::vector<ast::expression>
        case 7:     // std::vector<primitive_argument_type>
            return val;

        case 4:     // phylanx::ir::node_data<double>
            {
                auto const& v = util::get<4>(val);
                if (v.is_ref())
                {
                    return v;
                }
                return v.ref();
            }
            break;

        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "phylanx::execution_tree::extract_ref_value",
            "primitive_argument_type does not hold a value type");
    }

    primitive_argument_type extract_value(primitive_argument_type&& val)
    {
        switch (val.index())
        {
        case 0:     // nil
        case 1:     // bool
        case 2:     // std::uint64_t
        case 3:     // std::string
        case 4:     // phylanx::ir::node_data<double>
        case 5:     // primitive
        case 6:     // std::vector<ast::expression>
        case 7:     // std::vector<primitive_argument_type>
            return std::move(val);

        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "phylanx::execution_tree::extract_value",
            "primitive_argument_type does not hold a value type");
    }

    primitive_argument_type extract_copy_value(primitive_argument_type&& val)
    {
        switch (val.index())
        {
        case 0:     // nil
        case 1:     // bool
        case 2:     // std::uint64_t
        case 3:     // std::string
        case 5:     // primitive
        case 6:     // std::vector<ast::expression>
        case 7:     // std::vector<primitive_argument_type>
            return std::move(val);

        case 4:     // phylanx::ir::node_data<double>
            {
                auto && v = util::get<4>(std::move(val));
                if (v.is_ref())
                {
                    return v.copy();
                }
                return std::move(v);
            }
            break;

        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "phylanx::execution_tree::extract_value",
            "primitive_argument_type does not hold a value type");
    }

    primitive_argument_type extract_ref_value(primitive_argument_type&& val)
    {
        switch (val.index())
        {
        case 0:     // nil
        case 1:     // bool
        case 2:     // std::uint64_t
        case 3:     // std::string
        case 5:     // primitive
        case 6:     // std::vector<ast::expression>
        case 7:     // std::vector<primitive_argument_type>
            return std::move(val);

        case 4:     // phylanx::ir::node_data<double>
            {
                auto && v = util::get<4>(std::move(val));
                if (v.is_ref())
                {
                    return std::move(v);
                }
                return v.ref();
            }
            break;

        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "phylanx::execution_tree::extract_value",
            "primitive_argument_type does not hold a value type");
    }

    ///////////////////////////////////////////////////////////////////////////
    primitive_argument_type extract_literal_value(
        primitive_argument_type const& val)
    {
        switch (val.index())
        {
        case 0:     // nil
            return ast::nil{};

        case 1:     // bool
            return util::get<1>(val);

        case 2:     // std::uint64_t
            return util::get<2>(val);

        case 3:     // std::string
            return util::get<3>(val);

        case 4:     // phylanx::ir::node_data<double>
            return util::get<4>(val);

        case 5: HPX_FALLTHROUGH;    // primitive
        case 6: HPX_FALLTHROUGH;    // std::vector<ast::expression>
        case 7: HPX_FALLTHROUGH;    // std::vector<primitive_argument_type>
        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "phylanx::execution_tree::extract_literal_value",
            "primitive_argument_type does not hold a literal value type");
    }

    primitive_argument_type extract_literal_ref_value(
        primitive_argument_type const& val)
    {
        switch (val.index())
        {
        case 0:     // nil
            return ast::nil{};

        case 1:     // bool
            return util::get<1>(val);

        case 2:     // std::uint64_t
            return util::get<2>(val);

        case 3:     // std::string
            return util::get<3>(val);

        case 4:     // phylanx::ir::node_data<double>
            return util::get<4>(val).ref();

        case 5: HPX_FALLTHROUGH;    // primitive
        case 6: HPX_FALLTHROUGH;    // std::vector<ast::expression>
        case 7: HPX_FALLTHROUGH;    // std::vector<primitive_argument_type>
        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "phylanx::execution_tree::extract_literal_value",
            "primitive_argument_type does not hold a literal value type");
    }

    primitive_argument_type extract_literal_value(primitive_argument_type&& val)
    {
        switch (val.index())
        {
        case 0:     // nil
            return ast::nil{};

        case 1:     // bool
            return util::get<1>(std::move(val));

        case 2:     // std::uint64_t
            return util::get<2>(std::move(val));

        case 3:     // std::string
            return util::get<3>(std::move(val));

        case 4:     // phylanx::ir::node_data<double>
            return util::get<4>(std::move(val));

        case 5: HPX_FALLTHROUGH;    // primitive
        case 6: HPX_FALLTHROUGH;    // std::vector<ast::expression>
        case 7: HPX_FALLTHROUGH;    // std::vector<primitive_argument_type>
        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "phylanx::execution_tree::extract_literal_value",
            "primitive_argument_type does not hold a literal value type");
    }

    ///////////////////////////////////////////////////////////////////////////
    ir::node_data<double> extract_numeric_value(
        primitive_argument_type const& val)
    {
        switch (val.index())
        {
        case 1:     // bool
            return ir::node_data<double>{double(util::get<1>(val))};

        case 2:     // std::uint64_t
            return ir::node_data<double>{double(util::get<2>(val))};

        case 4:     // phylanx::ir::node_data<double>
            return util::get<4>(val).ref();

        case 0: HPX_FALLTHROUGH;    // nil
        case 3: HPX_FALLTHROUGH;    // string
        case 5: HPX_FALLTHROUGH;    // primitive
        case 6: HPX_FALLTHROUGH;    // std::vector<ast::expression>
        case 7: HPX_FALLTHROUGH;    // std::vector<primitive_argument_type>
        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "phylanx::execution_tree::extract_numeric_value",
            "primitive_argument_type does not hold a numeric value type");
    }

    ir::node_data<double> extract_numeric_value(primitive_argument_type&& val)
    {
        switch (val.index())
        {
        case 1:     // bool
            return ir::node_data<double>{double(util::get<1>(std::move(val)))};

        case 2:     // std::uint64_t
            return ir::node_data<double>{double(util::get<2>(std::move(val)))};

        case 4:     // phylanx::ir::node_data<double>
            return util::get<4>(std::move(val));

        case 0: HPX_FALLTHROUGH;    // nil
        case 3: HPX_FALLTHROUGH;    // string
        case 5: HPX_FALLTHROUGH;    // primitive
        case 6: HPX_FALLTHROUGH;    // std::vector<ast::expression>
        case 7: HPX_FALLTHROUGH;    // std::vector<primitive_argument_type>
        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "phylanx::execution_tree::extract_numeric_value",
            "primitive_argument_type does not hold a numeric value type");
    }

    ///////////////////////////////////////////////////////////////////////////
    std::int64_t extract_integer_value(primitive_argument_type const& val)
    {
        switch (val.index())
        {
        case 1:     // bool
            return std::int64_t{util::get<1>(val)};

        case 2:     // std::uint64_t
            return util::get<2>(val);

        case 4:     // phylanx::ir::node_data<double>
            return std::int64_t(util::get<4>(val)[0]);

        case 0: HPX_FALLTHROUGH;    // nil
        case 3: HPX_FALLTHROUGH;    // string
        case 5: HPX_FALLTHROUGH;    // primitive
        case 6: HPX_FALLTHROUGH;    // std::vector<ast::expression>
        case 7: HPX_FALLTHROUGH;    // std::vector<primitive_argument_type>
        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "phylanx::execution_tree::extract_integer_value",
            "primitive_argument_type does not hold a numeric value type");
    }

    std::int64_t extract_integer_value(primitive_argument_type&& val)
    {
        switch (val.index())
        {
        case 1:     // bool
            return std::int64_t{util::get<1>(std::move(val))};

        case 2:     // std::uint64_t
            return util::get<2>(std::move(val));

        case 4:     // phylanx::ir::node_data<double>
            return std::int64_t(util::get<4>(std::move(val))[0]);

        case 0: HPX_FALLTHROUGH;    // nil
        case 3: HPX_FALLTHROUGH;    // string
        case 5: HPX_FALLTHROUGH;    // primitive
        case 6: HPX_FALLTHROUGH;    // std::vector<ast::expression>
        case 7: HPX_FALLTHROUGH;    // std::vector<primitive_argument_type>
        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "phylanx::execution_tree::extract_numeric_value",
            "primitive_argument_type does not hold a numeric value type");
    }

    ///////////////////////////////////////////////////////////////////////////
    std::uint8_t extract_boolean_value(primitive_argument_type const& val)
    {
        switch (val.index())
        {
        case 0:     // nil
            return false;

        case 1:     // bool
            return util::get<1>(val);

        case 2:     // std::uint64_t
            return util::get<2>(val) != 0;

        case 4:     // phylanx::ir::node_data<double>
            return bool(util::get<4>(val));

        case 7:     // std::vector<primitive_argument_type>
            return !(util::get<7>(val).get().empty());

        case 3: HPX_FALLTHROUGH;    // string
        case 5: HPX_FALLTHROUGH;    // primitive
        case 6: HPX_FALLTHROUGH;    // std::vector<ast::expression>
        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "phylanx::execution_tree::extract_boolean_value",
            "primitive_argument_type does not hold a boolean value type");
    }

    std::uint8_t extract_boolean_value(primitive_argument_type && val)
    {
        switch (val.index())
        {
        case 0:     // nil
            return false;

        case 1:     // bool
            return util::get<1>(std::move(val));

        case 2:     // std::uint64_t
            return util::get<2>(std::move(val)) != 0;

        case 4:     // phylanx::ir::node_data<double>
            return bool(util::get<4>(std::move(val)));

        case 7:     // std::vector<primitive_argument_type>
            return !(util::get<7>(std::move(val)).get().empty());

        case 3: HPX_FALLTHROUGH;    // string
        case 5: HPX_FALLTHROUGH;    // primitive
        case 6: HPX_FALLTHROUGH;    // std::vector<ast::expression>
        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "phylanx::execution_tree::extract_boolean_value",
            "primitive_argument_type does not hold a boolean value type");
    }

    ///////////////////////////////////////////////////////////////////////////
    std::string extract_string_value(primitive_argument_type const& val)
    {
        switch (val.index())
        {
        case 3:     // string
            return util::get<3>(val);

        case 0: HPX_FALLTHROUGH;    // nil
        case 1: HPX_FALLTHROUGH;    // bool
        case 2: HPX_FALLTHROUGH;    // std::uint64_t
        case 4: HPX_FALLTHROUGH;    // phylanx::ir::node_data<double>
        case 7: HPX_FALLTHROUGH;    // std::vector<primitive_argument_type>
        case 5: HPX_FALLTHROUGH;    // primitive
        case 6: HPX_FALLTHROUGH;    // std::vector<ast::expression>
        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "phylanx::execution_tree::extract_string_value",
            "primitive_argument_type does not hold a string value type");
    }

    std::string extract_string_value(primitive_argument_type && val)
    {
        switch (val.index())
        {
        case 3:     // string
            return util::get<3>(std::move(val));

        case 0: HPX_FALLTHROUGH;    // nil
        case 1: HPX_FALLTHROUGH;    // bool
        case 2: HPX_FALLTHROUGH;    // std::uint64_t
        case 4: HPX_FALLTHROUGH;    // phylanx::ir::node_data<double>
        case 7: HPX_FALLTHROUGH;    // std::vector<primitive_argument_type>
        case 5: HPX_FALLTHROUGH;    // primitive
        case 6: HPX_FALLTHROUGH;    // std::vector<ast::expression>
        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "phylanx::execution_tree::extract_string_value",
            "primitive_argument_type does not hold a string value type");
    }

    ///////////////////////////////////////////////////////////////////////////
    std::vector<ast::expression> extract_ast_value(
        primitive_argument_type const& val)
    {
        switch (val.index())
        {
        case 1:     // bool
            return {ast::expression(util::get<1>(val))};

        case 2:     // std::uint64_t
            return {ast::expression(util::get<2>(val))};

        case 3:     // string
            return {ast::expression(util::get<3>(val))};

        case 4:     // phylanx::ir::node_data<double>
            return {ast::expression(util::get<4>(val))};

        case 6:     // std::vector<ast::expression>
            return util::get<6>(val);

        case 0: HPX_FALLTHROUGH;    // nil
        case 5: HPX_FALLTHROUGH;    // primitive
        case 7: HPX_FALLTHROUGH;    // std::vector<primitive_argument_type>
        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "phylanx::execution_tree::extract_ast_value",
            "primitive_argument_type does not hold a boolean value type");
    }

    std::vector<ast::expression> extract_ast_value(
        primitive_argument_type && val)
    {
        switch (val.index())
        {
        case 1:     // bool
            return {ast::expression(util::get<1>(std::move(val)))};

        case 2:     // std::uint64_t
            return {ast::expression(util::get<2>(std::move(val)))};

        case 3:     // string
            return {ast::expression(util::get<3>(std::move(val)))};

        case 4:     // phylanx::ir::node_data<double>
            return {ast::expression(util::get<4>(std::move(val)))};

        case 6:     // std::vector<ast::expression>
            return util::get<6>(std::move(val));

        case 0: HPX_FALLTHROUGH;    // nil
        case 5: HPX_FALLTHROUGH;    // primitive
        case 7: HPX_FALLTHROUGH;    // std::vector<primitive_argument_type>
        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "phylanx::execution_tree::extract_ast_value",
            "primitive_argument_type does not hold a boolean value type");
    }

    ///////////////////////////////////////////////////////////////////////////
    std::vector<primitive_argument_type> extract_list_value(
        primitive_argument_type const& val)
    {
        switch (val.index())
        {
        case 0:     // nil
            return {util::get<0>(val)};

        case 1:     // bool
            return {util::get<1>(val)};

        case 2:     // std::uint64_t
            return {util::get<2>(val)};

        case 3:     // string
            return {util::get<3>(val)};

        case 4:     // phylanx::ir::node_data<double>
            return {util::get<4>(val)};

        case 6:     // std::vector<ast::expression>
            return {util::get<6>(val)};

        case 7:     // std::vector<primitive_argument_type>
//             {
//                 auto const& v = util::get<7>(val).get();
//                 std::vector<primitive_argument_type> result;
//                 result.reserve(v.size());
//                 for (auto const& elem : v)
//                 {
//                     result.emplace_back(extract_list_value(elem));
//                 }
//                 return result;
//             }
            return util::get<7>(val).get();

        case 5: HPX_FALLTHROUGH;    // primitive
        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "phylanx::execution_tree::extract_list_value",
            "primitive_argument_type does not hold a boolean value type");
    }

    std::vector<primitive_argument_type> extract_list_value(
        primitive_argument_type && val)
    {
        switch (val.index())
        {
        case 0:     // nil
            return {util::get<0>(std::move(val))};

        case 1:     // bool
            return {util::get<1>(std::move(val))};

        case 2:     // std::uint64_t
            return {util::get<2>(std::move(val))};

        case 3:     // string
            return {util::get<3>(std::move(val))};

        case 4:     // phylanx::ir::node_data<double>
            return {util::get<4>(std::move(val))};

        case 6:     // std::vector<ast::expression>
            return {util::get<6>(std::move(val))};

        case 7:     // std::vector<primitive_argument_type>
            {
                auto && v = util::get<7>(std::move(val)).get();
                std::vector<primitive_argument_type> result;
                result.reserve(v.size());
                for (auto && elem : v)
                {
                    result.push_back(extract_list_value(std::move(elem)));
                }
                return result;
            }

        case 5: HPX_FALLTHROUGH;    // primitive
        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "phylanx::execution_tree::extract_list_value",
            "primitive_argument_type does not hold a boolean value type");
    }

    ///////////////////////////////////////////////////////////////////////////
    primitive primitive_operand(primitive_argument_type const& val)
    {
        primitive const* p = util::get_if<primitive>(&val);
        if (p != nullptr)
            return *p;

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "phylanx::execution_tree::extract_primitive",
            "primitive_value_type does not hold a primitive");
    }

    bool is_primitive_operand(primitive_argument_type const& val)
    {
        return util::get_if<primitive>(&val) != nullptr;
    }

    ///////////////////////////////////////////////////////////////////////////
    hpx::future<primitive_argument_type> value_operand(
        primitive_argument_type const& val,
        std::vector<primitive_argument_type> const& args)
    {
        primitive const* p = util::get_if<primitive>(&val);
        if (p != nullptr)
        {
            return p->eval(args).then(
                [](hpx::future<primitive_argument_type> && f)
                {
                    return extract_value(f.get());
                });
        }

        HPX_ASSERT(valid(val));
        return hpx::make_ready_future(extract_ref_value(val));
    }

    primitive_argument_type value_operand_sync(
        primitive_argument_type const& val,
        std::vector<primitive_argument_type> const& args)
    {
        primitive const* p = util::get_if<primitive>(&val);
        if (p != nullptr)
        {
            return extract_value(p->eval_direct(args));
        }

        HPX_ASSERT(valid(val));
        return extract_value(val);
    }

    primitive_argument_type value_operand_ref_sync(
        primitive_argument_type const& val,
        std::vector<primitive_argument_type> const& args)
    {
        primitive const* p = util::get_if<primitive>(&val);
        if (p != nullptr)
        {
            return extract_value(p->eval_direct(args));
        }

        HPX_ASSERT(valid(val));
        return extract_ref_value(val);
    }

    ///////////////////////////////////////////////////////////////////////////
    hpx::future<primitive_argument_type> literal_operand(
        primitive_argument_type const& val,
        std::vector<primitive_argument_type> const& args)
    {
        primitive const* p = util::get_if<primitive>(&val);
        if (p != nullptr)
        {
            return p->eval(args).then(
                [](hpx::future<primitive_argument_type> && f)
                {
                    return extract_literal_value(f.get());
                });
        }

        HPX_ASSERT(valid(val));
        return hpx::make_ready_future(extract_literal_ref_value(val));
    }

    primitive_argument_type literal_operand_sync(
        primitive_argument_type const& val,
        std::vector<primitive_argument_type> const& args)
    {
        primitive const* p = util::get_if<primitive>(&val);
        if (p != nullptr)
        {
            return extract_literal_value(p->eval_direct(args));
        }

        HPX_ASSERT(valid(val));
        return extract_literal_ref_value(val);
    }

    ///////////////////////////////////////////////////////////////////////////
    hpx::future<ir::node_data<double>> numeric_operand(
        primitive_argument_type const& val,
        std::vector<primitive_argument_type> const& args)
    {
        primitive const* p = util::get_if<primitive>(&val);
        if (p != nullptr)
        {
            return p->eval(args).then(
                [](hpx::future<primitive_argument_type> && f)
                {
                    return extract_numeric_value(f.get());
                });
        }

        HPX_ASSERT(valid(val));
        return hpx::make_ready_future(extract_numeric_value(val));
    }

    ir::node_data<double> numeric_operand_sync(
        primitive_argument_type const& val,
        std::vector<primitive_argument_type> const& args)
    {
        primitive const* p = util::get_if<primitive>(&val);
        if (p != nullptr)
        {
            return extract_numeric_value(p->eval_direct(args));
        }

        HPX_ASSERT(valid(val));
        return extract_numeric_value(val);
    }

    ///////////////////////////////////////////////////////////////////////////
    hpx::future<std::uint8_t> boolean_operand(
        primitive_argument_type const& val,
        std::vector<primitive_argument_type> const& args)
    {
        primitive const* p = util::get_if<primitive>(&val);
        if (p != nullptr)
        {
            return p->eval(args).then(
                [](hpx::future<primitive_argument_type> && f)
                {
                    return extract_boolean_value(f.get());
                });
        }

        HPX_ASSERT(valid(val));
        return hpx::make_ready_future(extract_boolean_value(val));
    }

    std::uint8_t boolean_operand_sync(primitive_argument_type const& val,
        std::vector<primitive_argument_type> const& args)
    {
        primitive const* p = util::get_if<primitive>(&val);
        if (p != nullptr)
        {
            return extract_boolean_value(p->eval_direct(args));
        }

        HPX_ASSERT(valid(val));
        return extract_boolean_value(val);
    }

    ///////////////////////////////////////////////////////////////////////////
    hpx::future<std::string> string_operand(
        primitive_argument_type const& val,
        std::vector<primitive_argument_type> const& args)
    {
        primitive const* p = util::get_if<primitive>(&val);
        if (p != nullptr)
        {
            return p->eval(args).then(
                [](hpx::future<primitive_argument_type> && f)
                {
                    return extract_string_value(f.get());
                });
        }

        HPX_ASSERT(valid(val));
        return hpx::make_ready_future(extract_string_value(val));
    }

    std::string string_operand_sync(primitive_argument_type const& val,
        std::vector<primitive_argument_type> const& args)
    {
        primitive const* p = util::get_if<primitive>(&val);
        if (p != nullptr)
        {
            return extract_string_value(p->eval_direct(args));
        }

        HPX_ASSERT(valid(val));
        return extract_string_value(val);
    }

    ///////////////////////////////////////////////////////////////////////////
    hpx::future<std::vector<ast::expression>> ast_operand(
        primitive_argument_type const& val,
        std::vector<primitive_argument_type> const& args)
    {
        primitive const* p = util::get_if<primitive>(&val);
        if (p != nullptr)
        {
            return p->eval(args).then(
                [](hpx::future<primitive_argument_type> && f)
                {
                    return extract_ast_value(f.get());
                });
        }

        HPX_ASSERT(valid(val));
        return hpx::make_ready_future(extract_ast_value(val));
    }

    std::vector<ast::expression> ast_operand_sync(
        primitive_argument_type const& val,
        std::vector<primitive_argument_type> const& args)
    {
        primitive const* p = util::get_if<primitive>(&val);
        if (p != nullptr)
        {
            return extract_ast_value(p->eval_direct(args));
        }

        HPX_ASSERT(valid(val));
        return extract_ast_value(val);
    }

    ///////////////////////////////////////////////////////////////////////////
    hpx::future<std::vector<primitive_argument_type>> list_operand(
        primitive_argument_type const& val,
        std::vector<primitive_argument_type> const& args)
    {
        primitive const* p = util::get_if<primitive>(&val);
        if (p != nullptr)
        {
            return p->eval(args).then(
                [](hpx::future<primitive_argument_type> && f)
                {
                    return extract_list_value(f.get());
                });
        }

        HPX_ASSERT(valid(val));
        return hpx::make_ready_future(extract_list_value(val));
    }

    std::vector<primitive_argument_type> list_operand_sync(
        primitive_argument_type const& val,
        std::vector<primitive_argument_type> const& args)
    {
        primitive const* p = util::get_if<primitive>(&val);
        if (p != nullptr)
        {
            return extract_list_value(p->eval_direct(args));
        }

        HPX_ASSERT(valid(val));
        return extract_list_value(val);
    }

    ///////////////////////////////////////////////////////////////////////////
    primitive_argument_type to_primitive_value_type(
        ast::literal_value_type && val)
    {
        switch (val.index())
        {
        case 0:
            return ast::nil{};

        case 1:     // bool
            return util::get<1>(std::move(val));

        case 2:     // std::uint64_t
            return util::get<2>(std::move(val));

        case 3:     // std::string
            return util::get<3>(std::move(val));

        case 4:     // phylanx::ir::node_data<double>
            return util::get<4>(std::move(val));

        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "phylanx::execution_tree::primitives::to_primitive_value_type",
            "unsupported primitive_argument_type");
    }

    ///////////////////////////////////////////////////////////////////////////
    std::ostream& operator<<(std::ostream& os,
        primitive_argument_type const& val)
    {
        switch (val.index())
        {
        case 0:     // nil
            return os;  // no output

        case 1:     // bool
            os << util::get<1>(val);
            return os;

        case 2:     // std::uint64_t
            os << util::get<2>(val);
            return os;

        case 3:     // std::string
            os << util::get<3>(val);
            return os;

        case 4:     // phylanx::ir::node_data<double>
            os << util::get<4>(val);
            return os;

        case 5:
            os << value_operand_sync(util::get<5>(val), {});
            return os;

        case 6:     // std::vector<ast::expression>
            for (auto const& ast : util::get<6>(val))
            {
                os << ast;
            }
            return os;

        case 7:     // std::vector<primitive_argument_type>
            for (auto const& elem : util::get<7>(val).get())
            {
                os << elem;
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
}}

