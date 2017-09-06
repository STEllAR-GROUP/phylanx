//  Copyright (c) 2017 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_IR_NODE_DATA_AUG_26_2017_0924AM)
#define PHYLANX_IR_NODE_DATA_AUG_26_2017_0924AM

#include <phylanx/config.hpp>

#include <hpx/include/serialization.hpp>
#include <hpx/include/util.hpp>

#include <Eigen/Dense>

#include <boost/intrusive_ptr.hpp>

#include <array>
#include <cstddef>
#include <iterator>
#include <vector>

namespace phylanx { namespace ir
{
    ///////////////////////////////////////////////////////////////////////////
    namespace detail
    {
        /// \cond NOINTERNAL

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
        template <typename T> class node_data_storage_base;

        template <typename T>
        void intrusive_ptr_add_ref(node_data_storage_base<T>* p);
        template <typename T>
        void intrusive_ptr_release(node_data_storage_base<T>* p);

        template <typename T>
        class node_data_storage_base
        {
        public:
            constexpr static std::size_t const max_dimensions = 2;

            using dimensions_type = std::array<std::ptrdiff_t, max_dimensions>;

        public:
            constexpr node_data_storage_base(std::size_t num_dims,
                    dimensions_type const& dims = {0, 0})
              : num_dimensions_(num_dims)
              , dimensions_(dims)
              , count_(0)
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
            constexpr dimensions_type const& dimensions() const
            {
                return dimensions_;
            }

            void set_dimensions(dimensions_type const& dims)
            {
                dimensions_ = dims;
            }

            virtual T const& operator[](std::ptrdiff_t index) const = 0;
            virtual T const& operator[](dimensions_type const& indicies) const = 0;

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
            dimensions_type dimensions_;

            // support for intrusive_ptr
            template <typename T_>
            friend void intrusive_ptr_add_ref(node_data_storage_base<T_>* p)
            {
                ++p->count_;
            }
            template <typename T_>
            friend void intrusive_ptr_release(node_data_storage_base<T_>* p)
            {
                if (--p->count_ == 0)
                    delete p;
            }

            hpx::util::atomic_count count_;
        };

        ///////////////////////////////////////////////////////////////////////
        template <std::size_t N, typename T>
        class node_data_storage;

        template <typename T>
        class node_data_storage<0, T> : public node_data_storage_base<T>
        {
        public:
            using dimensions_type =
                typename node_data_storage_base<T>::dimensions_type;
            using base_type = node_data_storage_base<T>;

            constexpr node_data_storage()
              : node_data_storage_base<T>(0, {1, 0})
              , data_(0.0)
            {
            }

            constexpr node_data_storage(T const& value)
              : node_data_storage_base<T>(0, {1, 0})
              , data_(value)
            {
            }

            constexpr node_data_storage(T && value)
              : node_data_storage_base<T>(0, {1, 0})
              , data_(std::move(value))
            {
            }

            T const& operator[](std::ptrdiff_t index) const override
            {
                HPX_ASSERT(index == 0);
                return data_;
            }
            T const& operator[](dimensions_type const& indicies) const override
            {
                HPX_ASSERT(indicies[0] == 0 && indicies[1] == 0);
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
            using dimensions_type =
                typename node_data_storage_base<T>::dimensions_type;
            using storage1d_type =
                Eigen::Matrix<T, Eigen::Dynamic, 1>;
            using base_type = node_data_storage_base<T>;

            node_data_storage()
              : node_data_storage_base<T>(1)
              , data_()
            {
            }

            node_data_storage(std::size_t dim)
              : node_data_storage_base<T>(1)
              , data_(dim)
            {
            }

            node_data_storage(storage1d_type const& values)
              : node_data_storage_base<T>(1,
                    {static_cast<std::ptrdiff_t>(values.size()), 0})
              , data_(values)
            {
            }
            node_data_storage(storage1d_type && values)
              : node_data_storage_base<T>(1,
                    {static_cast<std::ptrdiff_t>(values.size()), 0})
              , data_(std::move(values))
            {
            }
            node_data_storage(std::vector<T> const& values)
              : node_data_storage_base<T>(1,
                    {static_cast<std::ptrdiff_t>(values.size()), 0})
              , data_(Eigen::Map<storage1d_type const, Eigen::Unaligned>(
                    values.data(), values.size()))
            {
            }

            node_data_storage& operator=(storage1d_type const& data)
            {
                this->base_type::set_dimensions({data.rows(), 0});
                data_ = data;
                return *this;
            }
            node_data_storage& operator=(storage1d_type && data)
            {
                this->base_type::set_dimensions({data.rows(), 0});
                data_ = std::move(data_);
                return *this;
            }
            node_data_storage& operator=(std::vector<T> const& values)
            {
                this->base_type::set_dimensions(
                    {static_cast<std::ptrdiff_t>(values.size()), 0});
                data_ = Eigen::Map<storage1d_type const, Eigen::Unaligned>(
                    values.data(), values.size());
                return *this;
            }

            T const& operator[](std::ptrdiff_t index) const override
            {
                HPX_ASSERT(index < data_.size());
                return data_[index];
            }
            T const& operator[](dimensions_type const& indicies) const override
            {
                HPX_ASSERT(indicies[0] < data_.size() && indicies[1] == 0);
                return data_[indicies[0]];
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
                ar & hpx::serialization::template base_object<base_type>(*this);
                ar & data_;
            }
            HPX_SERIALIZATION_POLYMORPHIC_TEMPLATE(node_data_storage);

            storage1d_type data_;
        };

        ///////////////////////////////////////////////////////////////////////
        template <typename T>
        class node_data_storage<2, T> : public node_data_storage_base<T>
        {
        public:
            using base_type = node_data_storage_base<T>;
            using dimensions_type =
                typename node_data_storage_base<T>::dimensions_type;
            using storage2d_type =
                Eigen::Matrix<T, Eigen::Dynamic, Eigen::Dynamic>;

            node_data_storage()
              : node_data_storage_base<T>(2)
              , data_()
            {
            }

            node_data_storage(std::size_t dim1, std::size_t dim2)
              : node_data_storage_base<T>(2)
              , data_(dim1, dim2)
            {
            }

            node_data_storage(storage2d_type const& values)
              : node_data_storage_base<T>(2, {values.rows(), values.cols()})
              , data_(values)
            {
            }
            node_data_storage(storage2d_type && values)
              : node_data_storage_base<T>(2, {values.rows(), values.cols()})
              , data_(std::move(values))
            {
            }

            node_data_storage& operator=(storage2d_type const& data)
            {
                this->base_type::set_dimensions({data.rows(), data.cols()});
                data_ = data;
                return *this;
            }
            node_data_storage& operator=(storage2d_type && data)
            {
                this->base_type::set_dimensions({data.rows(), data.cols()});
                data_ = std::move(data);
                return *this;
            }

            T const& operator[](std::ptrdiff_t index) const override
            {
                HPX_ASSERT(index < data_.size());
                return data_(index);
            }
            T const& operator[](dimensions_type const& indicies) const override
            {
                HPX_ASSERT(indicies[0] < data_.rows() && indicies[1] != data_.cols());
                return data_(indicies[0], indicies[1]);
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
                ar & hpx::serialization::template base_object<base_type>(*this);
                ar & data_;
            }
            HPX_SERIALIZATION_POLYMORPHIC_TEMPLATE(node_data_storage);

            storage2d_type data_;
        };

        /// \endcond
    }

    ///////////////////////////////////////////////////////////////////////////
    template <typename T, typename Enable = void>
    struct node_data_traits
    {
        using type = T;
    };

    template <typename T, int Rows, int Cols, int Options, int MaxRows,
        int MaxCols>
    struct node_data_traits<
        Eigen::Matrix<T, Rows, Cols, Options, MaxRows, MaxCols>>
    {
        using type = T;
    };

    ///////////////////////////////////////////////////////////////////////////
    template <typename T>
    class node_data
    {
    public:
        using dimensions_type =
            typename detail::node_data_storage_base<T>::dimensions_type;
        using storage1d_type =
            Eigen::Matrix<T, Eigen::Dynamic, 1>;
        using storage2d_type =
            Eigen::Matrix<T, Eigen::Dynamic, Eigen::Dynamic>;

        node_data() = default;

        /// Create node data for a 0-dimensional value
        node_data(T const& value)
          : data_(new detail::node_data_storage<0, T>(value))
        {
        }
        node_data(T && value)
          : data_(new detail::node_data_storage<0, T>(std::move(value)))
        {
        }

        /// Create node data for a 1-dimensional value
        node_data(storage1d_type const& value)
          : data_(new detail::node_data_storage<1, T>(value))
        {
        }
        node_data(storage1d_type && value)
          : data_(new detail::node_data_storage<1, T>(std::move(value)))
        {
        }
        node_data(std::vector<T> const& value)
          : data_(new detail::node_data_storage<1, T>(value))
        {
        }

        /// Create node data for a 2-dimensional value
        node_data(storage2d_type const& value)
          : data_(new detail::node_data_storage<2, T>(value))
        {
        }
        node_data(storage2d_type && value)
          : data_(new detail::node_data_storage<2, T>(std::move(value)))
        {
        }

        /// Access a specific element of the underlying N-dimensional array
        T const& operator[](std::ptrdiff_t index) const
        {
            return (*data_)[index];
        }
        T const& operator[](dimensions_type const& indicies) const
        {
            return (*data_)[indicies];
        }

        using iterator = typename detail::node_data_storage_base<T>::iterator;

        /// Get iterator referring to the beginning of the underlying data
        iterator begin() const
        {
            return data_->begin();
        }
        /// Get iterator referring to the end of the underlying data
        iterator end() const
        {
            return data_->end();
        }

        /// Extract the dimensionality of the underlying data array.
        std::size_t num_dimensions() const
        {
            return data_->num_dimensions();
        }

        /// Extract the dimensional extends of the underlying data array.
        dimensions_type const& dimensions() const
        {
            return data_->dimensions();
        }
        std::size_t dimension(std::size_t dim) const
        {
            return data_->dimension(dim);
        }

    private:
        /// \cond NOINTERNAL
        friend class hpx::serialization::access;

        template <typename Archive>
        void serialize(Archive& ar, unsigned)
        {
            ar & data_;
        }

        boost::intrusive_ptr<detail::node_data_storage_base<T>> data_;
        /// \endcond
    };
}}

#endif
