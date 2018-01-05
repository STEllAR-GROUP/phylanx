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

#include <cstddef>
#include <iosfwd>
#include <string>

namespace phylanx { namespace ir
{
#if defined(PHYLANX_DEBUG)
    ///////////////////////////////////////////////////////////////////////////
    std::atomic<std::size_t> count_copy_constructions_;
    std::atomic<std::size_t> count_move_constructions_;
    std::atomic<std::size_t> count_copy_assignments_;
    std::atomic<std::size_t> count_move_assignments_;

    void reset_node_statistics()
    {
        count_copy_constructions_ = 0;
        count_move_constructions_ = 0;
        count_copy_assignments_ = 0;
        count_move_assignments_ = 0;
    }

    void print_node_statistics()
    {
        std::cout << "count_copy_constructions: "
                  << count_copy_constructions_ << "\n";
        std::cout << "count_move_constructions: "
                  << count_move_constructions_ << "\n";
        std::cout << "count_copy_assignments:   "
                  << count_copy_assignments_ << "\n";
        std::cout << "count_move_assignments:   "
                  << count_move_assignments_ << "\n";
    }
#else
    void reset_node_statistics()
    {
    }

    void print_node_statistics()
    {
    }
#endif

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
                out << std::to_string(r[i]);
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
            out << std::to_string(nd[0]);
            break;

        case 1: HPX_FALLTHROUGH;
        case 3:
            detail::print_array(out, nd.vector(), nd.size());
            break;

        case 2: HPX_FALLTHROUGH;
        case 4:
            {
                auto data = nd.matrix();
                for (std::size_t row = 0; row != data.rows(); ++row)
                {
                    if (row != 0)
                        out << ", ";
                    detail::print_array(
                        out, blaze::row(data, row), data.columns());
                }
            }
            break;

        default:
            HPX_THROW_EXCEPTION(hpx::invalid_status,
                "node_data<double>::operator bool",
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
            return vector().nonZeros() > 0;

        case 2: HPX_FALLTHROUGH;
        case 4:
            return matrix().nonZeros() > 0;

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
}}
