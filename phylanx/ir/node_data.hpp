//  Copyright (c) 2017-2018 Hartmut Kaiser
//  Copyright (c) 2017 Parsa Amini
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_IR_NODE_DATA_AUG_26_2017_0924AM)
#define PHYLANX_IR_NODE_DATA_AUG_26_2017_0924AM

#include <phylanx/config.hpp>
#include <phylanx/util/variant.hpp>

#include <hpx/include/serialization.hpp>
#include <hpx/include/util.hpp>
#include <hpx/throw_exception.hpp>

#include <blaze/Math.h>

#include <array>
#include <atomic>
#include <cstddef>
#include <iosfwd>
#include <vector>

namespace phylanx { namespace ir
{
#if defined(PHYLANX_DEBUG)
    PHYLANX_EXPORT extern std::atomic<std::size_t> count_copy_constructions_;
    PHYLANX_EXPORT extern std::atomic<std::size_t> count_move_constructions_;
    PHYLANX_EXPORT extern std::atomic<std::size_t> count_copy_assignments_;
    PHYLANX_EXPORT extern std::atomic<std::size_t> count_move_assignments_;
#endif

    PHYLANX_EXPORT void reset_node_statistics();
    PHYLANX_EXPORT void print_node_statistics();

    template <typename T>
    class node_data
    {
    public:
        constexpr static std::size_t const max_dimensions = 2;

        using dimensions_type = std::array<std::size_t, max_dimensions>;

        using storage0d_type = T;
        using storage1d_type = blaze::DynamicVector<T>;
        using storage2d_type = blaze::DynamicMatrix<T>;

        using custom_storage1d_type = blaze::CustomVector<T, true, true>;
        using custom_storage2d_type = blaze::CustomMatrix<T, true, true>;

        using storage_type =
            util::variant<storage0d_type, storage1d_type, storage2d_type,
                custom_storage1d_type, custom_storage2d_type>;

        node_data() = default;

        explicit node_data(dimensions_type const& dims)
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

        explicit node_data(dimensions_type const& dims, T default_value)
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
        explicit node_data(storage0d_type const& value)
          : data_(value)
        {
        }
        explicit node_data(storage0d_type && value)
          : data_(std::move(value))
        {
        }

        /// Create node data for a 1-dimensional value
        explicit node_data(storage1d_type const& values)
          : data_(values)
        {
#if defined(PHYLANX_DEBUG)
            ++count_copy_constructions_;
#endif
        }
        explicit node_data(storage1d_type && values)
          : data_(std::move(values))
        {
#if defined(PHYLANX_DEBUG)
            ++count_move_constructions_;
#endif
        }
        explicit node_data(std::vector<T> const& values)
          : data_(storage1d_type(values.size(), values.data()))
        {
        }

        explicit node_data(custom_storage1d_type const& values)
          : data_(custom_storage1d_type{values.data(), values.size()})
        {
#if defined(PHYLANX_DEBUG)
            ++count_move_constructions_;
#endif
        }
        explicit node_data(custom_storage1d_type && values)
          : data_(std::move(values))
        {
#if defined(PHYLANX_DEBUG)
            ++count_move_constructions_;
#endif
        }

        /// Create node data for a 2-dimensional value
        explicit node_data(storage2d_type const& values)
          : data_(values)
        {
#if defined(PHYLANX_DEBUG)
            ++count_copy_constructions_;
#endif
        }
        explicit node_data(storage2d_type && values)
          : data_(std::move(values))
        {
#if defined(PHYLANX_DEBUG)
            ++count_move_constructions_;
#endif
        }

        explicit node_data(custom_storage2d_type const& values)
          : data_(custom_storage2d_type{values.data(), values.rows(),
                values.columns(), values.spacing()})
        {
#if defined(PHYLANX_DEBUG)
            ++count_copy_constructions_;
#endif
        }
        explicit node_data(custom_storage2d_type && values)
          : data_(std::move(values))
        {
#if defined(PHYLANX_DEBUG)
            ++count_move_constructions_;
#endif
        }

    private:
        static storage_type init_data_from(node_data const& d)
        {
            switch (d.data_.index())
            {
            case 0:
                return d.data_;

            case 1: HPX_FALLTHROUGH;
            case 2:
                {
#if defined(PHYLANX_DEBUG)
                    ++count_copy_constructions_;
#endif
                    return d.data_;
                }
                break;

            case 3:
                {
#if defined(PHYLANX_DEBUG)
                    ++count_move_constructions_;
#endif
                    auto v = d.vector();
                    return custom_storage1d_type{
                        v.data(), v.size(), v.spacing()};
                }
                break;

            case 4:
                {
#if defined(PHYLANX_DEBUG)
                    ++count_move_constructions_;
#endif
                    auto m = d.matrix();
                    return custom_storage2d_type{
                        m.data(), m.rows(), m.columns(), m.spacing()};
                }
                break;

            default:
                HPX_THROW_EXCEPTION(hpx::invalid_status,
                    "phylanx::ir::node_data<T>::node_data<T>",
                    "node_data object holds unsupported data type");
            }
        }

    public:
        /// Create node data from a node data
        node_data(node_data const& d)
          : data_(init_data_from(d))
        {
        }
        node_data(node_data && d)
          : data_(std::move(d.data_))
        {
#if defined(PHYLANX_DEBUG)
            ++count_move_constructions_;
#endif
        }

        node_data& operator=(storage0d_type val)
        {
            data_ = val;
            return *this;
        }

        node_data& operator=(storage1d_type const& val)
        {
#if defined(PHYLANX_DEBUG)
            ++count_copy_assignments_;
#endif
            data_ = val;
            return *this;
        }
        node_data& operator=(storage1d_type && val)
        {
#if defined(PHYLANX_DEBUG)
            ++count_move_assignments_;
#endif
            data_ = std::move(val);
            return *this;
        }

        node_data& operator=(custom_storage1d_type const& val)
        {
#if defined(PHYLANX_DEBUG)
            ++count_move_assignments_;
#endif
            data_ =
                custom_storage1d_type{val.data(), val.size(), val.spacing()};
            return *this;
        }
        node_data& operator=(custom_storage1d_type && val)
        {
#if defined(PHYLANX_DEBUG)
            ++count_move_assignments_;
#endif
            data_ = std::move(val);
            return *this;
        }

        node_data& operator=(storage2d_type const& val)
        {
#if defined(PHYLANX_DEBUG)
            ++count_copy_assignments_;
#endif
            data_ = val;
            return *this;
        }
        node_data& operator=(storage2d_type && val)
        {
#if defined(PHYLANX_DEBUG)
            ++count_move_assignments_;
#endif
            data_ = std::move(val);
            return *this;
        }

        node_data& operator=(custom_storage2d_type const& val)
        {
#if defined(PHYLANX_DEBUG)
            ++count_move_assignments_;
#endif
            data_ = custom_storage2d_type{
                val.data(), val.rows(), val.columns(), val.spacing()};
            return *this;
        }
        node_data& operator=(custom_storage2d_type && val)
        {
#if defined(PHYLANX_DEBUG)
            ++count_move_assignments_;
#endif
            data_ = std::move(val);
            return *this;
        }

    private:
        storage_type copy_data_from(node_data const& d)
        {
            switch (d.data_.index())
            {
            case 0:
                return d.data_;

            case 1: HPX_FALLTHROUGH;
            case 2:
                {
#if defined(PHYLANX_DEBUG)
                    ++count_copy_assignments_;
#endif
                    return d.data_;
                }
                break;

            case 3:
                {
#if defined(PHYLANX_DEBUG)
                    ++count_move_assignments_;
#endif
                    auto v = d.vector();
                    return custom_storage1d_type{
                        v.data(), v.size(), v.spacing()};
                }
                break;

            case 4:
                {
#if defined(PHYLANX_DEBUG)
                    ++count_move_assignments_;
#endif
                    auto m = d.matrix();
                    return custom_storage2d_type{
                        m.data(), m.rows(), m.columns(), m.spacing()};
                }
                break;

            default:
                HPX_THROW_EXCEPTION(hpx::invalid_status,
                    "phylanx::ir::node_data<T>::node_data<T>",
                    "node_data object holds unsupported data type");
            }
        }

    public:
        node_data& operator=(node_data const& d)
        {
            if (this != &d)
            {
                data_ = copy_data_from(d);
            }
            return *this;
        }
        node_data& operator=(node_data && d)
        {
#if defined(PHYLANX_DEBUG)
            ++count_move_assignments_;
#endif
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

            case 1: HPX_FALLTHROUGH;
            case 3:
                return vector()[index];

            case 2: HPX_FALLTHROUGH;
            case 4:
                {
                    auto m = matrix();
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

            case 1: HPX_FALLTHROUGH;
            case 3:
                return vector()[indicies[0]];

            case 2: HPX_FALLTHROUGH;
            case 4:
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

            case 1: HPX_FALLTHROUGH;
            case 3:
                return vector()[index];

            case 2: HPX_FALLTHROUGH;
            case 4:
                {
                    auto m = matrix();
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

            case 1: HPX_FALLTHROUGH;
            case 3:
                return vector()[indicies[0]];

            case 2: HPX_FALLTHROUGH;
            case 4:
                return matrix()(indicies[0], indicies[1]);

            default:
                break;
            }

            HPX_THROW_EXCEPTION(hpx::invalid_status,
                "phylanx::ir::node_data<T>::operator[]()",
                "node_data object holds unsupported data type");
        }

        std::size_t size() const
        {
            switch(data_.index())
            {
            case 0:
                return 1;

            case 1: HPX_FALLTHROUGH;
            case 3:
                return vector().size();

            case 2: HPX_FALLTHROUGH;
            case 4:
                {
                    auto m = matrix();
                    return m.rows() * m.columns();
                }

            default:
                break;
            }

            HPX_THROW_EXCEPTION(hpx::invalid_status,
                "phylanx::ir::node_data<T>::operator[]()",
                "node_data object holds unsupported data type");
        }

        storage2d_type& matrix_non_ref()
        {
            storage2d_type* m = util::get_if<storage2d_type>(&data_);
            if (m == nullptr)
            {
                HPX_THROW_EXCEPTION(hpx::invalid_status,
                    "phylanx::ir::node_data<T>::matrix_non_ref()",
                    "node_data object holds unsupported data type");
            }
            return *m;
        }
        storage2d_type const& matrix_non_ref() const
        {
            storage2d_type const* m = util::get_if<storage2d_type>(&data_);
            if (m == nullptr)
            {
                HPX_THROW_EXCEPTION(hpx::invalid_status,
                    "phylanx::ir::node_data<T>::matrix_non_ref()",
                    "node_data object holds unsupported data type");
            }
            return *m;
        }

        storage2d_type matrix_copy() const
        {
            custom_storage2d_type const* cm =
                util::get_if<custom_storage2d_type>(&data_);
            if (cm != nullptr)
            {
                return storage2d_type{*cm};
            }

            storage2d_type const* m = util::get_if<storage2d_type>(&data_);
            if (m != nullptr)
            {
                return *m;
            }

            HPX_THROW_EXCEPTION(hpx::invalid_status,
                "phylanx::ir::node_data<T>::matrix()",
                "node_data object holds unsupported data type");
        }

        custom_storage2d_type matrix()
        {
            custom_storage2d_type* cm =
                util::get_if<custom_storage2d_type>(&data_);
            if (cm != nullptr)
            {
                return *cm;
            }

            storage2d_type* m = util::get_if<storage2d_type>(&data_);
            if (m != nullptr)
            {
                return custom_storage2d_type(
                    m->data(), m->rows(), m->columns(), m->spacing());
            }

            HPX_THROW_EXCEPTION(hpx::invalid_status,
                "phylanx::ir::node_data<T>::matrix()",
                "node_data object holds unsupported data type");
        }
        custom_storage2d_type matrix() const
        {
            custom_storage2d_type const* cm =
                util::get_if<custom_storage2d_type>(&data_);
            if (cm != nullptr)
            {
                return custom_storage2d_type(const_cast<T*>(cm->data()),
                    cm->rows(), cm->columns(), cm->spacing());
            }

            storage2d_type const* m = util::get_if<storage2d_type>(&data_);
            if (m != nullptr)
            {
                return custom_storage2d_type(const_cast<T*>(m->data()),
                    m->rows(), m->columns(), m->spacing());
            }

            HPX_THROW_EXCEPTION(hpx::invalid_status,
                "phylanx::ir::node_data<T>::matrix()",
                "node_data object holds unsupported data type");
        }

        storage1d_type& vector_non_ref()
        {
            storage1d_type* v = util::get_if<storage1d_type>(&data_);
            if (v == nullptr)
            {
                HPX_THROW_EXCEPTION(hpx::invalid_status,
                    "phylanx::ir::node_data<T>::vector_non_ref()",
                    "node_data object holds unsupported data type");
            }
            return *v;
        }
        storage1d_type const& vector_non_ref() const
        {
            storage1d_type const* v = util::get_if<storage1d_type>(&data_);
            if (v == nullptr)
            {
                HPX_THROW_EXCEPTION(hpx::invalid_status,
                    "phylanx::ir::node_data<T>::vector_non_ref()",
                    "node_data object holds unsupported data type");
            }
            return *v;
        }

        storage1d_type vector_copy() const
        {
            custom_storage1d_type const* cv =
                util::get_if<custom_storage1d_type>(&data_);
            if (cv != nullptr)
            {
                return storage1d_type{*cv};
            }

            storage1d_type const* v = util::get_if<storage1d_type>(&data_);
            if (v != nullptr)
            {
                return *v;
            }

            HPX_THROW_EXCEPTION(hpx::invalid_status,
                "phylanx::ir::node_data<T>::vector()",
                "node_data object holds unsupported data type");
        }

        custom_storage1d_type vector()
        {
            custom_storage1d_type* cv =
                util::get_if<custom_storage1d_type>(&data_);
            if (cv != nullptr)
                return *cv;

            storage1d_type* v = util::get_if<storage1d_type>(&data_);
            if (v != nullptr)
            {
                return custom_storage1d_type(v->data(), v->size(), v->spacing());
            }

            HPX_THROW_EXCEPTION(hpx::invalid_status,
                "phylanx::ir::node_data<T>::vector()",
                "node_data object holds unsupported data type");
        }
        custom_storage1d_type vector() const
        {
            custom_storage1d_type const* cv =
                util::get_if<custom_storage1d_type>(&data_);
            if (cv != nullptr)
            {
                return custom_storage1d_type{
                    const_cast<T*>(cv->data()), cv->size(), cv->spacing()};
            }

            storage1d_type const* v = util::get_if<storage1d_type>(&data_);
            if (v != nullptr)
            {
                return custom_storage1d_type{
                    const_cast<T*>(v->data()), v->size(), v->spacing()};
            }

            HPX_THROW_EXCEPTION(hpx::invalid_status,
                "phylanx::ir::node_data<T>::vector()",
                "node_data object holds unsupported data type");
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

        /// Extract the dimensionality of the underlying data array.
        std::size_t num_dimensions() const
        {
            switch(data_.index())
            {
            case 0:
                return 0;

            case 1: HPX_FALLTHROUGH;
            case 3:
                return 1;

            case 2: HPX_FALLTHROUGH;
            case 4:
                return 2;

            default:
                break;
            }

            HPX_THROW_EXCEPTION(hpx::invalid_status,
                "phylanx::ir::node_data<T>::num_dimensions()",
                "node_data object holds unsupported data type");
        }

        /// Extract the dimensional extends of the underlying data array.
        dimensions_type dimensions() const
        {
            switch(data_.index())
            {
            case 0:
                return dimensions_type{1ul, 1ul};

            case 1: HPX_FALLTHROUGH;
            case 3:
                return dimensions_type{vector().size(), 1ul};

            case 2: HPX_FALLTHROUGH;
            case 4:
                {
                    auto m = matrix();
                    return dimensions_type{m.rows(), m.columns()};
                }

            default:
                break;
            }

            HPX_THROW_EXCEPTION(hpx::invalid_status,
                "phylanx::ir::node_data<T>::dimensions()",
                "node_data object holds unsupported data type");
        }

        std::size_t dimension(int dim) const
        {
            switch(data_.index())
            {
            case 0:
                return 1ul;

            case 1: HPX_FALLTHROUGH;
            case 3:
                return (dim == 0) ? vector().size() : 1ul;

            case 2: HPX_FALLTHROUGH;
            case 4:
                {
                    auto m = matrix();
                    return (dim == 0) ? m.rows() : m.columns();
                }

            default:
                break;
            }

            HPX_THROW_EXCEPTION(hpx::invalid_status,
                "phylanx::ir::node_data<T>::dimension()",
                "node_data object holds unsupported data type");
        }

        /// Return a new instance of node_data referring to this instance.
        node_data<T> ref() const
        {
            switch(data_.index())
            {
            case 1:
                return node_data<T>{vector()};

            case 2:
                return node_data<T>{matrix()};

            case 0: HPX_FALLTHROUGH;
            case 3: HPX_FALLTHROUGH;
            case 4:
                return *this;

            default:
                break;
            }

            HPX_THROW_EXCEPTION(hpx::invalid_status,
                "phylanx::ir::node_data<T>::ref()",
                "node_data object holds unsupported data type");
        }

        /// Return a new instance of node_data holding a copy of this instance.
        node_data<T> copy() const
        {
            switch(data_.index())
            {
            case 0: HPX_FALLTHROUGH;
            case 1: HPX_FALLTHROUGH;
            case 2:
                return *this;

            case 3:
                return node_data<T>{vector_copy()};

            case 4:
                return node_data<T>{matrix_copy()};

            default:
                break;
            }

            HPX_THROW_EXCEPTION(hpx::invalid_status,
                "phylanx::ir::node_data<T>::ref()",
                "node_data object holds unsupported data type");
        }

        /// Return whether the internal representation is referring to another
        /// instance of node_data
        bool is_ref() const
        {
            switch(data_.index())
            {
            case 0: HPX_FALLTHROUGH;
            case 1: HPX_FALLTHROUGH;
            case 2:
                return false;

            case 3: HPX_FALLTHROUGH;
            case 4:
                return true;

            default:
                break;
            }

            HPX_THROW_EXCEPTION(hpx::invalid_status,
                "phylanx::ir::node_data<T>::is_ref()",
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

        PHYLANX_EXPORT void serialize(
            hpx::serialization::input_archive& ar, unsigned);
        PHYLANX_EXPORT void serialize(
            hpx::serialization::output_archive& ar, unsigned);

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

        case 1: HPX_FALLTHROUGH;
        case 3:
            return lhs.vector() == rhs.vector();

        case 2: HPX_FALLTHROUGH;
        case 4:
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
