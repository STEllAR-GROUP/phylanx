//  Copyright (c) 2017 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/base_primitive.hpp>
#include <phylanx/ir/node_data.hpp>

#include <hpx/include/actions.hpp>
#include <hpx/include/components.hpp>

#include <string>
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
HPX_DEFINE_GET_COMPONENT_TYPE(base_primitive_type)

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace execution_tree
{
    ///////////////////////////////////////////////////////////////////////////
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

    hpx::future<void> primitive::store(primitive_argument_type const& data)
    {
        using action_type = primitives::base_primitive::store_action;
        return hpx::async(action_type(), this->base_type::get_id(), data);
    }

    void primitive::store(hpx::launch::sync_policy,
        primitive_argument_type const& data)
    {
        return store(data).get();
    }

    ///////////////////////////////////////////////////////////////////////////
    primitive_argument_type extract_value(primitive_argument_type const& val)
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

        case 6:     // std::vector<ast::expression>
            return util::get<6>(val);

        case 7:     // std::vector<primitive_argument_type>
            {
                auto const& v = util::get<7>(val).get();
                std::vector<primitive_argument_type> result;
                result.reserve(v.size());
                for (auto const& elem : v)
                {
                    result.push_back(extract_value(elem));
                }
                return result;
            }

        case 5:     // primitive
        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "phylanx::execution_tree::extract_value",
            "primitive_argument_type does not hold a value type");
    }

    primitive_argument_type extract_value(primitive_argument_type&& val)
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

        case 6:     // std::vector<ast::expression>
            return util::get<6>(std::move(val));

        case 7:     // std::vector<primitive_argument_type>
            {
                auto && v = util::get<7>(std::move(val)).get();
                std::vector<primitive_argument_type> result;
                result.reserve(v.size());
                for (auto && elem : v)
                {
                    result.push_back(extract_value(std::move(elem)));
                }
                return result;
            }

        case 5:     // primitive
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
            return util::get<4>(val);

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
            "phylanx::execution_tree::extract_numeric_value",
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
            "phylanx::execution_tree::extract_numeric_value",
            "primitive_argument_type does not hold a boolean value type");
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
                [&args](hpx::future<primitive_argument_type> && f)
                {
                    return extract_value(f.get());
                });
        }

        HPX_ASSERT(valid(val));
        return hpx::make_ready_future(extract_value(val));
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

    ///////////////////////////////////////////////////////////////////////////
    hpx::future<primitive_argument_type> literal_operand(
        primitive_argument_type const& val,
        std::vector<primitive_argument_type> const& args)
    {
        primitive const* p = util::get_if<primitive>(&val);
        if (p != nullptr)
        {
            return p->eval(args).then(
                [&args](hpx::future<primitive_argument_type> && f)
                {
                    return extract_literal_value(f.get());
                });
        }

        HPX_ASSERT(valid(val));
        return hpx::make_ready_future(extract_literal_value(val));
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
        return extract_literal_value(val);
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
}}

