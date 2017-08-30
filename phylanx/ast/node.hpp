//   Copyright (c) 2001-2011 Joel de Guzman
//   Copyright (c) 2001-2017 Hartmut Kaiser
//
//   Distributed under the Boost Software License, Version 1.0. (See accompanying
//   file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_AST_NODE_HPP)
#define PHYLANX_AST_NODE_HPP

#include <phylanx/config.hpp>
#include <phylanx/ir/node_data.hpp>
#include <phylanx/util/optional.hpp>
#include <phylanx/util/variant.hpp>

#include <phylanx/util/serialization/optional.hpp>
#include <phylanx/util/serialization/variant.hpp>

#include <hpx/include/serialization.hpp>

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
//         int id; // Used to annotate the AST with the iterator position.
//                 // This id is used as a key to a map<int, Iterator>
//                 // (not really part of the AST.)
    };

    enum class optoken
    {
        op_unknown,

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
    };

    char const* const optoken_names[] =
    {
        "op_unknown",

        // precedence 1
        "op_comma",

        // precedence 2
        "op_assign",
        "op_plus_assign",
        "op_minus_assign",
        "op_times_assign",
        "op_divide_assign",
        "op_mod_assign",
        "op_bit_and_assign",
        "op_bit_xor_assign",
        "op_bitor_assign",
        "op_shift_left_assign",
        "op_shift_right_assign",

        // precedence 3
        "op_logical_or",

        // precedence 4
        "op_logical_and",

        // precedence 5
        "op_bit_or",

        // precedence 6
        "op_bit_xor",

        // precedence 7
        "op_bit_and",

        // precedence 8
        "op_equal",
        "op_not_equal",

        // precedence 9
        "op_less",
        "op_less_equal",
        "op_greater",
        "op_greater_equal",

        // precedence 10
        "op_shift_left",
        "op_shift_right",

        // precedence 11
        "op_plus",
        "op_minus",

        // precedence 12
        "op_times",
        "op_divide",
        "op_mod",

        // precedence 13
        "op_positive",
        "op_negative",
        "op_pre_incr",
        "op_pre_decr",
        "op_compl",
        "op_not",

        // precedence 14
        "op_post_incr",
        "op_post_decr",
    };

    template <typename Archive>
    void load(Archive& ar, optoken& id, unsigned)
    {
        int val;
        ar >> val;
        id = static_cast<optoken>(val);
    }

    template <typename Archive>
    void save(Archive& ar, optoken const& id, unsigned)
    {
        int val = static_cast<int>(id);
        ar << val;
    }
    HPX_SERIALIZATION_SPLIT_FREE(optoken);

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

        std::string name;

        template <typename Archive>
        void serialize(Archive& ar, unsigned)
        {
            ar & name;
        }
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
    template <typename T> struct unary_expr;
    template <typename T> struct function_call;
    template <typename T> struct expression;

    ///////////////////////////////////////////////////////////////////////////
    template <typename T>
    struct primary_expr : tagged
    {
        using expr_node_type = phylanx::util::variant<
                nil
              , bool
              , phylanx::ir::node_data<T>
              , identifier
              , phylanx::util::recursive_wrapper<expression<T>>
            >;

        expr_node_type value;

        primary_expr() = default;
        explicit primary_expr(bool val)
          : value(val)
        {
        }

        explicit primary_expr(phylanx::ir::node_data<T> const& val)
          : value(val)
        {
        }
        explicit primary_expr(phylanx::ir::node_data<T> && val)
          : value(std::move(val))
        {
        }

        explicit primary_expr(identifier const& val)
          : value(val)
        {
        }
        explicit primary_expr(identifier && val)
          : value(std::move(val))
        {
        }

        explicit primary_expr(expression<T> const& val)
          : value(val)
        {
        }
        explicit primary_expr(expression<T> && val)
          : value(std::move(val))
        {
        }

        template <typename Archive>
        void serialize(Archive& ar, unsigned)
        {
            ar & value;
        }
    };

    template <typename T>
    bool operator==(primary_expr<T> const& lhs, primary_expr<T> const& rhs)
    {
        return lhs.value == rhs.value;
    }
    template <typename T>
    bool operator!=(primary_expr<T> const& lhs, primary_expr<T> const& rhs)
    {
        return !(lhs == rhs);
    }

    ///////////////////////////////////////////////////////////////////////////
    template <typename T>
    struct operand : tagged
    {
        using operand_node_type = phylanx::util::variant<
                nil
              , phylanx::util::recursive_wrapper<primary_expr<T>>
              , phylanx::util::recursive_wrapper<unary_expr<T>>
//                   , phylanx::util::recursive_wrapper<function_call>
            >;

        operand_node_type value;

        operand() = default;

        explicit operand(primary_expr<T> const& val)
          : value(val)
        {
        }
        explicit operand(primary_expr<T> && val)
          : value(std::move(val))
        {
        }

        explicit operand(unary_expr<T> const& val)
          : value(val)
        {
        }
        explicit operand(unary_expr<T> && val)
          : value(std::move(val))
        {
        }

//         operand(function_call const& val)
//             : value(val)
//         {
//         }
//         operand(function_call && val)
//             : value(std::move(val))
//         {
//         }

        template <typename Archive>
        void serialize(Archive& ar, unsigned)
        {
            ar & value;
        }
    };

    template <typename T>
    bool operator==(operand<T> const& lhs, operand<T> const& rhs)
    {
        return lhs.value == rhs.value;
    }
    template <typename T>
    bool operator!=(operand<T> const& lhs, operand<T> const& rhs)
    {
        return !(lhs == rhs);
    }

    ///////////////////////////////////////////////////////////////////////////
    template <typename T>
    struct unary_expr : tagged
    {
        unary_expr()
          : operator_(optoken::op_unknown)
        {}

        explicit unary_expr(optoken id, operand<T> const& op)
          : operator_(id)
          , operand_(op)
        {}
        explicit unary_expr(optoken id, operand<T> && op)
          : operator_(id)
          , operand_(std::move(op))
        {}

        optoken operator_;
        phylanx::util::recursive_wrapper<operand<T>> operand_;

        template <typename Archive>
        void serialize(Archive& ar, unsigned)
        {
            ar & operator_ & operand_;
        }
    };

    template <typename T>
    bool operator==(unary_expr<T> const& lhs, unary_expr<T> const& rhs)
    {
        return lhs.operator_ == rhs.operator_ &&
            lhs.operand_ == rhs.operand_;
    }
    template <typename T>
    bool operator!=(unary_expr<T> const& lhs, unary_expr<T> const& rhs)
    {
        return !(lhs == rhs);
    }

    ///////////////////////////////////////////////////////////////////////////
    template <typename T>
    struct operation
    {
        operation()
          : operator_(optoken::op_unknown)
        {}

        explicit operation(optoken id, operand<T> const& op)
          : operator_(id)
          , operand_(op)
        {}
        explicit operation(optoken id, operand<T> && op)
          : operator_(id)
          , operand_(std::move(op))
        {}

        optoken operator_;
        operand<T> operand_;

        template <typename Archive>
        void serialize(Archive& ar, unsigned)
        {
            ar & operator_ & operand_;
        }
    };

    template <typename T>
    bool operator==(operation<T> const& lhs, operation<T> const& rhs)
    {
        return lhs.operator_ == rhs.operator_ &&
            lhs.operand_ == rhs.operand_;
    }
    template <typename T>
    bool operator!=(operation<T> const& lhs, operation<T> const& rhs)
    {
        return !(lhs == rhs);
    }

//         struct function_call
//         {
//             identifier function_name;
//             std::list<expression> args;
//
//             template <typename Archive>
//             void serialize(Archive& ar, unsigned)
//             {
//                 ar & function_name & args;
//             }
//         };

    ///////////////////////////////////////////////////////////////////////////
    template <typename T>
    struct expression
    {
        expression() = default;

        explicit expression(operand<T> const& f)
          : first(f)
        {}
        explicit expression(operand<T> && f)
          : first(std::move(f))
        {}

        void append(operation<T> const& op)
        {
            rest.push_back(op);
        }
        void append(operation<T> && op)
        {
            rest.emplace_back(std::move(op));
        }
        void append(std::list<operation<T>> const& l)
        {
            std::copy(l.begin(), l.end(), std::back_inserter(rest));
        }
        void append(std::list<operation<T>> && l)
        {
            std::move(l.begin(), l.end(), std::back_inserter(rest));
        }

        operand<T> first;
        std::list<operation<T>> rest;

        template <typename Archive>
        void serialize(Archive& ar, unsigned)
        {
            ar & first & rest;
        }
    };

    template <typename T>
    bool operator==(expression<T> const& lhs, expression<T> const& rhs)
    {
        return lhs.first == rhs.first && lhs.rest == rhs.rest;
    }
    template <typename T>
    bool operator!=(expression<T> const& lhs, expression<T> const& rhs)
    {
        return !(lhs == rhs);
    }

//         struct assignment
//         {
//             identifier lhs;
//             optoken operator_;
//             expression rhs;
//
//             template <typename Archive>
//             void serialize(Archive& ar, unsigned)
//             {
//                 ar & lhs & operator_ & rhs;
//             }
//         };
//
//         struct variable_declaration
//         {
//             identifier lhs;
//             phylanx::util::optional<expression> rhs;
//
//             template <typename Archive>
//             void serialize(Archive& ar, unsigned)
//             {
//                 ar & lhs & rhs;
//             }
//         };
//
//         struct if_statement;
//         struct while_statement;
//         struct statement_list;
//         struct return_statement;
//
//         using statement = phylanx::util::variant<
//                 nil
//               , variable_declaration
//               , assignment
//               , phylanx::util::recursive_wrapper<if_statement>
//               , phylanx::util::recursive_wrapper<while_statement>
//               , phylanx::util::recursive_wrapper<return_statement>
//               , phylanx::util::recursive_wrapper<statement_list>
//               , phylanx::util::recursive_wrapper<expression>
//             >;
//
//         struct statement_list : std::list<statement>
//         {
//             template <typename Archive>
//             void serialize(Archive& ar, unsigned)
//             {
//                 ar & *static_cast<std::list<statement>>(this);
//             }
//         };
//
//         struct if_statement
//         {
//             expression condition;
//             statement then;
//             phylanx::util::optional<statement> else_;
//
//             template <typename Archive>
//             void serialize(Archive& ar, unsigned)
//             {
//                 ar & condition & then & else_;
//             }
//         };
//
//         struct while_statement
//         {
//             expression condition;
//             statement body;
//
//             template <typename Archive>
//             void serialize(Archive& ar, unsigned)
//             {
//                 ar & condition & body;
//             }
//         };
//
//         struct return_statement : tagged
//         {
//             phylanx::util::optional<expression> expr;
//
//             template <typename Archive>
//             void serialize(Archive& ar, unsigned)
//             {
//                 ar & expr;
//             }
//         };
//
//         struct function
//         {
//             std::string return_type;
//             identifier function_name;
//             std::list<identifier> args;
//             phylanx::util::optional<statement_list> body;
//
//             template <typename Archive>
//             void serialize(Archive& ar, unsigned)
//             {
//                 ar & return_type & function_name & args & body;
//             }
//         };
//
//         using function_list = std::list<function>;
//     };

    ///////////////////////////////////////////////////////////////////////////
    // print functions for debugging
    inline std::ostream& operator<<(std::ostream& out, nil)
    {
        out << "nil";
        return out;
    }

    inline std::ostream& operator<<(std::ostream& out, optoken op)
    {
        out << optoken_names[static_cast<int>(op)];
        return out;
    }

    inline std::ostream& operator<<(std::ostream& out, identifier const& id)
    {
        out << "identifier: " << id.name;
        return out;
    }

    template <typename T>
    inline std::ostream& operator<<(std::ostream& out, primary_expr<T> const& p)
    {
        out << "primary_expr<T>";
        return out;
    }

    template <typename T>
    inline std::ostream& operator<<(std::ostream& out, operand<T> const& p)
    {
        out << "operand<T>";
        return out;
    }

    template <typename T>
    inline std::ostream& operator<<(std::ostream& out, unary_expr<T> const& p)
    {
        out << "unary_expr<T>";
        return out;
    }

    template <typename T>
    inline std::ostream& operator<<(std::ostream& out, operation<T> const& p)
    {
        out << "operation<T>";
        return out;
    }

    template <typename T>
    inline std::ostream& operator<<(std::ostream& out, expression<T> const& p)
    {
        out << "expression<T>";
        return out;
    }
}}

#endif
