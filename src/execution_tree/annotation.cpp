//  Copyright (c) 2019 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/ast/node.hpp>
#include <phylanx/execution_tree/annotation.hpp>
#include <phylanx/execution_tree/primitives/base_primitive.hpp>
#include <phylanx/execution_tree/primitives/primitive_argument_type.hpp>
#include <phylanx/ir/ranges.hpp>
#include <phylanx/util/generate_error_message.hpp>
#include <phylanx/util/repr_manip.hpp>

#include <hpx/format.hpp>
#include <hpx/runtime/serialization/serialize.hpp>
#include <hpx/throw_exception.hpp>

#include <cctype>
#include <cstdint>
#include <iosfwd>
#include <memory>
#include <string>
#include <utility>

namespace phylanx { namespace execution_tree
{
    ////////////////////////////////////////////////////////////////////////////
    primitive_argument_type as_primitive_argument_type(annotation&& ann)
    {
        auto&& r = ann.get_range();
        if (r.is_ref())
        {
            return primitive_argument_type{r.copy()};
        }
        return primitive_argument_type{std::move(r.args())};
    }

    primitive_argument_type as_primitive_argument_type(annotation const& ann)
    {
        return primitive_argument_type{ann.get_range_ref().copy()};
    }

    ////////////////////////////////////////////////////////////////////////////
    std::string annotation::get_type(
        std::string const& name, std::string const& codename) const
    {
        if (data_.empty())
        {
            return "<no annotation>";
        }

        return extract_string_value(*data_.begin(), name, codename);
    }

    ir::range annotation::get_data() const
    {
        if (data_.empty())
        {
            return ir::range{};
        }

        auto it = data_.begin();
        return ir::range(++it, data_.end());
    }

    bool annotation::find(std::string const& key, annotation& ann,
        std::string const& name, std::string const& codename) const
    {
        auto it = data_.begin();
        auto end = data_.end();
        if (it != end)
        {
            for (++it; it != end; ++it)
            {
                annotation r =
                    annotation{extract_list_value_strict(*it, name, codename)};
                if (r.get_type(name, codename) == key)
                {
                    ann = std::move(r);
                    return true;
                }
            }
        }
        return false;
    }

    bool annotation::get_if(std::string const& key,
        execution_tree::annotation& ann, std::string const& name,
        std::string const& codename) const
    {
        if (get_type() != key)
        {
            return false;
        }
        ann = get_range_ref();
        return true;
    }

    bool annotation::has_key(std::string const& key,
        std::string const& name, std::string const& codename) const
    {
        auto it = data_.begin();
        auto end = data_.end();
        if (it != end)
        {
            for (++it; it != end; ++it)
            {
                annotation r =
                    annotation{extract_list_value_strict(*it, name, codename)};
                if (r.get_type(name, codename) == key)
                {
                    return true;
                }
            }
        }
        return false;
    }

    void annotation::add_annotation(std::string const& key, ir::range&& data,
        std::string const& name, std::string const& codename)
    {
        if (has_key(key, name, codename))
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "annotation::add_annotation",
                util::generate_error_message(
                    hpx::util::format("duplicated annotation type: {}", key),
                    name, codename));
        }

        if (data_.is_ref())
        {
            primitive_arguments_type newdata;
            newdata.reserve(data_.size() + 1);
            for (auto&& val : data_)
            {
                newdata.emplace_back(std::move(val));
            }

            if (data.is_ref())
            {
                newdata.emplace_back(ir::range(key, data.copy()));
            }
            else
            {
                newdata.emplace_back(ir::range(key, std::move(data)));
            }

            data_ = std::move(newdata);
        }
        else
        {
            if (data.is_ref())
            {
                data_.args().emplace_back(ir::range(key, data.copy()));
            }
            else
            {
                data_.args().emplace_back(ir::range(key, std::move(data)));
            }
        }
    }

    void annotation::add_annotation(annotation&& data,
        std::string const& name, std::string const& codename)
    {
        if (has_key(data.get_type(), name, codename))
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "annotation::add_annotation",
                util::generate_error_message(
                    hpx::util::format(
                        "duplicated annotation type: {}", data.get_type()),
                    name, codename));
        }

        if (data_.is_ref())
        {
            primitive_arguments_type newdata;
            newdata.reserve(data_.size() + 1);
            for (auto&& val : data_)
            {
                newdata.emplace_back(std::move(val));
            }
            newdata.emplace_back(std::move(data.get_range()));

            data_ = std::move(newdata);
        }
        else
        {
            data_.args().emplace_back(std::move(data.get_range()));
        }
    }

    ////////////////////////////////////////////////////////////////////////////
    void annotation::replace_annotation(std::string const& key,
        annotation&& data, std::string const& name, std::string const& codename)
    {
        if (!has_key(key, name, codename))
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "annotation::add_annotation",
                util::generate_error_message(
                    hpx::util::format(
                        "attempt to replace non-existing annotation type: {}",
                        data.get_type()),
                    name, codename));
        }

        primitive_arguments_type newdata;
        newdata.reserve(data_.size() + 1);

        newdata.emplace_back(get_type());

        for (auto&& val : get_data())
        {
            auto&& list = execution_tree::extract_list_value_strict(
                val, name, codename);

            if (extract_string_value(*list.begin(), name, codename) == key)
            {
                newdata.emplace_back(data.get_range());
            }
            else
            {
                newdata.emplace_back(std::move(val));
            }
        }

        data_ = std::move(newdata);
    }

    void annotation::replace_annotation(annotation&& data,
        std::string const& name, std::string const& codename)
    {
        std::string key = data.get_type();
        replace_annotation(key, std::move(data), name, codename);
    }

    ////////////////////////////////////////////////////////////////////////////
    void annotation::serialize(hpx::serialization::output_archive& ar, unsigned)
    {
        ar << data_;
    }

    void annotation::serialize(hpx::serialization::input_archive& ar, unsigned)
    {
        ar >> data_;
    }

    ////////////////////////////////////////////////////////////////////////////
    std::ostream& operator<<(std::ostream& os, annotation const& ann)
    {
        util::repr_wrapper wrap(os);

        os << "annotation(";
        bool first = true;
        for (auto const& elem : ann.get_range_ref())
        {
            if (!first)
            {
                os << ", ";
            }
            first = false;
            ast::detail::to_string{os}(elem);
        }
        os << ")";
        return os;
    }

    ////////////////////////////////////////////////////////////////////////////
    annotation_wrapper::annotation_wrapper(primitive_argument_type const& op)
        : ann_(op.annotation())
    {}

    annotation_wrapper::annotation_wrapper(primitive_argument_type const& op1,
        primitive_argument_type const& op2)
    {
        if (!!op1)
        {
            if (!!op2)
            {
                // FIXME: merge annotations?
                ann_ = op1.annotation();
            }
            else
            {
                ann_ = op1.annotation();
            }
        }
        else if (!!op2)
        {
            ann_ = op2.annotation();
        }
    }

    primitive_argument_type annotation_wrapper::propagate(
        primitive_argument_type&& val)
    {
        if (!val.has_annotation() && !!ann_)
        {
            val.set_annotation(std::move(ann_));
        }
        return std::move(val);
    }

    ////////////////////////////////////////////////////////////////////////////
    annotation_information::annotation_information(annotation const& ann,
        std::string const& name, std::string const& codename)
    {
        auto&& key = ann.get_type(name, codename);
        if (key != "name")
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "annotation_information::annotation_information",
                util::generate_error_message(
                    hpx::util::format("unexpected annotation type ({})", key),
                    name, codename));
        }

        auto it = ann.get_data().begin();

        name_ = execution_tree::extract_string_value_strict(
            *(ann.get_data().begin()), name, codename);

        extract_from_name();
    }

    annotation_information::annotation_information(
            std::string name, std::int64_t generation)
      : name_(std::move(name))
      , generation_(generation)
    {
        extract_from_name();
    }

    void annotation_information::extract_from_name()
    {
        auto p = name_.find_last_of("/");
        if (p != std::string::npos && std::isdigit(name_[p + 1]))
        {
            generation_ = std::stoll(name_.substr(p + 1));
            name_ = name_.erase(p);
        }
    }

    std::string annotation_information::generate_name() const
    {
        return hpx::util::format("{}/{}", name_, generation_);
    }

    annotation annotation_information::as_annotation() const
    {
        return annotation("name", generate_name());
    }

    ////////////////////////////////////////////////////////////////////////////
    annotation_information extract_annotation_information(
        annotation const& ann, std::string const& name,
        std::string const& codename)
    {
        execution_tree::annotation name_ann;
        if (!ann.get_if("name", name_ann, name, codename) &&
            !ann.find("name", name_ann, name, codename))
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "extract_localities_information",
                util::generate_error_message(
                    "'localities' annotation type not given", name, codename));
        }
        return annotation_information(name_ann, name, codename);
    }
}}

