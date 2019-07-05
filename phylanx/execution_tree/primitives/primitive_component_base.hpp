//  Copyright (c) 2017-2019 Hartmut Kaiser
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
#include <hpx/util/internal_allocator.hpp>

#include <cstddef>
#include <cstdint>
#include <iosfwd>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <utility>
#include <vector>

#ifdef HPX_HAVE_APEX
#include <phylanx/util/apex_task_inlining_policy.hpp>
#endif

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
                primitive_arguments_type&& params,
                std::string const& name, std::string const& codename,
                bool eval_direct = false);

            virtual ~primitive_component_base() = default;

            // eval_action
            virtual hpx::future<primitive_argument_type> eval(
                primitive_arguments_type const& params, eval_context ctx) const;

            virtual hpx::future<primitive_argument_type> eval(
                primitive_argument_type && param, eval_context ctx) const;

            // eval implementation
            virtual hpx::future<primitive_argument_type> eval(
                primitive_arguments_type const& operands,
                primitive_arguments_type const& args, eval_context ctx) const;

            // store_action
            virtual void store(primitive_arguments_type&&,
                primitive_arguments_type&&, eval_context ctx);

            virtual void store(primitive_argument_type&&,
                primitive_arguments_type&&, eval_context ctx);

            // extract_topology_action
            virtual topology expression_topology(
                std::set<std::string>&& functions,
                std::set<std::string>&& resolve_children) const;

            // bind_action
            virtual bool bind(
                primitive_arguments_type const& params, eval_context ctx) const;

            // initialize evaluation context (used by target-reference only)
            virtual void set_eval_context(eval_context ctx);

        protected:
            friend class primitive_component;

            // helper functions to invoke eval functionalities
            hpx::future<primitive_argument_type> do_eval(
                primitive_arguments_type const& params,
                eval_context ctx) const;

            hpx::future<primitive_argument_type> do_eval(
                primitive_argument_type && param, eval_context ctx) const;

            // access data for performance counter
            std::int64_t get_eval_count(bool reset) const;
            std::int64_t get_eval_duration(bool reset) const;
            std::int64_t get_direct_execution(bool reset) const;

            void enable_measurements();

            // decide whether to execute eval directly
            hpx::launch select_direct_eval_execution(hpx::launch policy) const;

            // A primitive was constructed with no operands if the list of
            // operands is empty or the only provided operand is 'nil' (used
            // for function invocations like 'func()').
            bool no_operands() const
            {
                return operands_.empty();
            }
            primitive_arguments_type const& operands() const
            {
                return operands_.empty() ||
                        (operands_.size() == 1 &&
                            is_implicit_nil(operands_[0])) ?
                    noargs : operands_;
            }

            std::string extract_function_name(std::string const& name);

        protected:
            std::string generate_error_message(std::string const& msg) const;
            static bool get_sync_execution();
            static std::int64_t get_ec_threshold();
            static std::int64_t get_exec_upper_threshold();
            static std::int64_t get_exec_lower_threshold();

        protected:
            static primitive_arguments_type noargs;
            mutable primitive_arguments_type operands_;

            std::string const name_;        // the unique name of this primitive
            std::string const codename_;    // the name of the original code source

            // Performance counter data
            mutable std::int64_t eval_count_;
            mutable std::int64_t eval_duration_;
            mutable std::int64_t execute_directly_;
            bool measurements_enabled_;

#if defined(HPX_HAVE_APEX)
            std::string eval_name_;
#ifdef PHYLANX_HAVE_TASK_INLINING_POLICY
            std::int64_t get_exec_threshold() const;
            std::int64_t get_exec_hysteresis() const;
            std::int64_t exec_threshold_ = 425000;
            std::int64_t exec_hysteresis_ = 75000;

            bool policy_initialized_ = false;

            hpx::launch select_direct_eval_policy_exec_directly(
            hpx::launch policy);

            hpx::launch select_direct_eval_policy_thres(
            hpx::launch policy);

            std::unique_ptr<phylanx::util::apex_inlining_policy>
            inlining_policy_instance;
#endif
#endif
        };
    }

    ///////////////////////////////////////////////////////////////////////////
    // Factory functions
    using factory_function_type = primitive (*)(
        hpx::id_type const&, primitive_arguments_type&&,
        std::string const&, std::string const&);

    using primitive_factory_function_type =
        std::shared_ptr<primitives::primitive_component_base> (*)(
            primitive_arguments_type&&, std::string const&,
            std::string const&);

    struct match_pattern_type
    {
        using match_pattern_data = hpx::util::tuple<std::string,
            std::vector<std::string>, factory_function_type,
            primitive_factory_function_type, std::string>;

        match_pattern_type(match_pattern_data && data)
          : primitive_type_(std::move(hpx::util::get<0>(data)))
          , patterns_(std::move(hpx::util::get<1>(data)))
          , create_primitive_(hpx::util::get<2>(data))
          , create_instance_(hpx::util::get<3>(data))
          , help_string_(std::move(hpx::util::get<4>(data)))
          , supports_dtype_(false)
        {}

        match_pattern_type(char const* primitive_type,
                std::vector<std::string> && patterns,
                factory_function_type create_primitive,
                primitive_factory_function_type create_instance,
                std::string && help_string,
                bool supports_dtype = false)
          : primitive_type_(primitive_type)
          , patterns_(std::move(patterns))
          , create_primitive_(create_primitive)
          , create_instance_(create_instance)
          , help_string_(std::move(help_string))
          , supports_dtype_(supports_dtype)
        {}

        match_pattern_type(char const* primitive_type,
                std::vector<std::string> && patterns,
                factory_function_type create_primitive,
                primitive_factory_function_type create_instance,
                std::string const& help_string,
                bool supports_dtype = false)
          : primitive_type_(primitive_type)
          , patterns_(std::move(patterns))
          , create_primitive_(create_primitive)
          , create_instance_(create_instance)
          , help_string_(help_string)
          , supports_dtype_(supports_dtype)
        {}

        std::string primitive_type_;
        std::vector<std::string> patterns_;
        factory_function_type create_primitive_;
        primitive_factory_function_type create_instance_;
        std::string help_string_;
        bool supports_dtype_;
    };

    struct pattern
    {
        std::string name_;
        match_pattern_type data_;
    };

    using pattern_list = std::vector<pattern>;

    ///////////////////////////////////////////////////////////////////////////
    PHYLANX_EXPORT void register_pattern(std::string const& name,
        match_pattern_type const& pattern, std::string const& fullpath);

    PHYLANX_EXPORT void show_patterns();
    PHYLANX_EXPORT void show_patterns(std::ostream& ostrm);
    PHYLANX_EXPORT std::map<std::string,std::vector<std::string>> list_patterns();
    PHYLANX_EXPORT std::string find_help(std::string const& s);

    ///////////////////////////////////////////////////////////////////////////
    PHYLANX_EXPORT primitive create_primitive_component(
        hpx::id_type const& locality, std::string const& type,
        primitive_arguments_type&& operands,
        std::string const& name = "",
        std::string const& codename = "<unknown>",
        bool register_with_agas = true);

    PHYLANX_EXPORT primitive create_primitive_component(
        hpx::id_type const& locality, std::string const& type,
        primitive_arguments_type&& operands, eval_context ctx,
        std::string const& name = "",
        std::string const& codename = "<unknown>",
        bool register_with_agas = true);

    PHYLANX_EXPORT primitive create_primitive_component(
        hpx::id_type const& locality, std::string const& type,
        primitive_argument_type operand, std::string const& name = "",
        std::string const& codename = "<unknown>",
        bool register_with_agas = true);

    ///////////////////////////////////////////////////////////////////////////
    template <typename Primitive>
    std::shared_ptr<primitives::primitive_component_base>
    create_primitive(primitive_arguments_type&& args,
        std::string const& name, std::string const& codename)
    {
        static hpx::util::internal_allocator<Primitive> alloc_;

        return std::static_pointer_cast<primitives::primitive_component_base>(
            std::allocate_shared<Primitive>(
                alloc_, std::move(args), name, codename));
    }
}}

#endif
