//  Copyright (c) 2017-2018 Hartmut Kaiser
//  Copyright (c) 2017 Parsa Amini
//  Copyright (c) 2018 Tianyi Zhang
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/ir/node_data.hpp>
#include <phylanx/util/serialization/blaze.hpp>
#include <phylanx/util/serialization/variant.hpp>

#include <hpx/exception.hpp>
#include <hpx/include/serialization.hpp>
#include <hpx/include/util.hpp>

#include <atomic>
#include <cstddef>
#include <cstdint>
#include <iosfwd>
#include <string>
#include <utility>
#include <vector>

namespace phylanx { namespace ir
{
    ///////////////////////////////////////////////////////////////////////////
    // performance counter data (for now, counting all node_data<T>)
    static std::atomic<std::int64_t> count_copy_constructions_;
    static std::atomic<std::int64_t> count_move_constructions_;
    static std::atomic<std::int64_t> count_copy_assignments_;
    static std::atomic<std::int64_t> count_move_assignments_;
    static std::atomic<bool> enable_counts_;

    template <typename T>
    void node_data<T>::increment_copy_construction_count()
    {
        if (enable_counts_.load(std::memory_order_relaxed))
            ++count_copy_constructions_;
    }

    template <typename T>
    void node_data<T>::increment_move_construction_count()
    {
        if (enable_counts_.load(std::memory_order_relaxed))
            ++count_move_constructions_;
    }

    template <typename T>
    void node_data<T>::increment_copy_assignment_count()
    {
        if (enable_counts_.load(std::memory_order_relaxed))
            ++count_copy_assignments_;
    }

    template <typename T>
    void node_data<T>::increment_move_assignment_count()
    {
        if (enable_counts_.load(std::memory_order_relaxed))
            ++count_move_assignments_;
    }

    template <typename T>
    bool node_data<T>::enable_counts(bool enable)
    {
        return enable_counts_.exchange(enable, std::memory_order_relaxed);
    }

    template <typename T>
    std::int64_t node_data<T>::copy_construction_count(bool reset)
    {
        return hpx::util::get_and_reset_value(count_copy_constructions_, reset);
    }

    template <typename T>
    std::int64_t node_data<T>::move_construction_count(bool reset)
    {
        return hpx::util::get_and_reset_value(count_move_constructions_, reset);
    }

    template <typename T>
    std::int64_t node_data<T>::copy_assignment_count(bool reset)
    {
        return hpx::util::get_and_reset_value(count_copy_assignments_, reset);
    }

    template <typename T>
    std::int64_t node_data<T>::move_assignment_count(bool reset)
    {
        return hpx::util::get_and_reset_value(count_move_assignments_, reset);
    }

    ///////////////////////////////////////////////////////////////////////////
    /// Create node data for a 0-dimensional value
    template <typename T>
    node_data<T>::node_data(storage0d_type const& value)
        : data_(value)
    {
    }

    template <typename T>
    node_data<T>::node_data(storage0d_type&& value)
      : data_(std::move(value))
    {
    }

    /// Create node data for a 1-dimensional value
    template <typename T>
    node_data<T>::node_data(storage1d_type const& values)
      : data_(values)
    {
        increment_copy_construction_count();
    }

    template <typename T>
    node_data<T>::node_data(storage1d_type&& values)
        : data_(std::move(values))
    {
        increment_move_construction_count();
    }

    template <typename T>
    node_data<T>::node_data(dimensions_type const& dims)
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

    template <typename T>
    node_data<T>::node_data(dimensions_type const& dims, T default_value)
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

    template <typename T>
    node_data<T>::node_data(custom_storage1d_type const& values)
      : data_(custom_storage1d_type{
            const_cast<T*>(values.data()), values.size(), values.spacing()})
    {
        increment_move_construction_count();
    }

    template <typename T>
    node_data<T>::node_data(custom_storage1d_type&& values)
      : data_(std::move(values))
    {
        increment_move_construction_count();
    }

        /// Create node data for a 2-dimensional value
    template <typename T>
    node_data<T>::node_data(storage2d_type const& values)
      : data_(values)
    {
        increment_copy_construction_count();
    }

    template <typename T>
    node_data<T>::node_data(storage2d_type&& values)
      : data_(std::move(values))
    {
        increment_move_construction_count();
    }

    template <typename T>
    node_data<T>::node_data(custom_storage2d_type const& values)
      : data_(custom_storage2d_type{const_cast<T*>(values.data()),
            values.rows(), values.columns(), values.spacing()})
    {
        increment_copy_construction_count();
    }

    template <typename T>
    node_data<T>::node_data(custom_storage2d_type&& values)
      : data_(std::move(values))
    {
        increment_move_construction_count();
    }

    // conversion helpers for Python bindings
    template <typename T>
    node_data<T>::node_data(std::vector<T> const& values)
      : data_(storage1d_type(values.size()))
    {
        std::size_t const nx = values.size();
        for (std::size_t i = 0; i != nx; ++i)
        {
            util::get<1>(data_)[i] = values[i];
        }
    }

    template <typename T>
    node_data<T>::node_data(std::vector<std::vector<T>> const& values)
      : data_(storage2d_type{values.size(), values[0].size()})
    {
        std::size_t const nx = values.size();
        for (std::size_t i = 0; i != nx; ++i)
        {
            std::vector<T> const& row = values[i];
            std::size_t const ny = row.size();
            for (std::size_t j = 0; j != ny; ++j)
            {
                util::get<2>(data_)(i, j) = row[j];
            }
        }
    }

    template <typename T>
    typename node_data<T>::storage_type node_data<T>::init_data_from(
        node_data const& d)
    {
        switch (d.data_.index())
        {
        case 0:
            return d.data_;

        case 1: HPX_FALLTHROUGH;
        case 2:
            {
                increment_copy_construction_count();
                return d.data_;
            }
            break;

        case 3:
            {
                increment_move_construction_count();
                auto v = d.vector();
                return custom_storage1d_type{v.data(), v.size(), v.spacing()};
            }
            break;

        case 4:
            {
                increment_move_construction_count();
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

    /// Create node data from a node data
    template <typename T>
    node_data<T>::node_data(node_data const& d)
      : data_(init_data_from(d))
    {
    }

    template <typename T>
    node_data<T>::node_data(node_data&& d)
      : data_(std::move(d.data_))
    {
        increment_move_construction_count();
    }

    template <typename T>
    node_data<T>& node_data<T>::operator=(storage0d_type val)
    {
        data_ = val;
        return *this;
    }

    template <typename T>
    node_data<T>& node_data<T>::operator=(storage1d_type const& val)
    {
        increment_copy_assignment_count();
        data_ = val;
        return *this;
    }

    template <typename T>
    node_data<T>& node_data<T>::operator=(storage1d_type && val)
    {
        increment_move_assignment_count();
        data_ = std::move(val);
        return *this;
    }

    template <typename T>
    node_data<T>& node_data<T>::operator=(custom_storage1d_type const& val)
    {
        increment_move_assignment_count();
        data_ = custom_storage1d_type{
            const_cast<T*>(val.data()), val.size(), val.spacing()};
        return *this;
    }

    template <typename T>
    node_data<T>& node_data<T>::operator=(custom_storage1d_type && val)
    {
        increment_move_assignment_count();
        data_ = std::move(val);
        return *this;
    }

    template <typename T>
    node_data<T>& node_data<T>::operator=(storage2d_type const& val)
    {
        increment_copy_assignment_count();
        data_ = val;
        return *this;
    }

    template <typename T>
    node_data<T>& node_data<T>::operator=(storage2d_type && val)
    {
        increment_move_assignment_count();
        data_ = std::move(val);
        return *this;
    }

    template <typename T>
    node_data<T>& node_data<T>::operator=(custom_storage2d_type const& val)
    {
        increment_move_assignment_count();
        data_ = custom_storage2d_type{const_cast<T*>(val.data()), val.rows(),
            val.columns(), val.spacing()};
        return *this;
    }

    template <typename T>
    node_data<T>& node_data<T>::operator=(custom_storage2d_type && val)
    {
        increment_move_assignment_count();
        data_ = std::move(val);
        return *this;
    }

    template <typename T>
    node_data<T>& node_data<T>::operator=(std::vector<T> const& values)
    {
        data_ = storage1d_type(values.size());
        std::size_t const nx = values.size();
        for (std::size_t i = 0; i != nx; ++i)
        {
            util::get<1>(data_)[i] = values[i];
        }
        return *this;
    }

    template <typename T>
    node_data<T>& node_data<T>::operator=(
        std::vector<std::vector<T>> const& values)
    {
        data_ = storage2d_type{values.size(), values[0].size()};
        std::size_t const nx = values.size();
        for (std::size_t i = 0; i != nx; ++i)
        {
            std::vector<T> const& row = values[i];
            std::size_t const ny = row.size();
            for (std::size_t j = 0; j != ny; ++j)
            {
                util::get<2>(data_)(i, j) = row[j];
            }
        }
        return *this;
    }

    template <typename T>
    typename node_data<T>::storage_type node_data<T>::copy_data_from(
        node_data const& d)
    {
        switch (d.data_.index())
        {
        case 0:
            return d.data_;

        case 1: HPX_FALLTHROUGH;
        case 2:
            {
                increment_copy_assignment_count();
                return d.data_;
            }
            break;

        case 3:
            {
                increment_move_assignment_count();
                auto v = d.vector();
                return custom_storage1d_type{
                    v.data(), v.size(), v.spacing()};
            }
            break;

        case 4:
            {
                increment_move_assignment_count();
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

    template <typename T>
    node_data<T>& node_data<T>::operator=(node_data const& d)
    {
        if (this != &d)
        {
            data_ = copy_data_from(d);
        }
        return *this;
    }

    template <typename T>
    node_data<T>& node_data<T>::operator=(node_data && d)
    {
        if (this != &d)
        {
            increment_move_assignment_count();
            data_ = std::move(d.data_);
        }
        return *this;
    }

    /// Access a specific element of the underlying N-dimensional array
    template <typename T>
    T& node_data<T>::operator[](std::size_t index)
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

    template <typename T>
    T& node_data<T>::operator[](dimensions_type const& indicies)
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

    template <typename T>
    T& node_data<T>::at(std::size_t index1, std::size_t index2)
    {
        switch(data_.index())
        {
        case 0:
            return scalar();

        case 1: HPX_FALLTHROUGH;
        case 3:
            return vector()[index1];

        case 2: HPX_FALLTHROUGH;
        case 4:
            return matrix()(index1, index2);

        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::invalid_status,
            "phylanx::ir::node_data<T>::at()",
            "node_data object holds unsupported data type");
    }

    template <typename T>
    T const& node_data<T>::operator[](std::size_t index) const
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

    template <typename T>
    T const& node_data<T>::operator[](dimensions_type const& indicies) const
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

    template <typename T>
    T const& node_data<T>::at(std::size_t index1, std::size_t index2) const
    {
        switch(data_.index())
        {
        case 0:
            return scalar();

        case 1: HPX_FALLTHROUGH;
        case 3:
            return vector()[index1];

        case 2: HPX_FALLTHROUGH;
        case 4:
            return matrix()(index1, index2);

        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::invalid_status,
            "phylanx::ir::node_data<T>::at()",
            "node_data object holds unsupported data type");
    }

    template <typename T>
    std::size_t node_data<T>::size() const
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

    template <typename T>
    typename node_data<T>::const_iterator node_data<T>::begin() const
    {
        return const_iterator(*this, 0);
    }

    template <typename T>
    typename node_data<T>::const_iterator  node_data<T>::end() const
    {
        return const_iterator(*this, size());
    }

    template <typename T>
    typename node_data<T>::const_iterator  node_data<T>::cbegin() const
    {
        return const_iterator(*this, 0);
    }

    template <typename T>
    typename node_data<T>::const_iterator  node_data<T>::cend() const
    {
        return const_iterator(*this, size());
    }

    template <typename T>
    typename node_data<T>::storage2d_type& node_data<T>::matrix_non_ref()
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

    template <typename T>
    typename node_data<T>::storage2d_type const& node_data<T>::matrix_non_ref()
        const
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

    template <typename T>
    typename node_data<T>::storage2d_type node_data<T>::matrix_copy() const
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

    template <typename T>
    typename node_data<T>::custom_storage2d_type node_data<T>::matrix() &
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

    template <typename T>
    typename node_data<T>::custom_storage2d_type node_data<T>::matrix() const&
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

    template <typename T>
    typename node_data<T>::custom_storage2d_type node_data<T>::matrix() &&
    {
        custom_storage2d_type* cm =
            util::get_if<custom_storage2d_type>(&data_);
        if (cm != nullptr)
        {
            return *cm;
        }

        HPX_THROW_EXCEPTION(hpx::invalid_status,
            "phylanx::ir::node_data<T>::matrix()",
            "node_data::matrix() shouldn't be called on an rvalue");
    }

    template <typename T>
    typename node_data<T>::custom_storage2d_type node_data<T>::matrix() const&&
    {
        custom_storage2d_type const* cm =
            util::get_if<custom_storage2d_type>(&data_);
        if (cm != nullptr)
        {
            return *cm;
        }

        HPX_THROW_EXCEPTION(hpx::invalid_status,
            "phylanx::ir::node_data<T>::matrix()",
            "node_data::matrix() shouldn't be called on an rvalue");
    }

    template <typename T>
    typename node_data<T>::storage1d_type& node_data<T>::vector_non_ref()
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

    template <typename T>
    typename node_data<T>::storage1d_type const& node_data<T>::vector_non_ref()
        const
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

    template <typename T>
    typename node_data<T>::storage1d_type node_data<T>::vector_copy() const
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

    template <typename T>
    typename node_data<T>::custom_storage1d_type node_data<T>::vector() &
    {
        custom_storage1d_type* cv =
            util::get_if<custom_storage1d_type>(&data_);
        if (cv != nullptr)
        {
            return *cv;
        }

        storage1d_type* v = util::get_if<storage1d_type>(&data_);
        if (v != nullptr)
        {
            return custom_storage1d_type(v->data(), v->size(), v->spacing());
        }

        HPX_THROW_EXCEPTION(hpx::invalid_status,
            "phylanx::ir::node_data<T>::vector()",
            "node_data object holds unsupported data type");
    }

    template <typename T>
    typename node_data<T>::custom_storage1d_type node_data<T>::vector() const&
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

    template <typename T>
    typename node_data<T>::custom_storage1d_type node_data<T>::vector() &&
    {
        custom_storage1d_type* cv =
            util::get_if<custom_storage1d_type>(&data_);
        if (cv != nullptr)
        {
            return *cv;
        }

        HPX_THROW_EXCEPTION(hpx::invalid_status,
            "phylanx::ir::node_data<T>::vector()",
            "node_data::vector shouldn't be called on an rvalue");
    }

    template <typename T>
    typename node_data<T>::custom_storage1d_type node_data<T>::vector() const&&
    {
        custom_storage1d_type const* cv =
            util::get_if<custom_storage1d_type>(&data_);
        if (cv != nullptr)
        {
            return *cv;
        }

        HPX_THROW_EXCEPTION(hpx::invalid_status,
            "phylanx::ir::node_data<T>::vector()",
            "node_data::vector shouldn't be called on an rvalue");
    }

    template <typename T>
    typename node_data<T>::storage0d_type& node_data<T>::scalar()
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

    template <typename T>
    typename node_data<T>::storage0d_type const& node_data<T>::scalar() const
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
    template <typename T>
    std::size_t node_data<T>::num_dimensions() const
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
    template <typename T>
    typename node_data<T>::dimensions_type node_data<T>::dimensions() const
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

    template <typename T>
    std::size_t node_data<T>::dimension(int dim) const
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
    template <typename T>
    node_data<T> node_data<T>::ref() &
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

    template <typename T>
    node_data<T> node_data<T>::ref() const&
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

    template <typename T>
    node_data<T> node_data<T>::ref() &&
    {
        HPX_THROW_EXCEPTION(hpx::invalid_status,
            "phylanx::ir::node_data<T>::ref()",
            "node_data object holds unsupported data type");
    }

    template <typename T>
    node_data<T> node_data<T>::ref() const&&
    {
        HPX_THROW_EXCEPTION(hpx::invalid_status,
            "phylanx::ir::node_data<T>::ref()",
            "node_data object holds unsupported data type");
    }

    /// Return a new instance of node_data holding a copy of this instance.
    template <typename T>
    node_data<T> node_data<T>::copy() const
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
    template <typename T>
    bool node_data<T>::is_ref() const
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

    // conversion helpers for Python bindings
    template <typename T>
    std::vector<T> node_data<T>::as_vector() const
    {
        switch(data_.index())
        {
        case 1: HPX_FALLTHROUGH;
        case 3:
            {
                auto v = vector();
                return std::vector<T>(v.begin(), v.end());
            }

        case 0: HPX_FALLTHROUGH;
        case 2: HPX_FALLTHROUGH;
        case 4: HPX_FALLTHROUGH;
        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::invalid_status,
            "phylanx::ir::node_data<T>::as_vector()",
            "node_data object holds unsupported data type");
    }

    template <typename T>
    std::vector<std::vector<T>> node_data<T>::as_matrix() const
    {
        switch(data_.index())
        {
        case 2: HPX_FALLTHROUGH;
        case 4:
            {
                auto m = matrix();
                std::vector<std::vector<T>> result(m.rows());
                for (std::size_t i = 0; i != m.rows(); ++i)
                {
                    result[i].assign(m.begin(i), m.end(i));
                }
                return result;
            }

        case 0: HPX_FALLTHROUGH;
        case 1: HPX_FALLTHROUGH;
        case 3: HPX_FALLTHROUGH;
        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::invalid_status,
            "phylanx::ir::node_data<T>::as_matrix()",
            "node_data object holds unsupported data type");
    }

    ///////////////////////////////////////////////////////////////////////////
    bool operator==(node_data<double> const& lhs, node_data<double> const& rhs)
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

    bool operator==(
        node_data<std::uint8_t> const& lhs, node_data<std::uint8_t> const& rhs)
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

    bool operator==(
        node_data<std::int64_t> const& lhs, node_data<std::int64_t> const& rhs)
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
            HPX_FALLTHROUGH;
        case 3:
            return lhs.vector() == rhs.vector();

        case 2:
            HPX_FALLTHROUGH;
        case 4:
            return lhs.matrix() == rhs.matrix();

        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::invalid_status,
            "phylanx::ir::node_data<T>::operator==()",
            "node_data object holds unsupported data type");
    }
    ///////////////////////////////////////////////////////////////////////////
    namespace detail
    {
        template <typename T, typename Range>
        void print_array(std::ostream& out, Range const& r, std::size_t size)
        {
            out << "[";
            for (std::size_t i = 0; i != size; ++i)
            {
                if (i != 0)
                {
                    out << ", ";
                }
                out << T(r[i]);
            }
            out << "]";
        }
    }

    ///////////////////////////////////////////////////////////////////////////
    std::ostream& operator<<(std::ostream& out, node_data<double> const& nd)
    {
        std::size_t dims = nd.num_dimensions();
        switch (dims)
        {
        case 0:
            out << nd[0];
            break;

        case 1: HPX_FALLTHROUGH;
        case 3:
            detail::print_array<double>(out, nd.vector(), nd.size());
            break;

        case 2: HPX_FALLTHROUGH;
        case 4:
            {
                out << "[";
                auto data = nd.matrix();
                for (std::size_t row = 0; row != data.rows(); ++row)
                {
                    if (row != 0)
                        out << ", ";
                    detail::print_array<double>(
                        out, blaze::row(data, row), data.columns());
                }
                out << "]";
            }
            break;

        default:
            HPX_THROW_EXCEPTION(hpx::invalid_status,
                "node_data<double>::operator<<()",
                "invalid dimensionality: " + std::to_string(dims));
        }
        return out;
    }

    std::ostream& operator<<(std::ostream& out, node_data<std::int64_t> const& nd)
    {
        std::size_t dims = nd.num_dimensions();
        switch (dims)
        {
            case 0:
                out << nd[0];
                break;

            case 1: HPX_FALLTHROUGH;
            case 3:
                detail::print_array<std::int64_t>(out, nd.vector(), nd.size());
                break;

            case 2: HPX_FALLTHROUGH;
            case 4:
            {
                out << "[";
                auto data = nd.matrix();
                for (std::size_t row = 0; row != data.rows(); ++row)
                {
                    if (row != 0)
                        out << ", ";
                    detail::print_array<std::int64_t>(
                            out, blaze::row(data, row), data.columns());
                }
                out << "]";
            }
                break;

            default:
                HPX_THROW_EXCEPTION(hpx::invalid_status,
                                    "node_data<std::int64_t>::operator<<()",
                                    "invalid dimensionality: " + std::to_string(dims));
        }
        return out;
    }

    ///////////////////////////////////////////////////////////////////////////
    template <typename T>
    node_data<T>::operator bool() const
    {
        std::size_t dims = num_dimensions();
        switch (dims)
        {
        case 0:
            return scalar() != 0;

        case 1: HPX_FALLTHROUGH;
        case 3:
            return vector().nonZeros() != 0;

        case 2: HPX_FALLTHROUGH;
        case 4:
            return matrix().nonZeros() != 0;

        default:
            HPX_THROW_EXCEPTION(hpx::invalid_status,
                "node_data<double>::operator bool",
                "invalid dimensionality: " + std::to_string(dims));
        }
        return false;
    }

    ///////////////////////////////////////////////////////////////////////////
    template <typename T>
    void node_data<T>::serialize(hpx::serialization::output_archive& ar,
        unsigned)
    {
        std::size_t index = data_.index();
        ar << index;

        switch (index)
        {
        case 0:
            ar << util::get<0>(data_);
            break;

        case 1:
            ar << util::get<1>(data_);
            break;

        case 2:
            ar << util::get<2>(data_);
            break;

        case 3:
            ar << util::get<3>(data_);
            break;

        case 4:
            ar << util::get<4>(data_);
            break;

        default:
            HPX_THROW_EXCEPTION(hpx::invalid_status,
                "node_data<T>::serialize",
                "node_data object holds unsupported data type");
        }
    }

    template <typename T>
    void node_data<T>::serialize(hpx::serialization::input_archive& ar,
        unsigned)
    {
        std::size_t index = 0;
        ar >> index;

        switch (index)
        {
        case 0:
            {
                double val = 0;
                ar >> val;
                data_ = val;
            }
            break;

        case 1: HPX_FALLTHROUGH;
        case 3:     // deserialize CustomVector as DynamicVector
            {
                storage1d_type v;
                ar >> v;
                data_ = std::move(v);
            }
            break;

        case 2: HPX_FALLTHROUGH;
        case 4:     // deserialize CustomMatrix as DynamicMatrix
            {
                storage2d_type m;
                ar >> m;
                data_ = std::move(m);
            }
            break;

        default:
            HPX_THROW_EXCEPTION(hpx::invalid_status,
                "node_data<T>::serialize",
                "node_data object holds unsupported data type");
        }
    }

    ///////////////////////////////////////////////////////////////////////////
    std::ostream& operator<<(std::ostream& out, node_data<std::uint8_t> const& nd)
    {
        std::size_t dims = nd.num_dimensions();
        switch (dims)
        {
        case 0:
            out << std::boolalpha << std::to_string(bool{nd[0] != 0});
            break;

        case 1: HPX_FALLTHROUGH;
        case 3:
            out << std::boolalpha;
            detail::print_array<bool>(out, nd.vector(), nd.size());
            break;

        case 2: HPX_FALLTHROUGH;
        case 4:
            {
                out << std::boolalpha << "[";
                auto data = nd.matrix();
                for (std::size_t row = 0; row != data.rows(); ++row)
                {
                    if (row != 0)
                        out << ", ";
                    detail::print_array<bool>(
                        out, blaze::row(data, row), data.columns());
                }
                out << "]";
            }
            break;

        default:
            HPX_THROW_EXCEPTION(hpx::invalid_status,
                "node_data<std::uint8_t>::operator<<()",
                "invalid dimensionality: " + std::to_string(dims));
        }
        return out;
    }
}}

template class PHYLANX_EXPORT phylanx::ir::node_data<double>;
template class PHYLANX_EXPORT phylanx::ir::node_data<std::uint8_t>;
template class PHYLANX_EXPORT phylanx::ir::node_data<std::int64_t>;
