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

#include <hpx/errors/exception.hpp>
#include <hpx/execution_base/register_locks.hpp>
#include <hpx/include/serialization.hpp>
#include <hpx/include/util.hpp>
#include <hpx/modules/execution_base.hpp>
#include <hpx/runtime/threads/run_as_os_thread.hpp>

#include <atomic>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <iosfwd>
#include <string>
#include <utility>
#include <vector>

#include <blaze/Math.h>
#include <blaze_tensor/Math.h>

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
        increment_copy_construction_count();
    }

    template <typename T>
    node_data<T>::node_data(storage0d_type&& value)
      : data_(std::move(value))
    {
        increment_move_construction_count();
    }

    template <typename T>
    node_data<T>::node_data(custom_storage0d_type const& value)
        : data_(value)
    {
        increment_move_construction_count();
    }

    template <typename T>
    node_data<T>::node_data(custom_storage0d_type&& value)
      : data_(std::move(value))
    {
        increment_move_construction_count();
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
        if (dims[3] != 0)
        {
            data_ = storage4d_type(dims[0], dims[1], dims[2], dims[3]);
        }
        else if (dims[2] != 0)
        {
            data_ = storage3d_type(dims[0], dims[1], dims[2]);
        }
        else if (dims[1] != 0)
        {
            data_ = storage2d_type(dims[0], dims[1]);
        }
        else if (dims[0] != 0)
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
        if (dims[3] != 0)
        {
            data_ = storage4d_type(blaze::init_from_value, default_value,
                dims[0], dims[1], dims[2], dims[3]);
        }
        else if (dims[2] != 0)
        {
            data_ = storage3d_type(dims[0], dims[1], dims[2], default_value);
        }
        else if (dims[1] != 0)
        {
            data_ = storage2d_type(dims[0], dims[1], default_value);
        }
        else if (dims[0] != 0)
        {
            data_ = storage1d_type(dims[1], default_value);
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

    // Create node data for a 2-dimensional value
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

    // Create node data for a 3-dimensional value
    template <typename T>
    node_data<T>::node_data(storage3d_type const& values)
      : data_(values)
    {
        increment_copy_construction_count();
    }

    template <typename T>
    node_data<T>::node_data(storage3d_type&& values)
      : data_(std::move(values))
    {
        increment_move_construction_count();
    }

    template <typename T>
    node_data<T>::node_data(custom_storage3d_type const& values)
      : data_(custom_storage3d_type{const_cast<T*>(values.data()),
            values.pages(), values.rows(), values.columns(), values.spacing()})
    {
        increment_copy_construction_count();
    }

    template <typename T>
    node_data<T>::node_data(custom_storage3d_type&& values)
      : data_(std::move(values))
    {
        increment_move_construction_count();
    }

    // Create node data for a 4-dimensional value
    template <typename T>
    node_data<T>::node_data(storage4d_type const& values)
      : data_(values)
    {
        increment_copy_construction_count();
    }

    template <typename T>
    node_data<T>::node_data(storage4d_type&& values)
      : data_(std::move(values))
    {
        increment_move_construction_count();
    }

    template <typename T>
    node_data<T>::node_data(custom_storage4d_type const& values)
      : data_(custom_storage4d_type{const_cast<T*>(values.data()),
            values.quats(), values.pages(), values.rows(), values.columns(),
            values.spacing()})
    {
        increment_copy_construction_count();
    }

    template <typename T>
    node_data<T>::node_data(custom_storage4d_type&& values)
      : data_(std::move(values))
    {
        increment_move_construction_count();
    }

    // conversion helpers for Python bindings and AST parsing
    template <typename T>
    node_data<T>::node_data(std::vector<T> const& values)
      : data_(storage1d_type(values.size()))
    {
        std::size_t const nx = values.size();
        for (std::size_t i = 0; i != nx; ++i)
        {
            util::get<storage1d>(data_)[i] = values[i];
        }
    }

    template <typename T>
    node_data<T>::node_data(std::vector<std::vector<T>> const& values)
      : data_(storage2d_type{
            values.size(), !values.empty() ? values[0].size() : 0})
    {
        std::size_t const nx = values.size();
        for (std::size_t i = 0; i != nx; ++i)
        {
            std::vector<T> const& row = values[i];
            std::size_t const ny = row.size();
            for (std::size_t j = 0; j != ny; ++j)
            {
                util::get<storage2d>(data_)(i, j) = row[j];
            }
        }
    }

    template <typename T>
    node_data<T>::node_data(
            std::vector<std::vector<std::vector<T>>> const& values)
      : data_(storage3d_type{
              values.size(), !values.empty() ? values[0].size() : 0,
              !values.empty() && !values[0].empty() ? values[0][0].size() : 0})
    {
        std::size_t const nx = values.size();
        for (std::size_t k = 0; k != nx; ++k)
        {
            std::vector<std::vector<T>> const& page = values[k];
            std::size_t const ny = page.size();
            for (std::size_t i = 0; i != ny; ++i)
            {
                std::vector<T> const& row = page[i];
                std::size_t const nz = row.size();
                for (std::size_t j = 0; j != nz; ++j)
                {
                    util::get<storage3d>(data_)(k, i, j) = row[j];
                }
            }
        }
    }

    template <typename T>
    node_data<T>::node_data(
        std::vector<std::vector<std::vector<std::vector<T>>>> const& values)
      : data_(storage4d_type{values.size(),
            !values.empty() ? values[0].size() : 0,
            !values.empty() && !values[0].empty() ? values[0][0].size() : 0,
            !values.empty() && !values[0].empty() && !values[0][0].empty() ?
                values[0][0][0].size() :
                0})
    {
        std::size_t const nw = values.size();
        for (std::size_t l = 0; l != nw; ++l)
        {
            std::vector<std::vector<std::vector<T>>> const& quat = values[l];
            std::size_t const nx = quat.size();
            for (std::size_t k = 0; k != nx; ++k)
            {
                std::vector<std::vector<T>> const& page = quat[k];
                std::size_t const ny = page.size();
                for (std::size_t i = 0; i != ny; ++i)
                {
                    std::vector<T> const& row = page[i];
                    std::size_t const nz = row.size();
                    for (std::size_t j = 0; j != nz; ++j)
                    {
                        util::get<storage4d>(data_)(l, k, i, j) = row[j];
                    }
                }
            }
        }
    }

    template <typename T>
    typename node_data<T>::storage_type node_data<T>::init_data_from(
        node_data const& d)
    {
        switch (d.data_.index())
        {
        case storage0d: HPX_FALLTHROUGH;
        case storage1d: HPX_FALLTHROUGH;
        case storage2d:
            {
                increment_copy_construction_count();
                return d.data_;
            }
            break;

        case custom_storage0d:
            {
                increment_move_construction_count();
                auto& s = d.scalar();
                return custom_storage0d_type(const_cast<T&>(s));
            }
            break;

        case custom_storage1d:
            {
                increment_move_construction_count();
                auto v = d.vector();
                return custom_storage1d_type{v.data(), v.size(), v.spacing()};
            }
            break;

        case custom_storage2d:
            {
                increment_move_construction_count();
                auto m = d.matrix();
                return custom_storage2d_type{
                    m.data(), m.rows(), m.columns(), m.spacing()};
            }
            break;

        case storage3d:
            {
                increment_copy_construction_count();
                return d.data_;
            }
            break;

        case custom_storage3d:
            {
                increment_move_construction_count();
                auto t = d.tensor();
                return custom_storage3d_type{
                    t.data(), t.pages(), t.rows(), t.columns(), t.spacing()};
            }
            break;

        case storage4d:
            {
                increment_copy_construction_count();
                return d.data_;
            }
            break;

        case custom_storage4d:
            {
                increment_move_construction_count();
                auto q = d.quatern();
                return custom_storage4d_type{q.data(), q.quats(), q.pages(),
                    q.rows(), q.columns(), q.spacing()};
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
        increment_copy_assignment_count();
        data_ = val;
        return *this;
    }

    template <typename T>
    node_data<T>& node_data<T>::operator=(custom_storage0d_type const& val)
    {
        increment_copy_assignment_count();
        data_ = val;
        return *this;
    }

    template <typename T>
    node_data<T>& node_data<T>::operator=(custom_storage0d_type && val)
    {
        increment_move_assignment_count();
        data_ = std::move(val);
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
    node_data<T>& node_data<T>::operator=(storage3d_type const& val)
    {
        increment_copy_assignment_count();
        data_ = val;
        return *this;
    }

    template <typename T>
    node_data<T>& node_data<T>::operator=(storage3d_type && val)
    {
        increment_move_assignment_count();
        data_ = std::move(val);
        return *this;
    }

    template <typename T>
    node_data<T>& node_data<T>::operator=(custom_storage3d_type const& val)
    {
        increment_move_assignment_count();
        data_ = custom_storage3d_type{const_cast<T*>(val.data()), val.pages(),
            val.rows(), val.columns(), val.spacing()};
        return *this;
    }

    template <typename T>
    node_data<T>& node_data<T>::operator=(custom_storage3d_type && val)
    {
        increment_move_assignment_count();
        data_ = std::move(val);
        return *this;
    }

    ///////////////////////////////////////////////////////////////////////////
    template <typename T>
    node_data<T>& node_data<T>::operator=(storage4d_type const& val)
    {
        increment_copy_assignment_count();
        data_ = val;
        return *this;
    }

    template <typename T>
    node_data<T>& node_data<T>::operator=(storage4d_type && val)
    {
        increment_move_assignment_count();
        data_ = std::move(val);
        return *this;
    }

    template <typename T>
    node_data<T>& node_data<T>::operator=(custom_storage4d_type const& val)
    {
        increment_move_assignment_count();
        data_ = custom_storage4d_type{const_cast<T*>(val.data()), val.quats(),
            val.pages(), val.rows(), val.columns(), val.spacing()};
        return *this;
    }

    template <typename T>
    node_data<T>& node_data<T>::operator=(custom_storage4d_type && val)
    {
        increment_move_assignment_count();
        data_ = std::move(val);
        return *this;
    }

    // conversion helpers for Python bindings and AST parsing
    template <typename T>
    node_data<T>& node_data<T>::operator=(std::vector<T> const& values)
    {
        data_ = storage1d_type(values.size());
        std::size_t const nx = values.size();
        for (std::size_t i = 0; i != nx; ++i)
        {
            util::get<storage1d>(data_)[i] = values[i];
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
                util::get<storage2d>(data_)(i, j) = row[j];
            }
        }
        return *this;
    }

    template <typename T>
    node_data<T>& node_data<T>::operator=(
        std::vector<std::vector<std::vector<T>>> const& values)
    {
        data_ = storage3d_type{
            values.size(), values[0].size(), values[0][0].size()};

        std::size_t const nx = values.size();
        for (std::size_t k = 0; k != nx; ++k)
        {
            std::vector<std::vector<T>> const& page = values[k];
            std::size_t const ny = page.size();
            for (std::size_t i = 0; i != ny; ++i)
            {
                std::vector<T> const& row = page[i];
                std::size_t const nz = row.size();
                for (std::size_t j = 0; j != nz; ++j)
                {
                    util::get<storage3d>(data_)(k, i, j) = row[j];
                }
            }
        }
        return *this;
    }

    template <typename T>
    node_data<T>& node_data<T>::operator=(
        std::vector<std::vector<std::vector<std::vector<T>>>> const& values)
    {
        data_ = storage4d_type{values.size(), values[0].size(),
            values[0][0].size(), values[0][0][0].size()};

        std::size_t const nw = values.size();
        for (std::size_t l = 0; l != nw; ++l)
        {
            std::vector<std::vector<std::vector<T>>> const& quat = values[l];
            std::size_t const nx = quat.size();
            for (std::size_t k = 0; k != nx; ++k)
            {
                std::vector<std::vector<T>> const& page = quat[k];
                std::size_t const ny = page.size();
                for (std::size_t i = 0; i != ny; ++i)
                {
                    std::vector<T> const& row = page[i];
                    std::size_t const nz = row.size();
                    for (std::size_t j = 0; j != nz; ++j)
                    {
                        util::get<storage4d>(data_)(l, k, i, j) = row[j];
                    }
                }
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
        case storage0d: HPX_FALLTHROUGH;
        case storage1d: HPX_FALLTHROUGH;
        case storage2d:
            {
                increment_copy_assignment_count();
                return d.data_;
            }
            break;

        case custom_storage0d:
            {
                increment_move_assignment_count();
                auto& s = d.scalar();
                return custom_storage0d_type(const_cast<T&>(s));
            }
            break;

        case custom_storage1d:
            {
                increment_move_assignment_count();
                auto v = d.vector();
                return custom_storage1d_type{
                    v.data(), v.size(), v.spacing()};
            }
            break;

        case custom_storage2d:
            {
                increment_move_assignment_count();
                auto m = d.matrix();
                return custom_storage2d_type{
                    m.data(), m.rows(), m.columns(), m.spacing()};
            }
            break;

        case storage3d:
            {
                increment_copy_assignment_count();
                return d.data_;
            }
            break;

        case custom_storage3d:
            {
                increment_move_construction_count();
                auto t = d.tensor();
                return custom_storage3d_type{
                    t.data(), t.pages(), t.rows(), t.columns(), t.spacing()};
            }
            break;

        case storage4d:
            {
                increment_copy_assignment_count();
                return d.data_;
            }
            break;

        case custom_storage4d:
            {
                increment_move_construction_count();
                auto q = d.quatern();
                return custom_storage4d_type{q.data(), q.quats(), q.pages(),
                    q.rows(), q.columns(), q.spacing()};
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
        case storage0d:         HPX_FALLTHROUGH;
        case custom_storage0d:
            return scalar();

        case storage1d:         HPX_FALLTHROUGH;
        case custom_storage1d:
            return vector()[index];

        case storage2d:         HPX_FALLTHROUGH;
        case custom_storage2d:
            {
                auto m = matrix();
                std::size_t idx_m = index / m.columns();
                std::size_t idx_n = index % m.columns();
                return m(idx_m, idx_n);
            }

        case storage3d:         HPX_FALLTHROUGH;
        case custom_storage3d:  HPX_FALLTHROUGH;
        case storage4d:         HPX_FALLTHROUGH;
        case custom_storage4d:
            {
                HPX_THROW_EXCEPTION(hpx::invalid_status,
                    "phylanx::ir::node_data<T>::operator[]()",
                    "node_data object holds data type that does not support "
                    "1d indexing");
            }
            break;

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
        case storage0d:         HPX_FALLTHROUGH;
        case custom_storage0d:
            return scalar();

        case storage1d:         HPX_FALLTHROUGH;
        case custom_storage1d:
            return vector()[indicies[0]];

        case storage2d:         HPX_FALLTHROUGH;
        case custom_storage2d:
            return matrix()(indicies[0], indicies[1]);

        case storage3d:         HPX_FALLTHROUGH;
        case custom_storage3d:
            return tensor()(indicies[0], indicies[1], indicies[2]);

        case storage4d:         HPX_FALLTHROUGH;
        case custom_storage4d:
            return quatern()(
                indicies[0], indicies[1], indicies[2], indicies[3]);

        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::invalid_status,
            "phylanx::ir::node_data<T>::operator[]()",
            "node_data object holds unsupported data type");
    }

    template <typename T>
    T& node_data<T>::at(std::size_t index1, std::size_t index2,
        std::size_t index3, std::size_t index4)
    {
        switch(data_.index())
        {
        case storage0d:         HPX_FALLTHROUGH;
        case custom_storage0d:
            return scalar();

        case storage1d:         HPX_FALLTHROUGH;
        case custom_storage1d:
            return vector()[index1];

        case storage2d:         HPX_FALLTHROUGH;
        case custom_storage2d:
            return matrix()(index1, index2);

        case storage3d:         HPX_FALLTHROUGH;
        case custom_storage3d:
            return tensor()(index1, index2, index3);

        case storage4d:         HPX_FALLTHROUGH;
        case custom_storage4d:
            return quatern()(index1, index2, index3, index4);

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
        case storage0d: HPX_FALLTHROUGH;
        case custom_storage0d:
            return scalar();

        case storage1d: HPX_FALLTHROUGH;
        case custom_storage1d:
            return vector()[index];

        case storage2d: HPX_FALLTHROUGH;
        case custom_storage2d:
            {
                auto m = matrix();
                std::size_t idx_m = index / m.columns();
                std::size_t idx_n = index % m.columns();
                return m(idx_m, idx_n);
            }

        case storage3d:         HPX_FALLTHROUGH;
        case custom_storage3d:  HPX_FALLTHROUGH;
        case storage4d:         HPX_FALLTHROUGH;
        case custom_storage4d:
            {
                HPX_THROW_EXCEPTION(hpx::invalid_status,
                    "phylanx::ir::node_data<T>::operator[]()",
                    "node_data object holds data type that does not support "
                    "1d indexing");
            }
            break;

        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::invalid_status,
            "phylanx::ir::node_data<T>::operator[]()",
            "node_data object holds unsupported data type");
    }

    template <typename T>
    T const& node_data<T>::operator[](dimensions_type const& indices) const
    {
        switch(data_.index())
        {
        case storage0d: HPX_FALLTHROUGH;
        case custom_storage0d:
            return scalar();

        case storage1d: HPX_FALLTHROUGH;
        case custom_storage1d:
            return vector()[indices[0]];

        case storage2d: HPX_FALLTHROUGH;
        case custom_storage2d:
            return matrix()(indices[0], indices[1]);

        case storage3d:         HPX_FALLTHROUGH;
        case custom_storage3d:
            return tensor()(indices[0], indices[1], indices[2]);

        case storage4d:         HPX_FALLTHROUGH;
        case custom_storage4d:
            return quatern()(indices[0], indices[1], indices[2], indices[3]);

        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::invalid_status,
            "phylanx::ir::node_data<T>::operator[]()",
            "node_data object holds unsupported data type");
    }

    template <typename T>
    T const& node_data<T>::at(std::size_t index1, std::size_t index2,
        std::size_t index3, std::size_t index4) const
    {
        switch(data_.index())
        {
        case storage0d:         HPX_FALLTHROUGH;
        case custom_storage0d:
            return scalar();

        case storage1d:         HPX_FALLTHROUGH;
        case custom_storage1d:
            return vector()[index1];

        case storage2d:         HPX_FALLTHROUGH;
        case custom_storage2d:
            return matrix()(index1, index2);

        case storage3d:         HPX_FALLTHROUGH;
        case custom_storage3d:
            return tensor()(index1, index2, index3);

        case storage4d:         HPX_FALLTHROUGH;
        case custom_storage4d:
            return quatern()(index1, index2, index3, index4);

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
        case storage0d:         HPX_FALLTHROUGH;
        case custom_storage0d:
            return 1;

        case storage1d:         HPX_FALLTHROUGH;
        case custom_storage1d:
            return vector().size();

        case storage2d:         HPX_FALLTHROUGH;
        case custom_storage2d:
            {
                auto m = matrix();
                return m.rows() * m.columns();
            }

        case storage3d:         HPX_FALLTHROUGH;
        case custom_storage3d:
            {
                auto t = tensor();
                return t.rows() * t.columns() * t.pages();
            }

        case storage4d:         HPX_FALLTHROUGH;
        case custom_storage4d:
            {
                auto q = quatern();
                return q.quats() * q.pages() * q.rows() * q.columns() ;
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

    ///////////////////////////////////////////////////////////////////////////
    template <typename T>
    typename node_data<T>::storage4d_type& node_data<T>::quatern_non_ref()
    {
        storage4d_type* t = util::get_if<storage4d_type>(&data_);
        if (t == nullptr)
        {
            HPX_THROW_EXCEPTION(hpx::invalid_status,
                "phylanx::ir::node_data<T>::quatern_non_ref()",
                "node_data object holds unsupported data type");
        }
        return *t;
    }

    template <typename T>
    typename node_data<T>::storage4d_type const& node_data<T>::quatern_non_ref()
        const
    {
        storage4d_type const* t = util::get_if<storage4d_type>(&data_);
        if (t == nullptr)
        {
            HPX_THROW_EXCEPTION(hpx::invalid_status,
                "phylanx::ir::node_data<T>::quatern_non_ref()",
                "node_data object holds unsupported data type");
        }
        return *t;
    }

    ///////////////////////////////////////////////////////////////////////////
    template <typename T>
    typename node_data<T>::storage4d_type node_data<T>::quatern_copy() &
    {
        custom_storage4d_type* ct =
            util::get_if<custom_storage4d_type>(&data_);
        if (ct != nullptr)
        {
            return storage4d_type{*ct};
        }

        storage4d_type* t = util::get_if<storage4d_type>(&data_);
        if (t != nullptr)
        {
            return *t;
        }

        HPX_THROW_EXCEPTION(hpx::invalid_status,
            "phylanx::ir::node_data<T>::quatern_copy() &",
            "node_data object holds unsupported data type");
    }

    template <typename T>
    typename node_data<T>::storage4d_type node_data<T>::quatern_copy() const&
    {
        custom_storage4d_type const* ct =
            util::get_if<custom_storage4d_type>(&data_);
        if (ct != nullptr)
        {
            return storage4d_type{*ct};
        }

        storage4d_type const* t = util::get_if<storage4d_type>(&data_);
        if (t != nullptr)
        {
            return *t;
        }

        HPX_THROW_EXCEPTION(hpx::invalid_status,
            "phylanx::ir::node_data<T>::quatern_copy() const&",
            "node_data object holds unsupported data type");
    }

    template <typename T>
    typename node_data<T>::storage4d_type node_data<T>::quatern_copy() &&
    {
        custom_storage4d_type* ct =
            util::get_if<custom_storage4d_type>(&data_);
        if (ct != nullptr)
        {
            return storage4d_type{*ct};
        }

        storage4d_type* t = util::get_if<storage4d_type>(&data_);
        if (t != nullptr)
        {
            return std::move(*t);
        }

        HPX_THROW_EXCEPTION(hpx::invalid_status,
            "phylanx::ir::node_data<T>::quatern_copy() &&",
            "node_data object holds unsupported data type");
    }

    template <typename T>
    typename node_data<T>::storage4d_type node_data<T>::quatern_copy() const&&
    {
        custom_storage4d_type const* ct =
            util::get_if<custom_storage4d_type>(&data_);
        if (ct != nullptr)
        {
            return storage4d_type{*ct};
        }

        storage4d_type const* t = util::get_if<storage4d_type>(&data_);
        if (t != nullptr)
        {
            return *t;
        }

        HPX_THROW_EXCEPTION(hpx::invalid_status,
            "phylanx::ir::node_data<T>::quatern_copy() const&&",
            "node_data object holds unsupported data type");
    }

    ///////////////////////////////////////////////////////////////////////////
    template <typename T>
    typename node_data<T>::custom_storage4d_type node_data<T>::quatern() &
    {
        custom_storage4d_type* ct =
            util::get_if<custom_storage4d_type>(&data_);
        if (ct != nullptr)
        {
            return *ct;
        }

        storage4d_type* t = util::get_if<storage4d_type>(&data_);
        if (t != nullptr)
        {
            return custom_storage4d_type(t->data(), t->quats(), t->pages(),
                t->rows(), t->columns(), t->spacing());
        }

        HPX_THROW_EXCEPTION(hpx::invalid_status,
            "phylanx::ir::node_data<T>::quatern() &",
            "node_data object holds unsupported data type");
    }

    template <typename T>
    typename node_data<T>::custom_storage4d_type node_data<T>::quatern() const&
    {
        custom_storage4d_type const* ct =
            util::get_if<custom_storage4d_type>(&data_);
        if (ct != nullptr)
        {
            return custom_storage4d_type(const_cast<T*>(ct->data()),
                ct->quats(), ct->pages(), ct->rows(), ct->columns(),
                ct->spacing());
        }

        storage4d_type const* t = util::get_if<storage4d_type>(&data_);
        if (t != nullptr)
        {
            return custom_storage4d_type(const_cast<T*>(t->data()), t->quats(),
                t->pages(), t->rows(), t->columns(), t->spacing());
        }

        HPX_THROW_EXCEPTION(hpx::invalid_status,
            "phylanx::ir::node_data<T>::quatern() const&",
            "node_data object holds unsupported data type");
    }

    template <typename T>
    typename node_data<T>::custom_storage4d_type node_data<T>::quatern() &&
    {
        custom_storage4d_type* ct =
            util::get_if<custom_storage4d_type>(&data_);
        if (ct != nullptr)
        {
            return *ct;
        }

        HPX_THROW_EXCEPTION(hpx::invalid_status,
            "phylanx::ir::node_data<T>::quatern() &&",
            "node_data::quatern() shouldn't be called on an rvalue");
    }

    template <typename T>
    typename node_data<T>::custom_storage4d_type node_data<T>::quatern() const&&
    {
        custom_storage4d_type const* ct =
            util::get_if<custom_storage4d_type>(&data_);
        if (ct != nullptr)
        {
            return *ct;
        }

        HPX_THROW_EXCEPTION(hpx::invalid_status,
            "phylanx::ir::node_data<T>::quatern() const&&",
            "node_data::quatern() shouldn't be called on an rvalue");
    }

    ///////////////////////////////////////////////////////////////////////////
    template <typename T>
    typename node_data<T>::storage3d_type& node_data<T>::tensor_non_ref()
    {
        storage3d_type* t = util::get_if<storage3d_type>(&data_);
        if (t == nullptr)
        {
            HPX_THROW_EXCEPTION(hpx::invalid_status,
                "phylanx::ir::node_data<T>::tensor_non_ref()",
                "node_data object holds unsupported data type");
        }
        return *t;
    }

    template <typename T>
    typename node_data<T>::storage3d_type const& node_data<T>::tensor_non_ref()
        const
    {
        storage3d_type const* t = util::get_if<storage3d_type>(&data_);
        if (t == nullptr)
        {
            HPX_THROW_EXCEPTION(hpx::invalid_status,
                "phylanx::ir::node_data<T>::tensor_non_ref()",
                "node_data object holds unsupported data type");
        }
        return *t;
    }

    ///////////////////////////////////////////////////////////////////////////
    template <typename T>
    typename node_data<T>::storage3d_type node_data<T>::tensor_copy() &
    {
        custom_storage3d_type* ct =
            util::get_if<custom_storage3d_type>(&data_);
        if (ct != nullptr)
        {
            return storage3d_type{*ct};
        }

        storage3d_type* t = util::get_if<storage3d_type>(&data_);
        if (t != nullptr)
        {
            return *t;
        }

        HPX_THROW_EXCEPTION(hpx::invalid_status,
            "phylanx::ir::node_data<T>::tensor_copy() &",
            "node_data object holds unsupported data type");
    }

    template <typename T>
    typename node_data<T>::storage3d_type node_data<T>::tensor_copy() const&
    {
        custom_storage3d_type const* ct =
            util::get_if<custom_storage3d_type>(&data_);
        if (ct != nullptr)
        {
            return storage3d_type{*ct};
        }

        storage3d_type const* t = util::get_if<storage3d_type>(&data_);
        if (t != nullptr)
        {
            return *t;
        }

        HPX_THROW_EXCEPTION(hpx::invalid_status,
            "phylanx::ir::node_data<T>::tensor_copy() const&",
            "node_data object holds unsupported data type");
    }

    template <typename T>
    typename node_data<T>::storage3d_type node_data<T>::tensor_copy() &&
    {
        custom_storage3d_type* ct =
            util::get_if<custom_storage3d_type>(&data_);
        if (ct != nullptr)
        {
            return storage3d_type{*ct};
        }

        storage3d_type* t = util::get_if<storage3d_type>(&data_);
        if (t != nullptr)
        {
            return std::move(*t);
        }

        HPX_THROW_EXCEPTION(hpx::invalid_status,
            "phylanx::ir::node_data<T>::tensor_copy() &&",
            "node_data object holds unsupported data type");
    }

    template <typename T>
    typename node_data<T>::storage3d_type node_data<T>::tensor_copy() const&&
    {
        custom_storage3d_type const* ct =
            util::get_if<custom_storage3d_type>(&data_);
        if (ct != nullptr)
        {
            return storage3d_type{*ct};
        }

        storage3d_type const* t = util::get_if<storage3d_type>(&data_);
        if (t != nullptr)
        {
            return *t;
        }

        HPX_THROW_EXCEPTION(hpx::invalid_status,
            "phylanx::ir::node_data<T>::tensor_copy() const&&",
            "node_data object holds unsupported data type");
    }

    ///////////////////////////////////////////////////////////////////////////
    template <typename T>
    typename node_data<T>::custom_storage3d_type node_data<T>::tensor() &
    {
        custom_storage3d_type* ct =
            util::get_if<custom_storage3d_type>(&data_);
        if (ct != nullptr)
        {
            return *ct;
        }

        storage3d_type* t = util::get_if<storage3d_type>(&data_);
        if (t != nullptr)
        {
            return custom_storage3d_type(
                t->data(), t->pages(), t->rows(), t->columns(), t->spacing());
        }

        HPX_THROW_EXCEPTION(hpx::invalid_status,
            "phylanx::ir::node_data<T>::tensor() &",
            "node_data object holds unsupported data type");
    }

    template <typename T>
    typename node_data<T>::custom_storage3d_type node_data<T>::tensor() const&
    {
        custom_storage3d_type const* ct =
            util::get_if<custom_storage3d_type>(&data_);
        if (ct != nullptr)
        {
            return custom_storage3d_type(const_cast<T*>(ct->data()),
                ct->pages(), ct->rows(), ct->columns(), ct->spacing());
        }

        storage3d_type const* t = util::get_if<storage3d_type>(&data_);
        if (t != nullptr)
        {
            return custom_storage3d_type(const_cast<T*>(t->data()),
                t->pages(), t->rows(), t->columns(), t->spacing());
        }

        HPX_THROW_EXCEPTION(hpx::invalid_status,
            "phylanx::ir::node_data<T>::tensor() const&",
            "node_data object holds unsupported data type");
    }

    template <typename T>
    typename node_data<T>::custom_storage3d_type node_data<T>::tensor() &&
    {
        custom_storage3d_type* ct =
            util::get_if<custom_storage3d_type>(&data_);
        if (ct != nullptr)
        {
            return *ct;
        }

        HPX_THROW_EXCEPTION(hpx::invalid_status,
            "phylanx::ir::node_data<T>::tensor() &&",
            "node_data::tensor() shouldn't be called on an rvalue");
    }

    template <typename T>
    typename node_data<T>::custom_storage3d_type node_data<T>::tensor() const&&
    {
        custom_storage3d_type const* ct =
            util::get_if<custom_storage3d_type>(&data_);
        if (ct != nullptr)
        {
            return *ct;
        }

        HPX_THROW_EXCEPTION(hpx::invalid_status,
            "phylanx::ir::node_data<T>::tensor() const&&",
            "node_data::tensor() shouldn't be called on an rvalue");
    }

    ///////////////////////////////////////////////////////////////////////////
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

    ///////////////////////////////////////////////////////////////////////////
    template <typename T>
    typename node_data<T>::storage2d_type node_data<T>::matrix_copy() &
    {
        custom_storage2d_type* cm =
            util::get_if<custom_storage2d_type>(&data_);
        if (cm != nullptr)
        {
            return storage2d_type{*cm};
        }

        storage2d_type* m = util::get_if<storage2d_type>(&data_);
        if (m != nullptr)
        {
            return *m;
        }

        HPX_THROW_EXCEPTION(hpx::invalid_status,
            "phylanx::ir::node_data<T>::matrix_copy() &",
            "node_data object holds unsupported data type");
    }

    template <typename T>
    typename node_data<T>::storage2d_type node_data<T>::matrix_copy() const&
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
            "phylanx::ir::node_data<T>::matrix_copy() const&",
            "node_data object holds unsupported data type");
    }

    template <typename T>
    typename node_data<T>::storage2d_type node_data<T>::matrix_copy() &&
    {
        custom_storage2d_type* cm =
            util::get_if<custom_storage2d_type>(&data_);
        if (cm != nullptr)
        {
            return storage2d_type{*cm};
        }

        storage2d_type* m = util::get_if<storage2d_type>(&data_);
        if (m != nullptr)
        {
            return std::move(*m);
        }

        HPX_THROW_EXCEPTION(hpx::invalid_status,
            "phylanx::ir::node_data<T>::matrix_copy() &&",
            "node_data object holds unsupported data type");
    }

    template <typename T>
    typename node_data<T>::storage2d_type node_data<T>::matrix_copy() const&&
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
            "phylanx::ir::node_data<T>::matrix_copy() const&&",
            "node_data object holds unsupported data type");
    }

    ///////////////////////////////////////////////////////////////////////////
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
            "phylanx::ir::node_data<T>::matrix() &",
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
            "phylanx::ir::node_data<T>::matrix() const&",
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
            "phylanx::ir::node_data<T>::matrix() &&",
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
            "phylanx::ir::node_data<T>::matrix() const&&",
            "node_data::matrix() shouldn't be called on an rvalue");
    }

    ///////////////////////////////////////////////////////////////////////////
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

    ///////////////////////////////////////////////////////////////////////////
    template <typename T>
    typename node_data<T>::storage1d_type node_data<T>::vector_copy() &
    {
        custom_storage1d_type* cv = util::get_if<custom_storage1d_type>(&data_);
        if (cv != nullptr)
        {
            return storage1d_type{*cv};
        }

        storage1d_type* v = util::get_if<storage1d_type>(&data_);
        if (v != nullptr)
        {
            return *v;
        }

        HPX_THROW_EXCEPTION(hpx::invalid_status,
            "phylanx::ir::node_data<T>::vector_copy() &",
            "node_data object holds unsupported data type");
    }

    template <typename T>
    typename node_data<T>::storage1d_type node_data<T>::vector_copy() const&
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
            "phylanx::ir::node_data<T>::vector_copy() const&",
            "node_data object holds unsupported data type");
    }

    template <typename T>
    typename node_data<T>::storage1d_type node_data<T>::vector_copy() &&
    {
        custom_storage1d_type* cv =
            util::get_if<custom_storage1d_type>(&data_);
        if (cv != nullptr)
        {
            return storage1d_type{*cv};
        }

        storage1d_type* v = util::get_if<storage1d_type>(&data_);
        if (v != nullptr)
        {
            return std::move(*v);
        }

        HPX_THROW_EXCEPTION(hpx::invalid_status,
            "phylanx::ir::node_data<T>::vector_copy() &&",
            "node_data object holds unsupported data type");
    }

    template <typename T>
    typename node_data<T>::storage1d_type node_data<T>::vector_copy() const&&
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
            "phylanx::ir::node_data<T>::vector_copy() const&&",
            "node_data object holds unsupported data type");
    }

    ///////////////////////////////////////////////////////////////////////////
    template <typename T>
    typename node_data<T>::custom_storage1d_type node_data<T>::vector() &
    {
        custom_storage1d_type* cv = util::get_if<custom_storage1d_type>(&data_);
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
            "phylanx::ir::node_data<T>::vector() &",
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
            "phylanx::ir::node_data<T>::vector() const&",
            "node_data object holds unsupported data type");
    }

    template <typename T>
    typename node_data<T>::custom_storage1d_type node_data<T>::vector() &&
    {
        custom_storage1d_type* cv = util::get_if<custom_storage1d_type>(&data_);
        if (cv != nullptr)
        {
            return *cv;
        }

        HPX_THROW_EXCEPTION(hpx::invalid_status,
            "phylanx::ir::node_data<T>::vector() &&",
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
            "phylanx::ir::node_data<T>::vector() const&&",
            "node_data::vector shouldn't be called on an rvalue");
    }

    ///////////////////////////////////////////////////////////////////////////
    template <typename T>
    typename node_data<T>::storage0d_type node_data<T>::scalar_copy() &
    {
        custom_storage0d_type* cs = util::get_if<custom_storage0d_type>(&data_);
        if (cs != nullptr)
        {
            return storage0d_type{*cs};
        }

        storage0d_type* s = util::get_if<storage0d_type>(&data_);
        if (s != nullptr)
        {
            return *s;
        }

        HPX_THROW_EXCEPTION(hpx::invalid_status,
            "phylanx::ir::node_data<T>::scalar_copy() &",
            "node_data object holds unsupported data type");
    }

    template <typename T>
    typename node_data<T>::storage0d_type node_data<T>::scalar_copy() const&
    {
        custom_storage0d_type const* cs =
            util::get_if<custom_storage0d_type>(&data_);
        if (cs != nullptr)
        {
            return storage0d_type{*cs};
        }

        storage0d_type const* s = util::get_if<storage0d_type>(&data_);
        if (s != nullptr)
        {
            return *s;
        }

        HPX_THROW_EXCEPTION(hpx::invalid_status,
            "phylanx::ir::node_data<T>::scalar_copy() const&",
            "node_data object holds unsupported data type");
    }

    template <typename T>
    typename node_data<T>::storage0d_type node_data<T>::scalar_copy() &&
    {
        custom_storage0d_type* cs = util::get_if<custom_storage0d_type>(&data_);
        if (cs != nullptr)
        {
            return storage0d_type{*cs};
        }

        storage0d_type* s = util::get_if<storage0d_type>(&data_);
        if (s != nullptr)
        {
            return std::move(*s);
        }

        HPX_THROW_EXCEPTION(hpx::invalid_status,
            "phylanx::ir::node_data<T>::scalar_copy() &&",
            "node_data object holds unsupported data type");
    }

    template <typename T>
    typename node_data<T>::storage0d_type node_data<T>::scalar_copy() const&&
    {
        custom_storage0d_type const* cs =
            util::get_if<custom_storage0d_type>(&data_);
        if (cs != nullptr)
        {
            return storage0d_type{*cs};
        }

        storage0d_type const* s = util::get_if<storage0d_type>(&data_);
        if (s != nullptr)
        {
            return *s;
        }

        HPX_THROW_EXCEPTION(hpx::invalid_status,
            "phylanx::ir::node_data<T>::scalar_copy() const&&",
            "node_data object holds unsupported data type");
    }

    ///////////////////////////////////////////////////////////////////////////
    template <typename T>
    T& node_data<T>::scalar() &
    {
        custom_storage0d_type* cs = util::get_if<custom_storage0d_type>(&data_);
        if (cs != nullptr)
        {
            return cs->get();
        }

        storage0d_type* s = util::get_if<storage0d_type>(&data_);
        if (s != nullptr)
        {
            return *s;
        }

        HPX_THROW_EXCEPTION(hpx::invalid_status,
            "phylanx::ir::node_data<T>::scalar() &",
            "node_data object holds unsupported data type");
    }

    template <typename T>
    T const& node_data<T>::scalar() const&
    {
        custom_storage0d_type const* cs =
            util::get_if<custom_storage0d_type>(&data_);
        if (cs != nullptr)
        {
            return cs->get();
        }

        storage0d_type const* s = util::get_if<storage0d_type>(&data_);
        if (s != nullptr)
        {
            return *s;
        }

        HPX_THROW_EXCEPTION(hpx::invalid_status,
            "phylanx::ir::node_data<T>::scalar() const&",
            "node_data object holds unsupported data type");
    }

    template <typename T>
    T&& node_data<T>::scalar() &&
    {
        custom_storage0d_type* cs = util::get_if<custom_storage0d_type>(&data_);
        if (cs != nullptr)
        {
            return std::move(cs->get());
        }

        HPX_THROW_EXCEPTION(hpx::invalid_status,
            "phylanx::ir::node_data<T>::scalar() &&",
            "node_data::scalar shouldn't be called on an rvalue");
    }

    template <typename T>
    T const&& node_data<T>::scalar() const&&
    {
        custom_storage0d_type const* cs =
            util::get_if<custom_storage0d_type>(&data_);
        if (cs != nullptr)
        {
            return static_cast<T const&&>(cs->get());
        }

        HPX_THROW_EXCEPTION(hpx::invalid_status,
            "phylanx::ir::node_data<T>::scalar() const&&",
            "node_data::scalar shouldn't be called on an rvalue");
    }

    ///////////////////////////////////////////////////////////////////////////
    template <typename T>
    typename node_data<T>::storage0d_type& node_data<T>::scalar_non_ref()
    {
        storage0d_type* s = util::get_if<storage0d_type>(&data_);
        if (s == nullptr)
        {
            HPX_THROW_EXCEPTION(hpx::invalid_status,
                "phylanx::ir::node_data<T>::scalar_non_ref()",
                "node_data object holds unsupported data type");
        }
        return *s;
    }

    template <typename T>
    typename node_data<T>::storage0d_type const& node_data<T>::scalar_non_ref()
        const
    {
        storage0d_type const* s = util::get_if<storage0d_type>(&data_);
        if (s == nullptr)
        {
            HPX_THROW_EXCEPTION(hpx::invalid_status,
                "phylanx::ir::node_data<T>::scalar_non_ref()",
                "node_data object holds unsupported data type");
        }
        return *s;
    }

    ///////////////////////////////////////////////////////////////////////////
    /// Extract the dimensionality of the underlying data array.
    template <typename T>
    std::size_t node_data<T>::num_dimensions() const
    {
        switch(data_.index())
        {
        case storage0d:         HPX_FALLTHROUGH;
        case custom_storage0d:
            return 0;

        case storage1d:         HPX_FALLTHROUGH;
        case custom_storage1d:
            return 1;

        case storage2d:         HPX_FALLTHROUGH;
        case custom_storage2d:
            return 2;

        case storage3d:         HPX_FALLTHROUGH;
        case custom_storage3d:
            return 3;

        case storage4d:         HPX_FALLTHROUGH;
        case custom_storage4d:
            return 4;
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
        case storage0d: HPX_FALLTHROUGH;
        case custom_storage0d:
            return dimensions_type{0ul};

        case storage1d: HPX_FALLTHROUGH;
        case custom_storage1d:
            return dimensions_type{vector().size()};

        case storage2d: HPX_FALLTHROUGH;
        case custom_storage2d:
            {
                auto m = matrix();
                return dimensions_type{m.rows(), m.columns()};
            }

        case storage3d:         HPX_FALLTHROUGH;
        case custom_storage3d:
            {
                auto t = tensor();
                return dimensions_type{t.pages(), t.rows(), t.columns()};
            }

        case storage4d:         HPX_FALLTHROUGH;
        case custom_storage4d:
            {
                auto q = quatern();
                return dimensions_type{
                    q.quats(), q.pages(), q.rows(), q.columns()};
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
        case storage0d:         HPX_FALLTHROUGH;
        case custom_storage0d:
            return 0ul;

        case storage1d:         HPX_FALLTHROUGH;
        case custom_storage1d:
            {
                switch (dim)
                {
                case 0:
                    return vector().size();

                default:
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "phylanx::ir::node_data<T>::dimension()",
                        "unknown dimension requested");
                    break;
                }
            }

        case storage2d:         HPX_FALLTHROUGH;
        case custom_storage2d:
            {
                auto m = matrix();
                switch (dim)
                {
                case 0:
                    return m.rows();

                case 1:
                    return m.columns();

                default:
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "phylanx::ir::node_data<T>::dimension()",
                        "unknown dimension requested");
                    break;
                }
            }

        case storage3d:         HPX_FALLTHROUGH;
        case custom_storage3d:
            {
                auto t = tensor();
                switch (dim)
                {
                case 0:
                    return t.pages();

                case 1:
                    return t.rows();

                case 2:
                    return t.columns();

                default:
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "phylanx::ir::node_data<T>::dimension()",
                        "unknown dimension requested");
                    break;
                }
            }

        case storage4d:         HPX_FALLTHROUGH;
        case custom_storage4d:
            {
                auto q = quatern();
                switch (dim)
                {
                case 0:
                    return q.quats();

                case 1:
                    return q.pages();

                case 2:
                    return q.rows();

                case 3:
                    return q.columns();

                default:
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "phylanx::ir::node_data<T>::dimension()",
                        "unknown dimension requested");
                    break;
                }
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
        case storage0d:
            return node_data<T>{scalar()};

        case storage1d:
            return node_data<T>{vector()};

        case storage2d:
            return node_data<T>{matrix()};

        case storage3d:
            return node_data<T>{tensor()};

        case storage4d:
            return node_data<T>{quatern()};

        case custom_storage0d: HPX_FALLTHROUGH;
        case custom_storage1d: HPX_FALLTHROUGH;
        case custom_storage2d: HPX_FALLTHROUGH;
        case custom_storage3d: HPX_FALLTHROUGH;
        case custom_storage4d:
            return *this;

        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::invalid_status,
            "phylanx::ir::node_data<T>::ref() &",
            "node_data object holds unsupported data type");
    }

    template <typename T>
    node_data<T> node_data<T>::ref() const&
    {
        switch(data_.index())
        {
        case storage0d:
            return node_data<T>{scalar()};

        case storage1d:
            return node_data<T>{vector()};

        case storage2d:
            return node_data<T>{matrix()};

        case storage3d:
            return node_data<T>{tensor()};

        case storage4d:
            return node_data<T>{quatern()};

        case custom_storage0d: HPX_FALLTHROUGH;
        case custom_storage1d: HPX_FALLTHROUGH;
        case custom_storage2d: HPX_FALLTHROUGH;
        case custom_storage3d: HPX_FALLTHROUGH;
        case custom_storage4d:
            return *this;

        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::invalid_status,
            "phylanx::ir::node_data<T>::ref() const&",
            "node_data object holds unsupported data type");
    }

    template <typename T>
    node_data<T> node_data<T>::ref() &&
    {
        HPX_THROW_EXCEPTION(hpx::invalid_status,
            "phylanx::ir::node_data<T>::ref() &&",
            "node_data::ref() should not be called on an rvalue");
    }

    template <typename T>
    node_data<T> node_data<T>::ref() const&&
    {
        HPX_THROW_EXCEPTION(hpx::invalid_status,
            "phylanx::ir::node_data<T>::ref() const&&",
            "node_data::ref() should not be called on an rvalue");
    }

    /// Return a new instance of node_data holding a copy of this instance.
    template <typename T>
    node_data<T> node_data<T>::copy() const
    {
        switch(data_.index())
        {
        case storage0d: HPX_FALLTHROUGH;
        case storage1d: HPX_FALLTHROUGH;
        case storage2d: HPX_FALLTHROUGH;
        case storage3d: HPX_FALLTHROUGH;
        case storage4d:
            return *this;

        case custom_storage0d:
            return node_data<T>{scalar_copy()};

        case custom_storage1d:
            return node_data<T>{vector_copy()};

        case custom_storage2d:
            return node_data<T>{matrix_copy()};

        case custom_storage3d:
            return node_data<T>{tensor_copy()};

        case custom_storage4d:
            return node_data<T>{quatern_copy()};


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
        case storage0d: HPX_FALLTHROUGH;
        case storage1d: HPX_FALLTHROUGH;
        case storage2d:
            return false;

        case custom_storage0d: HPX_FALLTHROUGH;
        case custom_storage1d: HPX_FALLTHROUGH;
        case custom_storage2d:
            return true;

        case storage3d: HPX_FALLTHROUGH;
        case storage4d:
            return false;

        case custom_storage3d: HPX_FALLTHROUGH;
        case custom_storage4d:
            return true;

        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::invalid_status,
            "phylanx::ir::node_data<T>::is_ref()",
            "node_data object holds unsupported data type");
    }

    // conversion helpers for Python bindings and AST parsing
    template <typename T>
    std::vector<T> node_data<T>::as_vector() const
    {
        switch(data_.index())
        {
        case storage1d:         HPX_FALLTHROUGH;
        case custom_storage1d:
            {
                auto v = vector();
                return std::vector<T>(v.begin(), v.end());
            }

        case storage0d:         HPX_FALLTHROUGH;
        case storage2d:         HPX_FALLTHROUGH;
        case custom_storage0d:  HPX_FALLTHROUGH;
        case custom_storage2d:  HPX_FALLTHROUGH;
        case storage3d:         HPX_FALLTHROUGH;
        case custom_storage3d:  HPX_FALLTHROUGH;
        case storage4d:         HPX_FALLTHROUGH;
        case custom_storage4d:  HPX_FALLTHROUGH;

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
        case storage2d:         HPX_FALLTHROUGH;
        case custom_storage2d:
            {
                auto m = matrix();
                std::vector<std::vector<T>> result(m.rows());
                for (std::size_t i = 0; i != m.rows(); ++i)
                {
                    result[i].assign(m.begin(i), m.end(i));
                }
                return result;
            }

        case storage0d:         HPX_FALLTHROUGH;
        case storage1d:         HPX_FALLTHROUGH;
        case custom_storage0d:  HPX_FALLTHROUGH;
        case custom_storage1d:  HPX_FALLTHROUGH;
        case storage3d:         HPX_FALLTHROUGH;
        case custom_storage3d:  HPX_FALLTHROUGH;
        case storage4d:         HPX_FALLTHROUGH;
        case custom_storage4d:  HPX_FALLTHROUGH;
        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::invalid_status,
            "phylanx::ir::node_data<T>::as_matrix()",
            "node_data object holds unsupported data type");
    }

    template <typename T>
    std::vector<std::vector<std::vector<T>>> node_data<T>::as_tensor() const
    {
        switch(data_.index())
        {
        case storage3d:         HPX_FALLTHROUGH;
        case custom_storage3d:
            {
                auto t = tensor();
                std::vector<std::vector<std::vector<T>>> result(t.pages());
                for (std::size_t k = 0; k != t.pages(); ++k)
                {
                    std::vector<std::vector<T>> pages(t.rows());
                    for (std::size_t i = 0; i != t.rows(); ++i)
                    {
                        pages[i].assign(t.begin(i, k), t.end(i, k));
                    }
                    result[k] = std::move(pages);
                }
                return result;
            }

        case storage0d:         HPX_FALLTHROUGH;
        case storage1d:         HPX_FALLTHROUGH;
        case storage2d:         HPX_FALLTHROUGH;
        case custom_storage0d:  HPX_FALLTHROUGH;
        case custom_storage1d:  HPX_FALLTHROUGH;
        case custom_storage2d:  HPX_FALLTHROUGH;
        case storage4d:         HPX_FALLTHROUGH;
        case custom_storage4d:  HPX_FALLTHROUGH;
        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::invalid_status,
            "phylanx::ir::node_data<T>::as_tensor()",
            "node_data object holds unsupported data type");
    }

    template <typename T>
    std::vector<std::vector<std::vector<std::vector<T>>>>
    node_data<T>::as_quatern() const
    {
        switch(data_.index())
        {
        case storage4d:         HPX_FALLTHROUGH;
        case custom_storage4d:
            {
                auto q = quatern();
                std::vector<std::vector<std::vector<std::vector<T>>>> result(
                    q.quats());
                for (std::size_t l = 0; l != q.quats(); ++l)
                {
                    std::vector<std::vector<std::vector<T>>> quats(q.pages());
                    for (std::size_t k = 0; k != q.pages(); ++k)
                    {
                        std::vector<std::vector<T>> pages(q.rows());
                        for (std::size_t i = 0; i != q.rows(); ++i)
                        {
                            pages[i].assign(q.begin(i, l, k), q.end(i, l, k));
                        }
                        quats[k] = std::move(pages);
                    }
                    result[l] = std::move(quats);
                }
                return result;
            }

        case storage0d:         HPX_FALLTHROUGH;
        case storage1d:         HPX_FALLTHROUGH;
        case storage2d:         HPX_FALLTHROUGH;
        case custom_storage0d:  HPX_FALLTHROUGH;
        case custom_storage1d:  HPX_FALLTHROUGH;
        case custom_storage2d:  HPX_FALLTHROUGH;
        case storage3d:         HPX_FALLTHROUGH;
        case custom_storage3d:  HPX_FALLTHROUGH;
        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::invalid_status,
            "phylanx::ir::node_data<T>::as_quatern()",
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

        switch (lhs.index())
        {
        case node_data<double>::storage0d:          HPX_FALLTHROUGH;
        case node_data<double>::custom_storage0d:
            return lhs.scalar() == rhs.scalar();

        case node_data<double>::storage1d:          HPX_FALLTHROUGH;
        case node_data<double>::custom_storage1d:
            return lhs.vector() == rhs.vector();

        case node_data<double>::storage2d:          HPX_FALLTHROUGH;
        case node_data<double>::custom_storage2d:
            return lhs.matrix() == rhs.matrix();

        case node_data<double>::storage3d:          HPX_FALLTHROUGH;
        case node_data<double>::custom_storage3d:
            return lhs.tensor() == rhs.tensor();

        case node_data<double>::storage4d:          HPX_FALLTHROUGH;
        case node_data<double>::custom_storage4d:
            return lhs.quatern() == rhs.quatern();
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

        switch (lhs.index())
        {
        case node_data<std::uint8_t>::storage0d:          HPX_FALLTHROUGH;
        case node_data<std::uint8_t>::custom_storage0d:
            return lhs.scalar() == rhs.scalar();

        case node_data<std::uint8_t>::storage1d:          HPX_FALLTHROUGH;
        case node_data<std::uint8_t>::custom_storage1d:
            return lhs.vector() == rhs.vector();

        case node_data<std::uint8_t>::storage2d:          HPX_FALLTHROUGH;
        case node_data<std::uint8_t>::custom_storage2d:
            return lhs.matrix() == rhs.matrix();

        case node_data<std::uint8_t>::storage3d:          HPX_FALLTHROUGH;
        case node_data<std::uint8_t>::custom_storage3d:
            return lhs.tensor() == rhs.tensor();

        case node_data<std::uint8_t>::storage4d:          HPX_FALLTHROUGH;
        case node_data<std::uint8_t>::custom_storage4d:
            return lhs.quatern() == rhs.quatern();
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

        switch (lhs.index())
        {
        case node_data<std::int64_t>::storage0d:          HPX_FALLTHROUGH;
        case node_data<std::int64_t>::custom_storage0d:
            return lhs.scalar() == rhs.scalar();

        case node_data<std::int64_t>::storage1d:          HPX_FALLTHROUGH;
        case node_data<std::int64_t>::custom_storage1d:
            return lhs.vector() == rhs.vector();

        case node_data<std::int64_t>::storage2d:          HPX_FALLTHROUGH;
        case node_data<std::int64_t>::custom_storage2d:
            return lhs.matrix() == rhs.matrix();

        case node_data<std::int64_t>::storage3d:          HPX_FALLTHROUGH;
        case node_data<std::int64_t>::custom_storage3d:
            return lhs.tensor() == rhs.tensor();

        case node_data<std::int64_t>::storage4d:          HPX_FALLTHROUGH;
        case node_data<std::int64_t>::custom_storage4d:
            return lhs.quatern() == rhs.quatern();
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
        struct isclose
        {
            bool operator()(double lhs, double rhs) const
            {
                return std::abs(lhs - rhs) <= atol + rtol * std::abs(rhs) ||
                    (equal_nan && std::isnan(lhs) && std::isnan(rhs));
            }

            double rtol;
            double atol;
            bool equal_nan;
        };
    }

    bool allclose(node_data<double> const& lhs, node_data<double> const& rhs,
        double rtol, double atol, bool equal_nan)
    {
        if (lhs.num_dimensions() != rhs.num_dimensions() ||
            lhs.dimensions() != rhs.dimensions())
        {
            return false;
        }

        auto isclose = detail::isclose{atol, rtol, equal_nan};

        switch (lhs.index())
        {
        case node_data<double>::storage0d:          HPX_FALLTHROUGH;
        case node_data<double>::custom_storage0d:
            return isclose(lhs.scalar(), rhs.scalar());

        case node_data<double>::storage1d:          HPX_FALLTHROUGH;
        case node_data<double>::custom_storage1d:
            return blaze::reduce(
                blaze::map(lhs.vector(), rhs.vector(), isclose),
                std::logical_and<bool>{});

        case node_data<double>::storage2d:          HPX_FALLTHROUGH;
        case node_data<double>::custom_storage2d:
            return blaze::reduce(
                blaze::map(lhs.matrix(), rhs.matrix(), isclose),
                std::logical_and<bool>{});

        case node_data<double>::storage3d:          HPX_FALLTHROUGH;
        case node_data<double>::custom_storage3d:
            return blaze::reduce(
                blaze::map(lhs.tensor(), rhs.tensor(), isclose),
                std::logical_and<bool>{});

        case node_data<double>::storage4d:          HPX_FALLTHROUGH;
        case node_data<double>::custom_storage4d:
            return blaze::reduce(
                blaze::map(lhs.quatern(), rhs.quatern(), isclose),
                std::logical_and<bool>{});
        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::invalid_status,
            "phylanx::ir::node_data<double>::allclose)",
            "node_data object holds unsupported data type");
    }

    ///////////////////////////////////////////////////////////////////////////
    namespace detail
    {
        template <typename T, typename Range>
        void print_vector(std::ostream& out, Range const& r, std::size_t size)
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

        template <typename T, typename Matrix>
        void print_matrix(std::ostream& out, Matrix const& data,
            std::size_t rows, std::size_t columns)
        {
            out << "[";
            for (std::size_t row = 0; row != rows; ++row)
            {
                if (row != 0)
                    out << ", ";
                print_vector<T>(out, blaze::row(data, row), columns);
            }
            out << "]";
        }

        template <typename T, typename Tensor>
        void print_tensor(std::ostream& out, Tensor const& data,
            std::size_t pages, std::size_t rows, std::size_t columns)
        {
            out << "[";
            for (std::size_t page = 0; page != pages; ++page)
            {
                if (page != 0)
                    out << ", ";
                print_matrix<T>(
                    out, blaze::pageslice(data, page), rows, columns);
            }
            out << "]";
        }

        template <typename T, typename Quatern>
        void print_quatern(std::ostream& out, Quatern const& data,
            std::size_t quats, ::size_t pages, std::size_t rows,
            std::size_t columns)
        {
            out << "[";
            for (std::size_t quat = 0; quat != quats; ++quat)
            {
                if (quat != 0)
                    out << ", ";
                print_tensor<T>(
                    out, blaze::quatslice(data, quat), pages, rows, columns);
            }
            out << "]";
        }
    }

    ///////////////////////////////////////////////////////////////////////////
    std::ostream& operator<<(std::ostream& out, node_data<double> const& nd)
    {
        auto f = [&]()
        {
            switch (nd.index())
            {
            case node_data<double>::storage0d:          HPX_FALLTHROUGH;
            case node_data<double>::custom_storage0d:
                out << nd.scalar();
                break;

            case node_data<double>::storage1d:          HPX_FALLTHROUGH;
            case node_data<double>::custom_storage1d:
                detail::print_vector<double>(out, nd.vector(), nd.size());
                break;

            case node_data<double>::storage2d:          HPX_FALLTHROUGH;
            case node_data<double>::custom_storage2d:
                {
                    auto m = nd.matrix();
                    detail::print_matrix<double>(out, m, m.rows(), m.columns());
                }
                break;

            case node_data<double>::storage3d:          HPX_FALLTHROUGH;
            case node_data<double>::custom_storage3d:
                {
                    auto t = nd.tensor();
                    detail::print_tensor<double>(
                        out, t, t.pages(), t.rows(), t.columns());
                }
                break;

            case node_data<double>::storage4d:          HPX_FALLTHROUGH;
            case node_data<double>::custom_storage4d:
                {
                    auto q = nd.quatern();
                    detail::print_quatern<double>(
                        out, q, q.quats(), q.pages(), q.rows(), q.columns());
                }
                break;
            default:
                throw std::runtime_error("invalid dimensionality: " +
                    std::to_string(nd.num_dimensions()));
            }
        };

        f();

        return out;
    }

    std::ostream& operator<<(
        std::ostream& out, node_data<std::int64_t> const& nd)
    {

        auto f = [&]()
        {
            switch (nd.index())
            {
            case node_data<std::int64_t>::storage0d:          HPX_FALLTHROUGH;
            case node_data<std::int64_t>::custom_storage0d:
                out << nd.scalar();
                break;

            case node_data<std::int64_t>::storage1d:          HPX_FALLTHROUGH;
            case node_data<std::int64_t>::custom_storage1d:
                detail::print_vector<std::int64_t>(out, nd.vector(), nd.size());
                break;

            case node_data<std::int64_t>::storage2d:          HPX_FALLTHROUGH;
            case node_data<std::int64_t>::custom_storage2d:
                {
                    auto m = nd.matrix();
                    detail::print_matrix<std::int64_t>(
                        out, m, m.rows(), m.columns());
                }
                break;

            case node_data<std::int64_t>::storage3d:          HPX_FALLTHROUGH;
            case node_data<std::int64_t>::custom_storage3d:
                {
                    auto t = nd.tensor();
                    detail::print_tensor<std::int64_t>(
                        out, t, t.pages(), t.rows(), t.columns());
                }
                break;

            case node_data<std::int64_t>::storage4d:          HPX_FALLTHROUGH;
            case node_data<std::int64_t>::custom_storage4d:
                {
                    auto q = nd.quatern();
                    detail::print_quatern<std::int64_t>(
                        out, q, q.quats(), q.pages(), q.rows(), q.columns());
                }
                break;
            default:
                throw std::runtime_error("invalid dimensionality: " +
                    std::to_string(nd.num_dimensions()));
            }
        };

        f();

        return out;
    }

    ///////////////////////////////////////////////////////////////////////////
    std::ostream& operator<<(
        std::ostream& out, node_data<std::uint8_t> const& nd)
    {
        auto f = [&]()
        {
            switch (nd.index())
            {
            case node_data<std::uint8_t>::storage0d:          HPX_FALLTHROUGH;
            case node_data<std::uint8_t>::custom_storage0d:
                out << std::boolalpha << bool{nd.scalar() != 0};
                break;

            case node_data<std::uint8_t>::storage1d:          HPX_FALLTHROUGH;
            case node_data<std::uint8_t>::custom_storage1d:
                out << std::boolalpha;
                detail::print_vector<bool>(out, nd.vector(), nd.size());
                break;

            case node_data<std::uint8_t>::storage2d:          HPX_FALLTHROUGH;
            case node_data<std::uint8_t>::custom_storage2d:
                {
                    auto m = nd.matrix();
                    out << std::boolalpha;
                    detail::print_matrix<bool>(out, m, m.rows(), m.columns());
                }
                break;

            case node_data<std::uint8_t>::storage3d:          HPX_FALLTHROUGH;
            case node_data<std::uint8_t>::custom_storage3d:
                {
                    auto t = nd.tensor();
                    detail::print_tensor<bool>(
                        out, t, t.pages(), t.rows(), t.columns());
                }
                break;

            case node_data<std::uint8_t>::storage4d:          HPX_FALLTHROUGH;
            case node_data<std::uint8_t>::custom_storage4d:
                {
                    auto q = nd.quatern();
                    detail::print_quatern<bool>(
                        out, q, q.quats(), q.pages(), q.rows(), q.columns());
                }
                break;
            default:
                throw std::runtime_error("invalid dimensionality: " +
                    std::to_string(nd.num_dimensions()));
            }
        };

        f();

        return out;
    }

    ///////////////////////////////////////////////////////////////////////////
    template <typename T>
    node_data<T>::operator bool() const
    {
        switch (index())
        {
        case storage0d:          HPX_FALLTHROUGH;
        case custom_storage0d:
            return scalar() != T(0);

        case storage1d:          HPX_FALLTHROUGH;
        case custom_storage1d:
            return vector().nonZeros() != 0;

        case storage2d:          HPX_FALLTHROUGH;
        case custom_storage2d:
            return matrix().nonZeros() != 0;

        case storage3d:          HPX_FALLTHROUGH;
        case custom_storage3d:
            return tensor().nonZeros() != 0;

        case storage4d:          HPX_FALLTHROUGH;
        case custom_storage4d:
            return quatern().nonZeros() != 0;
        default:
            HPX_THROW_EXCEPTION(hpx::invalid_status,
                "node_data<double>::operator bool",
                "invalid dimensionality: " + std::to_string(num_dimensions()));
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
        case storage0d:
            ar << util::get<storage0d>(data_);
            break;

        case storage1d:
            ar << util::get<storage1d>(data_);
            break;

        case storage2d:
            ar << util::get<storage2d>(data_);
            break;

        case custom_storage0d:
            ar << util::get<custom_storage0d>(data_).get();
            break;

        case custom_storage1d:
            ar << util::get<custom_storage1d>(data_);
            break;

        case custom_storage2d:
            ar << util::get<custom_storage2d>(data_);
            break;

        case storage3d:
            ar << util::get<storage3d>(data_);
            break;

        case storage4d:
            ar << util::get<storage4d>(data_);
            break;

        case custom_storage3d:
            ar << util::get<custom_storage3d>(data_);
            break;

        case custom_storage4d:
            ar << util::get<custom_storage4d>(data_);
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
        case storage0d:         HPX_FALLTHROUGH;
        case custom_storage0d:     // deserialize reference_wrapper<T> as T
            {
                T val = 0;
                ar >> val;
                data_ = val;
            }
            break;

        case storage1d:         HPX_FALLTHROUGH;
        case custom_storage1d:     // deserialize CustomVector as DynamicVector
            {
                storage1d_type v;
                ar >> v;
                data_ = std::move(v);
            }
            break;

        case storage2d:         HPX_FALLTHROUGH;
        case custom_storage2d:     // deserialize CustomMatrix as DynamicMatrix
            {
                storage2d_type m;
                ar >> m;
                data_ = std::move(m);
            }
            break;

        case storage3d:         HPX_FALLTHROUGH;
        case custom_storage3d:     // deserialize CustomTensor as DynamicTensor
            {
                storage3d_type t;
                ar >> t;
                data_ = std::move(t);
            }
            break;

        case storage4d:
        case custom_storage4d:     // deserialize 4d CustomArray as 4d DynamicArray
            {
                storage4d_type q;
                ar >> q;
                data_ = std::move(q);
            }
            break;
        default:
            HPX_THROW_EXCEPTION(hpx::invalid_status,
                "node_data<T>::serialize",
                "node_data object holds unsupported data type");
        }
    }
}}

template class PHYLANX_EXPORT phylanx::ir::node_data<double>;
template class PHYLANX_EXPORT phylanx::ir::node_data<std::uint8_t>;
template class PHYLANX_EXPORT phylanx::ir::node_data<std::int64_t>;
