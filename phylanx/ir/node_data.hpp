//  Copyright (c) 2017 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_IR_NODE_DATA_AUG_26_2017_0924AM)
#define PHYLANX_IR_NODE_DATA_AUG_26_2017_0924AM

#include <phylanx/config.hpp>

#include <hpx/include/serialization.hpp>
#include <hpx/include/util.hpp>

#include <array>
#include <cstddef>
#include <iterator>
#include <memory>
#include <vector>

namespace phylanx { namespace ir
{
    ///////////////////////////////////////////////////////////////////////////
    namespace detail
    {
        ///////////////////////////////////////////////////////////////////////
        template <typename T>
        class node_data_iterator
          : public hpx::util::iterator_facade<node_data_iterator<T>,
                T,
                std::random_access_iterator_tag,
                T const&,
                std::ptrdiff_t,
                T const*>
        {
        private:
            using base_type =
                hpx::util::iterator_facade<node_data_iterator<T>,
                    T,
                    std::random_access_iterator_tag,
                    T const&,
                    std::ptrdiff_t,
                    T const*>;

        public:
            node_data_iterator(T const* p)
              : p_(p)
            {
            }

        private:
            friend class hpx::util::iterator_core_access;

            typename base_type::reference dereference() const
            {
                return *p_;
            }

            bool equal(node_data_iterator const& x) const
            {
                return p_ == x.p_;
            }

            void advance(typename base_type::difference_type n)
            {
                p_ += n;
            }

            void increment()
            {
                ++p_;
            }

            void decrement()
            {
                --p_;
            }

            typename base_type::difference_type distance_to(
                node_data_iterator const& y) const
            {
                return y.p_ - p_;
            }

            T const* p_;
        };

        ///////////////////////////////////////////////////////////////////////
        template <typename T>
        class node_data_storage_base
        {
        public:
            constexpr static std::size_t const max_dimensions = 2;

            constexpr node_data_storage_base(std::size_t num_dims,
                std::array<std::size_t, max_dimensions> const& dims = {0, 0})
              : num_dimensions_(num_dims)
              , dimensions_(dims)
            {
            }
            virtual ~node_data_storage_base() = default;

            constexpr std::size_t num_dimensions() const
            {
                return num_dimensions_;
            }

            constexpr std::size_t dimension(std::size_t dim) const
            {
                return dimensions_[dim];
            }
            void set_dimensions(
                std::array<std::size_t, max_dimensions> const& dims)
            {
                dimensions_ = dims;
            }

            virtual T const& operator[](std::size_t index) const = 0;

            using iterator = node_data_iterator<T>;

            virtual iterator begin() const = 0;
            virtual iterator end() const = 0;

        private:
            friend class hpx::serialization::access;

            template <typename Archive>
            void serialize(Archive& ar, unsigned)
            {
                ar & num_dimensions_ & dimensions_;
            }
            HPX_SERIALIZATION_POLYMORPHIC_ABSTRACT(node_data_storage_base);

            std::size_t num_dimensions_;
            std::array<std::size_t, max_dimensions> dimensions_;
        };

        ///////////////////////////////////////////////////////////////////////
        template <std::size_t N, typename T>
        class node_data_storage;

        template <typename T>
        class node_data_storage<0, T> : public node_data_storage_base<T>
        {
        public:
            constexpr node_data_storage()
              : node_data_storage_base<T>(0)
              , data_(0.0)
            {
            }

            constexpr node_data_storage(T const& value)
              : node_data_storage_base<T>(0)
              , data_(value)
            {
            }

            constexpr node_data_storage(T && value)
              : node_data_storage_base<T>(0)
              , data_(std::move(value))
            {
            }

            T const& operator[](std::size_t index) const override
            {
                return data_;
            }

            node_data_storage& operator=(T const& data)
            {
                this->base_type::set_dimensions({0, 0});
                data_ = data;
                return *this;
            }
            node_data_storage& operator=(T && data)
            {
                this->base_type::set_dimensions({0, 0});
                data_ = std::move(data_);
                return *this;
            }

            using iterator = typename node_data_storage_base<T>::iterator;

            iterator begin() const override
            {
                return iterator(&data_);
            }
            iterator end() const override
            {
                return iterator(&data_ + 1);
            }

        private:
            friend class hpx::serialization::access;

            template <typename Archive>
            void serialize(Archive& ar, unsigned)
            {
                using base_type = node_data_storage_base<T>;
                ar & hpx::serialization::template base_object<base_type>(*this);
                ar & data_;
            }
            HPX_SERIALIZATION_POLYMORPHIC_TEMPLATE(node_data_storage);

            T data_;
        };

        ///////////////////////////////////////////////////////////////////////
        template <typename T>
        class node_data_storage<1, T> : public node_data_storage_base<T>
        {
        public:
            node_data_storage()
              : node_data_storage_base(1)
              , data_()
            {
            }

            node_data_storage(std::size_t dim)
              : node_data_storage_base(1)
              , data_(dim)
            {
            }

            node_data_storage(std::vector<T> const& values)
              : node_data_storage_base<T>(1, {values.size(), 0})
              , data_(values)
            {
            }
            node_data_storage(std::vector<T> && values)
              : node_data_storage_base<T>(1, {values.size(), 0})
              , data_(std::move(values))
            {
            }

            node_data_storage& operator=(std::vector<T> const& data)
            {
                this->base_type::set_dimensions({values.size(), 0});
                data_ = data;
                return *this;
            }
            node_data_storage& operator=(std::vector<T> && data)
            {
                this->base_type::set_dimensions({values.size(), 0});
                data_ = std::move(data_);
                return *this;
            }

            T const& operator[](std::size_t index) const override
            {
                return data_[index];
            }

            using iterator = typename node_data_storage_base<T>::iterator;

            iterator begin() const override
            {
                return iterator(data_.data());
            }
            iterator end() const override
            {
                return iterator(data_.data() + data_.size());
            }

        private:
            friend class hpx::serialization::access;

            template <typename Archive>
            void serialize(Archive& ar, unsigned)
            {
                using base_type = node_data_storage_base<T>;
                ar & hpx::serialization::template base_object<base_type>(*this);
                ar & data_;
            }
            HPX_SERIALIZATION_POLYMORPHIC_TEMPLATE(node_data_storage);

            std::vector<T> data_;
        };
    }

    ///////////////////////////////////////////////////////////////////////////
    template <typename T>
    class node_data
    {
    public:
        node_data() = default;

        node_data(T const& value)
          : data_(new detail::node_data_storage<0, T>(value))
        {
        }
        node_data(T && value)
          : data_(new detail::node_data_storage<0, T>(std::move(value)))
        {
        }

        node_data(std::vector<T> const& value)
          : data_(new detail::node_data_storage<1, T>(value))
        {
        }
        node_data(std::vector<T> && value)
          : data_(new detail::node_data_storage<1, T>(std::move(value)))
        {
        }

        T const& operator[](std::size_t index) const
        {
            return (*data_)[index];
        }

        using iterator = typename detail::node_data_storage_base<T>::iterator;

        iterator begin() const
        {
            return data_->begin();
        }
        iterator end() const
        {
            return data_->end();
        }

    private:
        friend class hpx::serialization::access;

        template <typename Archive>
        void serialize(Archive& ar, unsigned)
        {
            ar & data_;
        }

        std::unique_ptr<detail::node_data_storage_base<T>> data_;
    };
}}

#endif
