// Copyright (c) 2017-2019 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/base_primitive.hpp>
#include <phylanx/execution_tree/primitives/primitive_argument_type.hpp>
#include <phylanx/util/generate_error_message.hpp>

#include <hpx/include/serialization.hpp>
#include <hpx/util/internal_allocator.hpp>

#include <cstdint>
#include <memory>

namespace phylanx { namespace execution_tree
{
    ///////////////////////////////////////////////////////////////////////////
    hpx::util::internal_allocator<variable_frame> eval_context::alloc_;

    ///////////////////////////////////////////////////////////////////////////
    void topology::serialize(hpx::serialization::output_archive& ar, unsigned)
    {
        ar & children_ & name_;
    }

    void topology::serialize(hpx::serialization::input_archive& ar, unsigned)
    {
        ar & children_ & name_;
    }

    ///////////////////////////////////////////////////////////////////////////
    void eval_context::serialize(hpx::serialization::output_archive& ar, unsigned)
    {
        std::int32_t mode = mode_;
        ar & mode & variables_;
    }

    void eval_context::serialize(hpx::serialization::input_archive& ar, unsigned)
    {
        std::int32_t mode = 0;
        ar & mode & variables_;
        mode_ = eval_mode(mode);
    }

    ///////////////////////////////////////////////////////////////////////////
    void variable_frame::serialize(
        hpx::serialization::output_archive& ar, unsigned)
    {
        ar & variables_;
    }

    void variable_frame::serialize(
        hpx::serialization::input_archive& ar, unsigned)
    {
        ar & variables_;
    }

    ///////////////////////////////////////////////////////////////////////////
    void primitive_argument_type::serialize(
        hpx::serialization::output_archive& ar, unsigned)
    {
        ar & variant() & annotation_;
    }

    void primitive_argument_type::serialize(
        hpx::serialization::input_archive& ar, unsigned)
    {
        ar & variant() & annotation_;
    }

    ////////////////////////////////////////////////////////////////////////////
    // general annotation support
    void primitive_argument_type::set_annotation(ir::range&& ann,
        std::string const& name, std::string const& codename)
    {
        if (ann.empty())
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "primitive_argument_type::set_annotation",
                util::generate_error_message(
                    "the annotation data should hold at least the annotation "
                    "type", name, codename));
        }
        annotation_ =
            std::make_shared<execution_tree::annotation>(std::move(ann));
    }

    void primitive_argument_type::set_annotation(
        execution_tree::annotation&& ann, std::string const& name,
        std::string const& codename)
    {
        if (ann.get_range().empty())
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "primitive_argument_type::set_annotation",
                util::generate_error_message(
                    "the annotation data should hold at least the annotation "
                    "type", name, codename));
        }
        annotation_ =
            std::make_shared<execution_tree::annotation>(std::move(ann));
    }

    void primitive_argument_type::set_annotation(annotation_ptr ann)
    {
        annotation_ = std::move(ann);
    }

    std::string primitive_argument_type::get_annotation_type(
        std::string const& name, std::string const& codename) const
    {
        if (!annotation_)
        {
            return "<no annotation>";
        }
        return annotation_->get_type(name, codename);
    }

    ir::range primitive_argument_type::get_annotation_data() const
    {
        if (!annotation_)
        {
            return ir::range{};
        }
        return annotation_->get_data();
    }

    bool primitive_argument_type::get_annotation_if(std::string const& key,
        execution_tree::annotation& ann, std::string const& name,
        std::string const& codename) const
    {
        if (!annotation_ || annotation_->get_type() != key)
        {
            return false;
        }
        ann = annotation_->get_range_ref();
        return true;
    }

    bool primitive_argument_type::find_annotation(std::string const& key,
        execution_tree::annotation& ann, std::string const& name,
        std::string const& codename) const
    {
        if (!annotation_)
        {
            return false;
        }
        return annotation_->find(key, ann, name, codename);
    }
}}
