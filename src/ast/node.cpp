//  Copyright (c) 2017 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/ast/node.hpp>

#include <hpx/include/serialization.hpp>

#include <iosfwd>

namespace phylanx { namespace ast
{
    ///////////////////////////////////////////////////////////////////////////
    void serialize(hpx::serialization::input_archive& ar, optoken& id, unsigned)
    {
        int val;
        ar >> val;
        id = static_cast<optoken>(val);
    }

    void serialize(
        hpx::serialization::output_archive& ar, optoken const& id, unsigned)
    {
        int val = static_cast<int>(id);
        ar << val;
    }

    ///////////////////////////////////////////////////////////////////////////
    void identifier::serialize(hpx::serialization::output_archive& ar, unsigned)
    {
        ar << name;
    }

    void identifier::serialize(hpx::serialization::input_archive& ar, unsigned)
    {
        ar >> name;
    }

    ///////////////////////////////////////////////////////////////////////////
    void primary_expr::serialize(
        hpx::serialization::output_archive& ar, unsigned)
    {
        ar << *static_cast<expr_node_type*>(this);
    }

    void primary_expr::serialize(
        hpx::serialization::input_archive& ar, unsigned)
    {
        ar >> *static_cast<expr_node_type*>(this);
    }

    ///////////////////////////////////////////////////////////////////////////
    void operand::serialize(hpx::serialization::output_archive& ar, unsigned)
    {
        ar << *static_cast<operand_node_type*>(this);
    }

    void operand::serialize(hpx::serialization::input_archive& ar, unsigned)
    {
        ar >> *static_cast<operand_node_type*>(this);
    }

    ///////////////////////////////////////////////////////////////////////////
    void unary_expr::serialize(hpx::serialization::output_archive& ar, unsigned)
    {
        ar << operator_ << operand_;
    }

    void unary_expr::serialize(hpx::serialization::input_archive& ar, unsigned)
    {
        ar >> operator_ >> operand_;
    }

    ///////////////////////////////////////////////////////////////////////////
    void operation::serialize(hpx::serialization::output_archive& ar, unsigned)
    {
        ar << operator_ << operand_;
    }

    void operation::serialize(hpx::serialization::input_archive& ar, unsigned)
    {
        ar >> operator_ >> operand_;
    }

    ///////////////////////////////////////////////////////////////////////////
    void expression::serialize(hpx::serialization::output_archive& ar, unsigned)
    {
        ar << first << rest;
    }

    void expression::serialize(hpx::serialization::input_archive& ar, unsigned)
    {
        ar >> first >> rest;
    }

    ///////////////////////////////////////////////////////////////////////////
    void function_call::serialize(
        hpx::serialization::output_archive& ar, unsigned)
    {
        ar << function_name << args;
    }

    void function_call::serialize(
        hpx::serialization::input_archive& ar, unsigned)
    {
        ar >> function_name >> args;
    }

//     ///////////////////////////////////////////////////////////////////////////
//     void assignment::serialize(hpx::serialization::output_archive& ar, unsigned)
//     {
//         ar << lhs << operator_ << rhs;
//     }
//
//     void assignment::serialize(hpx::serialization::input_archive& ar, unsigned)
//     {
//         ar >> lhs >> operator_ >> rhs;
//     }
//
//     ///////////////////////////////////////////////////////////////////////////
//     void variable_declaration::serialize(
//         hpx::serialization::output_archive& ar, unsigned)
//     {
//         ar << lhs << rhs;
//     }
//
//     void variable_declaration::serialize(
//         hpx::serialization::input_archive& ar, unsigned)
//     {
//         ar >> lhs >> rhs;
//     }
//
//     ///////////////////////////////////////////////////////////////////////////
//     void statement::serialize(hpx::serialization::output_archive& ar, unsigned)
//     {
//         ar << *static_cast<statement_node_type*>(this);
//     }
//
//     void statement::serialize(hpx::serialization::input_archive& ar, unsigned)
//     {
//         ar >> *static_cast<statement_node_type*>(this);
//     }
//
//     ///////////////////////////////////////////////////////////////////////////
//     void if_statement::serialize(
//         hpx::serialization::output_archive& ar, unsigned)
//     {
//         ar << condition << then << else_;
//     }
//
//     void if_statement::serialize(
//         hpx::serialization::input_archive& ar, unsigned)
//     {
//         ar >> condition >> then >> else_;
//     }
//
//     ///////////////////////////////////////////////////////////////////////////
//     void while_statement::serialize(
//         hpx::serialization::output_archive& ar, unsigned)
//     {
//         ar << condition << body;
//     }
//
//     void while_statement::serialize(
//         hpx::serialization::input_archive& ar, unsigned)
//     {
//         ar >> condition >> body;
//     }
//
//     ///////////////////////////////////////////////////////////////////////////
//     void return_statement::serialize(
//         hpx::serialization::output_archive& ar, unsigned)
//     {
//         ar << expr;
//     }
//
//     void return_statement::serialize(
//         hpx::serialization::input_archive& ar, unsigned)
//     {
//         ar >> expr;
//     }
//
//     ///////////////////////////////////////////////////////////////////////////
//     void function::serialize(hpx::serialization::output_archive& ar, unsigned)
//     {
//         ar << return_type << function_name << args << body;
//     }
//
//     void function::serialize(hpx::serialization::input_archive& ar, unsigned)
//     {
//         ar >> return_type >> function_name >> args >> body;
//     }

    ///////////////////////////////////////////////////////////////////////////
    namespace detail
    {
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
    }

    std::ostream& operator<<(std::ostream& out, nil)
    {
        out << "nil";
        return out;
    }

    std::ostream& operator<<(std::ostream& out, optoken op)
    {
        out << detail::optoken_names[static_cast<int>(op)];
        return out;
    }

    std::ostream& operator<<(std::ostream& out, identifier const& id)
    {
        out << "identifier: " << id.name;
        return out;
    }

    std::ostream& operator<<(std::ostream& out, primary_expr const& pe)
    {
        out << "primary_expr";
        return out;
    }

    std::ostream& operator<<(std::ostream& out, operand const& op)
    {
        out << "operand";
        return out;
    }

    std::ostream& operator<<(std::ostream& out, unary_expr const& ue)
    {
        out << "unary_expr";
        return out;
    }

    std::ostream& operator<<(std::ostream& out, operation const& op)
    {
        out << "operation";
        return out;
    }

    std::ostream& operator<<(std::ostream& out, expression const& expr)
    {
        out << "expression";
        return out;
    }

    std::ostream& operator<<(std::ostream& out, function_call const& f)
    {
        out << "function_call";
        return out;
    }

//     std::ostream& operator<<(std::ostream& out, assignment const& a)
//     {
//         out << "assignment";
//         return out;
//     }
//
//     std::ostream& operator<<(std::ostream& out, variable_declaration const& vd)
//     {
//         out << "variable_declaration";
//         return out;
//     }
//
//     std::ostream& operator<<(std::ostream& out, statement const& stmt)
//     {
//         out << "statement";
//         return out;
//     }
//
//     std::ostream& operator<<(std::ostream& out, if_statement const& if_)
//     {
//         out << "if_statement";
//         return out;
//     }
//
//     std::ostream& operator<<(std::ostream& out, while_statement const& while_)
//     {
//         out << "while_statement";
//         return out;
//     }
//
//     std::ostream& operator<<(std::ostream& out, return_statement const& ret)
//     {
//         out << "return_statement";
//         return out;
//     }
//
//     std::ostream& operator<<(std::ostream& out, function const& func)
//     {
//         out << "function";
//         return out;
//     }
//
//     std::ostream& operator<<(std::ostream& out, function_list const& fl)
//     {
//         out << "function_list";
//         return out;
//     }
}}
