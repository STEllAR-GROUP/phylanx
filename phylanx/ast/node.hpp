//   Copyright (c) 2001-2011 Joel de Guzman
//   Copyright (c) 2001-2018 Hartmut Kaiser
//
//   Distributed under the Boost Software License, Version 1.0. (See accompanying
//   file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_AST_NODE_HPP)
#define PHYLANX_AST_NODE_HPP

#include <phylanx/config.hpp>
#include <phylanx/ast/parser/extended_variant.hpp>
#include <phylanx/ir/node_data.hpp>
#include <phylanx/util/serialization/variant.hpp>

#include <hpx/serialization/serialization_fwd.hpp>

#include <boost/spirit/include/support_extended_variant.hpp>
#include <boost/spirit/include/support_attributes.hpp>

#include <cstddef>
#include <cstdint>
#include <iosfwd>
#include <string>
#include <utility>
#include <vector>

namespace phylanx { namespace ast
{
    ///////////////////////////////////////////////////////////////////////////
    struct identifier;
    struct primary_expr;
    struct operand;
    struct unary_expr;
    struct operation;
    struct expression;
    struct function_call;

    ///////////////////////////////////////////////////////////////////////////
    //  The AST
    struct tagged
    {
        tagged()
          : id(--next_id)
          , col(-1)
        {}

        tagged(int tag)
          : id(tag)
          , col(-1)
        {
        }

        tagged(std::int64_t line, std::int64_t column)
          : id(line)
          , col(column)
        {
        }

        std::int64_t id;    // Used to annotate the AST with the iterator position.
                            // This id is used as a key to a map<int, Iterator>
                            // (not really part of the AST.)
                            // if 'col' != -1, then id represents the line number
        std::int64_t col;   // if != -1, represents the column_offset

        // default-initialized tags are negative
        PHYLANX_EXPORT static std::int64_t next_id;
    };

    inline bool operator==(tagged const& lhs, tagged const& rhs)
    {
        // if one of the col values is == -1 we consider the instances to be equal
        if (lhs.col == -1 || rhs.col == -1)
        {
            return true;
        }
        return lhs.id == rhs.id && lhs.col == rhs.col;
    }
    inline bool operator!=(tagged const& lhs, tagged const& rhs)
    {
        return !(lhs == rhs);
    }

    ///////////////////////////////////////////////////////////////////////////
    enum class optoken
    {
        // precedence 1
        op_comma,

        // precedence 2
        op_assign,
        op_plus_assign,
        op_minus_assign,
        op_times_assign,
        op_divide_assign,
        op_mod_assign,
        op_bit_and_assign,
        op_bit_xor_assign,
        op_bitor_assign,
        op_shift_left_assign,
        op_shift_right_assign,

        // precedence 3
        op_logical_or,

        // precedence 4
        op_logical_and,

        // precedence 5
        op_bit_or,

        // precedence 6
        op_bit_xor,

        // precedence 7
        op_bit_and,

        // precedence 8
        op_equal,
        op_not_equal,

        // precedence 9
        op_less,
        op_less_equal,
        op_greater,
        op_greater_equal,

        // precedence 10
        op_shift_left,
        op_shift_right,

        // precedence 11
        op_plus,
        op_minus,

        // precedence 12
        op_times,
        op_divide,
        op_mod,

        // precedence 13
        op_positive,
        op_negative,
        op_pre_incr,
        op_pre_decr,
        op_compl,
        op_not,

        // precedence 14
        op_post_incr,
        op_post_decr,

        op_unknown
    };

    PHYLANX_EXPORT int precedence_of(optoken);

    ///////////////////////////////////////////////////////////////////////////
    struct nil
    {
        bool explicit_nil = false;

        // nil is always evaluated to false, if needed
        explicit operator bool() const
        {
            return false;
        }
    };

    PHYLANX_EXPORT void serialize(
        hpx::serialization::output_archive& ar, nil const&, unsigned);
    PHYLANX_EXPORT void serialize(
        hpx::serialization::input_archive& ar, nil&, unsigned);

    constexpr inline bool operator==(nil const&, nil const&)
    {
        return true;
    }
    constexpr inline bool operator!=(nil const&, nil const&)
    {
        return false;
    }
    constexpr inline bool operator>(nil const&, nil const&)
    {
        return false;
    }
    constexpr inline bool operator<(nil const&, nil const&)
    {
        return false;
    }
    constexpr inline bool operator>=(nil const&, nil const&)
    {
        return true;
    }
    constexpr inline bool operator<=(nil const&, nil const&)
    {
        return true;
    }

    ///////////////////////////////////////////////////////////////////////////
    struct identifier : tagged
    {
        identifier() = default;

        explicit identifier(std::string const& name)
          : name(name)
        {
        }
        explicit identifier(std::string && name)
          : name(std::move(name))
        {
        }

        identifier(
            std::string const& name, std::int64_t line, std::int64_t column)
          : tagged(line, column)
          , name(name)
        {
        }
        identifier(std::string&& name, std::int64_t line, std::int64_t column)
          : tagged(line, column)
          , name(std::move(name))
        {
        }

        std::string name;

    private:
        friend class hpx::serialization::access;

        PHYLANX_EXPORT void serialize(
            hpx::serialization::input_archive& ar, unsigned);
        PHYLANX_EXPORT void serialize(
            hpx::serialization::output_archive& ar, unsigned);
    };

    inline bool operator==(identifier const& lhs, identifier const& rhs)
    {
        return lhs.name == rhs.name;
    }
    inline bool operator!=(identifier const& lhs, identifier const& rhs)
    {
        return !(lhs == rhs);
    }

    ///////////////////////////////////////////////////////////////////////////
    using expr_node_type = phylanx::ast::parser::extended_variant<
            nil
          , bool
          , phylanx::ir::node_data<double>
          , identifier
          , std::string
          , phylanx::ir::node_data<std::int64_t>
          , phylanx::util::recursive_wrapper<expression>
          , phylanx::util::recursive_wrapper<function_call>
          , phylanx::util::recursive_wrapper<std::vector<ast::expression>>
          , phylanx::ir::node_data<std::uint8_t>
        >;

    struct primary_expr : expr_node_type, tagged
    {
        primary_expr() = default;

        primary_expr(nil val)
          : expr_node_type(val)
        {
        }

        primary_expr(bool val)
          : expr_node_type(val)
        {
        }

        primary_expr(double val)
          : expr_node_type(phylanx::ir::node_data<double>(val))
        {
        }

        primary_expr(std::uint64_t val)
          : expr_node_type(
                phylanx::ir::node_data<std::int64_t>(std::int64_t(val)))
        {
        }
        primary_expr(std::int64_t val)
          : expr_node_type(phylanx::ir::node_data<std::int64_t>(val))
        {
        }

        primary_expr(char const* val)
          : expr_node_type(std::string(val))
        {
        }
        primary_expr(std::string const& val)
          : expr_node_type(val)
        {
        }
        primary_expr(std::string && val)
          : expr_node_type(std::move(val))
        {
        }

        template <typename T>
        primary_expr(phylanx::ir::node_data<T> const& val)
          : expr_node_type(val)
        {
        }
        template <typename T>
        primary_expr(phylanx::ir::node_data<T> && val)
          : expr_node_type(std::move(val))
        {
        }

        template <typename T>
        primary_expr(std::vector<T> const& val)
          : expr_node_type(phylanx::ir::node_data<T>{val})
        {
        }
        template <typename T>
        primary_expr(std::vector<T> && val)
          : expr_node_type(phylanx::ir::node_data<T>{std::move(val)})
        {
        }

        template <typename T>
        primary_expr(std::vector<std::vector<T>> const& val)
          : expr_node_type(phylanx::ir::node_data<T>{val})
        {
        }
        template <typename T>
        primary_expr(std::vector<std::vector<T>> && val)
          : expr_node_type(phylanx::ir::node_data<T>{std::move(val)})
        {
        }

        template <typename T>
        primary_expr(std::vector<std::vector<std::vector<T>>> const& val)
          : expr_node_type(phylanx::ir::node_data<T>{val})
        {
        }
        template <typename T>
        primary_expr(std::vector<std::vector<std::vector<T>>> && val)
          : expr_node_type(phylanx::ir::node_data<T>{std::move(val)})
        {
        }

        template <typename T>
        primary_expr(
            std::vector<std::vector<std::vector<std::vector<T>>>> const& val)
          : expr_node_type(phylanx::ir::node_data<T>{val})
        {
        }
        template <typename T>
        primary_expr(
            std::vector<std::vector<std::vector<std::vector<T>>>>&& val)
          : expr_node_type(phylanx::ir::node_data<T>{std::move(val)})
        {
        }

        primary_expr(identifier const& val)
          : expr_node_type(val)
        {
        }
        primary_expr(identifier && val)
          : expr_node_type(std::move(val))
        {
        }

        primary_expr(expression const& val)
          : expr_node_type(val)
        {
        }
        primary_expr(expression && val)
          : expr_node_type(std::move(val))
        {
        }

        primary_expr(function_call const& fc)
          : expr_node_type(fc)
        {
        }
        primary_expr(function_call && fc)
          : expr_node_type(std::move(fc))
        {
        }

        primary_expr(std::vector<ast::expression> const& l)
          : expr_node_type(l)
        {
        }
        primary_expr(std::vector<ast::expression> && l)
          : expr_node_type(std::move(l))
        {
        }

    private:
        friend class hpx::serialization::access;

        PHYLANX_EXPORT void serialize(
            hpx::serialization::input_archive& ar, unsigned);
        PHYLANX_EXPORT void serialize(
            hpx::serialization::output_archive& ar, unsigned);
    };

    inline bool operator==(primary_expr const& lhs, primary_expr const& rhs)
    {
        return static_cast<expr_node_type const&>(lhs) ==
                static_cast<expr_node_type const&>(rhs);
    }
    inline bool operator!=(primary_expr const& lhs, primary_expr const& rhs)
    {
        return !(lhs == rhs);
    }

    ///////////////////////////////////////////////////////////////////////////
    using operand_node_type = phylanx::ast::parser::extended_variant<
            nil
          , phylanx::util::recursive_wrapper<primary_expr>
          , phylanx::util::recursive_wrapper<unary_expr>
        >;

    struct operand : operand_node_type
    {
        operand() = default;

        operand(bool val)
          : operand_node_type(primary_expr(val))
        {
        }
        operand(double val)
          : operand_node_type(primary_expr(val))
        {
        }
        template <typename T>
        operand(ir::node_data<T> const& val)
          : operand_node_type(primary_expr(val))
        {
        }
        template <typename T>
        operand(ir::node_data<T> && val)
          : operand_node_type(primary_expr(std::move(val)))
        {
        }

        operand(char const* val)
          : operand_node_type(primary_expr(val))
        {
        }
        operand(std::string const& val)
          : operand_node_type(primary_expr(val))
        {
        }
        operand(std::string && val)
          : operand_node_type(primary_expr(std::move(val)))
        {
        }
        operand(std::int64_t val)
          : operand_node_type(primary_expr(val))
        {
        }
        operand(std::uint64_t val)
          : operand_node_type(primary_expr(val))
        {
        }

        explicit operand(identifier const& val)
          : operand_node_type(primary_expr(val))
        {
        }
        explicit operand(identifier && val)
          : operand_node_type(primary_expr(std::move(val)))
        {
        }

        explicit operand(primary_expr const& val)
          : operand_node_type(val)
        {
        }
        explicit operand(primary_expr && val)
          : operand_node_type(primary_expr(std::move(val)))
        {
        }

        explicit operand(unary_expr const& val)
          : operand_node_type(val)
        {
        }
        explicit operand(unary_expr && val)
          : operand_node_type(std::move(val))
        {
        }

    private:
        friend class hpx::serialization::access;

        PHYLANX_EXPORT void serialize(
            hpx::serialization::input_archive& ar, unsigned);
        PHYLANX_EXPORT void serialize(
            hpx::serialization::output_archive& ar, unsigned);
    };

    ///////////////////////////////////////////////////////////////////////////
    struct unary_expr : tagged
    {
        unary_expr()
          : operator_(optoken::op_unknown)
        {}

        unary_expr(optoken id, operand const& op)
          : operator_(id)
          , operand_(op)
        {}
        unary_expr(optoken id, operand && op)
          : operator_(id)
          , operand_(std::move(op))
        {}

        optoken operator_;
        operand operand_;

    private:
        friend class hpx::serialization::access;

        PHYLANX_EXPORT void serialize(
            hpx::serialization::input_archive& ar, unsigned);
        PHYLANX_EXPORT void serialize(
            hpx::serialization::output_archive& ar, unsigned);
    };

    inline bool operator==(unary_expr const& lhs, unary_expr const& rhs)
    {
        return lhs.operator_ == rhs.operator_ &&
            lhs.operand_ == rhs.operand_;
    }
    inline bool operator!=(unary_expr const& lhs, unary_expr const& rhs)
    {
        return !(lhs == rhs);
    }

    ///////////////////////////////////////////////////////////////////////////
    struct operation
    {
        operation()
          : operator_(optoken::op_unknown)
        {}

        operation(optoken id, identifier const& ident)
          : operator_(id)
          , operand_(ident)
        {}
        operation(optoken id, identifier && ident)
          : operator_(id)
          , operand_(std::move(ident))
        {}

        operation(optoken id, operand const& op)
          : operator_(id)
          , operand_(op)
        {}
        operation(optoken id, operand && op)
          : operator_(id)
          , operand_(std::move(op))
        {}

        optoken operator_;
        operand operand_;

    private:
        friend class hpx::serialization::access;

        PHYLANX_EXPORT void serialize(
            hpx::serialization::input_archive& ar, unsigned);
        PHYLANX_EXPORT void serialize(
            hpx::serialization::output_archive& ar, unsigned);
    };

    inline bool operator==(operation const& lhs, operation const& rhs)
    {
        return lhs.operator_ == rhs.operator_ &&
            lhs.operand_ == rhs.operand_;
    }
    inline bool operator!=(operation const& lhs, operation const& rhs)
    {
        return !(lhs == rhs);
    }

    ///////////////////////////////////////////////////////////////////////////
    struct expression
    {
        expression() = default;

        explicit expression(operand const& f)
          : first(f)
        {}
        explicit expression(operand && f)
          : first(std::move(f))
        {}

        explicit expression(identifier const& id)
          : first(operand(id))
        {}
        explicit expression(identifier && id)
          : first(operand(std::move(id)))
        {}

        explicit expression(bool b)
          : first(operand(b))
        {}
        explicit expression(std::int64_t v)
          : first(operand(v))
        {}
        explicit expression(double d)
          : first(operand(d))
        {}
        template <typename T>
        explicit expression(ir::node_data<T> const& nd)
          : first(operand(nd))
        {}
        template <typename T>
        explicit expression(ir::node_data<T> && nd)
          : first(operand(std::move(nd)))
        {}

        explicit expression(char const* str)
          : first(str)
        {}
        explicit expression(std::string const& str)
          : first(str)
        {}
        explicit expression(std::string && str)
          : first(std::move(str))
        {}

        explicit expression(primary_expr const& pe)
          : first(operand(pe))
        {}
        explicit expression(primary_expr && pe)
          : first(operand(std::move(pe)))
        {}

        explicit expression(unary_expr const& ue)
          : first(operand(ue))
        {}
        explicit expression(unary_expr && ue)
          : first(operand(std::move(ue)))
        {}

        explicit expression(operation const& op)
          : first(op.operand_)
        {}
        explicit expression(operation && op)
          : first(std::move(op.operand_))
        {}

        explicit expression(function_call const& fc)
          : first(primary_expr(fc))
        {}
        explicit expression(function_call && fc)
          : first(primary_expr(std::move(fc)))
        {}

        explicit expression(std::vector<ast::expression> const& l)
          : first(primary_expr(l))
        {}
        explicit expression(std::vector<ast::expression> && l)
          : first(primary_expr(std::move(l)))
        {}

        expression(operand && expr, std::vector<operation> && l)
          : first(std::move(expr))
          , rest(std::move(l))
        {}

        void append(operation const& op)
        {
            rest.push_back(op);
        }
        void append(operation && op)
        {
            rest.emplace_back(std::move(op));
        }
        void append(std::vector<operation> const& l)
        {
            std::copy(l.begin(), l.end(), std::back_inserter(rest));
        }
        void append(std::vector<operation> && l)
        {
            std::move(l.begin(), l.end(), std::back_inserter(rest));
        }

        operand first;
        std::vector<operation> rest;

    private:
        friend class hpx::serialization::access;

        PHYLANX_EXPORT void serialize(
            hpx::serialization::input_archive& ar, unsigned);
        PHYLANX_EXPORT void serialize(
            hpx::serialization::output_archive& ar, unsigned);
    };

    inline bool operator==(expression const& lhs, expression const& rhs)
    {
        return lhs.first == rhs.first && lhs.rest == rhs.rest;
    }
    inline bool operator!=(expression const& lhs, expression const& rhs)
    {
        return !(lhs == rhs);
    }

    ///////////////////////////////////////////////////////////////////////////
    struct function_call
    {
        function_call() = default;

        explicit function_call(identifier const& name)
          : function_name(name)
        {}
        explicit function_call(identifier && name)
          : function_name(std::move(name))
        {}

        function_call(identifier name, std::vector<expression>&& l)
          : function_name(std::move(name))
          , args(std::move(l))
        {}

        function_call(identifier name, std::string attr,
                std::vector<expression>&& l)
          : function_name(std::move(name))
          , attribute(std::move(attr))
          , args(std::move(l))
        {}

        void append(expression const& expr)
        {
            args.push_back(expr);
        }
        void append(expression && expr)
        {
            args.emplace_back(std::move(expr));
        }
        void append(std::vector<expression> const& l)
        {
            std::copy(l.begin(), l.end(), std::back_inserter(args));
        }
        void append(std::vector<expression> && l)
        {
            std::move(l.begin(), l.end(), std::back_inserter(args));
        }

        identifier function_name;
        std::string attribute;
        std::vector<expression> args;

    private:
        friend class hpx::serialization::access;

        PHYLANX_EXPORT void serialize(
            hpx::serialization::input_archive& ar, unsigned);
        PHYLANX_EXPORT void serialize(
            hpx::serialization::output_archive& ar, unsigned);
    };

    inline bool operator==(function_call const& lhs, function_call const& rhs)
    {
        return lhs.function_name == rhs.function_name &&
            lhs.attribute == rhs.attribute && lhs.args == rhs.args;
    }
    inline bool operator!=(function_call const& lhs, function_call const& rhs)
    {
        return !(lhs == rhs);
    }

    ///////////////////////////////////////////////////////////////////////////
    // print functions for debugging
    PHYLANX_EXPORT std::ostream& operator<<(std::ostream& out, nil);
    PHYLANX_EXPORT std::ostream& operator<<(std::ostream& out, optoken op);
    PHYLANX_EXPORT std::ostream& operator<<(
        std::ostream& out, identifier const& id);
    PHYLANX_EXPORT std::ostream& operator<<(
        std::ostream& out, primary_expr const& p);
    PHYLANX_EXPORT std::ostream& operator<<(
        std::ostream& out, operand const& op);
    PHYLANX_EXPORT std::ostream& operator<<(
        std::ostream& out, unary_expr const& ue);
    PHYLANX_EXPORT std::ostream& operator<<(
        std::ostream& out, operation const& op);
    PHYLANX_EXPORT std::ostream& operator<<(
        std::ostream& out, expression const& expr);
    PHYLANX_EXPORT std::ostream& operator<<(
        std::ostream& out, function_call const& fc);
    PHYLANX_EXPORT std::ostream& operator<<(
        std::ostream& out, std::vector<ast::expression> const& fc);

    PHYLANX_EXPORT std::string to_string(
        expression const& expr, bool repr = false);

    namespace detail
    {
        struct to_string
        {
            PHYLANX_EXPORT void operator()(bool ast) const;
            PHYLANX_EXPORT void operator()(std::string const& ast) const;

            template <typename Ast>
            void operator()(util::recursive_wrapper<Ast> const& ast) const
            {
                out_ << ast.get();
            }

            template <typename Ast>
            void operator()(Ast const& ast) const
            {
                out_ << ast;
            }

            std::ostream& out_;
        };
    }

    ///////////////////////////////////////////////////////////////////////////
    struct literal_argument_type;

    using literal_value_type = phylanx::util::variant<
            nil
          , bool
          , std::int64_t
          , std::string
          , phylanx::ir::node_data<double>
          , phylanx::util::recursive_wrapper<std::vector<literal_argument_type>>
          , phylanx::ir::node_data<std::int64_t>
          , phylanx::ir::node_data<std::uint8_t>
        >;

    struct literal_argument_type : literal_value_type
    {
        // poor man's forwarding constructor
        template <typename ... Ts>
        literal_argument_type(Ts &&... ts)
          : literal_value_type{std::forward<Ts>(ts)...}
        {}

        // workaround for problem in implementation of MSVC14.12
        // variant::visit
        literal_value_type& variant() { return *this; }
        literal_value_type const& variant() const { return *this; }
    };

    // a literal value is valid of its not nil{}
    inline bool valid(literal_value_type const& val)
    {
        return val.index() != 0;
    }
    inline bool valid(literal_value_type && val)
    {
        return val.index() != 0;
    }
}}

#endif
