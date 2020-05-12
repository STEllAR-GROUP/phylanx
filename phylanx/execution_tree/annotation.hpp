//  Copyright (c) 2019 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_PRIMITIVES_ANNOTATION_JUN_18_2019_1002AM)
#define PHYLANX_PRIMITIVES_ANNOTATION_JUN_18_2019_1002AM

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/primitive_argument_type.hpp>
#include <phylanx/ir/ranges.hpp>

#include <hpx/serialization/serialization_fwd.hpp>

#include <cstdint>
#include <iosfwd>
#include <memory>
#include <string>
#include <utility>

namespace phylanx { namespace execution_tree {
    ////////////////////////////////////////////////////////////////////////////
    struct annotation;

    PHYLANX_EXPORT primitive_argument_type as_primitive_argument_type(
        annotation&& ann);
    PHYLANX_EXPORT primitive_argument_type as_primitive_argument_type(
        annotation const& ann);

    PHYLANX_EXPORT bool operator==(
        annotation const& lhs, annotation const& rhs);
    bool operator!=(
        annotation const& lhs, annotation const& rhs);

    ////////////////////////////////////////////////////////////////////////////
    struct annotation
    {
    public:
        annotation() = default;

        explicit annotation(ir::range const& rhs)
          : data_(rhs)
        {
        }
        explicit annotation(ir::range&& rhs)
          : data_(std::move(rhs))
        {
        }

        template <typename... Ts>
        annotation(char const* key, Ts&&... ts)
          : data_(key, as_primitive_argument_type(std::forward<Ts>(ts))...)
        {
        }
        template <typename... Ts>
        annotation(std::string const& key, Ts&&... ts)
          : data_(key, as_primitive_argument_type(std::forward<Ts>(ts))...)
        {
        }

        annotation& operator=(ir::range const& rhs)
        {
            data_ = rhs;
            return *this;
        }
        annotation& operator=(ir::range&& rhs)
        {
            data_ = std::move(rhs);
            return *this;
        }

        ////////////////////////////////////////////////////////////////////////
        PHYLANX_EXPORT std::string get_type(std::string const& name = "",
            std::string const& codename = "<unknown>") const;
        PHYLANX_EXPORT ir::range get_data() const;

        ir::range& get_range()
        {
            return data_;
        }
        ir::range const& get_range() const
        {
            return data_;
        }
        ir::range get_range_ref() const
        {
            return data_.ref();
        }

        PHYLANX_EXPORT bool find(std::string const& key, annotation& ann,
            std::string const& name = "",
            std::string const& codename = "<unknown>") const;
        PHYLANX_EXPORT bool has_key(std::string const& key,
            std::string const& name = "",
            std::string const& codename = "<unknown>") const;

        PHYLANX_EXPORT void add_annotation(std::string const& key,
            ir::range&& data, std::string const& name,
            std::string const& codename);
        PHYLANX_EXPORT void add_annotation(annotation&& data,
            std::string const& name, std::string const& codename);

        PHYLANX_EXPORT void replace_annotation(annotation&& data,
            std::string const& name, std::string const& codename);
        PHYLANX_EXPORT void replace_annotation(std::string const& key,
            annotation&& data, std::string const& name,
            std::string const& codename);

        PHYLANX_EXPORT bool get_if(std::string const& key,
            execution_tree::annotation& ann, std::string const& name = "",
            std::string const& codename = "<unknown>") const;

        PHYLANX_EXPORT friend bool operator==(annotation const& lhs,
            annotation const& rhs);

        friend bool operator!=(annotation const& lhs, annotation const& rhs)
        {
            return !(lhs == rhs);
        }

        PHYLANX_EXPORT void increment_generation(
            std::string const& name, std::string const& codename);

    private:
        ir::range data_;

        friend class hpx::serialization::access;

        PHYLANX_EXPORT void serialize(hpx::serialization::output_archive& ar,
            unsigned);
        PHYLANX_EXPORT void serialize(hpx::serialization::input_archive& ar,
            unsigned);
    };

    ////////////////////////////////////////////////////////////////////////////
    PHYLANX_EXPORT std::ostream& operator<<(
        std::ostream& os, annotation const& ann);

    ////////////////////////////////////////////////////////////////////////////
    struct PHYLANX_EXPORT annotation_wrapper
    {
        explicit annotation_wrapper(primitive_argument_type const& op);
        annotation_wrapper(primitive_argument_type const& op1,
            primitive_argument_type const& op2);

        primitive_argument_type propagate(primitive_argument_type&& val,
            std::string const& name, std::string const& codename);

        std::shared_ptr<execution_tree::annotation> ann_;
    };

    ////////////////////////////////////////////////////////////////////////////
    struct annotation_information
    {
        annotation_information() = default;

        PHYLANX_EXPORT annotation_information(
            std::string name, std::int64_t generation);

        PHYLANX_EXPORT annotation_information(annotation const& ann,
            std::string const& name, std::string const& codename);

        PHYLANX_EXPORT std::string generate_name() const;
        PHYLANX_EXPORT annotation as_annotation() const;

        PHYLANX_EXPORT std::int64_t increment_generation();

    private:
        void extract_from_name();

    public:
        std::string name_;
        std::int64_t generation_ = 0;
    };

    PHYLANX_EXPORT annotation_information extract_annotation_information(
        annotation const& ann, std::string const& name,
        std::string const& codename);
}}

#endif
