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

#include <hpx/include/components.hpp>
#include <hpx/include/util.hpp>
#include <hpx/util/scoped_timer.hpp>
#include <hpx/include/serialization.hpp>

#include <cstdint>
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

        primitive(primitive const&) = default;
        primitive(primitive &&) = default;

        primitive& operator=(primitive const&) = default;
        primitive& operator=(primitive &&) = default;

        hpx::future<primitive_argument_type> eval() const;
        hpx::future<primitive_argument_type> eval(
            std::vector<primitive_argument_type> && args) const;
        hpx::future<primitive_argument_type> eval(
            std::vector<primitive_argument_type> const& args) const;

        primitive_argument_type eval_direct() const;
        primitive_argument_type eval_direct(
            std::vector<primitive_argument_type> && args) const;
        primitive_argument_type eval_direct(
            std::vector<primitive_argument_type> const& args) const;

        hpx::future<void> store(primitive_argument_type);
        void store(hpx::launch::sync_policy, primitive_argument_type);

        hpx::future<topology> expression_topology() const;
        topology expression_topology(hpx::launch::sync_policy) const;
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

    struct primitive_argument_type;
    using primitive_result_type = primitive_argument_type;


    PHYLANX_EXPORT primitive_result_type value_operand_sync(
        primitive_argument_type const& val,
        std::vector<primitive_argument_type> const& args);
    PHYLANX_EXPORT primitive_result_type value_operand_sync(
        primitive_argument_type const& val,
        std::vector<primitive_argument_type> && args);
    PHYLANX_EXPORT primitive_result_type value_operand_sync(
        primitive_argument_type && val,
        std::vector<primitive_argument_type> const& args);
    PHYLANX_EXPORT primitive_result_type value_operand_sync(
        primitive_argument_type && val,
        std::vector<primitive_argument_type> && args);

    PHYLANX_EXPORT primitive_result_type value_operand_ref_sync(
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
}}

namespace phylanx { namespace execution_tree { namespace primitives
{
    ///////////////////////////////////////////////////////////////////////////
    class HPX_COMPONENT_EXPORT base_primitive
      : public hpx::traits::detail::component_tag
    {
    public:
        base_primitive()
          : eval_count_(0ll)
          , eval_duration_(0ll)
          , eval_direct_count_(0ll)
          , eval_direct_duration_(0ll)
        {}

        base_primitive(std::vector<primitive_argument_type>&& operands)
          : operands_(std::move(operands))
          , eval_count_(0ll)
          , eval_duration_(0ll)
          , eval_direct_count_(0ll)
          , eval_direct_duration_(0ll)
        {}

        virtual ~base_primitive() = default;

        hpx::future<primitive_result_type> eval_nonvirtual(
            std::vector<primitive_argument_type> const& args) const
        {
            hpx::util::scoped_timer<std::int64_t> timer(eval_duration_);
            ++eval_count_;
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
            hpx::util::scoped_timer<std::int64_t> timer(eval_direct_duration_);
            ++eval_direct_count_;
            return eval_direct(args);
        }
        virtual primitive_result_type eval_direct(
            std::vector<primitive_argument_type> const& params) const
        {
            return eval(params).get();
        }

        void store_nonvirtual(primitive_result_type data)
        {
            store(std::move(data));
        }
        virtual void store(primitive_result_type &&)
        {
            HPX_THROW_EXCEPTION(hpx::invalid_status,
                "phylanx::execution_tree::primitives::base_primitive",
                "store function should only be called in store_primitive");
        }

        topology expression_topology_nonvirtual() const
        {
            return expression_topology();
        }
        virtual topology expression_topology() const;

    public:

#if defined(PHYLANX_DEBUG)
        HPX_DEFINE_COMPONENT_DIRECT_ACTION(
            base_primitive, eval_nonvirtual, eval_action);
        HPX_DEFINE_COMPONENT_DIRECT_ACTION(
            base_primitive, expression_topology_nonvirtual,
            expression_topology_action);
#else
        HPX_DEFINE_COMPONENT_ACTION(
            base_primitive, eval_nonvirtual, eval_action);
        HPX_DEFINE_COMPONENT_ACTION(
            base_primitive, expression_topology_nonvirtual,
            expression_topology_action);
#endif
        HPX_DEFINE_COMPONENT_DIRECT_ACTION(
            base_primitive, eval_direct_nonvirtual, eval_direct_action);
        HPX_DEFINE_COMPONENT_ACTION(
            base_primitive, store_nonvirtual, store_action);

    public:
        // Pinning functionality helper functions
        void pin()
        {
        }
        void unpin()
        {
        }
        std::uint32_t pin_count() const
        {
            return 0;
        }

        // access data for performance counter
        std::int64_t get_eval_count(bool reset, bool direct) const
        {
            if (!direct)
            {
                return hpx::util::get_and_reset_value(eval_count_, reset);
            }
            return hpx::util::get_and_reset_value(eval_direct_count_, reset);
        }
        std::int64_t get_eval_duration(bool reset, bool direct) const
        {
            if (!direct)
            {
                return hpx::util::get_and_reset_value(eval_duration_, reset);
            }
            return hpx::util::get_and_reset_value(eval_direct_duration_, reset);
        }

    protected:
        static std::vector<primitive_argument_type> noargs;
        std::vector<primitive_argument_type> operands_;

        // Performance counter data
        mutable std::int64_t eval_count_;
        mutable std::int64_t eval_duration_;
        mutable std::int64_t eval_direct_count_;
        mutable std::int64_t eval_direct_duration_;
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
    phylanx::execution_tree::primitives::
        base_primitive::expression_topology_action,
    phylanx_primitive_expression_topology_action);

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
    PHYLANX_EXPORT primitive_result_type extract_ref_value(
        primitive_argument_type const& val);
    PHYLANX_EXPORT primitive_result_type extract_copy_value(
        primitive_argument_type const& val);
    PHYLANX_EXPORT primitive_result_type extract_value(
        primitive_result_type && val);
    PHYLANX_EXPORT primitive_result_type extract_ref_value(
        primitive_result_type && val);
    PHYLANX_EXPORT primitive_result_type extract_copy_value(
        primitive_result_type && val);

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
    PHYLANX_EXPORT primitive_result_type extract_literal_value(
        primitive_argument_type const& val);
    PHYLANX_EXPORT primitive_result_type extract_literal_ref_value(
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
    PHYLANX_EXPORT hpx::future<primitive_result_type> value_operand(
        primitive_argument_type const& val,
        std::vector<primitive_argument_type> && args);
    PHYLANX_EXPORT hpx::future<primitive_result_type> value_operand(
        primitive_argument_type && val,
        std::vector<primitive_argument_type> const& args);
    PHYLANX_EXPORT hpx::future<primitive_result_type> value_operand(
        primitive_argument_type && val,
        std::vector<primitive_argument_type> && args);

    namespace functional
    {
        struct value_operand
        {
            template <typename T1, typename T2>
            hpx::future<primitive_result_type> operator()(
                T1&& t1, T2&& t2) const
            {
                return execution_tree::value_operand(
                    std::forward<T1>(t1), std::forward<T2>(t2));
            }
        };
    }

// was declared above
//     PHYLANX_EXPORT primitive_result_type value_operand_sync(
//         primitive_argument_type const& val,
//         std::vector<primitive_argument_type> const& args);

    // Extract a primitive_result_type from a primitive_argument_type (that
    // could be a primitive or a literal value).
    PHYLANX_EXPORT hpx::future<primitive_result_type> literal_operand(
        primitive_argument_type const& val,
        std::vector<primitive_argument_type> const& args);
    PHYLANX_EXPORT hpx::future<primitive_result_type> literal_operand(
        primitive_argument_type const& val,
        std::vector<primitive_argument_type> && args);
    PHYLANX_EXPORT hpx::future<primitive_result_type> literal_operand(
        primitive_argument_type && val,
        std::vector<primitive_argument_type> const& args);
    PHYLANX_EXPORT hpx::future<primitive_result_type> literal_operand(
        primitive_argument_type && val,
        std::vector<primitive_argument_type> && args);

    namespace functional
    {
        struct literal_operand
        {
            template <typename T1, typename T2>
            hpx::future<primitive_result_type> operator()(
                T1&& t1, T2&& t2) const
            {
                return execution_tree::literal_operand(
                    std::forward<T1>(t1), std::forward<T2>(t2));
            }
        };
    }

    PHYLANX_EXPORT primitive_argument_type literal_operand_sync(
        primitive_argument_type const& val,
        std::vector<primitive_argument_type> const& args);

    // Extract a node_data<double> from a primitive_argument_type (that
    // could be a primitive or a literal value).
    PHYLANX_EXPORT hpx::future<ir::node_data<double>> numeric_operand(
        primitive_argument_type const& val,
        std::vector<primitive_argument_type> const& args);
    PHYLANX_EXPORT hpx::future<ir::node_data<double>> numeric_operand(
        primitive_argument_type const& val,
        std::vector<primitive_argument_type> && args);
    PHYLANX_EXPORT hpx::future<ir::node_data<double>> numeric_operand(
        primitive_argument_type && val,
        std::vector<primitive_argument_type> const& args);
    PHYLANX_EXPORT hpx::future<ir::node_data<double>> numeric_operand(
        primitive_argument_type && val,
        std::vector<primitive_argument_type> && args);

    namespace functional
    {
        struct numeric_operand
        {
            template <typename T1, typename T2>
            hpx::future<ir::node_data<double>> operator()(
                T1&& t1, T2&& t2) const
            {
                return execution_tree::numeric_operand(
                    std::forward<T1>(t1), std::forward<T2>(t2));
            }
        };
    }

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

    using match_pattern_type = hpx::util::tuple<std::string,
        std::vector<std::string>, factory_function_type>;

    using pattern_list = std::vector<match_pattern_type>;

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


