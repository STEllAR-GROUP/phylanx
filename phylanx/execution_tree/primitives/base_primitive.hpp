//  Copyright (c) 2017 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_PRIMITIVES_BASE_PRIMITIVE_SEP_05_2017_1102AM)
#define PHYLANX_PRIMITIVES_BASE_PRIMITIVE_SEP_05_2017_1102AM

#include <phylanx/config.hpp>
#include <phylanx/ast/node.hpp>
#include <phylanx/ir/node_data.hpp>
#include <phylanx/util/optional.hpp>
#include <phylanx/util/serialization/optional.hpp>

#include <hpx/include/components.hpp>
#include <hpx/include/util.hpp>

#include <initializer_list>
#include <iosfwd>
#include <map>
#include <string>
#include <utility>
#include <vector>

namespace phylanx { namespace execution_tree
{
    ///////////////////////////////////////////////////////////////////////////
    namespace primitives
    {
        class HPX_COMPONENT_EXPORT base_primitive;
    }

    class HPX_COMPONENT_EXPORT primitive;

    ///////////////////////////////////////////////////////////////////////////
    struct primitive_argument_type;

    ///////////////////////////////////////////////////////////////////////////
    class primitive
      : public hpx::components::client_base<primitive,
            primitives::base_primitive>
    {
    private:
        using base_type =
            hpx::components::client_base<primitive, primitives::base_primitive>;

    public:
        primitive() = default;

        primitive(hpx::future<hpx::id_type> && fid)
          : base_type(std::move(fid))
        {
        }

        primitive(hpx::future<hpx::id_type> && fid, std::string const& name);

        hpx::future<primitive_argument_type> eval() const;
        hpx::future<primitive_argument_type> eval(
            std::vector<primitive_argument_type> const& args) const;

        primitive_argument_type eval_direct() const;
        primitive_argument_type eval_direct(
            std::vector<primitive_argument_type> const& args) const;

        hpx::future<void> store(primitive_argument_type const&);
        void store(hpx::launch::sync_policy, primitive_argument_type const&);

        hpx::future<bool> bind(std::vector<primitive_argument_type> const&);
        bool bind(hpx::launch::sync_policy,
            std::vector<primitive_argument_type> const&);
    };

    ///////////////////////////////////////////////////////////////////////////
    using argument_value_type =
        phylanx::util::variant<
            ast::nil
          , bool
          , std::int64_t
          , std::string
          , phylanx::ir::node_data<double>
          , primitive
          , std::vector<ast::expression>
          , phylanx::util::recursive_wrapper<std::vector<primitive_argument_type>>
        >;

    struct primitive_argument_type;
    using primitive_result_type = primitive_argument_type;


    PHYLANX_EXPORT primitive_result_type value_operand_sync(
        primitive_argument_type const& val,
        std::vector<primitive_argument_type> const& args);

    struct primitive_argument_type : argument_value_type
    {
        // poor man's forwarding constructor
        template <typename ... Ts>
        primitive_argument_type(Ts &&... ts)
          : argument_value_type{std::forward<Ts>(ts)...}
        {}

        primitive_argument_type(double val)
          : argument_value_type{phylanx::ir::node_data<double>{val}}
        {}

        primitive_result_type operator()() const
        {
            return value_operand_sync(*this, {});
        }

        primitive_result_type
        operator()(std::vector<primitive_argument_type> const& args) const
        {
            return value_operand_sync(*this, args);
        }

        // workaround for problem in implementation of MSVC14.12
        // variant::visit
        argument_value_type& variant() { return *this; }
        argument_value_type const& variant() const { return *this; }
    };

    // a argument is valid of its not nil{}
    inline bool valid(primitive_argument_type const& val)
    {
        return val.index() != 0;
    }
    inline bool valid(primitive_argument_type && val)
    {
        return val.index() != 0;
    }

    PHYLANX_EXPORT std::ostream& operator<<(std::ostream& os,
        primitive_argument_type const&);
}}

namespace phylanx { namespace execution_tree { namespace primitives
{
    ///////////////////////////////////////////////////////////////////////////
    class HPX_COMPONENT_EXPORT base_primitive
      : public hpx::traits::detail::component_tag
    {
    public:
        base_primitive() = default;

        base_primitive(std::vector<primitive_argument_type> && operands)
          : operands_(std::move(operands))
        {}

        virtual ~base_primitive() = default;

        hpx::future<primitive_result_type> eval_nonvirtual(
            std::vector<primitive_argument_type> const& args) const
        {
            return eval(args);
        }
        virtual hpx::future<primitive_result_type> eval(
            std::vector<primitive_argument_type> const& params) const
        {
            return hpx::make_ready_future(eval_direct(params));
        }

        primitive_result_type eval_direct_nonvirtual(
            std::vector<primitive_argument_type> const& args) const
        {
            return eval_direct(args);
        }
        virtual primitive_result_type eval_direct(
            std::vector<primitive_argument_type> const& params) const
        {
            return eval(params).get();
        }

        void store_nonvirtual(primitive_result_type const& data)
        {
            store(data);
        }
        virtual void store(primitive_result_type const&)
        {
            HPX_THROW_EXCEPTION(hpx::invalid_status,
                "phylanx::execution_tree::primitives::base_primitive",
                "store function should only be called in store_primitive");
        }

        bool bind_nonvirtual(std::vector<primitive_argument_type> const& args)
        {
            return bind(args);
        }
        virtual bool bind(std::vector<primitive_argument_type> const& args);

    public:

#if defined(PHYLANX_DEBUG)
        HPX_DEFINE_COMPONENT_DIRECT_ACTION(
            base_primitive, eval_nonvirtual, eval_action);
        HPX_DEFINE_COMPONENT_DIRECT_ACTION(
            base_primitive, bind_nonvirtual, bind_action);
#else
        HPX_DEFINE_COMPONENT_ACTION(
            base_primitive, eval_nonvirtual, eval_action);
        HPX_DEFINE_COMPONENT_ACTION(
            base_primitive, bind_nonvirtual, bind_action);
#endif
        HPX_DEFINE_COMPONENT_DIRECT_ACTION(
            base_primitive, eval_direct_nonvirtual, eval_direct_action);
        HPX_DEFINE_COMPONENT_ACTION(
            base_primitive, store_nonvirtual, store_action);

    protected:
        static std::vector<primitive_argument_type> noargs;
        std::vector<primitive_argument_type> operands_;
    };
}}}

// Declaration of serialization support for the local_file actions
HPX_REGISTER_ACTION_DECLARATION(
    phylanx::execution_tree::primitives::base_primitive::eval_action,
    phylanx_primitive_eval_action);
HPX_REGISTER_ACTION_DECLARATION(
    phylanx::execution_tree::primitives::base_primitive::eval_direct_action,
    phylanx_primitive_eval_direct_action);
HPX_REGISTER_ACTION_DECLARATION(
    phylanx::execution_tree::primitives::base_primitive::store_action,
    phylanx_primitive_store_action);
HPX_REGISTER_ACTION_DECLARATION(
    phylanx::execution_tree::primitives::base_primitive::bind_action,
    phylanx_primitive_bind_action);

namespace phylanx { namespace execution_tree
{
    ///////////////////////////////////////////////////////////////////////////
    PHYLANX_EXPORT primitive_argument_type to_primitive_value_type(
        ast::literal_value_type && val);

    ///////////////////////////////////////////////////////////////////////////
    // Extract a value type from a given primitive_argument_type, throw
    // if it doesn't hold one.
    PHYLANX_EXPORT primitive_result_type extract_value(
        primitive_argument_type const& val);
    PHYLANX_EXPORT primitive_result_type && extract_value(
        primitive_result_type && val);

    // Extract a literal type from a given primitive_argument_type, throw
    // if it doesn't hold one.
    PHYLANX_EXPORT primitive_result_type extract_literal_value(
        primitive_argument_type const& val);
    PHYLANX_EXPORT primitive_result_type extract_literal_value(
        primitive_result_type && val);

    // Extract a ir::node_data<double> type from a given primitive_argument_type,
    // throw if it doesn't hold one.
    PHYLANX_EXPORT ir::node_data<double> extract_numeric_value(
        primitive_argument_type const& val);
    PHYLANX_EXPORT ir::node_data<double> extract_numeric_value(
        primitive_result_type && val);

    // Extract a std::int64_t type from a given primitive_argument_type,
    // throw if it doesn't hold one.
    PHYLANX_EXPORT std::int64_t extract_integer_value(
        primitive_argument_type const& val);
    PHYLANX_EXPORT std::int64_t extract_integer_value(
        primitive_result_type && val);

    // Extract a boolean type from a given primitive_argument_type,
    // throw if it doesn't hold one.
    PHYLANX_EXPORT std::uint8_t extract_boolean_value(
        primitive_argument_type const& val);
    PHYLANX_EXPORT std::uint8_t extract_boolean_value(
        primitive_result_type && val);

    // Extract a std::string type from a given primitive_argument_type,
    // throw if it doesn't hold one.
    PHYLANX_EXPORT std::string extract_string_value(
        primitive_argument_type const& val);
    PHYLANX_EXPORT std::string extract_string_value(
        primitive_result_type && val);

    // Extract an AST type from a given primitive_argument_type,
    // throw if it doesn't hold one.
    PHYLANX_EXPORT std::vector<ast::expression> extract_ast_value(
        primitive_argument_type const& val);
    PHYLANX_EXPORT std::vector<ast::expression> extract_ast_value(
        primitive_result_type && val);

    // Extract a list type from a given primitive_argument_type,
    // throw if it doesn't hold one.
    PHYLANX_EXPORT std::vector<primitive_result_type> extract_list_value(
        primitive_argument_type const& val);
    PHYLANX_EXPORT std::vector<primitive_result_type> extract_list_value(
        primitive_result_type && val);

    ///////////////////////////////////////////////////////////////////////////
    // Extract a primitive from a given primitive_argument_type, throw
    // if it doesn't hold one.
    PHYLANX_EXPORT primitive primitive_operand(
        primitive_argument_type const& val);
    PHYLANX_EXPORT bool is_primitive_operand(
        primitive_argument_type const& val);

    // Extract a primitive_result_type from a primitive_argument_type (that
    // could be a value type).
    PHYLANX_EXPORT hpx::future<primitive_result_type> value_operand(
        primitive_argument_type const& val,
        std::vector<primitive_argument_type> const& args);
// was declared above
//     PHYLANX_EXPORT primitive_result_type value_operand_sync(
//         primitive_argument_type const& val,
//         std::vector<primitive_argument_type> const& args);

    // Extract a primitive_result_type from a primitive_argument_type (that
    // could be a primitive or a literal value).
    PHYLANX_EXPORT hpx::future<primitive_result_type> literal_operand(
        primitive_argument_type const& val,
        std::vector<primitive_argument_type> const& args);
    PHYLANX_EXPORT primitive_argument_type literal_operand_sync(
        primitive_argument_type const& val,
        std::vector<primitive_argument_type> const& args);

    // Extract a node_data<double> from a primitive_argument_type (that
    // could be a primitive or a literal value).
    PHYLANX_EXPORT hpx::future<ir::node_data<double>> numeric_operand(
        primitive_argument_type const& val,
        std::vector<primitive_argument_type> const& args);
    PHYLANX_EXPORT ir::node_data<double> numeric_operand_sync(
        primitive_argument_type const& val,
        std::vector<primitive_argument_type> const& args);

    // Extract a boolean from a primitive_argument_type (that
    // could be a primitive or a literal value).
    PHYLANX_EXPORT hpx::future<std::uint8_t> boolean_operand(
        primitive_argument_type const& val,
        std::vector<primitive_argument_type> const& args);
    PHYLANX_EXPORT std::uint8_t boolean_operand_sync(
        primitive_argument_type const& val,
        std::vector<primitive_argument_type> const& args);

    // Extract a std::string from a primitive_argument_type (that
    // could be a primitive or a string value).
    PHYLANX_EXPORT hpx::future<std::string> string_operand(
        primitive_argument_type const& val,
        std::vector<primitive_argument_type> const& args);
    PHYLANX_EXPORT std::string string_operand_sync(
        primitive_argument_type const& val,
        std::vector<primitive_argument_type> const& args);

    // Extract an AST from a primitive_argument_type (that
    // could be a primitive or a literal value).
    PHYLANX_EXPORT hpx::future<std::vector<ast::expression>> ast_operand(
        primitive_argument_type const& val,
        std::vector<primitive_argument_type> const& args);
    PHYLANX_EXPORT std::vector<ast::expression> ast_operand_sync(
        primitive_argument_type const& val,
        std::vector<primitive_argument_type> const& args);

    // Extract a list from a primitive_argument_type (that
    // could be a primitive or a literal value).
    PHYLANX_EXPORT hpx::future<std::vector<primitive_result_type>> list_operand(
        primitive_argument_type const& val,
        std::vector<primitive_argument_type> const& args);
    PHYLANX_EXPORT std::vector<primitive_result_type> list_operand_sync(
        primitive_argument_type const& val,
        std::vector<primitive_argument_type> const& args);

    ///////////////////////////////////////////////////////////////////////////
    // Factory functions
    using factory_function_type = primitive (*)(
        hpx::id_type, std::vector<primitive_argument_type>&&,
        std::string const&);

    using match_pattern_type =
        hpx::util::tuple<std::string, std::string, factory_function_type>;

    using pattern_list = std::vector<std::vector<match_pattern_type>>;

    ///////////////////////////////////////////////////////////////////////////
    // Generic creation helper for creating an instance of the given primitive.
    template <typename Primitive>
    primitive create(hpx::id_type locality,
        std::vector<primitive_argument_type>&& operands, std::string const& name)
    {
        return primitive(
            hpx::new_<Primitive>(locality, std::move(operands)), name);
    }
}}

namespace phylanx { namespace execution_tree { namespace primitives
{
    namespace detail
    {
        // Invoke the given function on all items in the input vector, while
        // returning another vector holding the respective results.
        template <typename T, typename F, typename ... Ts>
        auto map_operands(std::vector<T> const& in, F && f, Ts && ... ts)
        ->  std::vector<decltype(hpx::util::invoke(f, std::declval<T>(), ts...))>
        {
            std::vector<
                    decltype(hpx::util::invoke(f, std::declval<T>(), ts...))
                > out;
            out.reserve(in.size());

            for (auto const& d : in)
            {
                out.push_back(hpx::util::invoke(f, d, ts...));
            }
            return out;
        }

        ///////////////////////////////////////////////////////////////////////
        // check if one of the optionals in the list of operands is empty
        inline bool verify_argument_values(
            std::vector<primitive_result_type> const& ops)
        {
            for (auto const& op : ops)
            {
                if (!valid(op))
                    return false;
            }
            return true;
        }
    }
}}}

#endif


