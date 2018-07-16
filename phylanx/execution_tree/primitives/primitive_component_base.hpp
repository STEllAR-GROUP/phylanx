//  Copyright (c) 2017-2018 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_PRIMITIVES_PRIMITIVE_COMPONENTBASE__FEB_10_2018_0141PM)
#define PHYLANX_PRIMITIVES_PRIMITIVE_COMPONENTBASE__FEB_10_2018_0141PM

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/compiler/primitive_name.hpp>
#include <phylanx/execution_tree/primitives/base_primitive.hpp>

#include <hpx/include/lcos.hpp>
#include <hpx/include/util.hpp>
#include <hpx/runtime/launch_policy.hpp>
#include <hpx/runtime/naming_fwd.hpp>

#include <cstddef>
#include <cstdint>
#include <memory>
#include <set>
#include <string>
#include <utility>
#include <vector>

namespace phylanx { namespace execution_tree
{
    ///////////////////////////////////////////////////////////////////////////
    struct primitive_argument_type;
    class primitive;

    ///////////////////////////////////////////////////////////////////////////
    namespace primitives
    {
        class primitive_component;

        struct PHYLANX_EXPORT primitive_component_base
        {
            primitive_component_base() = default;

            primitive_component_base(
                std::vector<primitive_argument_type>&& params,
                std::string const& name, std::string const& codename,
                bool eval_direct = false);

            virtual ~primitive_component_base() = default;

            // eval_action
            virtual hpx::future<primitive_argument_type> eval(
                std::vector<primitive_argument_type> const& params,
                eval_mode mode) const;

            // store_action
            virtual void store(primitive_argument_type&&);
            virtual void store_set_1d(
                phylanx::ir::node_data<double>&&, std::vector<int64_t>&&);
            virtual void store_set_2d(phylanx::ir::node_data<double>&&,
                std::vector<int64_t>&&, std::vector<int64_t>&&);

            // extract_topology_action
            virtual topology expression_topology(
                std::set<std::string>&& functions,
                std::set<std::string>&& resolve_children) const;

            // bind_action
            virtual bool bind(
                std::vector<primitive_argument_type> const& params) const;

        protected:
            friend class primitive_component;

            // helper functions to invoke eval functionalities
            hpx::future<primitive_argument_type> do_eval(
                std::vector<primitive_argument_type> const& params,
                eval_mode mode) const;

            virtual hpx::future<primitive_argument_type> eval(
                std::vector<primitive_argument_type> const& params) const;

            // access data for performance counter
            std::int64_t get_eval_count(bool reset) const;
            std::int64_t get_eval_duration(bool reset) const;
            std::int64_t get_direct_execution(bool reset) const;

            // decide whether to execute eval directly
            hpx::launch select_direct_eval_execution(hpx::launch policy) const;

            // A primitive was constructed with no operands if the list of
            // operands is empty or the only provided operand is 'nil' (used
            // for function invocations like 'func()').
            bool no_operands() const
            {
                return operands_.empty();
            }
            std::vector<primitive_argument_type> const& operands() const
            {
                return operands_.empty() ||
                        (operands_.size() == 1 && !valid(operands_[0])) ?
                    noargs : operands_;
            }

        protected:
            std::string generate_error_message(std::string const& msg) const;
            static bool get_sync_execution();

        protected:
            static std::vector<primitive_argument_type> noargs;
            mutable std::vector<primitive_argument_type> operands_;

            std::string const name_;        // the unique name of this primitive
            std::string const codename_;    // the name of the original code source

            // Performance counter data
            mutable std::int64_t eval_count_;
            mutable std::int64_t eval_duration_;
            mutable std::int64_t execute_directly_;

#if defined(HPX_HAVE_APEX)
            std::string eval_name_;
#endif
        };
    }

    ///////////////////////////////////////////////////////////////////////////
    // Factory functions
    using factory_function_type = primitive (*)(
        hpx::id_type const&, std::vector<primitive_argument_type>&&,
        std::string const&, std::string const&);

    using primitive_factory_function_type =
        std::shared_ptr<primitives::primitive_component_base> (*)(
            std::vector<primitive_argument_type>&&, std::string const&,
            std::string const&);

    using match_pattern_type = hpx::util::tuple<std::string,
        std::vector<std::string>, factory_function_type,
        primitive_factory_function_type>;

    using pattern_list =
        std::vector<hpx::util::tuple<std::string, match_pattern_type>>;

    ///////////////////////////////////////////////////////////////////////////
    PHYLANX_EXPORT void register_pattern(
        std::string const&, match_pattern_type const& pattern);

    PHYLANX_EXPORT void show_patterns();

    ///////////////////////////////////////////////////////////////////////////
    PHYLANX_EXPORT primitive create_primitive_component(
        hpx::id_type const& locality, std::string const& type,
        std::vector<primitive_argument_type>&& operands,
        std::string const& name = "",
        std::string const& codename = "<unknown>");

    PHYLANX_EXPORT primitive create_primitive_component(
        hpx::id_type const& locality, std::string const& type,
        primitive_argument_type operand, std::string const& name = "",
        std::string const& codename = "<unknown>");

    ///////////////////////////////////////////////////////////////////////////
    PHYLANX_EXPORT std::string generate_error_message(std::string const& msg,
        std::string const& name = "", std::string const& codename = "");
    PHYLANX_EXPORT std::string generate_error_message(std::string const& msg,
        compiler::primitive_name_parts const& parts,
        std::string const& codename = "");

    ///////////////////////////////////////////////////////////////////////////
    template <typename Primitive>
    std::shared_ptr<primitives::primitive_component_base>
    create_primitive(std::vector<primitive_argument_type>&& args,
        std::string const& name, std::string const& codename)
    {
        return std::static_pointer_cast<primitives::primitive_component_base>(
            std::make_shared<Primitive>(std::move(args), name, codename));
    }
}}

#endif
