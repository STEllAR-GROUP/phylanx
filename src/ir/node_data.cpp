//  Copyright (c) 2017-2018 Hartmut Kaiser
//  Copyright (c) 2017 Parsa Amini
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

namespace phylanx { namespace ir
{
    ///////////////////////////////////////////////////////////////////////////
    // performance counter data (for now, counting all node_data<T>)
    static std::atomic<std::int64_t> count_copy_constructions_;
    static std::atomic<std::int64_t> count_move_constructions_;
    static std::atomic<std::int64_t> count_copy_assignments_;
    static std::atomic<std::int64_t> count_move_assignments_;

    template <>
    void node_data<double>::increment_copy_construction_count()
    {
        ++count_copy_constructions_;
    }

    template <>
    void node_data<double>::increment_move_construction_count()
    {
        ++count_move_constructions_;
    }

    template <>
    void node_data<double>::increment_copy_assignment_count()
    {
        ++count_copy_assignments_;
    }

    template <>
    void node_data<double>::increment_move_assignment_count()
    {
        ++count_move_assignments_;
    }

    template <>
    void node_data<bool>::increment_copy_construction_count()
    {
        ++count_copy_constructions_;
    }

    template <>
    void node_data<bool>::increment_move_construction_count()
    {
        ++count_move_constructions_;
    }

    template <>
    void node_data<bool>::increment_copy_assignment_count()
    {
        ++count_copy_assignments_;
    }

    template <>
    void node_data<bool>::increment_move_assignment_count()
    {
        ++count_move_assignments_;
    }

    template <>
    std::int64_t node_data<double>::copy_construction_count(bool reset)
    {
        return hpx::util::get_and_reset_value(count_copy_constructions_, reset);
    }

    template <>
    std::int64_t node_data<double>::move_construction_count(bool reset)
    {
        return hpx::util::get_and_reset_value(count_move_constructions_, reset);
    }

    template <>
    std::int64_t node_data<double>::copy_assignment_count(bool reset)
    {
        return hpx::util::get_and_reset_value(count_copy_assignments_, reset);
    }

    template <>
    std::int64_t node_data<double>::move_assignment_count(bool reset)
    {
        return hpx::util::get_and_reset_value(count_move_assignments_, reset);
    }

    ///////////////////////////////////////////////////////////////////////////
    namespace detail
    {
        template <typename Range>
        void print_array(std::ostream& out, Range const& r, std::size_t size)
        {
            out << "[";
            for (std::size_t i = 0; i != size; ++i)
            {
                if (i != 0)
                {
                    out << ", ";
                }
                out << r[i];
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
            detail::print_array(out, nd.vector(), nd.size());
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
                    detail::print_array(
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

    ///////////////////////////////////////////////////////////////////////////
    template <>
    node_data<double>::operator bool() const
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
    template <>
    void node_data<double>::serialize(hpx::serialization::output_archive& ar,
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
                "node_data<double>::serialize",
                "node_data object holds unsupported data type");
        }
    }

    template <>
    void node_data<double>::serialize(hpx::serialization::input_archive& ar,
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
                "node_data<double>::serialize",
                "node_data object holds unsupported data type");
        }
    }

    ///////////////////////////////////////////////////////////////////////////
    std::ostream& operator<<(std::ostream& out, node_data<bool> const& nd)
    {
        std::size_t dims = nd.num_dimensions();
        switch (dims)
        {
        case 0:
            out << std::to_string(nd[0]);
            break;

        case 1: HPX_FALLTHROUGH;
        case 3:
            detail::print_array(out, nd.vector(), nd.size());
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
                    detail::print_array(
                        out, blaze::row(data, row), data.columns());
                }
                out << "]";
            }
            break;

        default:
            HPX_THROW_EXCEPTION(hpx::invalid_status,
                "node_data<bool>::operator<<()",
                "invalid dimensionality: " + std::to_string(dims));
        }
        return out;
    }

    ///////////////////////////////////////////////////////////////////////////
    template <>
    node_data<bool>::operator bool() const
    {
        std::size_t dims = num_dimensions();
        switch (dims)
        {
        case 0:
            return scalar();

        case 1: HPX_FALLTHROUGH;
        case 3:
            return vector().nonZeros() != 0;

        case 2: HPX_FALLTHROUGH;
        case 4:
            return matrix().nonZeros() != 0;

        default:
            HPX_THROW_EXCEPTION(hpx::invalid_status,
                "node_data<bool>::operator bool",
                "invalid dimensionality: " + std::to_string(dims));
        }
        return false;
    }

    ///////////////////////////////////////////////////////////////////////////
    template <>
    void node_data<bool>::serialize(hpx::serialization::output_archive& ar,
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
                "node_data<bool>::serialize",
                "node_data object holds unsupported data type");
        }
    }

    template <>
    void node_data<bool>::serialize(hpx::serialization::input_archive& ar,
        unsigned)
    {
        std::size_t index = 0;
        ar >> index;

        switch (index)
        {
        case 0:
            {
                bool val = false;
                ar >> val;
                data_ = val;
            }
            break;

        case 1: HPX_FALLTHROUGH;
        case 3:    // deserialize CustomVector as DynamicVector
            {
                storage1d_type v;
                ar >> v;
                data_ = std::move(v);
            }
            break;

        case 2: HPX_FALLTHROUGH;
        case 4:    // deserialize CustomMatrix as DynamicMatrix
            {
                storage2d_type m;
                ar >> m;
                data_ = std::move(m);
            }
            break;

        default:
            HPX_THROW_EXCEPTION(hpx::invalid_status,
                "node_data<bool>::serialize",
                "node_data object holds unsupported data type");
        }
    }
}}
