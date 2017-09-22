//  Copyright (c) 2017 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_IR_NODE_DATA_AUG_26_2017_0924AM)
#define PHYLANX_IR_NODE_DATA_AUG_26_2017_0924AM

#include <phylanx/config.hpp>
#include <phylanx/util/serialization/eigen.hpp>

#include <hpx/include/serialization.hpp>
#include <hpx/include/util.hpp>

#include <Eigen/Dense>

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

        /// \endcond
    }

    ///////////////////////////////////////////////////////////////////////////
    template <typename T>
    class node_data
    {
    public:
        constexpr static std::size_t const max_dimensions = 2;

        using dimensions_type = std::array<std::ptrdiff_t, max_dimensions>;
        using storage_type = Eigen::Matrix<T, Eigen::Dynamic, Eigen::Dynamic>;
        using array_storage_type = Eigen::Array<T, Eigen::Dynamic, Eigen::Dynamic>;
        using constant_type = Eigen::DenseBase<storage_type>;

        using storage1d_type = Eigen::Matrix<T, Eigen::Dynamic, 1>;

        node_data() = default;

        node_data(dimensions_type const& dims)
        {
            if (dims[1] != 0)
            {
                data_ = constant_type::Constant(dims[0], dims[1]);
            }
            else if (dims[0] != 0)
            {
                data_ = constant_type::Constant(dims[0], 1);
            }
            else
            {
                data_ = constant_type::Constant(1, 1);
            }
        }

        node_data(dimensions_type const& dims, T default_value)
        {
            if (dims[1] != 0)
            {
                data_ = constant_type::Constant(dims[0], dims[1], default_value);
            }
            else if (dims[0] != 0)
            {
                data_ = constant_type::Constant(dims[0], 1, default_value);
            }
            else
            {
                data_ = constant_type::Constant(1, 1, default_value);
            }
        }

        /// Create node data for a 0-dimensional value
        node_data(T const& value)
          : data_(constant_type::Constant(1, 1, value))
        {
        }
        node_data(T && value)
          : data_(constant_type::Constant(1, 1, std::move(value)))
        {
        }

        /// Create node data for a 1-dimensional value
        node_data(storage1d_type const& values)
          : data_(values)
        {
        }
        node_data(storage1d_type && values)
          : data_(std::move(values))
        {
        }
        node_data(std::vector<T> const& values)
          : data_(Eigen::Map<storage1d_type const, Eigen::Unaligned>(
                values.data(), values.size()))
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

        /// Access a specific element of the underlying N-dimensional array
        T const& operator[](std::ptrdiff_t index) const
        {
            return data_.data()[index];
        }
        T const& operator[](dimensions_type const& indicies) const
        {
            return data_(indicies[0], indicies[1]);
        }

        using iterator = detail::node_data_iterator<T>;
        using const_iterator = detail::node_data_iterator<T>;

        /// Get iterator referring to the beginning of the underlying data
        iterator begin() const
        {
            return iterator{data_.data()};
        }
        /// Get iterator referring to the end of the underlying data
        iterator end() const
        {
            return iterator{data_.data() + data_.size()};
        }

        iterator cbegin() const
        {
            return begin();
        }
        /// Get iterator referring to the end of the underlying data
        iterator cend() const
        {
            return end();
        }

//         T const* data() const
//         {
//             return data_.data();
//         }
        std::size_t size() const
        {
            return data_.size();
        }

        storage_type const& matrix() const
        {
            return data_;
        }

        /// Extract the dimensionality of the underlying data array.
        std::size_t num_dimensions() const
        {
            if (data_.cols() != 1)
            {
                return 2;
            }
            if (data_.rows() != 1)
            {
                return 1;
            }
            return 0;
        }

        /// Extract the dimensional extends of the underlying data array.
        dimensions_type dimensions() const
        {
            return dimensions_type{data_.rows(), data_.cols()};
        }
        std::size_t dimension(std::size_t dim) const
        {
            return (dim == 0) ? data_.rows() : data_.cols();
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

        return std::equal(hpx::util::begin(lhs), hpx::util::end(lhs),
            hpx::util::begin(rhs), hpx::util::end(rhs));
    }

    template <typename T>
    bool operator!=(node_data<T> const& lhs, node_data<T> const& rhs)
    {
        return !(lhs == rhs);
    }

    template <typename T>
    inline std::ostream& operator<<(std::ostream& out, node_data<T> const& nd)
    {
        out << "node_data<T>: ";

        std::ptrdiff_t dims = nd.num_dimensions();
        out << std::string(dims, '[');
        out << nd[0];
        if (dims)
            out << ", ...";
        out << std::string(dims, ']');
        return out;
    }
}}

#endif
