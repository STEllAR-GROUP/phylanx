//  Copyright (c) 2019 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_UTIL_HASHED_STRING_JAN_05_2019_0555PM)
#define PHYLANX_UTIL_HASHED_STRING_JAN_05_2019_0555PM

#include <phylanx/config.hpp>

#include <hpx/runtime/serialization/serialization_fwd.hpp>
#include <hpx/util/jenkins_hash.hpp>

#include <iosfwd>
#include <string>
#include <utility>

namespace phylanx { namespace util
{
    struct hashed_string
    {
        using hasher = hpx::util::jenkins_hash;
        using size_type = hasher::size_type;

        hashed_string() = default;

        hashed_string(std::string const& key)
          : key_(key)
          , hash_(hasher{}(key_))
        {
        }

        hashed_string(std::string&& key)
          : key_(std::move(key))
          , hash_(hasher{}(key_))
        {
        }

        hashed_string(char const* key)
          : key_(key)
          , hash_(hasher{}(key_))
        {
        }

        friend bool operator<(hashed_string const& lhs,
            hashed_string const& rhs)
        {
            return (lhs.hash_ < rhs.hash_) ||
                (lhs.hash_ == rhs.hash_ && lhs.key_ < rhs.key_);
        }

        PHYLANX_EXPORT friend std::ostream& operator<<(std::ostream& os,
            hashed_string const& s);

        std::string const& key() const
        {
            return key_;
        }

        size_type hash() const
        {
            return hash_;
        }

    private:
        friend class hpx::serialization::access;
        PHYLANX_EXPORT void serialize(hpx::serialization::output_archive& ar,
            unsigned);
        PHYLANX_EXPORT void serialize(hpx::serialization::input_archive& ar,
            unsigned);

        std::string key_;
        size_type hash_;
    };
}}

#endif
