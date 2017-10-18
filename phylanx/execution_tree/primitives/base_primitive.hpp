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
#include <map>
#include <string>
#include <utility>
#include <vector>

namespace phylanx { namespace execution_tree
{
    class HPX_COMPONENT_EXPORT primitive;

    ///////////////////////////////////////////////////////////////////////////
    struct primitive_result_type;

    using result_value_type =
        phylanx::util::variant<
            ast::nil
          , bool
          , std::int64_t
          , std::string
          , phylanx::ir::node_data<double>
          , std::vector<ast::expression>
          , phylanx::util::recursive_wrapper<std::vector<primitive_result_type>>
        >;

    struct primitive_result_type : result_value_type
    {
        // poor man's forwarding constructor
        template <typename ... Ts>
        primitive_result_type(Ts &&... ts)
          : result_value_type{std::forward<Ts>(ts)...}
        {}
    };

    // a result value is valid of its not nil{}
    inline bool valid(primitive_result_type const& val)
    {
        return val.index() != 0;
    }
    inline bool valid(primitive_result_type && val)
    {
        return val.index() != 0;
    }
}}

namespace phylanx { namespace execution_tree { namespace primitives
{
    ///////////////////////////////////////////////////////////////////////////
    class HPX_COMPONENT_EXPORT base_primitive
      : public hpx::traits::detail::component_tag
    {
    public:
        base_primitive() = default;
        virtual ~base_primitive() = default;

        hpx::future<primitive_result_type> eval_nonvirtual()
        {
            return eval();
        }
        virtual hpx::future<primitive_result_type> eval() const = 0;

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

    public:
        HPX_DEFINE_COMPONENT_ACTION(base_primitive, eval_nonvirtual, eval_action);
        HPX_DEFINE_COMPONENT_ACTION(base_primitive, store_nonvirtual, store_action);
    };
}}}

// Declaration of serialization support for the local_file actions
HPX_REGISTER_ACTION_DECLARATION(
    phylanx::execution_tree::primitives::base_primitive::eval_action,
    phylanx_primitive_eval_action);
HPX_REGISTER_ACTION_DECLARATION(
    phylanx::execution_tree::primitives::base_primitive::store_action,
    phylanx_primitive_store_action);

namespace phylanx { namespace execution_tree
{
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

        explicit primitive(hpx::id_type const& id)
          : base_type(id)
        {
        }
        explicit primitive(hpx::id_type && id)
          : base_type(std::move(id))
        {
        }
        primitive(hpx::future<hpx::id_type> && fid)
          : base_type(std::move(fid))
        {
        }

        hpx::future<primitive_result_type> eval() const;

        hpx::future<void> store(primitive_result_type const&);
        void store(hpx::launch::sync_policy, primitive_result_type const&);
    };

    ///////////////////////////////////////////////////////////////////////////
    struct primitive_argument_type;

    using argument_value_type =
        phylanx::util::variant<
            phylanx::ast::nil
          , bool
          , std::int64_t
          , std::string
          , phylanx::ir::node_data<double>
          , primitive
          , std::vector<ast::expression>
          , phylanx::util::recursive_wrapper<std::vector<primitive_argument_type>>
        >;

    struct primitive_argument_type : argument_value_type
    {
        // poor man's forwarding constructor
        template <typename ... Ts>
        primitive_argument_type(Ts &&... ts)
          : argument_value_type{std::forward<Ts>(ts)...}
        {}
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

    ///////////////////////////////////////////////////////////////////////////
    PHYLANX_EXPORT primitive_argument_type to_primitive_value_type(
        ast::literal_value_type && val);

    ///////////////////////////////////////////////////////////////////////////
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

    // Extract a boolean type from a given primitive_argument_type,
    // throw if it doesn't hold one.
    PHYLANX_EXPORT std::uint8_t extract_boolean_value(
        primitive_argument_type const& val);
    PHYLANX_EXPORT std::uint8_t extract_boolean_value(
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
    // could be a primitive or a literal value).
    PHYLANX_EXPORT hpx::future<primitive_result_type>
        literal_operand(primitive_argument_type const& val);

    // Extract a node_data<double> from a primitive_argument_type (that
    // could be a primitive or a literal value).
    PHYLANX_EXPORT hpx::future<ir::node_data<double>>
        numeric_operand(primitive_argument_type const& val);

    // Extract a boolean from a primitive_argument_type (that
    // could be a primitive or a literal value).
    PHYLANX_EXPORT hpx::future<std::uint8_t>
        boolean_operand(primitive_argument_type const& val);

    // Extract an AST from a primitive_argument_type (that
    // could be a primitive or a literal value).
    PHYLANX_EXPORT hpx::future<std::vector<ast::expression>>
        ast_operand(primitive_argument_type const& val);

    // Extract a list from a primitive_argument_type (that
    // could be a primitive or a literal value).
    PHYLANX_EXPORT hpx::future<std::vector<primitive_result_type>>
        list_operand(primitive_argument_type const& val);

    ///////////////////////////////////////////////////////////////////////////
    // Symbol table
    struct variables
    {
        using map_type = std::map<std::string, primitive_argument_type>;
        using value_type = typename map_type::value_type;
        using iterator = typename map_type::iterator;

        variables(variables* prev = nullptr)
          : previous_(prev)
        {}

        explicit variables(map_type const& vars)
          : variables_(vars)
          , previous_(nullptr)
        {}
        explicit variables(map_type && vars)
          : variables_(std::move(vars))
          , previous_(nullptr)
        {}
        variables(std::initializer_list<value_type> ilist)
          : variables_(ilist)
          , previous_(nullptr)
        {}

        std::size_t size() const { return variables_.size(); }

        std::pair<iterator, iterator> find(std::string const& name)
        {
            iterator it = variables_.find(name);
            if (it == variables_.end())
            {
                if (previous_ != nullptr)
                {
                    return previous_->find(name);
                }
            }
            return std::make_pair(it, variables_.end());
        }

        std::pair<iterator, bool> insert(value_type const& val)
        {
            return variables_.insert(val);
        }
        std::pair<iterator, bool> insert(value_type && val)
        {
            return variables_.insert(std::move(val));
        }

    private:
        map_type variables_;
        variables* previous_;
    };

    ///////////////////////////////////////////////////////////////////////////
    struct functions;

    // Factory functions
    using factory_function_type =
        primitive(*)(
            hpx::id_type, std::vector<primitive_argument_type>&&,
            variables&, functions&
        );

    // Function description
    struct function_description
    {
        explicit function_description(std::vector<ast::expression> const& ast,
                factory_function_type factory = nullptr)
          : ast_(ast)
          , factory_(factory)
        {}

        explicit function_description(std::vector<ast::expression> && ast,
                factory_function_type factory = nullptr)
          : ast_(std::move(ast))
          , factory_(factory)
        {}

        std::vector<ast::expression> const& ast() const { return ast_; }
        factory_function_type const& factory() const { return factory_; }

    private:
        std::vector<ast::expression> ast_;
        factory_function_type factory_;
    };

    // Function definition table
    struct functions
    {
        using map_type = std::map<std::string, function_description>;
        using value_type = typename map_type::value_type;
        using iterator = typename map_type::iterator;

        functions(functions* prev = nullptr)
          : previous_(prev)
        {}

        explicit functions(map_type const& vars)
          : functions_(vars)
          , previous_(nullptr)
        {}
        explicit functions(map_type && vars)
          : functions_(std::move(vars))
          , previous_(nullptr)
        {}
        functions(std::initializer_list<value_type> ilist)
          : functions_(ilist)
          , previous_(nullptr)
        {}

        std::size_t size() const { return functions_.size(); }

        std::pair<iterator, iterator> find(std::string const& name)
        {
            iterator it = functions_.find(name);
            if (it == functions_.end())
            {
                if (previous_ != nullptr)
                {
                    return previous_->find(name);
                }
            }
            return std::make_pair(it, functions_.end());
        }

        std::pair<iterator, bool> insert(value_type const& val)
        {
            return functions_.insert(val);
        }
        std::pair<iterator, bool> insert(value_type && val)
        {
            return functions_.insert(std::move(val));
        }

    private:
        map_type functions_;
        functions* previous_;
    };

    template <typename Iterator>
    bool is_empty_range(std::pair<Iterator, Iterator> const& p)
    {
        return p.first == p.second;
    }

    ///////////////////////////////////////////////////////////////////////////
    // Generic creation helper for creating an instance of the given primitive.
    template <typename Primitive>
    primitive create(hpx::id_type locality,
        std::vector<primitive_argument_type>&& operands, variables&, functions&)
    {
        return primitive(hpx::new_<Primitive>(locality, std::move(operands)));
    }

    ///////////////////////////////////////////////////////////////////////////
    using match_pattern_type =
        hpx::util::tuple<std::string, std::string, factory_function_type>;
    using pattern_list = std::vector<std::vector<match_pattern_type>>;
}}

namespace phylanx { namespace execution_tree { namespace primitives
{
    namespace detail
    {
        // Invoke the given function on all items in the input vector, while
        // returning another vector holding the respective results.
        template <typename T, typename F>
        auto map_operands(std::vector<T> const& in, F && f)
        ->  std::vector<decltype(hpx::util::invoke(f, std::declval<T>()))>
        {
            std::vector<decltype(hpx::util::invoke(f, std::declval<T>()))> out;
            out.reserve(in.size());

            for (auto const& d : in)
            {
                out.push_back(hpx::util::invoke(f, d));
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


