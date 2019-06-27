//  Copyright (c) 2019 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/annotation.hpp>
#include <phylanx/execution_tree/primitives/base_primitive.hpp>
#include <phylanx/ir/ranges.hpp>
#include <phylanx/util/generate_error_message.hpp>

#include <hpx/throw_exception.hpp>
#include <hpx/runtime/serialization/serialize.hpp>
#include <hpx/util/format.hpp>

namespace phylanx { namespace execution_tree
{
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

        primitive_arguments_type newdata;
        newdata.reserve(data_.size() + 1);
        for (auto&& val : data_)
        {
            newdata.emplace_back(std::move(val));
        }
        newdata.emplace_back(ir::range(key, data.copy()));

        data_ = std::move(newdata);
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
}}

