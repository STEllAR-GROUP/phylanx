//  Copyright (c) 2017-2018 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_PRIMITIVES_PRIMITIVE_COMPONENTBASE__FEB_10_2018_0141PM)
#define PHYLANX_PRIMITIVES_PRIMITIVE_COMPONENTBASE__FEB_10_2018_0141PM

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/base_primitive.hpp>

#include <hpx/include/lcos.hpp>
#include <hpx/include/util.hpp>

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

        struct primitive_component_base
        {
            primitive_component_base() = default;

            primitive_component_base(
                std::vector<primitive_argument_type>&& params,
                std::string const& name, std::string const& codename);

            virtual ~primitive_component_base() = default;

            // eval_action
            virtual hpx::future<primitive_argument_type> eval(
                std::vector<primitive_argument_type> const& params) const;

            // direct_eval_action
            virtual primitive_argument_type eval_direct(
                std::vector<primitive_argument_type> const& params) const;

            // store_action
            virtual void store(primitive_argument_type &&);

            // extract_topology_action
            virtual topology expression_topology(
                std::set<std::string>&& functions) const;

            // bind_action
            virtual primitive_argument_type bind(
                std::vector<primitive_argument_type> const& params) const;

            // set_body_action (define_function only)
            virtual void set_body(primitive_argument_type&& target);

        protected:
            friend class primitive_component;

            // helper functions to invoke eval functionalities
            hpx::future<primitive_argument_type> do_eval(
                std::vector<primitive_argument_type> const& params) const;
            primitive_argument_type do_eval_direct(
                std::vector<primitive_argument_type> const& params) const;

            // access data for performance counter
            std::int64_t get_eval_count(bool reset, bool direct) const;
            std::int64_t get_eval_duration(bool reset, bool direct) const;

        protected:
            std::string generate_error_message(std::string const& msg) const;

        protected:
            static std::vector<primitive_argument_type> noargs;
            mutable std::vector<primitive_argument_type> operands_;

            std::string name_;          // the unique name of this primitive
            std::string codename_;      // the name of the original code source

            // Performance counter data
            mutable std::int64_t eval_count_;
            mutable std::int64_t eval_duration_;
            mutable std::int64_t eval_direct_count_;
            mutable std::int64_t eval_direct_duration_;

#if defined(HPX_HAVE_APEX)
            std::string eval_name_;
            std::string eval_direct_name_;
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

    using pattern_list = std::vector<match_pattern_type>;

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
        std::string const& name, std::string const& codename);

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
