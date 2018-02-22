//  Copyright (c) 2017-2018 Hartmut Kaiser
//  Copyright (c) 2018 Parsa Amini
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_PRIMITIVES_BASE_PRIMITIVE_SEP_05_2017_1102AM)
#define PHYLANX_PRIMITIVES_BASE_PRIMITIVE_SEP_05_2017_1102AM

#include <phylanx/config.hpp>
#include <phylanx/ast/node.hpp>
#include <phylanx/ir/node_data.hpp>

#include <hpx/include/util.hpp>
#include <hpx/include/serialization.hpp>

#include <cstdint>
#include <initializer_list>
#include <iosfwd>
#include <map>
#include <set>
#include <string>
#include <utility>
#include <vector>

namespace phylanx { namespace execution_tree
{
    ///////////////////////////////////////////////////////////////////////////
    namespace primitives
    {
        class primitive_component;
    }

    class primitive;

    ///////////////////////////////////////////////////////////////////////////
    struct topology
    {
        topology() = default;

        topology(std::string name)
          : name_(std::move(name))
        {}

        topology(std::vector<topology> names)
          : children_(std::move(names))
        {}

        topology(std::vector<topology> names, std::string name)
          : children_(std::move(names)), name_(std::move(name))
        {}

        template <typename Archive>
        void serialize(Archive & ar, unsigned)
        {
            ar & children_ & name_;
        }

        std::vector<topology> children_;
        std::string name_;
    };

    ///////////////////////////////////////////////////////////////////////////
    // Generate Newick tree (string) format from given tree topology
    PHYLANX_EXPORT std::string newick_tree(
        std::string const& name, topology const& t);

    // Generate Dot tree (string) format from given tree topology
    PHYLANX_EXPORT std::string dot_tree(
        std::string const& name, topology const& t);

    ///////////////////////////////////////////////////////////////////////////
    struct primitive_argument_type;

    ///////////////////////////////////////////////////////////////////////////
    class primitive
      : public hpx::components::client_base<primitive,
            primitives::primitive_component>
    {
    private:
        using base_type = hpx::components::client_base<primitive,
            primitives::primitive_component>;

    public:
        primitive() = default;

        primitive(hpx::future<hpx::id_type> && fid)
          : base_type(std::move(fid))
        {
        }

        PHYLANX_EXPORT primitive(
            hpx::future<hpx::id_type>&& fid, std::string const& name);

        primitive(primitive const&) = default;
        primitive(primitive &&) = default;

        primitive& operator=(primitive const&) = default;
        primitive& operator=(primitive &&) = default;

        PHYLANX_EXPORT hpx::future<primitive_argument_type> eval() const;
        PHYLANX_EXPORT hpx::future<primitive_argument_type> eval(
            std::vector<primitive_argument_type> && args) const;
        PHYLANX_EXPORT hpx::future<primitive_argument_type> eval(
            std::vector<primitive_argument_type> const& args) const;

        PHYLANX_EXPORT primitive_argument_type eval_direct() const;
        PHYLANX_EXPORT primitive_argument_type eval_direct(
            std::vector<primitive_argument_type> && args) const;
        PHYLANX_EXPORT primitive_argument_type eval_direct(
            std::vector<primitive_argument_type> const& args) const;

        PHYLANX_EXPORT hpx::future<void> store(primitive_argument_type);
        PHYLANX_EXPORT void store(
            hpx::launch::sync_policy, primitive_argument_type);

        PHYLANX_EXPORT hpx::future<topology> expression_topology(
            std::set<std::string>&& functions) const;
        PHYLANX_EXPORT topology expression_topology(hpx::launch::sync_policy,
            std::set<std::string>&& functions) const;

        PHYLANX_EXPORT void set_body(
            hpx::launch::sync_policy, primitive_argument_type&& target);

    public:
        static bool enable_tracing;
    };

    ///////////////////////////////////////////////////////////////////////////
    using argument_value_type =
        phylanx::util::variant<
            ast::nil
          , phylanx::ir::node_data<bool>
          , std::int64_t
          , std::string
          , phylanx::ir::node_data<double>
          , primitive
          , std::vector<ast::expression>
          , phylanx::util::recursive_wrapper<std::vector<primitive_argument_type>>
        >;

    struct primitive_argument_type : argument_value_type
    {
        primitive_argument_type() = default;

        primitive_argument_type(ast::nil val)
          : argument_value_type{val}
        {}

        explicit primitive_argument_type(bool val)
          : argument_value_type{phylanx::ir::node_data<bool>{val}}
        {}

        primitive_argument_type(phylanx::ir::node_data<bool> const& val)
          : argument_value_type{val}
        {}
        primitive_argument_type(phylanx::ir::node_data<bool>&& val)
          : argument_value_type{std::move(val)}
        {}

        primitive_argument_type(std::int64_t val)
          : argument_value_type{val}
        {}

        primitive_argument_type(std::string const& val)
          : argument_value_type{val}
        {}
        primitive_argument_type(std::string && val)
          : argument_value_type{std::move(val)}
        {}

        explicit primitive_argument_type(double val)
          : argument_value_type{phylanx::ir::node_data<double>{val}}
        {}
        explicit primitive_argument_type(
                blaze::DynamicVector<double> const& val)
          : argument_value_type{phylanx::ir::node_data<double>{val}}
        {}
        explicit primitive_argument_type(blaze::DynamicVector<double>&& val)
          : argument_value_type{phylanx::ir::node_data<double>{std::move(val)}}
        {}
        explicit primitive_argument_type(
                blaze::DynamicMatrix<double> const& val)
          : argument_value_type{phylanx::ir::node_data<double>{val}}
        {}
        explicit primitive_argument_type(blaze::DynamicMatrix<double>&& val)
          : argument_value_type{phylanx::ir::node_data<double>{std::move(val)}}
        {}

        primitive_argument_type(phylanx::ir::node_data<double> const& val)
          : argument_value_type{val}
        {}
        primitive_argument_type(phylanx::ir::node_data<double>&& val)
          : argument_value_type{std::move(val)}
        {}

        primitive_argument_type(primitive const& val)
          : argument_value_type{val}
        {}
        primitive_argument_type(primitive && val)
          : argument_value_type{std::move(val)}
        {}

        primitive_argument_type(std::vector<ast::expression> const& val)
          : argument_value_type{val}
        {}
        primitive_argument_type(std::vector<ast::expression>&& val)
          : argument_value_type{std::move(val)}
        {}

        primitive_argument_type(std::vector<primitive_argument_type> const& val)
          : argument_value_type{phylanx::util::recursive_wrapper<
                std::vector<primitive_argument_type>>{val}}
        {}
        primitive_argument_type(std::vector<primitive_argument_type>&& val)
          : argument_value_type{phylanx::util::recursive_wrapper<
                std::vector<primitive_argument_type>>{std::move(val)}}
        {}

        inline primitive_argument_type operator()() const;

        inline primitive_argument_type
        operator()(std::vector<primitive_argument_type> const& args) const;

        explicit operator bool() const
        {
            return variant().index() != 0;
        }

        // workaround for problem in implementation of MSVC14.12
        // variant::visit
        argument_value_type& variant() { return *this; }
        argument_value_type const& variant() const { return *this; }
    };

    // a argument is valid of its not nil{}
    inline bool valid(primitive_argument_type const& val)
    {
        return bool(val);
    }
    inline bool valid(primitive_argument_type && val)
    {
        return bool(val);
    }

    inline bool operator==(primitive_argument_type const& lhs,
        primitive_argument_type const& rhs)
    {
        return lhs.variant() == rhs.variant();
    }
    inline bool operator!=(primitive_argument_type const& lhs,
        primitive_argument_type const& rhs)
    {
        return !(lhs == rhs);
    }

    PHYLANX_EXPORT std::ostream& operator<<(std::ostream& os,
        primitive_argument_type const&);

    PHYLANX_EXPORT std::ostream& operator<<(std::ostream& os,
        primitive const&);

    ///////////////////////////////////////////////////////////////////////////
    PHYLANX_EXPORT primitive_argument_type value_operand_sync(
        primitive_argument_type const& val,
        std::vector<primitive_argument_type> const& args,
        std::string const& name = "",
        std::string const& codename = "<unknown>");
    PHYLANX_EXPORT primitive_argument_type value_operand_sync(
        primitive_argument_type const& val,
        std::vector<primitive_argument_type> && args,
        std::string const& name = "",
        std::string const& codename = "<unknown>");
    PHYLANX_EXPORT primitive_argument_type value_operand_sync(
        primitive_argument_type && val,
        std::vector<primitive_argument_type> const& args,
        std::string const& name = "",
        std::string const& codename = "<unknown>");
    PHYLANX_EXPORT primitive_argument_type value_operand_sync(
        primitive_argument_type && val,
        std::vector<primitive_argument_type> && args,
        std::string const& name = "",
        std::string const& codename = "<unknown>");

    namespace functional
    {
        struct value_operand_sync
        {
            template <typename... Ts>
            primitive_argument_type operator()(Ts&&... ts) const
            {
                return execution_tree::value_operand_sync(
                    std::forward<Ts>(ts)...);
            }
        };
    }

    PHYLANX_EXPORT primitive_argument_type value_operand_ref_sync(
        primitive_argument_type const& val,
        std::vector<primitive_argument_type> const& args,
        std::string const& name = "",
        std::string const& codename = "<unknown>");

    ///////////////////////////////////////////////////////////////////////////
    inline primitive_argument_type primitive_argument_type::operator()() const
    {
        return value_operand_sync(*this, {});
    }

    inline primitive_argument_type primitive_argument_type::operator()(
        std::vector<primitive_argument_type> const& args) const
    {
        return value_operand_sync(*this, args);
    }

    ///////////////////////////////////////////////////////////////////////////
    PHYLANX_EXPORT primitive_argument_type to_primitive_value_type(
        ast::literal_value_type && val);
    PHYLANX_EXPORT ir::node_data<double> to_primitive_numeric_type(
        ast::literal_value_type && val);
    PHYLANX_EXPORT std::string to_primitive_string_type(
        ast::literal_value_type && val);
    PHYLANX_EXPORT std::int64_t to_primitive_int_type(
        ast::literal_value_type && val);
    PHYLANX_EXPORT bool to_primitive_bool_type(
        ast::literal_value_type && val);

    ///////////////////////////////////////////////////////////////////////////
    // Extract a value type from a given primitive_argument_type, throw
    // if it doesn't hold one.
    PHYLANX_EXPORT primitive_argument_type extract_value(
        primitive_argument_type const& val,
        std::string const& name = "",
        std::string const& codename = "<unknown>");
    PHYLANX_EXPORT primitive_argument_type extract_ref_value(
        primitive_argument_type const& val,
        std::string const& name = "",
        std::string const& codename = "<unknown>");
    PHYLANX_EXPORT primitive_argument_type extract_copy_value(
        primitive_argument_type const& val,
        std::string const& name = "",
        std::string const& codename = "<unknown>");
    PHYLANX_EXPORT primitive_argument_type extract_value(
        primitive_argument_type && val,
        std::string const& name = "",
        std::string const& codename = "<unknown>");
    PHYLANX_EXPORT primitive_argument_type extract_ref_value(
        primitive_argument_type && val,
        std::string const& name = "",
        std::string const& codename = "<unknown>");
    PHYLANX_EXPORT primitive_argument_type extract_copy_value(
        primitive_argument_type && val,
        std::string const& name = "",
        std::string const& codename = "<unknown>");

    template <typename T>
    primitive_argument_type extract_ref_value(ir::node_data<T> const& val)
    {
        if (val.is_ref())
        {
            return primitive_argument_type{val};
        }
        return primitive_argument_type{val.ref()};
    }
    template <typename T>
    primitive_argument_type extract_ref_value(ir::node_data<T> && val)
    {
        return primitive_argument_type{std::move(val)};
    }

    template <typename T>
    primitive_argument_type extract_copy_value(ir::node_data<T> const& val)
    {
        if (val.is_ref())
        {
            return primitive_argument_type{val.copy()};
        }
        return primitive_argument_type{val};
    }
    template <typename T>
    primitive_argument_type extract_copy_value(ir::node_data<T> && val)
    {
        if (val.is_ref())
        {
            return primitive_argument_type{val.copy()};
        }
        return primitive_argument_type{std::move(val)};
    }

    // Extract a literal type from a given primitive_argument_type, throw
    // if it doesn't hold one.
    PHYLANX_EXPORT primitive_argument_type extract_literal_value(
        primitive_argument_type const& val,
        std::string const& name = "",
        std::string const& codename = "<unknown>");
    PHYLANX_EXPORT primitive_argument_type extract_literal_ref_value(
        primitive_argument_type const& val,
        std::string const& name = "",
        std::string const& codename = "<unknown>");
    PHYLANX_EXPORT primitive_argument_type extract_literal_value(
        primitive_argument_type && val,
        std::string const& name = "",
        std::string const& codename = "<unknown>");

    // Extract a ir::node_data<double> type from a given primitive_argument_type,
    // throw if it doesn't hold one.
    PHYLANX_EXPORT ir::node_data<double> extract_numeric_value(
        primitive_argument_type const& val,
        std::string const& name = "",
        std::string const& codename = "<unknown>");
    PHYLANX_EXPORT ir::node_data<double> extract_numeric_value(
        primitive_argument_type && val,
        std::string const& name = "",
        std::string const& codename = "<unknown>");

    // Extract a ir::node_data<bool> type from a given primitive_argument_type,
    // throw if it doesn't hold one.
    PHYLANX_EXPORT ir::node_data<bool> extract_boolean_data(
        primitive_argument_type const& val,
        std::string const& name = "",
        std::string const& codename = "<unknown>");
    PHYLANX_EXPORT ir::node_data<bool> extract_boolean_data(
        primitive_argument_type&& val,
        std::string const& name = "",
        std::string const& codename = "<unknown>");

    // Extract a std::int64_t type from a given primitive_argument_type,
    // throw if it doesn't hold one.
    PHYLANX_EXPORT std::int64_t extract_integer_value(
        primitive_argument_type const& val,
        std::string const& name = "",
        std::string const& codename = "<unknown>");
    PHYLANX_EXPORT std::int64_t extract_integer_value(
        primitive_argument_type && val,
        std::string const& name = "",
        std::string const& codename = "<unknown>");

    // Extract a boolean type from a given primitive_argument_type,
    // throw if it doesn't hold one.
    PHYLANX_EXPORT std::uint8_t extract_boolean_value(
        primitive_argument_type const& val,
        std::string const& name = "",
        std::string const& codename = "<unknown>");
    PHYLANX_EXPORT std::uint8_t extract_boolean_value(
        primitive_argument_type && val,
        std::string const& name = "",
        std::string const& codename = "<unknown>");

    // Extract a std::string type from a given primitive_argument_type,
    // throw if it doesn't hold one.
    PHYLANX_EXPORT std::string extract_string_value(
        primitive_argument_type const& val,
        std::string const& name = "",
        std::string const& codename = "<unknown>");
    PHYLANX_EXPORT std::string extract_string_value(
        primitive_argument_type && val,
        std::string const& name = "",
        std::string const& codename = "<unknown>");

    // Extract an AST type from a given primitive_argument_type,
    // throw if it doesn't hold one.
    PHYLANX_EXPORT std::vector<ast::expression> extract_ast_value(
        primitive_argument_type const& val,
        std::string const& name = "",
        std::string const& codename = "<unknown>");
    PHYLANX_EXPORT std::vector<ast::expression> extract_ast_value(
        primitive_argument_type && val,
        std::string const& name = "",
        std::string const& codename = "<unknown>");

    // Extract a list type from a given primitive_argument_type,
    // throw if it doesn't hold one.
    PHYLANX_EXPORT std::vector<primitive_argument_type> extract_list_value(
        primitive_argument_type const& val,
        std::string const& name = "",
        std::string const& codename = "<unknown>");
    PHYLANX_EXPORT std::vector<primitive_argument_type> extract_list_value(
        primitive_argument_type && val,
        std::string const& name = "",
        std::string const& codename = "<unknown>");

    ///////////////////////////////////////////////////////////////////////////
    // Extract a primitive from a given primitive_argument_type, throw
    // if it doesn't hold one.
    PHYLANX_EXPORT primitive primitive_operand(
        primitive_argument_type const& val,
        std::string const& name = "",
        std::string const& codename = "<unknown>");
    PHYLANX_EXPORT bool is_primitive_operand(
        primitive_argument_type const& val);

    // Extract a primitive_argument_type from a primitive_argument_type (that
    // could be a value type).
    PHYLANX_EXPORT hpx::future<primitive_argument_type> value_operand(
        primitive_argument_type const& val,
        std::vector<primitive_argument_type> const& args,
        std::string const& name = "",
        std::string const& codename = "<unknown>");
    PHYLANX_EXPORT hpx::future<primitive_argument_type> value_operand(
        primitive_argument_type const& val,
        std::vector<primitive_argument_type> && args,
        std::string const& name = "",
        std::string const& codename = "<unknown>");
    PHYLANX_EXPORT hpx::future<primitive_argument_type> value_operand(
        primitive_argument_type && val,
        std::vector<primitive_argument_type> const& args,
        std::string const& name = "",
        std::string const& codename = "<unknown>");
    PHYLANX_EXPORT hpx::future<primitive_argument_type> value_operand(
        primitive_argument_type && val,
        std::vector<primitive_argument_type> && args,
        std::string const& name = "",
        std::string const& codename = "<unknown>");

    namespace functional
    {
        struct value_operand
        {
            template <typename... Ts>
            hpx::future<primitive_argument_type> operator()(Ts&&... ts) const
            {
                return execution_tree::value_operand(std::forward<Ts>(ts)...);
            }
        };
    }

// was declared above
//     PHYLANX_EXPORT primitive_argument_type value_operand_sync(
//         primitive_argument_type const& val,
//         std::vector<primitive_argument_type> const& args);

    // Extract a primitive_argument_type from a primitive_argument_type (that
    // could be a primitive or a literal value).
    PHYLANX_EXPORT hpx::future<primitive_argument_type> literal_operand(
        primitive_argument_type const& val,
        std::vector<primitive_argument_type> const& args,
        std::string const& name = "",
        std::string const& codename = "<unknown>");
    PHYLANX_EXPORT hpx::future<primitive_argument_type> literal_operand(
        primitive_argument_type const& val,
        std::vector<primitive_argument_type> && args,
        std::string const& name = "",
        std::string const& codename = "<unknown>");
    PHYLANX_EXPORT hpx::future<primitive_argument_type> literal_operand(
        primitive_argument_type && val,
        std::vector<primitive_argument_type> const& args,
        std::string const& name = "",
        std::string const& codename = "<unknown>");
    PHYLANX_EXPORT hpx::future<primitive_argument_type> literal_operand(
        primitive_argument_type && val,
        std::vector<primitive_argument_type> && args,
        std::string const& name = "",
        std::string const& codename = "<unknown>");

    namespace functional
    {
        struct literal_operand
        {
            template <typename... Ts>
            hpx::future<primitive_argument_type> operator()(Ts&&... ts) const
            {
                return execution_tree::literal_operand(std::forward<Ts>(ts)...);
            }
        };
    }

    PHYLANX_EXPORT primitive_argument_type literal_operand_sync(
        primitive_argument_type const& val,
        std::vector<primitive_argument_type> const& args,
        std::string const& name = "",
        std::string const& codename = "<unknown>");

    // Extract a node_data<double> from a primitive_argument_type (that
    // could be a primitive or a literal value).
    PHYLANX_EXPORT hpx::future<ir::node_data<double>> numeric_operand(
        primitive_argument_type const& val,
        std::vector<primitive_argument_type> const& args,
        std::string const& name = "",
        std::string const& codename = "<unknown>");
    PHYLANX_EXPORT hpx::future<ir::node_data<double>> numeric_operand(
        primitive_argument_type const& val,
        std::vector<primitive_argument_type> && args,
        std::string const& name = "",
        std::string const& codename = "<unknown>");
    PHYLANX_EXPORT hpx::future<ir::node_data<double>> numeric_operand(
        primitive_argument_type && val,
        std::vector<primitive_argument_type> const& args,
        std::string const& name = "",
        std::string const& codename = "<unknown>");
    PHYLANX_EXPORT hpx::future<ir::node_data<double>> numeric_operand(
        primitive_argument_type && val,
        std::vector<primitive_argument_type> && args,
        std::string const& name = "",
        std::string const& codename = "<unknown>");

    namespace functional
    {
        struct numeric_operand
        {
            template <typename... Ts>
            hpx::future<ir::node_data<double>> operator()(Ts&&... ts) const
            {
                return execution_tree::numeric_operand(std::forward<Ts>(ts)...);
            }
        };
    }

    PHYLANX_EXPORT ir::node_data<double> numeric_operand_sync(
        primitive_argument_type const& val,
        std::vector<primitive_argument_type> const& args,
        std::string const& name = "",
        std::string const& codename = "<unknown>");

    // Extract a boolean from a primitive_argument_type (that
    // could be a primitive or a literal value).
    PHYLANX_EXPORT hpx::future<std::uint8_t> boolean_operand(
        primitive_argument_type const& val,
        std::vector<primitive_argument_type> const& args,
        std::string const& name = "",
        std::string const& codename = "<unknown>");
    PHYLANX_EXPORT std::uint8_t boolean_operand_sync(
        primitive_argument_type const& val,
        std::vector<primitive_argument_type> const& args,
        std::string const& name = "",
        std::string const& codename = "<unknown>");

    // Extract a std::string from a primitive_argument_type (that
    // could be a primitive or a string value).
    PHYLANX_EXPORT hpx::future<std::string> string_operand(
        primitive_argument_type const& val,
        std::vector<primitive_argument_type> const& args,
        std::string const& name = "",
        std::string const& codename = "<unknown>");
    PHYLANX_EXPORT std::string string_operand_sync(
        primitive_argument_type const& val,
        std::vector<primitive_argument_type> const& args,
        std::string const& name = "",
        std::string const& codename = "<unknown>");

    // Extract an AST from a primitive_argument_type (that
    // could be a primitive or a literal value).
    PHYLANX_EXPORT hpx::future<std::vector<ast::expression>> ast_operand(
        primitive_argument_type const& val,
        std::vector<primitive_argument_type> const& args,
        std::string const& name = "",
        std::string const& codename = "<unknown>");
    PHYLANX_EXPORT std::vector<ast::expression> ast_operand_sync(
        primitive_argument_type const& val,
        std::vector<primitive_argument_type> const& args,
        std::string const& name = "",
        std::string const& codename = "<unknown>");

    // Extract a list from a primitive_argument_type (that
    // could be a primitive or a literal value).
    PHYLANX_EXPORT hpx::future<std::vector<primitive_argument_type>> list_operand(
        primitive_argument_type const& val,
        std::vector<primitive_argument_type> const& args,
        std::string const& name = "",
        std::string const& codename = "<unknown>");
    PHYLANX_EXPORT std::vector<primitive_argument_type> list_operand_sync(
        primitive_argument_type const& val,
        std::vector<primitive_argument_type> const& args,
        std::string const& name = "",
        std::string const& codename = "<unknown>");
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

        template <typename T, typename F, typename ... Ts>
        auto map_operands(std::vector<T> && in, F && f, Ts && ... ts)
        ->  std::vector<decltype(hpx::util::invoke(f, std::declval<T>(), ts...))>
        {
            std::vector<
                    decltype(hpx::util::invoke(f, std::declval<T>(), ts...))
                > out;
            out.reserve(in.size());

            for (auto && d : in)
            {
                out.push_back(hpx::util::invoke(f, std::move(d), ts...));
            }
            return out;
        }

        ///////////////////////////////////////////////////////////////////////
        // check if one of the optionals in the list of operands is empty
        inline bool verify_argument_values(
            std::vector<primitive_argument_type> const& ops)
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

#include <phylanx/execution_tree/primitives/primitive_component_base.hpp>

#endif


