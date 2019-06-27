//  Copyright (c) 2019 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_PRIMITIVES_ANNOTATION_JUN_18_2019_1002AM)
#define PHYLANX_PRIMITIVES_ANNOTATION_JUN_18_2019_1002AM

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/primitive_argument_type.hpp>
#include <phylanx/ir/ranges.hpp>

#include <hpx/runtime/serialization/serialization_fwd.hpp>

#include <string>

namespace phylanx { namespace execution_tree
{
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
        annotation(char const* key, Ts&& ... ts)
          : data_(key, std::forward<Ts>(ts)...)
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

        PHYLANX_EXPORT bool find(std::string const& key, annotation& ann,
            std::string const& name = "",
            std::string const& codename = "<unknown>") const;
        PHYLANX_EXPORT bool has_key(std::string const& key,
            std::string const& name = "",
            std::string const& codename = "<unknown>") const;

        PHYLANX_EXPORT void add_annotation(std::string const& key,
            ir::range&& data, std::string const& name,
            std::string const& codename);

    private:
        ir::range data_;

        friend class hpx::serialization::access;

        PHYLANX_EXPORT void serialize(hpx::serialization::output_archive& ar,
            unsigned);
        PHYLANX_EXPORT void serialize(hpx::serialization::input_archive& ar,
            unsigned);
    };
}}

#endif
