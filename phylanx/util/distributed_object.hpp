// Copyright (c) 2019 Weile Wei
// Copyright (c) 2019 Maxwell Reeser
// Copyright (c) 2019 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_UTIL_DISTRIBUTED_OBJECT_HPP)
#define PHYLANX_UTIL_DISTRIBUTED_OBJECT_HPP

#include <phylanx/config.hpp>

#include <hpx/assertion.hpp>
#include <hpx/errors/throw_exception.hpp>
#include <hpx/preprocessor/cat.hpp>
#include <hpx/runtime/actions/component_action.hpp>
#include <hpx/runtime/basename_registration_fwd.hpp>
#include <hpx/runtime/components/component_factory.hpp>
#include <hpx/runtime/components/new.hpp>
#include <hpx/runtime/components/server/component.hpp>
#include <hpx/runtime/get_locality_id.hpp>
#include <hpx/runtime/get_num_localities.hpp>
#include <hpx/runtime/get_ptr.hpp>
#include <hpx/runtime/launch_policy.hpp>
#include <hpx/synchronization/spinlock.hpp>
#include <hpx/thread_support/unlock_guard.hpp>

#include <algorithm>
#include <cstddef>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <utility>

/// \cond NOINTERNAL
namespace phylanx { namespace util { namespace server
{
    ////////////////////////////////////////////////////////////////////////////
    template <typename T>
    class distributed_object_part
      : public hpx::components::component_base<distributed_object_part<T>>
    {
    public:
        using data_type = T;

        distributed_object_part() = default;

        explicit distributed_object_part(data_type const& data)
          : data_(data)
        {
        }

        explicit distributed_object_part(data_type&& data)
          : data_(std::move(data))
        {
        }

        data_type& operator*()
        {
            return data_;
        }

        data_type const& operator*() const
        {
            return data_;
        }

        data_type* operator->()
        {
            return &data_;
        }

        data_type const* operator->() const
        {
            return &data_;
        }

        data_type fetch() const
        {
            return data_;
        }

        HPX_DEFINE_COMPONENT_ACTION(distributed_object_part, fetch);

    private:
        data_type data_;
    };

    ////////////////////////////////////////////////////////////////////////////
    template <typename T>
    class distributed_object_part<T&>
      : public hpx::components::component_base<distributed_object_part<T&>>
    {
    public:
        using data_type = T&;

        distributed_object_part() = default;

        explicit distributed_object_part(data_type data)
          : data_(data)
        {
        }

        data_type operator*()
        {
            return data_;
        }

        data_type operator*() const
        {
            return data_;
        }

        T* operator->()
        {
            return data_;
        }

        T const* operator->() const
        {
            return data_;
        }

        T fetch() const
        {
            return data_;
        }

        HPX_DEFINE_COMPONENT_ACTION(distributed_object_part, fetch);

    private:
        data_type data_;
    };
}}}

#define REGISTER_DISTRIBUTED_OBJECT_DECLARATION(type)                          \
    HPX_REGISTER_ACTION_DECLARATION(                                           \
        phylanx::util::server::distributed_object_part<type>::fetch_action,    \
        HPX_PP_CAT(__distributed_object_part_fetch_action_, type));

/**/

#define REGISTER_DISTRIBUTED_OBJECT(type)                                      \
    HPX_REGISTER_ACTION(                                                       \
        phylanx::util::server::distributed_object_part<type>::fetch_action,    \
        HPX_PP_CAT(__distributed_object_part_fetch_action_, type));            \
    typedef ::hpx::components::component<                                      \
        phylanx::util::server::distributed_object_part<type>>                  \
        HPX_PP_CAT(__distributed_object_part_, type);                          \
    HPX_REGISTER_COMPONENT(HPX_PP_CAT(__distributed_object_part_, type))       \
    /**/

namespace phylanx { namespace util
{
    enum class construction_type
    {
        meta_object,
        all_to_all
    };

    ////////////////////////////////////////////////////////////////////////////
    // The front end for the distributed_object itself. Essentially wraps
    // actions for the server, and stores information locally about the
    // localities/servers that it needs to know about.

    /// The distributed_object is a single logical object partitioned over a set
    /// of localities/nodes/machines, where every locality shares the same global
    /// name locality for the distributed object (i.e. a universal name), but
    /// owns its local value. In other words, local data of the distributed
    /// object can be different, but they share access to one another's data
    /// globally.
    template <typename T, construction_type C = construction_type::all_to_all>
    class distributed_object;

    template <typename T>
    class distributed_object<T, construction_type::all_to_all>
    {
    private:
        using data_type =
            typename server::distributed_object_part<T>::data_type;

    public:
        /// Creates a distributed_object in every locality
        ///
        /// A distributed_object \a base_name is created through default
        /// constructor.
        distributed_object() = default;

        /// Creates a distributed_object in every locality with a given
        /// base_name string, data, and a type and construction_type in the
        /// template parameters.
        ///
        /// \param construction_type The construction_type in the template
        ///             parameters accepts either meta_object or all_to_all,
        ///             and it is set to all_to_all by default. The meta_object
        ///             option provides meta object registration in the root
        ///             locality and meta object is essentially a table that
        ///             can find the instances of distributed_object in all
        ///             localities. The all_to_all option only locally holds
        ///             the client and server of the distributed_object.
        /// \param base_name The name of the distributed_object, which should
        ///             be a unique string across the localities
        /// \param data The data of the type T of the distributed_object
        /// \param sub_localities The sub_localities accepts a list of locality
        ///             index. By default, it is initialized to a list of all
        ///             provided locality index.
        ///
        distributed_object(char const* basename, data_type const& data,
                std::size_t num_sites = std::size_t(-1),
                std::size_t this_site = std::size_t(-1))
          : num_sites_(num_sites == std::size_t(-1) ?
                    hpx::get_num_localities(hpx::launch::sync) :
                    num_sites)
          , this_site_(this_site == std::size_t(-1) ? hpx::get_locality_id() :
                                                      this_site)
          , basename_(basename)
        {
            if (this_site_ >= num_sites_)
            {
                HPX_THROW_EXCEPTION(hpx::no_success,
                    "distributed_object::distributed_object",
                    "attempting to construct invalid part of the "
                        "distributed object");
            }
            create_and_register_server(data);
        }

        /// Creates a distributed_object in every locality with a given
        /// base_name string, data, and a type and construction_type in the
        /// template parameters
        ///
        /// \param construction_type The construction_type in the template
        ///             parameters accepts either meta_object or all_to_all,
        ///             and it is set to all_to_all by default. The meta_object
        ///             option provides meta object registration in the root
        ///             locality and meta object is essentially a table that
        ///             can find the instances of distributed_object in all
        ///             localities. The all_to_all option only locally holds
        ///             the client and server of the distributed_object.
        /// \param base_name The name of the distributed_object, which should
        ///             be a unique string across the localities
        /// \param data The data of the type T of the distributed_object
        /// \param sub_localities The sub_localities accepts a list of locality
        ///             index. By default, it is initialized to a list of all
        ///             provided locality index.
        ///
        distributed_object(char const* basename, data_type&& data,
                std::size_t num_sites = std::size_t(-1),
                std::size_t this_site = std::size_t(-1))
          : num_sites_(num_sites == std::size_t(-1) ?
                    hpx::get_num_localities(hpx::launch::sync) :
                    num_sites)
          , this_site_(this_site == std::size_t(-1) ? hpx::get_locality_id() :
                                                      this_site)
          , basename_(basename)
        {
            if (this_site_ >= num_sites_)
            {
                HPX_THROW_EXCEPTION(hpx::no_success,
                    "distributed_object::distributed_object",
                    "attempting to construct invalid part of the "
                        "distributed object");
            }
            create_and_register_server(std::move(data));
        }

        /// Destroy the local reference to the distributed object, unregister
        /// the symbolic name
        ~distributed_object()
        {
            hpx::unregister_with_basename(basename_, this_site_).get();
        }

        /// Access the calling locality's value instance for this distributed_object
        data_type& operator*()
        {
            HPX_ASSERT(!!ptr_);
            return **ptr_;
        }

        /// Access the calling locality's value instance for this distributed_object
        data_type const& operator*() const
        {
            HPX_ASSERT(!!ptr_);
            return **ptr_;
        }

        /// Access the calling locality's value instance for this distributed_object
        data_type* operator->()
        {
            HPX_ASSERT(!!ptr_);
            return &**ptr_;
        }

        /// Access the calling locality's value instance for this distributed_object
        data_type const* operator->() const
        {
            HPX_ASSERT(!!ptr_);
            return &**ptr_;
        }

        /// fetch() function is an asynchronous function. This returns a future
        /// of a copy of the instance of this distributed_object associated with
        /// the given locality index. The provided locality index must be valid
        /// within the sub localities where this distributed object is
        /// constructed. Also, if the provided locality index is same as current
        /// locality, fetch function still returns a future of it local data copy.
        /// It is suggested to use star operator to access local data.
        hpx::future<data_type> fetch(std::size_t idx) const
        {
            /// \cond NOINTERNAL
            HPX_ASSERT(!!ptr_);
            using action_type =
                typename server::distributed_object_part<T>::fetch_action;

            return hpx::async<action_type>(get_part_id(idx));
            /// \endcond
        }

    private:
        /// \cond NOINTERNAL
        template <typename Arg>
        hpx::id_type create_and_register_server(Arg&& value)
        {
            hpx::id_type part_id =
                hpx::local_new<server::distributed_object_part<T>>(
                    hpx::launch::sync, std::forward<Arg>(value));

            hpx::register_with_basename(basename_, part_id, this_site_);

            part_ids_[this_site_] = part_id;
            ptr_ = hpx::get_ptr<server::distributed_object_part<T>>(
                hpx::launch::sync, part_id);

            return part_id;
        }

        hpx::id_type const& get_part_id(std::size_t idx) const
        {
            if (idx == this_site_)
            {
                return part_ids_[idx];
            }

            if (idx >= num_sites_)
            {
                HPX_THROW_EXCEPTION(hpx::no_success,
                    "distributed_object::get_part_id",
                    "attempting to access invalid part of the distributed "
                    "object");
            }

            std::lock_guard<hpx::lcos::local::spinlock> l(part_ids_mtx_);
            auto it = part_ids_.find(idx);
            if (it == part_ids_.end())
            {
                hpx::id_type id;

                {
                    hpx::util::unlock_guard<hpx::lcos::local::spinlock> ul(
                        part_ids_mtx_);

                    id = hpx::agas::on_symbol_namespace_event(
                        hpx::detail::name_from_basename(basename_, idx), true).get();
                }

                it = part_ids_.find(idx);
                if (it == part_ids_.end())
                {
                    it = part_ids_.emplace(idx, std::move(id)).first;
                }
            }
            return it->second;
        }

    private:
        std::size_t const num_sites_;
        std::size_t const this_site_;
        std::string const basename_;
        std::shared_ptr<server::distributed_object_part<T>> ptr_;

        mutable hpx::lcos::local::spinlock part_ids_mtx_;
        mutable std::map<std::size_t, hpx::id_type> part_ids_;
        /// \endcond
    };

    /// The distributed_object is a single logical object partitioned over a set of
    /// localities/nodes/machines, where every locality shares the same global
    /// name locality for the distributed object (i.e. a universal name), but
    /// owns its local value. In other words, local data of the distributed
    /// object can be different, but they share access to one another's data
    /// globally.
    template <typename T>
    class distributed_object<T&, construction_type::all_to_all>
    {
    private:
        /// \cond NOINTERNAL
        using data_type =
            typename server::distributed_object_part<T&>::data_type;
        /// \endcond

    public:
        /// Creates a distributed_object in every locality
        ///
        /// A distributed_object \a base_name is created through default constructor.
        distributed_object() = default;

        /// Creates a distributed_object in every locality with a given base_name
        /// string, data, and a type and construction_type in the template
        /// parameters. This constructor of the distributed_object wraps an
        /// existing local instance and thus is internally referring to the
        /// local instance.
        ///
        /// \param construction_type The construction_type in the template
        ///             parameters accepts either meta_object or all_to_all, and
        ///             it is set to all_to_all by default. The meta_object
        ///             option provides meta object registration in the root
        ///             locality and meta object is essentially a table that
        ///             can find the instances of distributed_object in all
        ///             localities. The all_to_all option only locally holds the
        ///             client and server of the distributed_object.
        /// \param base_name The name of the distributed_object, which should
        ///             be a unique string across the localities
        /// \param data The data of the type T of the distributed_object
        /// \param sub_localities The sub_localities accepts a list of locality
        ///             index. By
        /// default, it is initialized to a list of all provided locality index.
        ///
        distributed_object(char const* basename, data_type data,
                std::size_t num_sites = std::size_t(-1),
                std::size_t this_site = std::size_t(-1))
          : num_sites_(num_sites == std::size_t(-1) ?
                    hpx::get_num_localities(hpx::launch::sync) :
                    num_sites)
          , this_site_(this_site == std::size_t(-1) ? hpx::get_locality_id() :
                                                      this_site)
          , basename_(basename)
        {
            if (this_site_ >= num_sites_)
            {
                HPX_THROW_EXCEPTION(hpx::no_success,
                    "distributed_object::distributed_object",
                    "attempting to construct invalid part of the "
                        "distributed object");
            }
            create_and_register_server(data);
        }

        /// Destroy the local reference to the distributed object, unregister
        /// the symbolic name
        ~distributed_object()
        {
            hpx::unregister_with_basename(basename_, this_site_);
        }

        /// Access the calling locality's value instance for this distributed_object
        data_type const operator*() const
        {
            HPX_ASSERT(!!ptr_);
            return **ptr_;
        }

        /// Access the calling locality's value instance for this distributed_object
        data_type operator*()
        {
            HPX_ASSERT(!!ptr_);
            return **ptr_;
        }

        /// Access the calling locality's value instance for this distributed_object
        T const* operator->() const
        {
            HPX_ASSERT(!!ptr_);
            return &**ptr_;
        }

        /// Access the calling locality's value instance for this distributed_object
        T* operator->()
        {
            HPX_ASSERT(!!ptr_);
            return &**ptr_;
        }

        /// fetch() function is an asynchronous function. This returns a future
        /// of a copy of the instance of this distributed_object associated with
        /// the given locality index. The provided locality index must be valid
        /// within the sub localities where this distributed object is
        /// constructed. Also, if the provided locality index is same as current
        /// locality, fetch function still returns a future of it local data copy.
        /// It is suggested to use star operator to access local data.
        hpx::future<T> fetch(std::size_t idx)
        {
            HPX_ASSERT(!!ptr_);
            using action_type =
                typename server::distributed_object_part<T&>::fetch_action;

            return hpx::async<action_type>(get_part_id(idx));
        }

    private:
        /// \cond NOINTERNAL
        template <typename Arg>
        hpx::id_type create_and_register_server(Arg& value)
        {
            hpx::id_type part_id =
                hpx::local_new<server::distributed_object_part<T&>>(
                    hpx::launch::sync, value);

            hpx::register_with_basename(basename_, part_id, this_site_).get();

            part_ids_[this_site_] = part_id;
            ptr_ = hpx::get_ptr<server::distributed_object_part<T&>>(
                hpx::launch::sync, part_id);

            return part_id;
        }

        hpx::id_type const& get_part_id(std::size_t idx) const
        {
            if (idx == this_site_)
            {
                return part_ids_[idx];
            }

            if (idx >= num_sites_)
            {
                HPX_THROW_EXCEPTION(hpx::no_success,
                    "distributed_object::get_part_id",
                    "attempting to access invalid part of the distributed "
                    "object");
            }

            std::lock_guard<hpx::lcos::local::spinlock> l(part_ids_mtx_);
            auto it = part_ids_.find(idx);
            if (it == part_ids_.end())
            {
                hpx::id_type id;

                {
                    hpx::util::unlock_guard<hpx::lcos::local::spinlock> ul(
                        part_ids_mtx_);

                    id = hpx::agas::on_symbol_namespace_event(
                        hpx::detail::name_from_basename(basename_, idx), true).get();
                }

                it = part_ids_.find(idx);
                if (it == part_ids_.end())
                {
                    it = part_ids_.emplace(idx, std::move(id)).first;
                }
            }
            return it->second;
        }

    private:
        std::size_t const num_sites_;
        std::size_t const this_site_;
        std::string const basename_;
        std::shared_ptr<server::distributed_object_part<T&>> ptr_;

        mutable hpx::lcos::local::spinlock part_ids_mtx_;
        mutable std::map<std::size_t, hpx::id_type> part_ids_;
        /// \endcond
    };
}}
#endif /*HPX_LCOS_DISTRIBUTED_OBJECT_HPP*/
