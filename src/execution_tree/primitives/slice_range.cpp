// Copyright (c) 2018 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/base_primitive.hpp>
#include <phylanx/execution_tree/primitives/slice_range.hpp>
#include <phylanx/ir/ranges.hpp>
#include <phylanx/util/generate_error_message.hpp>
#include <phylanx/util/slicing_helpers.hpp>

#include <hpx/exception.hpp>
#include <hpx/util/assert.hpp>

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <iterator>
#include <string>
#include <utility>
#include <vector>

namespace phylanx { namespace execution_tree
{
    ///////////////////////////////////////////////////////////////////////////
    // extract a slice from the given range instance
    primitive_argument_type slice_list(ir::range&& list,
        ir::slicing_indices const& indices, std::string const& name,
        std::string const& codename)
    {
        std::size_t list_size = list.size();

        std::int64_t start = indices.start();
        std::int64_t stop = indices.stop();
        std::int64_t step = indices.step();

        if (step == 0 && !indices.single_value())
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "phylanx::execution_tree::slice_list",
                util::generate_error_message(
                    "step can not be zero", name, codename));
        }

        // handle single element to return
        if (indices.single_value())
        {
            if (list.is_ref())
            {
                auto it = list.begin();
                std::advance(it, start);
                return primitive_argument_type{std::move(*it)};
            }

            auto const& args = list.args();
            return primitive_argument_type{args[start]};
        }

        // handle case of consecutive elements to return
        if (indices.step() == 1)
        {
            primitive_arguments_type result;
            result.reserve(stop - start);

            if (list.is_ref())
            {
                auto begin = list.begin();
                std::advance(begin, start);
                auto end = list.begin();
                std::advance(end, stop);
                std::copy(begin, end, std::back_inserter(result));
                return primitive_argument_type{ir::range(std::move(result))};
            }

            auto const& args = list.args();
            std::copy(args.begin() + start, args.begin() + stop,
                std::back_inserter(result));
            return primitive_argument_type{ir::range(std::move(result))};
        }

        // list of indices to extract
        auto index_list =
            util::slicing_helpers::create_list_slice(start, stop, step);

        primitive_arguments_type result;
        result.reserve(index_list.size());

        auto idx_it = index_list.begin();
        auto idx_end = index_list.end();

        // extract elements from list at given indices
        if (start <= stop && step > 0)
        {
            auto list_it = list.begin();
            auto list_end = list.end();
            std::size_t idx = (idx_it != idx_end) ? *idx_it : 0;
            std::advance(list_it, (std::min)(list_size, idx));

            for (/**/; list_it != list_end && idx_it != idx_end;
                 ++idx, ++list_it)
            {
                if (idx == *idx_it)
                {
                    ++idx_it;
                    result.emplace_back(std::move(*list_it));
                }
            }
        }
        else
        {
            auto list_it = list.rbegin();
            auto list_end = list.rend();
            std::size_t idx = list_size - 1;
            for (/**/; list_it != list_end && idx_it != idx_end;
                 --idx, ++list_it)
            {
                if (idx == *idx_it)
                {
                    ++idx_it;
                    result.emplace_back(std::move(*list_it));
                }
            }
        }

        return primitive_argument_type{std::move(result)};
    }

    ///////////////////////////////////////////////////////////////////////////
    // extract a slice from the given range instance
    primitive_argument_type slice_list(ir::range&& data,
        primitive_argument_type const& indices, std::string const& name,
        std::string const& codename)
    {
        std::size_t size = data.size();
        return slice_list(std::move(data),
            util::slicing_helpers::extract_slicing(indices, size), name,
            codename);
    }

    ///////////////////////////////////////////////////////////////////////////
    template <typename F>
    primitive_argument_type slice_list(ir::range&& list,
        ir::slicing_indices const& indices, F && f,
        std::string const& name, std::string const& codename)
    {
        std::size_t list_size = list.size();

        std::int64_t start = indices.start();
        std::int64_t stop = indices.stop();
        std::int64_t step = indices.step();

        if (step == 0 && !indices.single_value())
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "phylanx::execution_tree::slice_list",
                util::generate_error_message(
                    "step can not be zero", name, codename));
        }

        // handle single element to return
        if (indices.single_value())
        {
            if (list.is_ref())
            {
                primitive_arguments_type result;
                result.reserve(list.size());

                std::copy(list.begin(), list.end(), std::back_inserter(result));

                auto it = result.begin();
                std::advance(it, start);

                f(*it);

                return primitive_argument_type{std::move(result)};
            }

            auto& args = list.args();
            f(args[start]);
            return primitive_argument_type{std::move(list)};
        }

        // handle case of consecutive elements to modify
        if (indices.step() == 1)
        {
            primitive_arguments_type result;

            if (list.is_ref())
            {
                primitive_arguments_type result;
                result.reserve(list.size());

                std::copy(list.begin(), list.end(), std::back_inserter(result));

                auto begin = result.begin();
                std::advance(begin, start);
                auto end = result.begin();
                std::advance(end, stop);

                f(begin, end);

                return primitive_argument_type{std::move(result)};
            }

            auto& args = list.args();
            f(args.begin() + start, args.begin() + stop);
            return primitive_argument_type{std::move(list)};
        }

//         // list of indices to extract
//         std::vector<std::int64_t> index_list =
//             util::slicing_helpers::create_list_slice(start, stop, step);
//
//         primitive_arguments_type result;
//         result.reserve(list.size());
//
//         auto idx_it = index_list.begin();
//         auto idx_end = index_list.end();
//
//         // extract elements from list at given indices
//         if (start <= stop && step > 0)
//         {
//             auto list_it = list.begin();
//             auto list_end = list.end();
//             std::size_t idx = (idx_it != idx_end) ? *idx_it : 0;
//             std::advance(list_it, (std::min)(list_size, idx));
//
//             for (/**/; list_it != list_end && idx_it != idx_end;
//                  ++idx, ++list_it)
//             {
//                 if (idx == *idx_it)
//                 {
//                     ++idx_it;
//                     result.emplace_back(std::move(*list_it));
//                 }
//             }
//         }
//         else
//         {
//             auto list_it = list.rbegin();
//             auto list_end = list.rend();
//             std::size_t idx = list_size - 1;
//             for (/**/; list_it != list_end && idx_it != idx_end;
//                  --idx, ++list_it)
//             {
//                 if (idx == *idx_it)
//                 {
//                     ++idx_it;
//                     result.emplace_back(std::move(*list_it));
//                 }
//             }
//         }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "phylanx::execution_tree::slice_list",
            util::generate_error_message(
                "assignment to list elements based on an index list is not "
                "supported (yet)", name, codename));

        return primitive_argument_type{std::move(list)};
    }

    namespace detail
    {
        struct slice_assign_value
        {
            primitive_argument_type& rhs_;

            void operator()(primitive_argument_type& value) const
            {
                value = rhs_;
            }

            template <typename Iterator>
            void operator()(Iterator begin, Iterator end) const
            {
                for (auto it = begin; it != end; ++it)
                {
                    *it = rhs_;
                }
            }
        };

        struct slice_assign_list
        {
            ir::range& rhs_;

            void operator()(primitive_argument_type& value) const
            {
                value = rhs_;
            }

            template <typename Iterator>
            void operator()(Iterator begin, Iterator end) const
            {
                if (std::distance(rhs_.begin(), rhs_.end()) !=
                    std::distance(begin, end))
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "phylanx::execution_tree::detail::slice_assign_list",
                        "cannot assign a list to a slice of a different size");
                }

                std::copy(rhs_.begin(), rhs_.end(), begin);
            }
        };
    }

    ///////////////////////////////////////////////////////////////////////////
    // modify a slice of the given range instance
    primitive_argument_type slice_list(ir::range&& data,
        primitive_argument_type const& indices, primitive_argument_type&& value,
        std::string const& name, std::string const& codename)
    {
        std::size_t size = data.size();

        if (is_list_operand_strict(value))
        {
            ir::range rhs = extract_list_value_strict(value, name, codename);
            return slice_list(std::move(data),
                util::slicing_helpers::extract_slicing(indices, size),
                detail::slice_assign_list{rhs}, name, codename);
        }

        return slice_list(std::move(data),
            util::slicing_helpers::extract_slicing(indices, size),
            detail::slice_assign_value{value}, name, codename);
    }
}}
