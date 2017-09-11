//   Copyright (c) 2001-2011 Joel de Guzman
//   Copyright (c) 2001-2017 Hartmut Kaiser
//
//   Distributed under the Boost Software License, Version 1.0. (See accompanying
//   file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_AST_NODE_HPP)
#define PHYLANX_AST_NODE_HPP

#include <phylanx/config.hpp>
#include <phylanx/ast/parser/extended_variant.hpp>
#include <phylanx/ir/node_data.hpp>
#include <phylanx/util/optional.hpp>
#include <phylanx/util/serialization/optional.hpp>
#include <phylanx/util/serialization/variant.hpp>

#include <hpx/runtime/serialization/serialization_fwd.hpp>

#include <boost/spirit/include/support_extended_variant.hpp>
#include <boost/spirit/include/support_attributes.hpp>

#include <cstddef>
#include <iosfwd>
#include <list>
#include <string>
#include <utility>

namespace phylanx { namespace ast
{
    ///////////////////////////////////////////////////////////////////////////
    //  The AST
    struct tagged
    {
        tagged()
          : id(0)
        {
        }

        std::size_t id; // Used to annotate the AST with the iterator position.
                        // This id is used as a key to a map<int, Iterator>
                        // (not really part of the AST.)
    };

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
    struct nil {};

    constexpr inline bool operator==(nil const&, nil const&)
    {
        return true;
    }
    constexpr inline bool operator!=(nil const&, nil const&)
    {
        return false;
    }

    template <typename Ast>
    bool is_placeholder(Ast const&)
    {
        return false;
    }

    template <typename Ast>
    std::string placeholder_name(Ast const&)
    {
        return "";
    }

    template <typename Ast>
    bool is_placeholder(util::recursive_wrapper<Ast> const& ast)
    {
        return is_placeholder(ast.get());
    }

    template <typename Ast>
    std::string placeholder_name(util::recursive_wrapper<Ast> const& ast)
    {
        return placeholder_name(ast.get());
    }

    ///////////////////////////////////////////////////////////////////////////
    struct identifier : tagged
    {
        identifier() = default;

        identifier(std::string const& name)
        : name(name)
        {
        }
        identifier(std::string && name)
          : name(std::move(name))
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

    inline bool is_placeholder(identifier const& id)
    {
        return !id.name.empty() && id.name[0] == '_';
    }

    inline std::string placeholder_name(identifier const& id)
    {
        return id.name;
    }

    ///////////////////////////////////////////////////////////////////////////
    struct unary_expr;
    struct expression;
    struct function_call;

//     struct if_statement;
//     struct while_statement;
//     struct statement;
//     struct return_statement;
//
//     using statement_list = std::list<statement>;

    ///////////////////////////////////////////////////////////////////////////
    using expr_node_type = phylanx::ast::parser::extended_variant<
            nil
          , bool
          , phylanx::ir::node_data<double>
          , identifier
          , phylanx::util::recursive_wrapper<expression>
          , phylanx::util::recursive_wrapper<function_call>
        >;

    struct primary_expr : tagged, expr_node_type
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

        primary_expr(phylanx::ir::node_data<double> const& val)
          : expr_node_type(val)
        {
        }
        primary_expr(phylanx::ir::node_data<double> && val)
          : expr_node_type(std::move(val))
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
        primary_expr(std::string const& val)
          : expr_node_type(identifier(val))
        {
        }
        primary_expr(std::string && val)
          : expr_node_type(identifier(std::move(val)))
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

    private:
        friend class hpx::serialization::access;

        PHYLANX_EXPORT void serialize(
            hpx::serialization::input_archive& ar, unsigned);
        PHYLANX_EXPORT void serialize(
            hpx::serialization::output_archive& ar, unsigned);
    };

    PHYLANX_EXPORT bool is_placeholder(primary_expr const& pe);
    PHYLANX_EXPORT std::string placeholder_name(primary_expr const& pe);

    ///////////////////////////////////////////////////////////////////////////
    using operand_node_type = phylanx::ast::parser::extended_variant<
            nil
          , phylanx::util::recursive_wrapper<primary_expr>
          , phylanx::util::recursive_wrapper<unary_expr>
        >;

    struct operand : operand_node_type, tagged
    {
        operand() = default;

        explicit operand(double val)
          : operand_node_type(
                phylanx::util::recursive_wrapper<primary_expr>(val))
        {
        }
        explicit operand(std::string const& val)
          : operand_node_type(
                phylanx::util::recursive_wrapper<primary_expr>(val))
        {
        }
        explicit operand(std::string && val)
          : operand_node_type(
                phylanx::util::recursive_wrapper<primary_expr>(std::move(val)))
        {
        }

        explicit operand(primary_expr const& val)
          : operand_node_type(val)
        {
        }
        explicit operand(primary_expr && val)
          : operand_node_type(std::move(val))
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

    PHYLANX_EXPORT bool is_placeholder(operand const& op);
    PHYLANX_EXPORT std::string placeholder_name(operand const& op);

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

    inline bool is_placeholder(unary_expr const& ue)
    {
        return is_placeholder(ue.operand_);
    }

    inline std::string placeholder_name(unary_expr const& ue)
    {
        return placeholder_name(ue.operand_);
    }

    ///////////////////////////////////////////////////////////////////////////
    struct operation
    {
        operation()
          : operator_(optoken::op_unknown)
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

    inline bool is_placeholder(operation const& op)
    {
        return is_placeholder(op.operand_);
    }

    inline std::string placeholder_name(operation const& op)
    {
        return placeholder_name(op.operand_);
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
          : first(operand(id.name))
        {}
        explicit expression(identifier && id)
          : first(operand(std::move(id.name)))
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

        void append(operation const& op)
        {
            rest.push_back(op);
        }
        void append(operation && op)
        {
            rest.emplace_back(std::move(op));
        }
        void append(std::list<operation> const& l)
        {
            std::copy(l.begin(), l.end(), std::back_inserter(rest));
        }
        void append(std::list<operation> && l)
        {
            std::move(l.begin(), l.end(), std::back_inserter(rest));
        }

        operand first;
        std::list<operation> rest;

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

    inline bool is_placeholder(expression const& expr)
    {
        if (!expr.rest.empty())
            return false;

        return is_placeholder(expr.first);
    }

    inline std::string placeholder_name(expression const& expr)
    {
        if (!expr.rest.empty())
        {
            return "";
        }
        return placeholder_name(expr.first);
    }

    ///////////////////////////////////////////////////////////////////////////
    struct function_call
    {
        function_call() = default;

        function_call(identifier const& name)
          : function_name(name)
        {}
        function_call(identifier && name)
          : function_name(std::move(name))
        {}

        void append(expression const& expr)
        {
            args.push_back(expr);
        }
        void append(expression && expr)
        {
            args.emplace_back(std::move(expr));
        }
        void append(std::list<expression> const& l)
        {
            std::copy(l.begin(), l.end(), std::back_inserter(args));
        }
        void append(std::list<expression> && l)
        {
            std::move(l.begin(), l.end(), std::back_inserter(args));
        }

        identifier function_name;
        std::list<expression> args;

    private:
        friend class hpx::serialization::access;

        PHYLANX_EXPORT void serialize(
            hpx::serialization::input_archive& ar, unsigned);
        PHYLANX_EXPORT void serialize(
            hpx::serialization::output_archive& ar, unsigned);
    };

    inline bool operator==(function_call const& lhs, function_call const& rhs)
    {
        return lhs.function_name == rhs.function_name && lhs.args == rhs.args;
    }
    inline bool operator!=(function_call const& lhs, function_call const& rhs)
    {
        return !(lhs == rhs);
    }

//     ///////////////////////////////////////////////////////////////////////////
//     struct assignment
//     {
//         identifier lhs;
//         optoken operator_;
//         expression rhs;
//
//         PHYLANX_EXPORT void serialize(
//             hpx::serialization::input_archive& ar, unsigned);
//         PHYLANX_EXPORT void serialize(
//             hpx::serialization::output_archive& ar, unsigned);
//     };
//
//     inline bool operator==(assignment const& lhs, assignment const& rhs)
//     {
//         return lhs.lhs == rhs.lhs && lhs.operator_ == rhs.operator_ &&
//             lhs.rhs == rhs.rhs;
//     }
//     inline bool operator!=(assignment const& lhs, assignment const& rhs)
//     {
//         return !(lhs == rhs);
//     }
//
//     ///////////////////////////////////////////////////////////////////////////
//     struct variable_declaration
//     {
//         identifier lhs;
//         phylanx::util::optional<expression> rhs;
//
//         PHYLANX_EXPORT void serialize(
//             hpx::serialization::input_archive& ar, unsigned);
//         PHYLANX_EXPORT void serialize(
//             hpx::serialization::output_archive& ar, unsigned);
//     };
//
//     inline bool operator==(
//         variable_declaration const& lhs, variable_declaration const& rhs)
//     {
//         return lhs.lhs == rhs.lhs && lhs.rhs == rhs.rhs;
//     }
//     inline bool operator!=(
//         variable_declaration const& lhs, variable_declaration const& rhs)
//     {
//         return !(lhs == rhs);
//     }
//
//     ///////////////////////////////////////////////////////////////////////////
//     using statement_node_type = phylanx::ast::parser::extended_variant<
//             nil
//           , variable_declaration
//           , assignment
//           , phylanx::util::recursive_wrapper<if_statement>
//           , phylanx::util::recursive_wrapper<while_statement>
//           , phylanx::util::recursive_wrapper<return_statement>
//           , phylanx::util::recursive_wrapper<statement_list>
//           , phylanx::util::recursive_wrapper<expression>
//         >;
//
//     struct statement : statement_node_type
//     {
//         statement() = default;
//
//         statement(nil val)
//           : statement_node_type(val)
//         {
//         }
//
//         statement(variable_declaration const& val)
//           : statement_node_type(val)
//         {
//         }
//         statement(variable_declaration && val)
//           : statement_node_type(std::move(val))
//         {
//         }
//
//         statement(assignment const& val)
//           : statement_node_type(val)
//         {
//         }
//         statement(assignment && val)
//           : statement_node_type(std::move(val))
//         {
//         }
//
//         statement(if_statement const& val)
//           : statement_node_type(val)
//         {
//         }
//         statement(if_statement && val)
//           : statement_node_type(std::move(val))
//         {
//         }
//
//         statement(while_statement const& val)
//           : statement_node_type(val)
//         {
//         }
//         statement(while_statement && val)
//           : statement_node_type(std::move(val))
//         {
//         }
//
//         statement(return_statement const& val)
//           : statement_node_type(val)
//         {
//         }
//         statement(return_statement && val)
//           : statement_node_type(std::move(val))
//         {
//         }
//
//         statement(statement_list const& val)
//           : statement_node_type(val)
//         {
//         }
//         statement(statement_list && val)
//           : statement_node_type(std::move(val))
//         {
//         }
//
//         statement(expression const& val)
//           : statement_node_type(val)
//         {
//         }
//         statement(expression && val)
//           : statement_node_type(std::move(val))
//         {
//         }
//
//         PHYLANX_EXPORT void serialize(
//             hpx::serialization::input_archive& ar, unsigned);
//         PHYLANX_EXPORT void serialize(
//             hpx::serialization::output_archive& ar, unsigned);
//     };
//
//     ///////////////////////////////////////////////////////////////////////////
//     struct if_statement
//     {
//         expression condition;
//         statement then;
//         phylanx::util::optional<statement> else_;
//
//         PHYLANX_EXPORT void serialize(
//             hpx::serialization::input_archive& ar, unsigned);
//         PHYLANX_EXPORT void serialize(
//             hpx::serialization::output_archive& ar, unsigned);
//     };
//
//     inline bool operator==(if_statement const& lhs, if_statement const& rhs)
//     {
//         return lhs.condition == rhs.condition && lhs.then == rhs.then &&
//             lhs.else_ == rhs.else_;
//     }
//     inline bool operator!=(if_statement const& lhs, if_statement const& rhs)
//     {
//         return !(lhs == rhs);
//     }
//
//     ///////////////////////////////////////////////////////////////////////////
//     struct while_statement
//     {
//         expression condition;
//         statement body;
//
//         PHYLANX_EXPORT void serialize(
//             hpx::serialization::input_archive& ar, unsigned);
//         PHYLANX_EXPORT void serialize(
//             hpx::serialization::output_archive& ar, unsigned);
//     };
//
//     inline bool operator==(
//         while_statement const& lhs, while_statement const& rhs)
//     {
//         return lhs.condition == rhs.condition && lhs.body == rhs.body;
//     }
//     inline bool operator!=(
//         while_statement const& lhs, while_statement const& rhs)
//     {
//         return !(lhs == rhs);
//     }
//
//     ///////////////////////////////////////////////////////////////////////////
//     struct return_statement : tagged
//     {
//         phylanx::util::optional<expression> expr;
//
//         PHYLANX_EXPORT void serialize(
//             hpx::serialization::input_archive& ar, unsigned);
//         PHYLANX_EXPORT void serialize(
//             hpx::serialization::output_archive& ar, unsigned);
//     };
//
//     inline bool operator==(
//         return_statement const& lhs, return_statement const& rhs)
//     {
//         return lhs.expr == rhs.expr;
//     }
//     inline bool operator!=(
//         return_statement const& lhs, return_statement const& rhs)
//     {
//         return !(lhs == rhs);
//     }
//
//     ///////////////////////////////////////////////////////////////////////////
//     struct function
//     {
//         std::string return_type;
//         identifier function_name;
//         std::list<identifier> args;
//         phylanx::util::optional<statement_list> body;
//
//         PHYLANX_EXPORT void serialize(
//             hpx::serialization::input_archive& ar, unsigned);
//         PHYLANX_EXPORT void serialize(
//             hpx::serialization::output_archive& ar, unsigned);
//     };
//
//     using function_list = std::list<function>;

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
//     PHYLANX_EXPORT std::ostream& operator<<(
//         std::ostream& out, assignment const& assign);
//     PHYLANX_EXPORT std::ostream& operator<<(
//         std::ostream& out, variable_declaration const& vd);
//     PHYLANX_EXPORT std::ostream& operator<<(
//         std::ostream& out, statement const& stmt);
//     PHYLANX_EXPORT std::ostream& operator<<(
//         std::ostream& out, if_statement const& if_);
//     PHYLANX_EXPORT std::ostream& operator<<(
//         std::ostream& out, while_statement const& while_);
//     PHYLANX_EXPORT std::ostream& operator<<(
//         std::ostream& out, return_statement const& ret);
//     PHYLANX_EXPORT std::ostream& operator<<(
//         std::ostream& out, function const& func);
//     PHYLANX_EXPORT std::ostream& operator<<(
//         std::ostream& out, function_list const& fl);
}}

#endif
