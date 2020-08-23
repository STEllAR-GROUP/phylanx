// Copyright (c) 2018 Weile Wei
// Copyright (c) 2018 Parsa Amini
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_DICTIONARY)
#define PHYLANX_DICTIONARY

#include <phylanx/config.hpp>
#include <phylanx/util/variant.hpp>

#include <hpx/serialization/serialization_fwd.hpp>
#include <hpx/serialization/unordered_map.hpp>

#include <cstddef>
#include <functional>
#include <unordered_map>

#include <hpx/config/warnings_prefix.hpp>

namespace phylanx { namespace execution_tree {
    struct primitive_argument_type;
}}    // namespace phylanx::execution_tree

namespace std {
    template <>
    struct hash<phylanx::util::recursive_wrapper<
        phylanx::execution_tree::primitive_argument_type>>
    {
        using argument_type = phylanx::util::recursive_wrapper<
            phylanx::execution_tree::primitive_argument_type>;
        using result_type = std::size_t;

        PHYLANX_EXPORT result_type operator()(
            argument_type const& s) const noexcept;
    };
}    // namespace std

namespace phylanx { namespace ir {

    ///////////////////////////////////////////////////////////////////////////
    struct PHYLANX_EXPORT dictionary
    {
        using dictionary_data_type = std::unordered_map<
            phylanx::util::recursive_wrapper<
                phylanx::execution_tree::primitive_argument_type>,
            phylanx::util::recursive_wrapper<
                phylanx::execution_tree::primitive_argument_type>>;

        using custom_dictionary_data_type =
            std::reference_wrapper<dictionary_data_type>;

        using const_custom_dictionary_data_type =
            std::reference_wrapper<dictionary_data_type const>;

        using storage_type =
            util::variant<dictionary_data_type, custom_dictionary_data_type>;

        enum variant_index
        {
            dictionary_data = 0,
            custom_dictionary_data = 1,
        };

        dictionary();

        explicit dictionary(dictionary_data_type const& value);
        explicit dictionary(dictionary_data_type&& value);

        explicit dictionary(custom_dictionary_data_type value);
        explicit dictionary(const_custom_dictionary_data_type value);

        dictionary(dictionary const& d);
        dictionary(dictionary&& d);

        dictionary& operator=(dictionary_data_type const& val);
        dictionary& operator=(dictionary_data_type&& val);

        dictionary& operator=(custom_dictionary_data_type val);
        dictionary& operator=(const_custom_dictionary_data_type val);

        dictionary& operator=(dictionary const& val);
        dictionary& operator=(dictionary&& val);

        dictionary_data_type& dict() &;
        dictionary_data_type const& dict() const&;
        dictionary_data_type& dict() &&;
        dictionary_data_type const& dict() const&&;

        bool is_ref() const;

        dictionary ref() &;
        dictionary ref() const&;
        dictionary ref() &&;
        dictionary ref() const&&;

        dictionary copy() &;
        dictionary copy() const&;
        dictionary copy() &&;
        dictionary copy() const&&;

        friend bool operator==(dictionary const& lhs, dictionary const& rhs);
        friend bool operator!=(dictionary const& lhs, dictionary const& rhs);

        std::size_t index() const
        {
            return data_.index();
        }

        bool insert(phylanx::execution_tree::primitive_argument_type const& key,
            phylanx::execution_tree::primitive_argument_type const& value);
        bool insert(phylanx::execution_tree::primitive_argument_type&& key,
            phylanx::execution_tree::primitive_argument_type&& value);

        void reserve(std::size_t count);

        std::size_t size() const;
        bool empty() const;

        phylanx::execution_tree::primitive_argument_type& operator[](
            phylanx::execution_tree::primitive_argument_type const& key);
        phylanx::execution_tree::primitive_argument_type& operator[](
            phylanx::execution_tree::primitive_argument_type&& key);

    private:
        friend class hpx::serialization::access;

        void serialize(hpx::serialization::input_archive& ar, unsigned);
        void serialize(hpx::serialization::output_archive& ar, unsigned);

        storage_type data_;
    };
}}    // namespace phylanx::ir

#include <hpx/config/warnings_suffix.hpp>

#endif
