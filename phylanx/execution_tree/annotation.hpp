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
//#include <hpx/util/lightweight_test.hpp>

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

        friend bool operator==(annotation const& lhs, annotation const& rhs)
        {
            // This makes the assumption that annotations contain something like:
            //annotation("localities", list("meta_0", list("tile", list("rows", 0, 1),
            //                                                     list("columns", 0, 2))),
            //                         list("meta_1", list("tile", list("rows", 1, 2),
            //                                                     list("columns", 0, 2))),
            //						   list("locality", 0, 2),
            //						   list("name", "test2d2d_1_1/1"))'
            // l_data should be a annotation->ir::range->range_type->
            // recursive_wrapper->primitive_arguments_type->std::vector<primitive_argument_type>
            ir::range l_data = lhs.get_data();
            //ir::range_iterator r_data_it = rhs.get_data();
            bool eq = true;
            ir::range_iterator it = l_data.begin();
            while (it != l_data.end())
            {
                // l_meta_level_list = list("meta_0", list("tile", list("rows", 0, 1),
                //                                                 list("columns", 0, 2)))
                /*
                ir::range l_meta_level_list_with_key =
                    phylanx::execution_tree::extract_list_value_strict(*it);
                std::string key = phylanx::execution_tree::extract_string_value(
                    *l_meta_level_list.begin());
                annotation r_meta_level_ann;
                if (lhs.find(key, r_meta_level_ann))
                {
                    ir::range l_meta_level_data_list(
                        l_meta_level_list_with_key.begin()++,
                        l_meta_level_list_with_key.end());
                    ir::range r_meta_level_data_list =
                        r_meta_level_ann.get_data();
                    if (key.substr(0, 4) == "meta")
                    {
                        ir::range l_tile_level_list =
                            phylanx::execution_tree::extract_list_value_strict(
                                *(l_meta_level_data_list.begin()));
                        ir::range r_tile_level_list =
                            phylanx::execution_tree::extract_list_value_strict(
                                *(r_meta_level_data_list.begin()));
                        ir::range_iterator l_tile = l_tile_level_list.begin();
                        ir::range_iterator r_tile = r_tile_level_list.begin();
                        if (phylanx::execution_tree::extract_string_value(
                                *l_tile) == "tile" &&
                            *l_tile == *r_tile)
                        {
                            l_tile++;
                            r_tile++;
                            while (l_tile != l_tile_level_list.end())
                            {
                                auto found = std::find(
                                    r_tile, r_tile_level_list.end(), *l_tile);
                                if (found == r_tile_level_list.end())
                                    eq = false;
                                l_tile++;
                            }
                        }
                        else
                        {
                            // This should be equal to 'tile', if not,
                            // something's wrong
                            // Maybe it should throw actually
                            eq = false;
                        }
                    }
                    else if (key == "locality")
                    {
                        ir::range_iterator l_it =
                            l_meta_level_data_list.begin();
                        ir::range_iterator r_it =
                            r_meta_level_data_list.begin();
                        /*
                        if (*l_it != *r_it || *(l_it++) != *(r_it++))
                            eq = false;
                        * /
                    }
                    else if (key == "name")
                    {
                        if (*l_meta_level_data_list.begin() !=
                            r_meta_level_data_list.begin())
                            eq = false;
                    }
                    else
                    {
                        // There are problems somewhere
                        eq = false;
                    }
                }
                else
                {
                    eq = false;
                }
                it++;
                */
                it++;
            }
            // Then the annotation(s) is(are) either invalid or they are not equal
            return eq;
        }

        friend bool operator!=(annotation const& lhs, annotation const& rhs)
        {
            return !(lhs == rhs);
        }

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

        primitive_argument_type propagate(primitive_argument_type&& val);

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
