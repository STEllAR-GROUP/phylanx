//  Copyright (c) 2017 Hartmut Kaiser
//  Copyright (c) 2017 Parsa Amini
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_IR_NODE_DATA_AUG_26_2017_0924AM)
#define PHYLANX_IR_NODE_DATA_AUG_26_2017_0924AM

#include <phylanx/config.hpp>
#include <phylanx/util/serialization/blaze.hpp>
#include <phylanx/util/serialization/variant.hpp>
#include <phylanx/util/variant.hpp>

#include <hpx/include/serialization.hpp>
#include <hpx/include/util.hpp>
#include <hpx/throw_exception.hpp>

#include <blaze/Math.h>

#include <algorithm>
#include <array>
#include <cstddef>
#include <iosfwd>
#include <iterator>
#include <vector>


namespace phylanx { namespace ir
{
    template <typename T>
    class node_data
    {
    public:
        constexpr static std::size_t const max_dimensions = 2;

        using dimensions_type = std::array<std::size_t, max_dimensions>;

        using storage0d_type = T;
        using storage1d_type = blaze::DynamicVector<T>;
        using storage2d_type = blaze::DynamicMatrix<T>;

        using storage_type =
            util::variant<storage0d_type, storage1d_type, storage2d_type>;

        node_data() = default;

        node_data(dimensions_type const& dims)
        {
            if (dims[1] != 1)
            {
                data_ = storage2d_type(dims[0], dims[1]);
            }
            else if (dims[0] != 1)
            {
                data_ = storage1d_type(dims[0]);
            }
            else
            {
                data_ = storage0d_type();
            }
        }

        node_data(dimensions_type const& dims, T default_value)
        {
            if (dims[1] != 1)
            {
                data_ = storage2d_type(dims[0], dims[1], default_value);
            }
            else if (dims[0] != 1)
            {
                data_ = storage1d_type(dims[0], default_value);
            }
            else
            {
                data_ = default_value;
            }
        }

        /// Create node data for a 0-dimensional value
        node_data(storage0d_type const& value)
          : data_(value)
        {
        }
        node_data(storage0d_type && value)
          : data_(std::move(value))
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
          : data_(storage1d_type(values.size(), values.data()))
        {
        }

        /// Create node data for a 2-dimensional value
        node_data(storage2d_type const& values)
          : data_(values)
        {
        }
        node_data(storage2d_type && values)
          : data_(std::move(values))
        {
        }

        /// Create node data from a node data
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
            switch(data_.index())
            {
            case 0:
                return scalar();

            case 1:
                return vector()[index];

            case 2:
                {
                    auto& m = matrix();
                    std::size_t idx_m = index / m.columns();
                    std::size_t idx_n = index % m.columns();
                    return m(idx_m, idx_n);
                }

            default:
                break;
            }

            HPX_THROW_EXCEPTION(hpx::invalid_status,
                "phylanx::ir::node_data<T>::operator[]()",
                "node_data object holds unsupported data type");
        }
        T& operator[](dimensions_type const& indicies)
        {
            switch(data_.index())
            {
            case 0:
                return scalar();

            case 1:
                return vector()[indicies[0]];

            case 2:
                return matrix()(indicies[0], indicies[1]);

            default:
                break;
            }

            HPX_THROW_EXCEPTION(hpx::invalid_status,
                "phylanx::ir::node_data<T>::operator[]()",
                "node_data object holds unsupported data type");
        }

        T const& operator[](std::size_t index) const
        {
            switch(data_.index())
            {
            case 0:
                return scalar();

            case 1:
                return vector()[index];

            case 2:
                {
                    auto const& m = matrix();
                    std::size_t idx_m = index / m.columns();
                    std::size_t idx_n = index % m.columns();
                    return m(idx_m, idx_n);
                }

            default:
                break;
            }

            HPX_THROW_EXCEPTION(hpx::invalid_status,
                "phylanx::ir::node_data<T>::operator[]()",
                "node_data object holds unsupported data type");
        }
        T const& operator[](dimensions_type const& indicies) const
        {
            switch(data_.index())
            {
            case 0:
                return scalar();

            case 1:
                return vector()[indicies[0]];

            case 2:
                return matrix()(indicies[0], indicies[1]);

            default:
                break;
            }

            HPX_THROW_EXCEPTION(hpx::invalid_status,
                "phylanx::ir::node_data<T>::operator[]()",
                "node_data object holds unsupported data type");
        }

        using iterator = blaze::DenseIterator<T, true>;
        using const_iterator = blaze::DenseIterator<T const, true>;

//         /// Get iterator referring to the beginning of the underlying data
//         const_iterator begin() const
//         {
//             return data_.begin(0UL);
//         }
//         /// Get iterator referring to the end of the underlying data
//         const_iterator end() const
//         {
//             return data_.end(data_.rows() - 1UL);
//         }
//
//         const_iterator cbegin() const
//         {
//             return data_.cbegin(0UL);
//         }
//         /// Get iterator referring to the end of the underlying data
//         const_iterator cend() const
//         {
//             return data_.cend(data_.rows() - 1UL);
//         }

        std::size_t size() const
        {
            switch(data_.index())
            {
            case 0:
                return 1;

            case 1:
                return vector().size();

            case 2:
                {
                    auto const& m = matrix();
                    return m.rows() * m.columns();
                }

            default:
                break;
            }

            HPX_THROW_EXCEPTION(hpx::invalid_status,
                "phylanx::ir::node_data<T>::operator[]()",
                "node_data object holds unsupported data type");
        }

        storage2d_type& matrix()
        {
            storage2d_type* m = util::get_if<storage2d_type>(&data_);
            if (m == nullptr)
            {
                HPX_THROW_EXCEPTION(hpx::invalid_status,
                    "phylanx::ir::node_data<T>::matrix()",
                    "node_data object holds unsupported data type");
            }
            return *m;
        }
        storage2d_type const& matrix() const
        {
            storage2d_type const* m = util::get_if<storage2d_type>(&data_);
            if (m == nullptr)
            {
                HPX_THROW_EXCEPTION(hpx::invalid_status,
                    "phylanx::ir::node_data<T>::matrix()",
                    "node_data object holds unsupported data type");
            }
            return *m;
        }
        void matrix(storage2d_type const& val)
        {
            data_ = val;
        }
        void matrix(storage2d_type && val)
        {
            data_ = std::move(val);
        }

        storage1d_type& vector()
        {
            storage1d_type* v = util::get_if<storage1d_type>(&data_);
            if (v == nullptr)
            {
                HPX_THROW_EXCEPTION(hpx::invalid_status,
                    "phylanx::ir::node_data<T>::vector()",
                    "node_data object holds unsupported data type");
            }
            return *v;
        }
        storage1d_type const& vector() const
        {
            storage1d_type const* v = util::get_if<storage1d_type>(&data_);
            if (v == nullptr)
            {
                HPX_THROW_EXCEPTION(hpx::invalid_status,
                    "phylanx::ir::node_data<T>::vector()",
                    "node_data object holds unsupported data type");
            }
            return *v;
        }
        void vector(storage1d_type const& val)
        {
            data_ = val;
        }
        void vector(storage1d_type && val)
        {
            data_ = std::move(val);
        }

        storage0d_type& scalar()
        {
            storage0d_type* s = util::get_if<storage0d_type>(&data_);
            if (s == nullptr)
            {
                HPX_THROW_EXCEPTION(hpx::invalid_status,
                    "phylanx::ir::node_data<T>::scalar()",
                    "node_data object holds unsupported data type");
            }
            return *s;
        }
        storage0d_type const& scalar() const
        {
            storage0d_type const* s = util::get_if<storage0d_type>(&data_);
            if (s == nullptr)
            {
                HPX_THROW_EXCEPTION(hpx::invalid_status,
                    "phylanx::ir::node_data<T>::scalar()",
                    "node_data object holds unsupported data type");
            }
            return *s;
        }
        void scalar(storage0d_type val)
        {
            data_ = val;
        }

        /// Extract the dimensionality of the underlying data array.
        std::size_t num_dimensions() const
        {
            return data_.index();
        }

        /// Extract the dimensional extends of the underlying data array.
        dimensions_type dimensions() const
        {
            switch(data_.index())
            {
            case 0:
                return dimensions_type{1ul, 1ul};

            case 1:
                return dimensions_type{vector().size(), 1ul};

            case 2:
                {
                    auto const& m = matrix();
                    return dimensions_type{m.rows(), m.columns()};
                }

            default:
                break;
            }

            HPX_THROW_EXCEPTION(hpx::invalid_status,
                "phylanx::ir::node_data<T>::dimensions()",
                "node_data object holds unsupported data type");
        }

        size_t dimension(int dim) const
        {
            switch(data_.index())
            {
            case 0:
                return 1ul;

            case 1:
                return (dim == 0) ? vector().size() : 1ul;

            case 2:
                {
                    auto const& m = matrix();
                    return (dim == 0) ? m.rows() : m.columns();
                }

            default:
                break;
            }

            HPX_THROW_EXCEPTION(hpx::invalid_status,
                "phylanx::ir::node_data<T>::dimension()",
                "node_data object holds unsupported data type");
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

        switch (lhs.num_dimensions())
        {
        case 0:
            return lhs.scalar() == rhs.scalar();

        case 1:
            return lhs.vector() == rhs.vector();

        case 2:
            return lhs.matrix() == rhs.matrix();

        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::invalid_status,
            "phylanx::ir::node_data<T>::operator==()",
            "node_data object holds unsupported data type");
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
