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

#include <string>
#include <utility>
#include <vector>

namespace phylanx { namespace execution_tree
{
    class HPX_COMPONENT_EXPORT primitive;

    using primitive_result_type = ast::literal_value_type;
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
    using primitive_argument_type = phylanx::util::variant<
            phylanx::ast::nil
          , bool
          , std::int64_t
          , std::string
          , phylanx::ir::node_data<double>
          , primitive
        >;

    ///////////////////////////////////////////////////////////////////////////
    // a literal value is valid of its not nil{}
    inline bool valid(primitive_argument_type const& val)
    {
        return val.index() != 0;
    }
    inline bool valid(primitive_argument_type && val)
    {
        return val.index() != 0;
    }

    using ast::valid;

    ///////////////////////////////////////////////////////////////////////////
    PHYLANX_EXPORT primitive_argument_type to_primitive_value_type(
        primitive_result_type && val);

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
        primitive_result_type const& val);

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

    ///////////////////////////////////////////////////////////////////////////
    // Factory functions
    using factory_function_type =
        hpx::util::function_nonser<
            primitive(hpx::id_type, std::vector<primitive_argument_type>&&)
        >;

    template <typename Primitive>
    primitive create(hpx::id_type locality,
        std::vector<primitive_argument_type>&& operands)
    {
        return primitive(hpx::new_<Primitive>(locality, std::move(operands)));
    }

    ///////////////////////////////////////////////////////////////////////////
    using match_pattern_type = std::pair<std::string, factory_function_type>;
    using pattern_list = std::vector<match_pattern_type>;
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


