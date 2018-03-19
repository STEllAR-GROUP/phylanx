// Copyright (c) 2018 Parsa Amini
// Copyright (c) 2018 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/base_primitive.hpp>
#include <phylanx/ir/ranges.hpp>

#include <hpx/include/util.hpp>
#include <hpx/throw_exception.hpp>

#include <cstdint>

namespace phylanx { namespace ir
{
    //////////////////////////////////////////////////////////////////////////
    execution_tree::primitive_argument_type range_iterator::dereference() const
    {
        switch (it_.index())
        {
        case 0:    // int_range_type
            return execution_tree::primitive_argument_type(
                util::get<0>(it_).first);
        case 1:    // arg_range_type
            return *(util::get<1>(it_));
        }

        HPX_THROW_EXCEPTION(hpx::invalid_status,
            "phylanx::ir::range_iterator::dereference",
            "range_iterator object holds unsupported data type");
    }

    bool range_iterator::equal(range_iterator const& other) const
    {
        switch (it_.index())
        {
        case 0:    // int_range_type
            return util::get<0>(it_).first == util::get<0>(other.it_).first;
        case 1:    // arg_range_type
            return util::get<1>(it_) == util::get<1>(other.it_);
        }

        HPX_THROW_EXCEPTION(hpx::invalid_status,
            "phylanx::ir::range_iterator::equal",
            "range_iterator object holds unsupported data type");
    }

    void range_iterator::increment()
    {
        switch (it_.index())
        {
        case 0:    // int_range_type
        {
            int_range_type& p = util::get<0>(it_);
            p.first += p.second;
            break;
        }
        case 1:    // arg_range_type
            ++util::get<1>(it_);
            break;
        default:
            HPX_THROW_EXCEPTION(hpx::invalid_status,
                "phylanx::ir::range_iterator::increment",
                "range_iterator object holds unsupported data type");
        }
    }

    //////////////////////////////////////////////////////////////////////////
    range_iterator range::begin()
    {
        switch (data_.index())
        {
        case 0:    // int_range_type
        {
            int_range_type& int_range = util::get<0>(data_);
            std::int64_t start = hpx::util::get<0>(int_range);
            std::int64_t step = hpx::util::get<2>(int_range);
            return range_iterator{start, step};
        }
        case 1:    // args_type
            return util::get<1>(data_).begin();
        case 2:    // arg_pair_type
            return util::get<2>(data_).first;
        }
        HPX_THROW_EXCEPTION(hpx::invalid_status,
            "phylanx::ir::range::begin",
            "range object holds unsupported data type");
    }

    range_iterator range::end()
    {
        switch (data_.index())
        {
        case 0:    // int_range_type
        {
            const int_range_type& int_range = util::get<0>(data_);
            const std::int64_t start = hpx::util::get<0>(int_range);
            const std::int64_t stop = hpx::util::get<1>(int_range);
            const std::int64_t step = hpx::util::get<2>(int_range);

            const std::int64_t distance_to_start = stop - start;
            const std::int64_t n_steps = distance_to_start / step;
            const std::int64_t remaining_step =
                distance_to_start % step != 0 ? step : 0;
            const std::int64_t actual_stop =
                start + n_steps * step + remaining_step;

            return range_iterator{actual_stop, step};
        }
        case 1:    // args_type
            return util::get<1>(data_).end();
        case 2:    // arg_pair_type
            return util::get<2>(data_).second;
        }
        HPX_THROW_EXCEPTION(hpx::invalid_status,
            "phylanx::ir::range::end",
            "range object holds unsupported data type");
    }
}}
