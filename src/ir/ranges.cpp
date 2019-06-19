// Copyright (c) 2018 Parsa Amini
// Copyright (c) 2018 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/base_primitive.hpp>
#include <phylanx/ir/ranges.hpp>

#include <hpx/include/serialization.hpp>
#include <hpx/include/util.hpp>
#include <hpx/throw_exception.hpp>

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <utility>
#include <vector>

namespace phylanx { namespace ir
{
    //////////////////////////////////////////////////////////////////////////
    reverse_range_iterator range_iterator::invert() const
    {
        switch (it_.index())
        {
        case 0:    // int_range_type
            {
                int_range_type const& p = util::get<0>(it_);
                return reverse_range_iterator{p.start(), p.stop()};
            }

        case 1:    // args_iterator_type
            return reverse_range_iterator(
                args_reverse_iterator_type(util::get<1>(it_)));

        case 2:    // args_const_iterator_type
            return reverse_range_iterator(
                args_reverse_const_iterator_type(util::get<2>(it_)));

        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::invalid_status,
            "phylanx::ir::range_iterator::invert",
            "range_iterator object holds unsupported data type");
    }

    execution_tree::primitive_argument_type range_iterator::dereference() const
    {
        switch (it_.index())
        {
        case 0:    // int_range_type
            return execution_tree::primitive_argument_type(
                util::get<0>(it_).start());

        case 1:    // args_iterator_type
            return *(util::get<1>(it_));

        case 2:    // args_const_iterator_type
            return *(util::get<2>(it_));

        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::invalid_status,
            "phylanx::ir::range_iterator::dereference",
            "range_iterator object holds unsupported data type");
    }

    bool range_iterator::equal(range_iterator const& other) const
    {
        // Ensure that have comparable iterators
        if (it_.index() != other.it_.index())
        {
            return false;
        }

        switch (it_.index())
        {
        case 0:    // int_range_type
            return util::get<0>(it_).start() == util::get<0>(other.it_).start();

        case 1:    // args_iterator_type
            return util::get<1>(it_) == util::get<1>(other.it_);

        case 2:    // args_const_iterator_type
            return util::get<2>(it_) == util::get<2>(other.it_);

        default:
            break;
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
                p.start(p.start() + p.step());
                return;
            }

        case 1:    // args_iterator_type
            ++util::get<1>(it_);
            return;

        case 2:    // args_const_iterator_type
            ++util::get<2>(it_);
            return;

        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::invalid_status,
            "phylanx::ir::range_iterator::increment",
            "range_iterator object holds unsupported data type");
    }

    //////////////////////////////////////////////////////////////////////////
    execution_tree::primitive_argument_type
    reverse_range_iterator::dereference() const
    {
        switch (it_.index())
        {
        case 0:    // int_range_type
            return execution_tree::primitive_argument_type(
                util::get<0>(it_).start());

        case 1:    // args_iterator_type
            return *(util::get<1>(it_));

        case 2:    // args_const_iterator_type
            return *(util::get<2>(it_));

        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::invalid_status,
            "phylanx::ir::reverse_range_iterator::dereference",
            "reverse_range_iterator object holds unsupported data type");
    }

    bool reverse_range_iterator::equal(reverse_range_iterator const& other) const
    {
        // Ensure that have comparable iterators
        if (it_.index() != other.it_.index())
        {
            return false;
        }

        switch (it_.index())
        {
        case 0:    // int_range_type
            return util::get<0>(it_).start() == util::get<0>(other.it_).start();

        case 1:    // args_iterator_type
            return util::get<1>(it_) == util::get<1>(other.it_);

        case 2:    // args_const_iterator_type
            return util::get<2>(it_) == util::get<2>(other.it_);

        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::invalid_status,
            "phylanx::ir::reverse_range_iterator::equal",
            "reverse_range_iterator object holds unsupported data type");
    }

    void reverse_range_iterator::increment()
    {
        switch(it_.index())
        {
        case 0:    // int_range_type
            {
                int_range_type& p = util::get<0>(it_);
                p.start(p.start() - p.step());
                return;
            }

        case 1:    // args_iterator_type
            ++util::get<1>(it_);
            return;

        case 2:    // args_const_iterator_type
            ++util::get<2>(it_);
            return;

        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::invalid_status,
            "phylanx::ir::reverse_range_iterator::increment",
            "reverse_range_iterator object holds unsupported data type");
    }

    //////////////////////////////////////////////////////////////////////////
    range_iterator range::begin() const
    {
        switch (data_.index())
        {
        case 0:    // int_range_type
            {
                int_range_type const& int_range = util::get<0>(data_);
                std::int64_t start = int_range.start();
                std::int64_t step = int_range.step();
                return range_iterator{start, step};
            }

        case 1:    // wrapped_args_type
            return util::get<1>(data_).get().begin();

        case 2:    // arg_pair_type
            return util::get<2>(data_).first;

        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::invalid_status,
            "phylanx::ir::range::begin",
            "range object holds unsupported data type");
    }

    range_iterator range::end() const
    {
        switch (data_.index())
        {
        case 0:    // int_range_type
            {
                int_range_type const& int_range = util::get<0>(data_);
                std::int64_t const start = int_range.start();
                std::int64_t const stop = int_range.stop();
                std::int64_t const step = int_range.step();

                std::int64_t const distance_to_start = stop - start;
                std::int64_t const n_steps = distance_to_start / step;
                std::int64_t const remaining_step =
                    distance_to_start % step != 0 ? step : 0;
                std::int64_t const actual_stop =
                    start + n_steps * step + remaining_step;

                return range_iterator{actual_stop, step};
            }

        case 1:    // wrapped_args_type
            return util::get<1>(data_).get().end();

        case 2:    // arg_pair_type
            return util::get<2>(data_).second;

        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::invalid_status,
            "phylanx::ir::range::end",
            "range object holds unsupported data type");
    }

    reverse_range_iterator range::rbegin() const
    {
        using args_iterator_type = std::vector<
            execution_tree::primitive_argument_type>::reverse_iterator;

        switch (data_.index())
        {
        case 0:    // int_range_type
            {
                int_range_type const& int_range = util::get<0>(data_);
                std::int64_t const start = int_range.start();
                std::int64_t const stop = int_range.stop();
                std::int64_t const step = int_range.step();

                std::int64_t const distance_to_start = stop - start;
                std::int64_t const n_steps = distance_to_start / step;
                std::int64_t const remaining_step =
                    distance_to_start % step != 0 ? 0 : -step;
                std::int64_t const actual_begin =
                    start + n_steps * step + remaining_step;

                return reverse_range_iterator{actual_begin, step};
            }

        case 1:    // wrapped_args_type
            return util::get<1>(data_).get().rbegin();

        case 2:    // arg_pair_type
            return util::get<2>(data_).second.invert();

        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::invalid_status,
            "phylanx::ir::range::rbegin",
            "range object holds unsupported data type");
    }

    reverse_range_iterator range::rend() const
    {
        using args_iterator_type = std::vector<
            execution_tree::primitive_argument_type>::reverse_iterator;

        switch (data_.index())
        {
        case 0:    // int_range_type
            {
                int_range_type const& int_range = util::get<0>(data_);
                std::int64_t start = int_range.start();
                std::int64_t step = int_range.step();
                return reverse_range_iterator{start - step, step};
            }

        case 1:    // wrapped_args_type
            return util::get<1>(data_).get().rend();

        case 2:    // arg_pair_type
            return util::get<2>(data_).first.invert();

        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::invalid_status,
            "phylanx::ir::range::rend",
            "range object holds unsupported data type");
    }

    std::ptrdiff_t range::size() const
    {
        switch (data_.index())
        {
        case 0:    // int_range_type
            {
                int_range_type const& int_range = util::get<0>(data_);
                std::int64_t const start = int_range.start();
                std::int64_t const stop = int_range.stop();
                std::int64_t const step = int_range.step();

                std::int64_t const distance_to_start = stop - start;

                // Adding step to start must get closer to stop
                if ((distance_to_start > 0) != (step > 0))
                {
                    return 0;
                }
                std::int64_t const n_steps = distance_to_start / step;
                std::int64_t const remaining_step =
                    (distance_to_start % step > 0) ? 1 : 0;

                return n_steps + remaining_step;
            }

        case 1:    // wrapped_args_type
            return util::get<1>(data_).get().size();

        case 2:    // arg_pair_type
            {
                auto const& first = util::get<2>(data_).first;
                auto const& second = util::get<2>(data_).second;
                return std::distance(first, second);
            }

        default:
            break;
        }
        HPX_THROW_EXCEPTION(hpx::invalid_status,
            "phylanx::ir::range::rend",
            "range object holds unsupported data type");
    }

    bool range::empty() const
    {
        switch (data_.index())
        {
        case 0:    // int_range_type
            {
                auto const& v = util::get<0>(data_);
                return v.start() == v.stop();
            }

        case 1:    // wrapped_args_type
            {
                auto const& v = util::get<1>(data_).get();
                return v.begin() == v.end();
            }

        case 2:    // arg_pair_type
            {
                auto const& v = util::get<2>(data_);
                return v.first == v.second;
            }

        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::invalid_status,
            "phylanx::ir::range::empty",
            "range object holds unsupported data type");
    }

    range::args_type& range::args()
    {
        wrapped_args_type* cv = util::get_if<wrapped_args_type>(&data_);
        if (cv != nullptr)
            return cv->get();

        HPX_THROW_EXCEPTION(hpx::invalid_status,
            "phylanx::ir::range::args()",
            "range object holds unsupported data type");
    }

    range::args_type const& range::args() const
    {
        wrapped_args_type const* cv = util::get_if<wrapped_args_type>(&data_);
        if (cv != nullptr)
            return cv->get();

        HPX_THROW_EXCEPTION(hpx::invalid_status,
            "phylanx::ir::range::args()",
            "range object holds unsupported data type");
    }

    range::arg_pair_type& range::args_ref()
    {
        arg_pair_type* cv = util::get_if<arg_pair_type>(&data_);
        if (cv != nullptr)
            return *cv;

        HPX_THROW_EXCEPTION(hpx::invalid_status,
            "phylanx::ir::range::args_ref()",
            "range object holds unsupported data type");
    }

    range::arg_pair_type const& range::args_ref() const
    {
        arg_pair_type const* cv = util::get_if<arg_pair_type>(&data_);
        if (cv != nullptr)
            return *cv;

        HPX_THROW_EXCEPTION(hpx::invalid_status,
            "phylanx::ir::range::args_ref()",
            "range object holds unsupported data type");
    }

    range::int_range_type& range::xrange()
    {
        int_range_type* cv = util::get_if<int_range_type>(&data_);
        if (cv != nullptr)
            return *cv;

        HPX_THROW_EXCEPTION(hpx::invalid_status,
            "phylanx::ir::range::xrange()",
            "range object holds unsupported data type");
    }

    range::int_range_type const& range::xrange() const
    {
        int_range_type const* cv = util::get_if<int_range_type>(&data_);
        if (cv != nullptr)
            return *cv;

        HPX_THROW_EXCEPTION(hpx::invalid_status,
            "phylanx::ir::range::xrange()",
            "range object holds unsupported data type");
    }

    range::args_type range::copy() const
    {
        switch (data_.index())
        {
        case 0:    // int_range_type
            {
                args_type result;
                result.reserve(size());
                std::copy(begin(), end(), std::back_inserter(result));
                return result;
            }

        case 1:    // wrapped_args_type
            return phylanx::util::get<1>(data_).get();

        case 2:    // arg_pair_type
            {
                args_type result;
                result.reserve(size());
                auto const& v = util::get<2>(data_);
                std::copy(v.first, v.second, std::back_inserter(result));
                return result;
            }

        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::invalid_status,
            "phylanx::ir::range::args()",
            "range object holds unsupported data type");
    }

    range range::ref() const
    {
        switch (data_.index())
        {
        case 0: HPX_FALLTHROUGH;    // int_range_type
        case 1: HPX_FALLTHROUGH;    // wrapped_args_type
        case 2:                     // arg_pair_type
            return range{begin(), end()};

        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::invalid_status,
            "phylanx::ir::range::ref() const",
            "range object holds unsupported data type");
    }

    bool range::is_ref() const
    {
        switch (data_.index())
        {
        case 1:                     // wrapped_args_type
            return false;

        case 0: HPX_FALLTHROUGH;    // int_range_type
        case 2:                     // arg_pair_type
            return true;

        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::invalid_status,
            "phylanx::ir::range::is_ref()",
            "range object holds unsupported data type");
    }

    bool range::is_args() const
    {
        switch (data_.index())
        {
        case 0:                     // int_range_type
            return false;

        case 1: HPX_FALLTHROUGH;    // wrapped_args_type
        case 2:                     // arg_pair_type
            return true;

        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::invalid_status,
            "phylanx::ir::range::is_args()",
            "range object holds unsupported data type");
    }

    bool range::is_args_ref() const
    {
        switch (data_.index())
        {
        case 0: HPX_FALLTHROUGH;    // int_range_type
        case 1:                     // wrapped_args_type
            return false;

        case 2:                     // arg_pair_type
            return true;

        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::invalid_status,
            "phylanx::ir::range::is_args_ref()",
            "range object holds unsupported data type");
    }

    bool range::is_xrange() const
    {
        switch (data_.index())
        {
        case 0:                     // int_range_type
            return true;

        case 1: HPX_FALLTHROUGH;    // wrapped_args_type
        case 2:                     // arg_pair_type
            return false;

        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::invalid_status,
            "phylanx::ir::range::is_xrange()",
            "range object holds unsupported data type");
    }

    ///////////////////////////////////////////////////////////////////////////
    bool operator==(range const& lhs, range const& rhs)
    {
        if (lhs.size() != rhs.size())
        {
            return false;
        }
        return std::equal(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
    }

    bool operator!=(range const& lhs, range const& rhs)
    {
        return !(lhs == rhs);
    }

    void range::serialize(hpx::serialization::output_archive& ar, unsigned)
    {
        std::size_t index = data_.index();
        ar << index;

        switch(index)
        {
        case 0:    // int_range_type
            {
                int_range_type& int_range = util::get<0>(data_);
                ar << int_range.start() << int_range.stop() << int_range.step();
                break;
            }

        case 1:    // wrapped_args_type
            {
                ar << util::get<1>(data_);
                break;
            }

        case 2:    // arg_pair_type
            {
                arg_pair_type p = util::get<2>(data_);
                args_type m;
                m.reserve(size());
                std::copy(p.first, p.second, std::back_inserter(m));
                ar << m;
                break;
            }

        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::invalid_status,
            "phylanx::ir::range::serialize()",
            "range object holds unsupported data type");
    }
    void range::serialize(hpx::serialization::input_archive& ar, unsigned)
    {
        std::size_t index = 0;
        ar >> index;

        switch (index)
        {
        case 0:    // int_range_type
            {
                std::int64_t start, stop, step;
                ar >> start >> stop >> step;
                data_ = int_range_type{start, stop, step};
                break;
            }

        case 1:    // wrapped_args_type
        case 2:    // arg_pair_type (serialized as wrapped_args_type)
            {
                args_type m;
                ar >> m;
                data_ = std::move(m);
                break;
            }

        default:
            HPX_THROW_EXCEPTION(hpx::invalid_status,
                "phylanx::ir::range::serialize()",
                "range object holds unsupported data type");
        }
    }
}}
