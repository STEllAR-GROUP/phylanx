////////////////////////////////////////////////////////////////////////////////
//  Copyright (c) 2018 Bibek Wagle
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
////////////////////////////////////////////////////////////////////////////////

#include <phylanx/config.hpp>
#include <phylanx/ast/detail/is_literal_value.hpp>
#include <phylanx/ir/node_data.hpp>
#include <phylanx/ir/ranges.hpp>
#include <phylanx/util/slicing_helpers.hpp>

#include <hpx/util/assert.hpp>

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <string>
#include <utility>
#include <vector>

namespace phylanx { namespace util { namespace slicing_helpers
{
    ///////////////////////////////////////////////////////////////////////////
    // extract a single integer from the given node_data instance
    std::int64_t extract_integer(
        execution_tree::primitive_argument_type const& val,
        std::int64_t default_value, std::string const& name,
        std::string const& codename)
    {
        if (valid(val))
        {
            auto&& nd =
                execution_tree::extract_integer_value(val, name, codename);

            if (nd.size() == 0)
            {
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "phylanx::util::slicing_helpers::extract_integer",
                    "slicing arguments cannot be empty");
            }

            return nd[0];
        }
        return default_value;
    }

    ///////////////////////////////////////////////////////////////////////////
    // generate a list of indices to extract from a given vector
    std::vector<std::size_t> create_list_slice(
        std::int64_t start, std::int64_t stop, std::int64_t step)
    {
        std::vector<std::size_t> result;

        if (step > 0)
        {
            result.reserve((std::max)(std::int64_t(0), stop - start));
            for(std::int64_t i = start; i < stop; i += step)
            {
                HPX_ASSERT(i >= 0);
                result.push_back(std::size_t(i));
            }
        }
        else if (step < 0)
        {
            result.reserve((std::max)(std::int64_t(0), start - stop));
            for(std::int64_t i = start; i > stop; i += step)
            {
                HPX_ASSERT(i >= 0);
                result.push_back(std::size_t(i));
            }
        }

        return result;
    }

    ///////////////////////////////////////////////////////////////////////////
    ir::node_data<std::int64_t> create_list_slice(
        ir::node_data<std::uint8_t> const& indices)
    {
        typename ir::node_data<std::int64_t>::storage1d_type result(
            indices.size());

        std::size_t count = 0;
        for (std::size_t i = 0; i != indices.size(); ++i)
        {
            if (indices[i])
            {
                result[count++] = std::int64_t(i);
            }
        }
        result.resize(count);
        result.shrinkToFit();

        return ir::node_data<std::int64_t>{std::move(result)};
    }

    ir::node_data<std::int64_t> create_list_slice(
        bool indices, std::size_t size)
    {
        if (indices)
        {
            typename ir::node_data<std::int64_t>::storage1d_type result(size);

            std::size_t count = 0;
            for (std::size_t i = 0; i != size; ++i)
            {
                result[count++] = std::int64_t(i);
            }
            result.resize(count);
            result.shrinkToFit();

            return ir::node_data<std::int64_t>{std::move(result)};
        }

        return ir::node_data<std::int64_t>{
            typename ir::node_data<std::int64_t>::storage1d_type(0)};
    }

    ///////////////////////////////////////////////////////////////////////////
    // extract a list of indices that correspond to the given slicing parameters
    ir::slicing_indices extract_slicing(
        execution_tree::primitive_argument_type const& arg,
        std::size_t arg_size, std::string const& name,
        std::string const& codename)
    {
        ir::slicing_indices indices;

        // Extract the list or the single integer index
        // from second argument (row-> start, stop, step)
        if (execution_tree::is_list_operand_strict(arg))
        {
            auto arg_list =
                execution_tree::extract_list_value(arg, name, codename);

            if (arg_list.is_xrange())
            {
                return arg_list.xrange();
            }

            std::size_t size = arg_list.size();
            if (size > 3)
            {
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "phylanx::util::slicing_helpers::extract_slicing",
                    "too many indicies given");
            }

            if (size == 0)
            {
                // an empty argument list means return all of the argument
                indices.start(0, false);
                indices.stop(arg_size);
                indices.step(1);
                return indices;
            }

            auto it = arg_list.begin();

            // if step is negative and start/stop are not given, then start
            // and stop must be swapped
            std::int64_t default_start = 0;
            std::int64_t default_stop = arg_size;
            std::int64_t step = 1;
            if (size == 3)
            {
                std::advance(it, 2);
                step = extract_integer(*it, 1ll, name, codename);
                if (step < 0)
                {
                    // we will add list size to these values below
                    default_start = -1;
                    default_stop = -arg_size - 1;
                }
            }

            // reinit iterator
            it = arg_list.begin();

            // default first index is '0' (if 'nil' was specified)
            indices.start(
                extract_integer(*it, default_start, name, codename), true);

            if (indices.start() < 0)
            {
                indices.start(arg_size + indices.start());
            }

            // default last index is 'size' (if 'nil' was specified); if no
            // stop value was specified, simply slice one element
            if (size > 1)
            {
                indices.stop(
                    extract_integer(*++it, default_stop, name, codename),
                    false);

                if (indices.stop() < 0)
                {
                    indices.stop(arg_size + indices.stop());
                }
            }

            // default step is '1' (if 'nil' was specified); if no step value
            // was specified, simply slice one element
            if (size > 1)
            {
                indices.step(step, false);
            }
        }
        else if (!valid(arg))
        {
            // no arguments given means return all of the argument
            indices.start(0, false);
            indices.stop(arg_size);
            indices.step(1);
        }
        else
        {
            // allow for the slicing parameters to be a single integer
            indices.start(extract_integer(arg, 0, name, codename), true);

            if (indices.start() < 0)
            {
                indices.start(arg_size + indices.start());
            }
        }

        return indices;
    }

    ///////////////////////////////////////////////////////////////////////////
    std::size_t slicing_size(
        execution_tree::primitive_argument_type const& arg,
        std::size_t arg_size, std::string const& name,
        std::string const& codename)
    {
        // Extract the list or the single integer index
        // from second argument (row-> start, stop, step)
        if (execution_tree::is_list_operand_strict(arg))
        {
            auto arg_list =
                execution_tree::extract_list_value(arg, name, codename);
            std::size_t size = arg_list.size();

            if (arg_list.is_xrange())
            {
                return size;
            }

            if (size > 3)
            {
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "phylanx::util::slicing_helpers::extract_slicing",
                    "too many indicies given");
            }

            if (size == 0)
            {
                // an empty argument list means return all of the argument
                return arg_size;
            }

            auto it = arg_list.begin();

            // if step is negative and start/stop are not given, then start
            // and stop must be swapped
            std::int64_t start = 0;
            std::int64_t stop = arg_size;
            std::int64_t step = 1;
            if (size == 3)
            {
                std::advance(it, 2);
                step = extract_integer(*it, 1ll, name, codename);
                if (step < 0)
                {
                    // we will add list size to these values below
                    start = -1;
                    stop = -arg_size - 1;
                }
            }

            // reinit iterator
            it = arg_list.begin();

            // default first index is '0' (if 'nil' was specified)
            bool single_element = true;
            start = extract_integer(*it, start, name, codename);
            if (start < 0)
            {
                start += arg_size;
                single_element = false;
            }

            // default last index is 'size' (if 'nil' was specified); if no
            // stop value was specified, simply slice one element
            if (size > 1)
            {
                single_element = false;
                stop = extract_integer(*++it, stop, name, codename);
                if (stop < 0)
                {
                    stop += arg_size;
                }
            }

            if (single_element)
            {
                return 1;
            }

            if (step > 0)
            {
                return (stop - start + step - 1ll) / step;
            }

            return (start - stop - step - 1ll) / -step;
        }
        else if (!valid(arg))
        {
            // no arguments given means return all of the argument
            return arg_size;
        }
        else
        {
            // allow for the slicing parameters to be a single integer
            return 1;
        }
    }
}}}
