//  Copyright (c) 2020 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/base_primitive.hpp>
#include <phylanx/ir/dictionary.hpp>
#include <phylanx/util/serialization/variant.hpp>
#include <phylanx/util/variant.hpp>

#include <hpx/modules/errors.hpp>
#include <hpx/modules/serialization.hpp>

#include <cstddef>
#include <functional>
#include <unordered_map>
#include <utility>

namespace phylanx { namespace ir {

    dictionary::dictionary() = default;

    dictionary::dictionary(dictionary_data_type const& value)
      : data_(value)
    {
    }

    dictionary::dictionary(dictionary_data_type&& value)
      : data_(std::move(value))
    {
    }

    dictionary::dictionary(custom_dictionary_data_type value)
      : data_(std::move(value))
    {
    }

    dictionary::dictionary(const_custom_dictionary_data_type value)
      : data_(custom_dictionary_data_type(
            const_cast<dictionary_data_type&>(value.get())))
    {
    }

    dictionary::dictionary(dictionary const& d) = default;
    dictionary::dictionary(dictionary&& d) = default;

    dictionary& dictionary::operator=(dictionary_data_type const& val)
    {
        data_ = val;
        return *this;
    }

    dictionary& dictionary::operator=(dictionary_data_type&& val)
    {
        data_ = std::move(val);
        return *this;
    }

    dictionary& dictionary::operator=(custom_dictionary_data_type val)
    {
        data_ = std::move(val);
        return *this;
    }

    dictionary& dictionary::operator=(const_custom_dictionary_data_type val)
    {
        data_ = custom_dictionary_data_type(
            const_cast<dictionary_data_type&>(val.get()));
        return *this;
    }

    dictionary& dictionary::operator=(dictionary const& val) = default;
    dictionary& dictionary::operator=(dictionary&& val) = default;

    dictionary::dictionary_data_type& dictionary::dict() &
    {
        custom_dictionary_data_type* cd =
            util::get_if<custom_dictionary_data_type>(&data_);
        if (cd != nullptr)
        {
            return cd->get();
        }

        dictionary_data_type* d = util::get_if<dictionary_data_type>(&data_);
        if (d != nullptr)
        {
            return *d;
        }

        HPX_THROW_EXCEPTION(hpx::invalid_status,
            "phylanx::ir::dictionary::dict() &",
            "dictionary object holds unsupported data type");
    }

    dictionary::dictionary_data_type const& dictionary::dict() const&
    {
        custom_dictionary_data_type const* cd =
            util::get_if<custom_dictionary_data_type>(&data_);
        if (cd != nullptr)
        {
            return cd->get();
        }

        dictionary_data_type const* d =
            util::get_if<dictionary_data_type>(&data_);
        if (d != nullptr)
        {
            return *d;
        }

        HPX_THROW_EXCEPTION(hpx::invalid_status,
            "phylanx::ir::dictionary::dict() const&",
            "dictionary object holds unsupported data type");
    }

    dictionary::dictionary_data_type& dictionary::dict() &&
    {
        custom_dictionary_data_type* cd =
            util::get_if<custom_dictionary_data_type>(&data_);
        if (cd != nullptr)
        {
            return cd->get();
        }

        HPX_THROW_EXCEPTION(hpx::invalid_status,
            "phylanx::ir::dictionary::dict() &&",
            "dictionary shouldn't be called on an rvalue");
    }

    dictionary::dictionary_data_type const& dictionary::dict() const&&
    {
        custom_dictionary_data_type const* cd =
            util::get_if<custom_dictionary_data_type>(&data_);
        if (cd != nullptr)
        {
            return cd->get();
        }

        HPX_THROW_EXCEPTION(hpx::invalid_status,
            "phylanx::ir::dictionary::dict() const&&",
            "dictionary shouldn't be called on an rvalue");
    }

    bool dictionary::is_ref() const
    {
        switch (data_.index())
        {
        case dictionary_data:
            return false;

        case custom_dictionary_data:
            return true;

        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::invalid_status,
            "phylanx::ir::dictionary::is_ref()",
            "dictionary object holds unsupported data type");
    }

    dictionary dictionary::ref() &
    {
        switch (data_.index())
        {
        case dictionary_data:
            return dictionary{std::ref(dict())};

        case custom_dictionary_data:
            return *this;

        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::invalid_status,
            "phylanx::ir::dictionary::ref() &",
            "dictionary object holds unsupported data type");
    }

    dictionary dictionary::ref() const&
    {
        switch (data_.index())
        {
        case dictionary_data:
            return dictionary{std::ref(dict())};

        case custom_dictionary_data:
            return *this;

        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::invalid_status,
            "phylanx::ir::dictionary::ref() const&",
            "dictionary object holds unsupported data type");
    }

    dictionary dictionary::ref() &&
    {
        HPX_THROW_EXCEPTION(hpx::invalid_status,
            "phylanx::ir::dictionary::ref() &&",
            "dictionary::ref() should not be called on an rvalue");
    }

    dictionary dictionary::ref() const&&
    {
        HPX_THROW_EXCEPTION(hpx::invalid_status,
            "phylanx::ir::dictionary::ref() const&&",
            "dictionary::ref() should not be called on an rvalue");
    }

    dictionary dictionary::copy() &
    {
        custom_dictionary_data_type* cd =
            util::get_if<custom_dictionary_data_type>(&data_);
        if (cd != nullptr)
        {
            return dictionary{dictionary_data_type{*cd}};
        }

        dictionary_data_type* d = util::get_if<dictionary_data_type>(&data_);
        if (d != nullptr)
        {
            return dictionary{*d};
        }

        HPX_THROW_EXCEPTION(hpx::invalid_status,
            "phylanx::ir::dictionary::copy() &",
            "dictionary object holds unsupported data type");
    }

    dictionary dictionary::copy() const&
    {
        custom_dictionary_data_type const* cd =
            util::get_if<custom_dictionary_data_type>(&data_);
        if (cd != nullptr)
        {
            return dictionary{dictionary_data_type{*cd}};
        }

        dictionary_data_type const* d =
            util::get_if<dictionary_data_type>(&data_);
        if (d != nullptr)
        {
            return dictionary{*d};
        }

        HPX_THROW_EXCEPTION(hpx::invalid_status,
            "phylanx::ir::dictionary::copy() const&",
            "dictionary object holds unsupported data type");
    }

    dictionary dictionary::copy() &&
    {
        custom_dictionary_data_type* cd =
            util::get_if<custom_dictionary_data_type>(&data_);
        if (cd != nullptr)
        {
            return dictionary{dictionary_data_type{std::move(*cd)}};
        }

        dictionary_data_type* d = util::get_if<dictionary_data_type>(&data_);
        if (d != nullptr)
        {
            return dictionary{std::move(*d)};
        }

        HPX_THROW_EXCEPTION(hpx::invalid_status,
            "phylanx::ir::dictionary::copy() &&",
            "dictionary object holds unsupported data type");
    }

    dictionary dictionary::copy() const&&
    {
        custom_dictionary_data_type const* cd =
            util::get_if<custom_dictionary_data_type>(&data_);
        if (cd != nullptr)
        {
            return dictionary{dictionary_data_type{*cd}};
        }

        dictionary_data_type const* d =
            util::get_if<dictionary_data_type>(&data_);
        if (d != nullptr)
        {
            return dictionary{*d};
        }

        HPX_THROW_EXCEPTION(hpx::invalid_status,
            "phylanx::ir::dictionary::copy() const&&",
            "dictionary object holds unsupported data type");
    }

    bool dictionary::insert(
        phylanx::execution_tree::primitive_argument_type const& key,
        phylanx::execution_tree::primitive_argument_type const& value)
    {
        auto p = dict().insert(dictionary_data_type::value_type(key, value));
        return p.second;
    }

    bool dictionary::insert(
        phylanx::execution_tree::primitive_argument_type&& key,
        phylanx::execution_tree::primitive_argument_type&& value)
    {
        auto p = dict().insert(
            dictionary_data_type::value_type(std::move(key), std::move(value)));
        return p.second;
    }

    void dictionary::reserve(std::size_t count)
    {
        dict().reserve(count);
    }

    std::size_t dictionary::size() const
    {
        return dict().size();
    }

    bool dictionary::empty() const
    {
        return dict().empty();
    }

    phylanx::execution_tree::primitive_argument_type& dictionary::operator[](
        phylanx::execution_tree::primitive_argument_type const& key)
    {
        return dict().operator[](key).get();
    }

    phylanx::execution_tree::primitive_argument_type& dictionary::operator[](
        phylanx::execution_tree::primitive_argument_type&& key)
    {
        return dict().operator[](std::move(key)).get();
    }

    ///////////////////////////////////////////////////////////////////////////
    bool operator==(dictionary const& lhs, dictionary const& rhs)
    {
        return lhs.dict() == rhs.dict();
    }

    bool operator!=(dictionary const& lhs, dictionary const& rhs)
    {
        return !(lhs == rhs);
    }

    void dictionary::serialize(hpx::serialization::input_archive& ar, unsigned)
    {
        std::size_t index = 0;
        ar >> index;

        switch (index)
        {
        case dictionary_data:
            HPX_FALLTHROUGH;
        case custom_dictionary_data:    // deserialize reference_wrapper<T> as T
        {
            dictionary_data_type val;
            ar >> val;
            data_ = std::move(val);
        }
        break;
        }
    }

    void dictionary::serialize(hpx::serialization::output_archive& ar, unsigned)
    {
        std::size_t index = data_.index();
        ar << index;

        switch (index)
        {
        case dictionary_data:
            ar << util::get<dictionary_data>(data_);
            break;

        case custom_dictionary_data:
            ar << util::get<custom_dictionary_data>(data_).get();
            break;
        }
    }
}}    // namespace phylanx::ir
