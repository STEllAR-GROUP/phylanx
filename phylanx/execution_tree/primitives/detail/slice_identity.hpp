// Copyright (c) 2018 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_DETAIL_SLICE_IDENTITY_SEP_18_2018_1205PM)
#define PHYLANX_DETAIL_SLICE_IDENTITY_SEP_18_2018_1205PM

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/base_primitive.hpp>
#include <phylanx/ir/node_data.hpp>

#include <cstddef>
#include <utility>

#include <blaze/Math.h>

namespace phylanx { namespace execution_tree { namespace detail
{
    ///////////////////////////////////////////////////////////////////////////
    struct always_true
    {
        HPX_FORCEINLINE std::size_t operator()(std::size_t index) const
        {
            return index;
        }
    };

    ///////////////////////////////////////////////////////////////////////////
    template <typename T>
    struct slice_identity
    {
        template <typename Data, typename Scalar, typename F = always_true>
        ir::node_data<T> scalar(
            Data const&, Scalar&& value, F const& f = F{}) const
        {
            if (f(0) != std::size_t(-1))
            {
                return ir::node_data<T>{std::forward<Scalar>(value)};
            }
            return ir::node_data<T>{typename ir::node_data<T>::storage0d_type{}};
        }

        template <typename Data, typename View, typename F = always_true>
        ir::node_data<T> vector(
            Data const&, View&& view, F const& f = F{}) const
        {
            std::size_t size = view.size();
            blaze::DynamicVector<T> result(size);

            std::size_t count = 0;
            for (std::size_t i = 0; i != size; ++i)
            {
                std::size_t idx = f(i);
                if (idx != std::size_t(-1))
                {
                    result[count++] = view[idx];
                }
            }

            result.resize(count);
            result.shrinkToFit();

            return ir::node_data<T>{std::move(result)};
        }

        template <typename Data, typename View, typename F = always_true>
        ir::node_data<T> trans_vector(
            Data const&, View&& view, F const& f = F{}) const
        {
            return ir::node_data<T>{blaze::DynamicVector<T>{
                blaze::trans(std::forward<View>(view))}};
        }

        template <typename Data, typename View, typename F = always_true>
        ir::node_data<T> matrix(
            Data const&, View&& view, F const& f = F{}) const
        {
            return ir::node_data<T>{
                blaze::DynamicMatrix<T>{std::forward<View>(view)}};
        }

        template <typename Data, typename View, typename F = always_true>
        ir::node_data<T> tensor(
            Data const&, View&& view, F const& f = F{}) const
        {
            return ir::node_data<T>{
                blaze::DynamicTensor<T>{std::forward<View>(view)}};
        }
    };
}}}

#endif
