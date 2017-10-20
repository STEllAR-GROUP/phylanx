//  Copyright (c) 2017 Hartmut Kaiser
//  Copyright (c) 2017 Parsa Amini
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_IR_NODE_DATA_AUG_26_2017_0924AM)
#define PHYLANX_IR_NODE_DATA_AUG_26_2017_0924AM

#include <phylanx/config.hpp>
#include <phylanx/util/serialization/blaze.hpp>

#include <hpx/include/serialization.hpp>
#include <hpx/include/util.hpp>

#include <blaze/Math.h>

#include <boost/intrusive_ptr.hpp>

#include <algorithm>
#include <array>
#include <cstddef>
#include <iosfwd>
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
        class node_value_iterator
          : public hpx::util::iterator_facade<node_value_iterator<T>,
                T,
                std::random_access_iterator_tag,
                T const&,
                std::size_t,
                T const*>
        {
        private:
            using base_type =
                hpx::util::iterator_facade<node_value_iterator<T>,
                    T,
                    std::random_access_iterator_tag,
                    T const&,
                    std::size_t,
                    T const*>;

        public:
            node_value_iterator(T const* p)
              : p_(p)
            {
            }

        private:
            friend class hpx::util::iterator_core_access;

            typename base_type::reference dereference() const
            {
                return *p_;
            }

            bool equal(node_value_iterator const& x) const
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
                node_value_iterator const& y) const
            {
                return y.p_ - p_;
            }

            T const* p_;
        };

        /// \endcond
    }

    ///////////////////////////////////////////////////////////////////////////
    template <typename T>
    class node_data
    {
    public:
        constexpr static std::size_t const max_dimensions = 2;

        using dimensions_type = std::array<std::size_t, max_dimensions>;
        using storage_type = blaze::DynamicMatrix<T>;
        using storage1d_type = blaze::DynamicVector<T, blaze::rowVector>;

        node_data() = default;

        node_data(dimensions_type const& dims)
        {
            if (dims[1] != 1)
            {
                data_ = storage_type(dims[0], dims[1]);
            }
            else if (dims[0] != 1)
            {
                data_ = storage_type(dims[0], 1UL);
            }
            else
            {
                data_ = storage_type(1UL, 1UL);
            }
        }

        node_data(dimensions_type const& dims, T default_value)
        {
            if (dims[1] != 1)
            {
                data_ = storage_type(dims[0], dims[1], default_value);
            }
            else if (dims[0] != 1)
            {
                data_ = storage_type(dims[0], 1UL, default_value);
            }
            else
            {
                data_ = storage_type(1UL, 1UL, default_value);
            }
        }

        /// Create node data for a 0-dimensional value
        node_data(T const& value)
          : data_(storage_type(1UL, 1UL, value))
        {
        }
        node_data(T && value)
          : data_(storage_type(1UL, 1UL, std::move(value)))
        {
        }

        /// Create node data for a 1-dimensional value
        node_data(storage1d_type const& values)
          : data_(storage_type(1UL, values.size()))
        {
            blaze::row(data_, 0UL) = values;
        }
        node_data(storage1d_type && values)
          : data_(storage_type(1UL, values.size()))
        {
            blaze::row(data_, 0UL) = std::move(values);
        }
        node_data(std::vector<T> const& values)
          : data_(storage_type(1UL, values.size(), values.data()))
        {
        }

        /// Create node data for a 2-dimensional value
        node_data(storage_type const& values)
          : data_(values)
        {
        }
        node_data(storage_type && values)
          : data_(std::move(values))
        {
        }

        node_data(node_data const& d)
          : data_(d.data_)
        {
        }
        node_data(node_data && d)
          : data_(std::move(d.data_))
        {
        }

        node_data& operator=(node_data const& d)
        {
            if (this != &d)
            {
                data_ = d.data_;
            }
            return *this;
        }
        node_data& operator=(node_data && d)
        {
            if (this != &d)
            {
                data_ = std::move(d.data_);
            }
            return *this;
        }


        /// Access a specific element of the underlying N-dimensional array
        T& operator[](std::size_t index)
        {
            std::size_t idx_m = index / data_.columns();
            std::size_t idx_n = index % data_.rows();
            return data_(idx_m, idx_n);
        }
        T& operator[](dimensions_type const& indicies)
        {
            return data_(indicies[0], indicies[1]);
        }

        T const& operator[](std::size_t index) const
        {
            std::size_t idx_m = index / data_.columns();
            std::size_t idx_n = index % data_.rows();
            return data_(idx_m, idx_n);
        }
        T const& operator[](dimensions_type const& indicies) const
        {
            return data_(indicies[0], indicies[1]);
        }

        /// Get iterator referring to the beginning of the underlying data
        blaze::DenseIterator<const T, true> begin() const
        {
            return data_.begin(0UL);
        }
        /// Get iterator referring to the end of the underlying data
        blaze::DenseIterator<const T, true> end() const
        {
            return data_.end(data_.rows() - 1UL);
        }

        blaze::DenseIterator<const T, true> cbegin() const
        {
            return data_.cbegin(0UL);
        }
        /// Get iterator referring to the end of the underlying data
        blaze::DenseIterator<const T, true> cend() const
        {
            return data_.cend(data_.rows() - 1UL);
        }

        //T* data()
        //{
        //    return data_.data();
        //}
        //T const* data() const
        //{
        //    return data_.data();
        //}
        std::size_t size() const
        {
            return data_.rows() * data_.columns();
        }

        storage_type& matrix()
        {
            return data_;
        }
        storage_type const& matrix() const
        {
            return data_;
        }

        /// Extract the dimensionality of the underlying data array.
        std::size_t num_dimensions() const
        {
            if (data_.rows() != 1)
            {
                return 2;
            }
            if (data_.columns() != 1)
            {
                return 1;
            }
            return 0;
        }

        /// Extract the dimensional extends of the underlying data array.
        dimensions_type dimensions() const
        {
            return dimensions_type{data_.columns(), data_.rows()};
        }
        std::size_t dimension(std::size_t dim) const
        {
            return (dim == 0) ? data_.columns() : data_.rows();
        }

        explicit operator bool() const;
        bool operator!() const
        {
            return !bool(*this);
        }

    private:
        /// \cond NOINTERNAL
        friend class hpx::serialization::access;

        template <typename Archive>
        void serialize(Archive& ar, unsigned)
        {
            ar & data_;
        }

        storage_type data_;
        /// \endcond
    };

    ///////////////////////////////////////////////////////////////////////////
    template <typename T>
    bool operator==(node_data<T> const& lhs, node_data<T> const& rhs)
    {
        if (lhs.num_dimensions() != rhs.num_dimensions() ||
            lhs.dimensions() != rhs.dimensions())
        {
            return false;
        }

        return lhs.matrix() == rhs.matrix();
    }

    template <typename T>
    bool operator!=(node_data<T> const& lhs, node_data<T> const& rhs)
    {
        return !(lhs == rhs);
    }

    PHYLANX_EXPORT std::ostream& operator<<(
        std::ostream& out, node_data<double> const& nd);
}}

#endif
