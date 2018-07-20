// Copyright (c) 2017-2018 Hartmut Kaiser
// Copyright (c) 2018 Parsa Amini
// Copyright (c) 2018 Tianyi Zhang
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_PRIMITIVES_BASE_PRIMITIVE_SEP_05_2017_1102AM)
#define PHYLANX_PRIMITIVES_BASE_PRIMITIVE_SEP_05_2017_1102AM

#include <phylanx/config.hpp>
#include <phylanx/ast/node.hpp>
#include <phylanx/execution_tree/compiler/primitive_name.hpp>
#include <phylanx/ir/node_data.hpp>
#include <phylanx/ir/ranges.hpp>

#include <hpx/include/runtime.hpp>
#include <hpx/include/util.hpp>
#include <hpx/include/serialization.hpp>

#include <array>
#include <cstddef>
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

    PHYLANX_EXPORT bool is_primitive_operand(
        primitive_argument_type const& val);

    ///////////////////////////////////////////////////////////////////////////
    struct primitive_argument_type;

    ///////////////////////////////////////////////////////////////////////////
    enum eval_mode
    {
        eval_default = 0x00,                // always evaluate everything
        eval_dont_wrap_functions = 0x01,    // don't wrap partially bound functions
        eval_dont_evaluate_partials = 0x02, // don't evaluate partially bound functions
        eval_dont_evaluate_lambdas = 0x04   // don't evaluate functions
    };

    class primitive
      : public hpx::components::client_base<primitive,
            primitives::primitive_component>
    {
    private:
        using base_type = hpx::components::client_base<primitive,
            primitives::primitive_component>;

    public:
        primitive() = default;

        primitive(hpx::id_type && id)
          : base_type(std::move(id))
        {
        }
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

        PHYLANX_EXPORT hpx::future<primitive_argument_type> eval(
            eval_mode mode = eval_default) const;
        PHYLANX_EXPORT hpx::future<primitive_argument_type> eval(
            std::vector<primitive_argument_type> && args,
            eval_mode mode = eval_default) const;
        PHYLANX_EXPORT hpx::future<primitive_argument_type> eval(
            std::vector<primitive_argument_type> const& args,
            eval_mode mode = eval_default) const;

        PHYLANX_EXPORT primitive_argument_type eval(hpx::launch::sync_policy,
            eval_mode mode = eval_default) const;
        PHYLANX_EXPORT primitive_argument_type eval(hpx::launch::sync_policy,
            std::vector<primitive_argument_type>&& args,
            eval_mode mode = eval_default) const;
        PHYLANX_EXPORT primitive_argument_type eval(hpx::launch::sync_policy,
            std::vector<primitive_argument_type> const& args,
            eval_mode mode = eval_default) const;

        PHYLANX_EXPORT hpx::future<void> store(primitive_argument_type);
        PHYLANX_EXPORT void store(hpx::launch::sync_policy,
            primitive_argument_type);

        PHYLANX_EXPORT hpx::future<void> store_set_1d(
            ir::node_data<double>, std::vector<int64_t>);
        PHYLANX_EXPORT void store_set_1d(hpx::launch::sync_policy,
            ir::node_data<double>, std::vector<int64_t>);

        PHYLANX_EXPORT hpx::future<void> store_set_2d(
            ir::node_data<double>, std::vector<int64_t>, std::vector<int64_t>);
        PHYLANX_EXPORT void store_set_2d(hpx::launch::sync_policy,
            ir::node_data<double>, std::vector<int64_t>, std::vector<int64_t>);

        PHYLANX_EXPORT hpx::future<topology> expression_topology(
            std::set<std::string>&& functions) const;
        PHYLANX_EXPORT topology expression_topology(hpx::launch::sync_policy,
            std::set<std::string>&& functions) const;

        PHYLANX_EXPORT hpx::future<topology> expression_topology(
            std::set<std::string>&& functions,
            std::set<std::string>&& resolve_children) const;
        PHYLANX_EXPORT topology expression_topology(hpx::launch::sync_policy,
            std::set<std::string>&& functions,
            std::set<std::string>&& resolve_children) const;

        PHYLANX_EXPORT bool bind(
            std::vector<primitive_argument_type>&& args) const;
        PHYLANX_EXPORT bool bind(
            std::vector<primitive_argument_type> const& args) const;

    public:
        static bool enable_tracing;
    };

    ///////////////////////////////////////////////////////////////////////////
    using argument_value_type =
        phylanx::util::variant<
            ast::nil
          , phylanx::ir::node_data<std::uint8_t>
          , phylanx::ir::node_data<std::int64_t>
          , std::string
          , phylanx::ir::node_data<double>
          , primitive
          , std::vector<ast::expression>
          , ir::range
        >;

    PHYLANX_EXPORT primitive_argument_type extract_copy_value(
        primitive_argument_type const& val,
        std::string const& name = "",
        std::string const& codename = "<unknown>");

    struct primitive_argument_type : argument_value_type
    {
        primitive_argument_type() = default;

        primitive_argument_type(ast::nil val)
          : argument_value_type{val}
        {}

        explicit primitive_argument_type(bool val)
          : argument_value_type{phylanx::ir::node_data<std::uint8_t>{val}}
        {}
        explicit primitive_argument_type(std::uint8_t val)
          : argument_value_type{phylanx::ir::node_data<std::uint8_t>{val}}
        {}
        explicit primitive_argument_type(
                blaze::DynamicVector<std::uint8_t> const& val)
          : argument_value_type{phylanx::ir::node_data<std::uint8_t>{val}}
        {}
        explicit primitive_argument_type(blaze::DynamicVector<std::uint8_t>&& val)
          : argument_value_type{phylanx::ir::node_data<std::uint8_t>{std::move(val)}}
        {}
        explicit primitive_argument_type(
                blaze::DynamicMatrix<std::uint8_t> const& val)
          : argument_value_type{phylanx::ir::node_data<std::uint8_t>{val}}
        {}
        explicit primitive_argument_type(blaze::DynamicMatrix<std::uint8_t>&& val)
          : argument_value_type{phylanx::ir::node_data<std::uint8_t>{std::move(val)}}
        {}
        primitive_argument_type(phylanx::ir::node_data<std::uint8_t> const& val)
          : argument_value_type{val}
        {}

        primitive_argument_type(phylanx::ir::node_data<std::uint8_t>&& val)
          : argument_value_type{std::move(val)}
        {}

        explicit primitive_argument_type(std::int64_t val)
          : argument_value_type{phylanx::ir::node_data<std::int64_t>{val}}
        {
        }
        explicit primitive_argument_type(
            blaze::DynamicVector<std::int64_t> const& val)
          : argument_value_type{phylanx::ir::node_data<std::int64_t>{val}}
        {
        }
        explicit primitive_argument_type(
            blaze::DynamicVector<std::int64_t>&& val)
          : argument_value_type{
                phylanx::ir::node_data<std::int64_t>{std::move(val)}}
        {
        }
        explicit primitive_argument_type(
            blaze::DynamicMatrix<std::int64_t> const& val)
          : argument_value_type{phylanx::ir::node_data<std::int64_t>{val}}
        {
        }
        explicit primitive_argument_type(
            blaze::DynamicMatrix<std::int64_t>&& val)
          : argument_value_type{
                phylanx::ir::node_data<std::int64_t>{std::move(val)}}
        {
        }

        primitive_argument_type(phylanx::ir::node_data<std::int64_t> const& val)
          : argument_value_type{val}
        {
        }
        primitive_argument_type(phylanx::ir::node_data<std::int64_t>&& val)
          : argument_value_type{std::move(val)}
        {
        }

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

        explicit primitive_argument_type(
            std::vector<primitive_argument_type> const& val)
          : argument_value_type{ir::range{val}}
        {}

        explicit primitive_argument_type(
            std::vector<primitive_argument_type>&& val)
          : argument_value_type{ir::range{std::move(val)}}
        {}

        primitive_argument_type(ir::range const& val)
          : argument_value_type{val}
        {}
        primitive_argument_type(ir::range&& val)
          : argument_value_type{std::move(val)}
        {}

        primitive_argument_type(argument_value_type const& val)
          : argument_value_type{val}
        {}
        primitive_argument_type(argument_value_type&& val)
          : argument_value_type{std::move(val)}
        {}

        inline primitive_argument_type run() const;

        PHYLANX_EXPORT primitive_argument_type operator()() const;

        PHYLANX_EXPORT primitive_argument_type
        operator()(std::vector<primitive_argument_type> const& args) const;

        PHYLANX_EXPORT primitive_argument_type
        operator()(std::vector<primitive_argument_type> && args) const;

        template <typename ... Ts>
        primitive_argument_type operator()(Ts &&... ts) const
        {
            std::vector<primitive_argument_type> args;
            args.reserve(sizeof...(Ts));

            int const sequencer_[] = {
                0, (args.emplace_back(
                        extract_copy_value(primitive_argument_type{
                            std::forward<Ts>(ts)
                        })), 0)...
            };
            (void)sequencer_;

            return (*this)(std::move(args));
        }

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

    PHYLANX_EXPORT std::string to_string(primitive_argument_type const&);

    PHYLANX_EXPORT std::ostream& operator<<(std::ostream& os,
        primitive const&);

    ///////////////////////////////////////////////////////////////////////////
    PHYLANX_EXPORT primitive_argument_type value_operand_sync(
        primitive_argument_type const& val,
        std::vector<primitive_argument_type> const& args,
        std::string const& name = "",
        std::string const& codename = "<unknown>",
        eval_mode mode = eval_default);
    PHYLANX_EXPORT primitive_argument_type value_operand_sync(
        primitive_argument_type const& val,
        std::vector<primitive_argument_type>&& args,
        std::string const& name = "",
        std::string const& codename = "<unknown>",
        eval_mode mode = eval_default);
    PHYLANX_EXPORT primitive_argument_type value_operand_sync(
        primitive_argument_type&& val,
        std::vector<primitive_argument_type> const& args,
        std::string const& name = "",
        std::string const& codename = "<unknown>",
        eval_mode mode = eval_default);
    PHYLANX_EXPORT primitive_argument_type value_operand_sync(
        primitive_argument_type&& val,
        std::vector<primitive_argument_type>&& args,
        std::string const& name = "",
        std::string const& codename = "<unknown>",
        eval_mode mode = eval_default);

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
    inline primitive_argument_type primitive_argument_type::run() const
    {
        return value_operand_sync(*this, {});
    }

    ///////////////////////////////////////////////////////////////////////////
    PHYLANX_EXPORT primitive_argument_type to_primitive_value_type(
        ast::literal_value_type && val);
    PHYLANX_EXPORT ir::node_data<double> to_primitive_numeric_type(
        ast::literal_value_type && val);
    PHYLANX_EXPORT std::string to_primitive_string_type(
        ast::literal_value_type && val);
    PHYLANX_EXPORT ir::node_data<std::int64_t> to_primitive_int_type(
        ast::literal_value_type && val);
    PHYLANX_EXPORT ir::node_data<std::uint8_t> to_primitive_bool_type(
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

    PHYLANX_EXPORT bool is_literal_operand(primitive_argument_type const& val);

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

    PHYLANX_EXPORT ir::node_data<double> extract_numeric_value_strict(
        primitive_argument_type const& val,
        std::string const& name = "",
        std::string const& codename = "<unknown>");
    PHYLANX_EXPORT ir::node_data<double> extract_numeric_value_strict(
        primitive_argument_type && val,
        std::string const& name = "",
        std::string const& codename = "<unknown>");

    PHYLANX_EXPORT bool is_numeric_operand(primitive_argument_type const& val);
    PHYLANX_EXPORT bool is_numeric_operand_strict(
        primitive_argument_type const& val);

    PHYLANX_EXPORT std::size_t extract_numeric_value_dimension(
        primitive_argument_type const& val,
        std::string const& name = "",
        std::string const& codename = "<unknown>");

    PHYLANX_EXPORT std::array<std::size_t, 2> extract_numeric_value_dimensions(
        primitive_argument_type const& val,
        std::string const& name = "",
        std::string const& codename = "<unknown>");

    // Extract a ir::node_data<std::uint8_t> type from a given primitive_argument_type,
    // throw if it doesn't hold one.
    PHYLANX_EXPORT ir::node_data<std::uint8_t> extract_boolean_data(
        primitive_argument_type const& val,
        std::string const& name = "",
        std::string const& codename = "<unknown>");
    PHYLANX_EXPORT ir::node_data<std::uint8_t> extract_boolean_data(
        primitive_argument_type&& val,
        std::string const& name = "",
        std::string const& codename = "<unknown>");

    PHYLANX_EXPORT bool is_boolean_data_operand(
        primitive_argument_type const& val);

    template <typename T>
    ir::node_data<T> extract_node_data(
        primitive_argument_type const& val,
        std::string const& name = "",
        std::string const& codename = "<unknown>");

    template <>
    inline ir::node_data<double> extract_node_data(
        primitive_argument_type const& val,
        std::string const& name,
        std::string const& codename)
    {
        return extract_numeric_value(val, name, codename);
    }
    template <>
    inline ir::node_data<std::uint8_t> extract_node_data(
        primitive_argument_type const& val,
        std::string const& name,
        std::string const& codename)
    {
        return extract_boolean_data(val, name, codename);
    }

    template <typename T>
    ir::node_data<T> extract_node_data(
        primitive_argument_type && val,
        std::string const& name = "",
        std::string const& codename = "<unknown>");

    template <>
    inline ir::node_data<double> extract_node_data(
        primitive_argument_type && val,
        std::string const& name,
        std::string const& codename)
    {
        return extract_numeric_value(std::move(val), name, codename);
    }
    template <>
    inline ir::node_data<std::uint8_t> extract_node_data(
        primitive_argument_type && val,
        std::string const& name,
        std::string const& codename)
    {
        return extract_boolean_data(std::move(val), name, codename);
    }

    ///////////////////////////////////////////////////////////////////////////
    // Extract a ir::node_data<std::int64_t> type from a given primitive_argument_type,
    // throw if it doesn't hold one.
    PHYLANX_EXPORT ir::node_data<std::int64_t> extract_integer_value(
        primitive_argument_type const& val,
        std::string const& name = "",
        std::string const& codename = "<unknown>");
    PHYLANX_EXPORT ir::node_data<std::int64_t> extract_integer_value(
        primitive_argument_type && val,
        std::string const& name = "",
        std::string const& codename = "<unknown>");
    PHYLANX_EXPORT std::int64_t extract_scalar_integer_value(
        primitive_argument_type const& val,
        std::string const& name = "",
        std::string const& codename = "<unknown>");
    PHYLANX_EXPORT std::int64_t extract_scalar_integer_value(
        primitive_argument_type && val,
        std::string const& name = "",
        std::string const& codename = "<unknown>");

    // Extract an integer value from a primitive_argument_type
    PHYLANX_EXPORT hpx::future<ir::node_data<std::int64_t>> integer_operand(
        primitive_argument_type const& val,
        std::vector<primitive_argument_type> const& args,
        std::string const& name = "",
        std::string const& codename = "<unknown>");
    PHYLANX_EXPORT hpx::future<ir::node_data<std::int64_t>> integer_operand(
        primitive_argument_type const& val,
        std::vector<primitive_argument_type> && args,
        std::string const& name = "",
        std::string const& codename = "<unknown>");
    PHYLANX_EXPORT hpx::future<ir::node_data<std::int64_t>> integer_operand(
        primitive_argument_type && val,
        std::vector<primitive_argument_type> const& args,
        std::string const& name = "",
        std::string const& codename = "<unknown>");
    PHYLANX_EXPORT hpx::future<ir::node_data<std::int64_t>> integer_operand(
        primitive_argument_type && val,
        std::vector<primitive_argument_type> && args,
        std::string const& name = "",
        std::string const& codename = "<unknown>");

    namespace functional
    {
        struct integer_operand
        {
            template <typename... Ts>
            hpx::future<ir::node_data<std::int64_t>> operator()(Ts&&... ts) const
            {
                return execution_tree::integer_operand(std::forward<Ts>(ts)...);
            }
        };
    }

    PHYLANX_EXPORT bool is_integer_operand(primitive_argument_type const& val);

    // Extract a ir::node_data<std::int64_t> type from a given primitive_argument_type,
    // throw if it doesn't hold one.
    PHYLANX_EXPORT ir::node_data<std::int64_t> extract_integer_value_strict(
        primitive_argument_type const& val,
        std::string const& name = "",
        std::string const& codename = "<unknown>");
    PHYLANX_EXPORT ir::node_data<std::int64_t> extract_integer_value_strict(
        primitive_argument_type && val,
        std::string const& name = "",
        std::string const& codename = "<unknown>");
    PHYLANX_EXPORT std::int64_t extract_scalar_integer_value_strict(
        primitive_argument_type const& val,
        std::string const& name = "",
        std::string const& codename = "<unknown>");
    PHYLANX_EXPORT std::int64_t extract_scalar_integer_value_strict(
        primitive_argument_type && val,
        std::string const& name = "",
        std::string const& codename = "<unknown>");

    // Extract an integer value from a primitive_argument_type
    PHYLANX_EXPORT hpx::future<ir::node_data<std::int64_t>> integer_operand_strict(
        primitive_argument_type const& val,
        std::vector<primitive_argument_type> const& args,
        std::string const& name = "",
        std::string const& codename = "<unknown>");
    PHYLANX_EXPORT hpx::future<std::int64_t> scalar_integer_operand_strict(
            primitive_argument_type const& val,
            std::vector<primitive_argument_type> const& args,
            std::string const& name = "",
            std::string const& codename = "<unknown>");
    PHYLANX_EXPORT hpx::future<ir::node_data<std::int64_t>> integer_operand_strict(
        primitive_argument_type const& val,
        std::vector<primitive_argument_type> && args,
        std::string const& name = "",
        std::string const& codename = "<unknown>");
    PHYLANX_EXPORT hpx::future<ir::node_data<std::int64_t>> integer_operand_strict(
        primitive_argument_type && val,
        std::vector<primitive_argument_type> const& args,
        std::string const& name = "",
        std::string const& codename = "<unknown>");
    PHYLANX_EXPORT hpx::future<ir::node_data<std::int64_t>> integer_operand_strict(
        primitive_argument_type && val,
        std::vector<primitive_argument_type> && args,
        std::string const& name = "",
        std::string const& codename = "<unknown>");

    namespace functional
    {
        struct integer_operand_strict
        {
            template <typename... Ts>
            hpx::future<ir::node_data<std::int64_t>> operator()(Ts&&... ts) const
            {
                return execution_tree::integer_operand(std::forward<Ts>(ts)...);
            }
        };
    }

    PHYLANX_EXPORT bool is_integer_operand_strict(
        primitive_argument_type const& val);

    ///////////////////////////////////////////////////////////////////////////
    // Extract a boolean type from a given primitive_argument_type,
    // throw if it doesn't hold one.
    PHYLANX_EXPORT ir::node_data<std::uint8_t> extract_boolean_value(
        primitive_argument_type const& val,
        std::string const& name = "",
        std::string const& codename = "<unknown>");
    PHYLANX_EXPORT ir::node_data<std::uint8_t> extract_boolean_value(
        primitive_argument_type && val,
        std::string const& name = "",
        std::string const& codename = "<unknown>");

    PHYLANX_EXPORT ir::node_data<std::uint8_t> extract_boolean_value_strict(
        primitive_argument_type const& val,
        std::string const& name = "",
        std::string const& codename = "<unknown>");
    PHYLANX_EXPORT ir::node_data<std::uint8_t> extract_boolean_value_strict(
        primitive_argument_type && val,
        std::string const& name = "",
        std::string const& codename = "<unknown>");

    PHYLANX_EXPORT std::uint8_t extract_scalar_boolean_value(
        primitive_argument_type const& val,
        std::string const& name = "",
        std::string const& codename = "<unknown>");
    PHYLANX_EXPORT std::uint8_t extract_scalar_boolean_value(
        primitive_argument_type && val,
        std::string const& name = "",
        std::string const& codename = "<unknown>");

    PHYLANX_EXPORT bool is_boolean_operand(primitive_argument_type const& val);
    PHYLANX_EXPORT bool is_boolean_operand_strict(
        primitive_argument_type const& val);

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

    PHYLANX_EXPORT bool is_string_operand(primitive_argument_type const& val);

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

    PHYLANX_EXPORT bool is_ast_operand(primitive_argument_type const& val);

    // Extract a list type from a given primitive_argument_type,
    // Create a list from argument if it does not hold one.
    PHYLANX_EXPORT ir::range extract_list_value(
        primitive_argument_type const& val,
        std::string const& name = "",
        std::string const& codename = "<unknown>");
    PHYLANX_EXPORT ir::range extract_list_value(
        primitive_argument_type && val,
        std::string const& name = "",
        std::string const& codename = "<unknown>");

    PHYLANX_EXPORT bool is_list_operand(primitive_argument_type const& val);

    // Extract a list type from a given primitive_argument_type,
    // throw if it doesn't hold one.
    PHYLANX_EXPORT ir::range
    extract_list_value_strict(primitive_argument_type const& val,
        std::string const& name = "",
        std::string const& codename = "<unknown>");
    PHYLANX_EXPORT ir::range
    extract_list_value_strict(primitive_argument_type&& val,
        std::string const& name = "",
        std::string const& codename = "<unknown>");

    PHYLANX_EXPORT bool is_list_operand_strict(
        primitive_argument_type const& val);

    ///////////////////////////////////////////////////////////////////////////
    // Extract a primitive from a given primitive_argument_type, throw
    // if it doesn't hold one.
    PHYLANX_EXPORT primitive primitive_operand(
        primitive_argument_type const& val,
        std::string const& name = "",
        std::string const& codename = "<unknown>");
    PHYLANX_EXPORT primitive primitive_operand(
        primitive_argument_type const& val,
        compiler::primitive_name_parts const& parts,
        std::string const& codename = "<unknown>");

    // Extract a primitive_argument_type from a primitive_argument_type (that
    // could be a value type).
    PHYLANX_EXPORT hpx::future<primitive_argument_type> value_operand(
        primitive_argument_type const& val,
        std::vector<primitive_argument_type> const& args,
        std::string const& name = "", std::string const& codename = "<unknown>",
        eval_mode mode = eval_default);
    PHYLANX_EXPORT hpx::future<primitive_argument_type> value_operand(
        primitive_argument_type const& val,
        std::vector<primitive_argument_type>&& args,
        std::string const& name = "", std::string const& codename = "<unknown>",
        eval_mode mode = eval_default);
    PHYLANX_EXPORT hpx::future<primitive_argument_type> value_operand(
        primitive_argument_type&& val,
        std::vector<primitive_argument_type> const& args,
        std::string const& name = "", std::string const& codename = "<unknown>",
        eval_mode mode = eval_default);
    PHYLANX_EXPORT hpx::future<primitive_argument_type> value_operand(
        primitive_argument_type&& val,
        std::vector<primitive_argument_type>&& args,
        std::string const& name = "", std::string const& codename = "<unknown>",
        eval_mode mode = eval_default);

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
    PHYLANX_EXPORT hpx::future<ir::range> list_operand(
        primitive_argument_type const& val,
        std::vector<primitive_argument_type> const& args,
        std::string const& name = "",
        std::string const& codename = "<unknown>");
    PHYLANX_EXPORT hpx::future<ir::range> list_operand(
        primitive_argument_type && val,
        std::vector<primitive_argument_type> const& args,
        std::string const& name = "",
        std::string const& codename = "<unknown>");
    PHYLANX_EXPORT hpx::future<ir::range> list_operand(
        primitive_argument_type const& val,
        std::vector<primitive_argument_type> && args,
        std::string const& name = "",
        std::string const& codename = "<unknown>");
    PHYLANX_EXPORT hpx::future<ir::range> list_operand(
        primitive_argument_type && val,
        std::vector<primitive_argument_type> && args,
        std::string const& name = "",
        std::string const& codename = "<unknown>");

    namespace functional
    {
        struct list_operand
        {
            template <typename... Ts>
            hpx::future<ir::range> operator()(
                Ts&&... ts) const
            {
                return execution_tree::list_operand(std::forward<Ts>(ts)...);
            }
        };
    }

    PHYLANX_EXPORT ir::range list_operand_sync(
        primitive_argument_type const& val,
        std::vector<primitive_argument_type> const& args,
        std::string const& name = "",
        std::string const& codename = "<unknown>");

    // Extract a list from a primitive_argument_type (that
    // could be a primitive or a literal value).
    PHYLANX_EXPORT hpx::future<ir::range> list_operand_strict(
        primitive_argument_type const& val,
        std::vector<primitive_argument_type> const& args,
        std::string const& name = "",
        std::string const& codename = "<unknown>");
    PHYLANX_EXPORT hpx::future<ir::range> list_operand_strict(
        primitive_argument_type && val,
        std::vector<primitive_argument_type> const& args,
        std::string const& name = "",
        std::string const& codename = "<unknown>");
    PHYLANX_EXPORT hpx::future<ir::range> list_operand_strict(
        primitive_argument_type const& val,
        std::vector<primitive_argument_type> && args,
        std::string const& name = "",
        std::string const& codename = "<unknown>");
    PHYLANX_EXPORT hpx::future<ir::range> list_operand_strict(
        primitive_argument_type && val,
        std::vector<primitive_argument_type> && args,
        std::string const& name = "",
        std::string const& codename = "<unknown>");

    namespace functional
    {
        struct list_operand_strict
        {
            template <typename... Ts>
            hpx::future<ir::range> operator()(
                Ts&&... ts) const
            {
                return execution_tree::list_operand_strict(
                    std::forward<Ts>(ts)...);
            }
        };
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


