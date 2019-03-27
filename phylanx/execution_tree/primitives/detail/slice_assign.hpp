// Copyright (c) 2018 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_DETAIL_SLICE_ASSIGN_SEP_18_2018_1205PM)
#define PHYLANX_DETAIL_SLICE_ASSIGN_SEP_18_2018_1205PM

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/detail/slice_identity.hpp>
#include <phylanx/execution_tree/primitives/base_primitive.hpp>
#include <phylanx/ir/node_data.hpp>

#include <cstddef>
#include <sstream>
#include <string>
#include <utility>

#include <hpx/throw_exception.hpp>

#include <blaze/Math.h>

namespace phylanx { namespace execution_tree { namespace detail
{
    ///////////////////////////////////////////////////////////////////////////
    template <typename Lhs, typename Rhs>
    void check_vector_sizes(Lhs const& lhs, Rhs const& rhs)
    {
        if (lhs.size() != rhs.size())
        {
            std::ostringstream msg;
            msg << "size mismatch during vector assignment, sizes: "
                << lhs.size() << ", " << rhs.size();

            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "phylanx::execution_tree::detail::check_vector_sizes",
                msg.str());
        }
    }

    template <typename Lhs, typename Rhs>
    void check_matrix_sizes(Lhs const& lhs, Rhs const& rhs)
    {
        if (lhs.rows() != rhs.rows() || lhs.columns() != rhs.columns())
        {
            std::ostringstream msg;
            msg << "size mismatch during matrix assignment, "
                << "rows: " << lhs.rows() << ", " << rhs.rows()
                << "columns: " << lhs.columns() << ", " << rhs.columns();

            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "phylanx::execution_tree::detail::check_matrix_sizes",
                msg.str());
        }
    }

#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
    template <typename Lhs, typename Rhs>
    void check_tensor_sizes(Lhs const& lhs, Rhs const& rhs)
    {
        if (lhs.rows() != rhs.rows() || lhs.columns() != rhs.columns() ||
            lhs.pages() != rhs.pages())
        {
            std::ostringstream msg;
            msg << "size mismatch during tensor assignment, "
                << "rows: " << lhs.rows() << ", " << rhs.rows()
                << "columns: " << lhs.columns() << ", " << rhs.columns()
                << "pages: " << lhs.pages() << ", " << rhs.pages();

            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "phylanx::execution_tree::detail::check_tensor_sizes",
                msg.str());
        }
    }
#endif

    ///////////////////////////////////////////////////////////////////////////
    template <typename T>
    struct slice_assign_scalar
    {
        ir::node_data<T>& rhs_;

        template <typename Data, typename Scalar, typename F = always_true>
        ir::node_data<T> scalar(
            Data& data, Scalar& value, F const& f = F{}) const
        {
            if (f(0) != std::size_t(-1))
            {
                value = rhs_.scalar();
            }
            return ir::node_data<T>{std::move(data)};
        }

        template <typename Data, typename View, typename F = always_true>
        ir::node_data<T> vector(Data& data, View&& view, F const& f = F{}) const
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "phylanx::execution_tree::detail::slice_assign_scalar<T>",
                "cannot assign a scalar to a vector");
            return ir::node_data<T>{};
        }

        template <typename Data, typename View, typename F = always_true>
        ir::node_data<T> trans_vector(Data& data,
            View&& view, F const& f = F{}) const
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "phylanx::execution_tree::detail::slice_assign_scalar<T>",
                "cannot assign a scalar to a vector");
            return ir::node_data<T>{};
        }

        template <typename Data, typename View, typename F = always_true>
        ir::node_data<T> matrix(Data& data, View&& view, F const& f = F{}) const
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "phylanx::execution_tree::detail::slice_assign_scalar<T>",
                "cannot assign a scalar to a matrix");
            return ir::node_data<T>{};
        }
    };

    template <typename T>
    struct slice_assign_vector
    {
        ir::node_data<T>& rhs_;

        template <typename Data, typename Scalar, typename F = always_true>
        ir::node_data<T> scalar(Data& data, Scalar& value, F const& f = F{}) const
        {
            if (rhs_.size() != 1)
            {
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "phylanx::execution_tree::detail::slice_assign_vector<T>",
                    "cannot assign a vector to a scalar");
            }

            if (f(0) != std::size_t(-1))
            {
                value = rhs_.vector()[0];
            }
            return ir::node_data<T>{std::move(data)};
        }

        template <typename Data, typename View, typename F = always_true>
        ir::node_data<T> vector(Data& data, View&& view, F const& f = F{}) const
        {
            std::size_t size = view.size();
            auto v = rhs_.vector();

            for (std::size_t i = 0; i != size; ++i)
            {
                std::size_t idx = f(i);
                if (idx != std::size_t(-1))
                {
                    view[i] = v[idx];
                }
            }
            return ir::node_data<T>{std::move(data)};
        }

        template <typename Data, typename View, typename F = always_true>
        ir::node_data<T> trans_vector(
            Data& data, View&& view, F const& f = F{}) const
        {
            std::size_t size = view.size();
            auto v = rhs_.vector();
            auto tv = blaze::trans(v);

            for (std::size_t i = 0; i != size; ++i)
            {
                std::size_t idx = f(i);
                if (idx != std::size_t(-1))
                {
                    view[i] = tv[idx];
                }
            }
            return ir::node_data<T>{std::move(data)};
        }

        template <typename Data, typename View, typename F = always_true>
        ir::node_data<T> matrix(Data& data, View&& view, F const& f = F{}) const
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "phylanx::execution_tree::detail::slice_assign_vector<T>",
                "cannot assign a vector to a matrix");
            return ir::node_data<T>{std::move(data)};
        }
    };

    template <typename T>
    struct slice_assign_matrix
    {
        ir::node_data<T>& rhs_;

        template <typename Data, typename Scalar, typename F = always_true>
        ir::node_data<T> scalar(Data& data, Scalar& value, F const& f = F{}) const
        {
            auto m = rhs_.matrix();
            if (m.rows() != 1 || m.columns() != 1)
            {
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "phylanx::execution_tree::detail::slice_assign_matrix<T>",
                    "cannot assign a matrix to a scalar");
            }

            if (f(0) != std::size_t(-1))
            {
                value = m(0, 0);
            }
            return ir::node_data<T>{std::move(data)};
        }

        template <typename Data, typename View, typename F = always_true>
        ir::node_data<T> vector(Data& data, View&& view, F const& f = F{}) const
        {
            auto m = rhs_.matrix();
            if (m.columns() != 1)
            {
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "phylanx::execution_tree::detail::slice_assign_matrix<T>",
                    "cannot assign a matrix to a vector");
            }

            std::size_t size = view.size();
            auto v = blaze::column(m, 0);

            for (std::size_t i = 0; i != size; ++i)
            {
                std::size_t idx = f(i);
                if (idx != std::size_t(-1))
                {
                    view[i] = v[idx];
                }
            }
            return ir::node_data<T>{std::move(data)};
        }

        template <typename Data, typename View, typename F = always_true>
        ir::node_data<T> trans_vector(
            Data& data, View&& view, F const& f = F{}) const
        {
            auto m = rhs_.matrix();
            if (m.rows() != 1)
            {
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "phylanx::execution_tree::detail::slice_assign_matrix<T>",
                    "cannot assign a matrix to a vector");
            }

            std::size_t size = view.size();
            auto v = blaze::row(m, 0);
            auto tv = blaze::trans(v);

            for (std::size_t i = 0; i != size; ++i)
            {
                std::size_t idx = f(i);
                if (idx != std::size_t(-1))
                {
                    view[i] = tv[idx];
                }
            }
            return ir::node_data<T>{std::move(data)};
        }

        template <typename Data, typename View, typename F = always_true>
        ir::node_data<T> matrix(Data& data, View&& view, F const& f = F{}) const
        {
            auto m = rhs_.matrix();

            check_matrix_sizes(view, m);

            for (std::size_t i = 0; i != m.rows(); ++i)
            {
                for (std::size_t j = 0; j != m.columns(); ++j)
                {
                    std::size_t idx = f(i * m.columns() + j);
                    if (idx != std::size_t(-1))
                    {
                        view(i, j) = m(i, j);
                    }
                }
            }
            return ir::node_data<T>{std::move(data)};
        }

        template <typename Data, typename View, typename F = always_true>
        ir::node_data<T> tensor(Data& data, View&& view, F const& f = F{}) const
        {
            auto t = rhs_.tensor();

            check_tensor_sizes(view, t);

            for (std::size_t k = 0; k != t.pages(); ++k)
            {
                for (std::size_t i = 0; i != t.rows(); ++i)
                {
                    std::size_t page_idx = (k * t.pages() + i) * t.columns();
                    for (std::size_t j = 0; j != t.columns(); ++j)
                    {
                        std::size_t idx = f(page_idx + j);
                        if (idx != std::size_t(-1))
                        {
                            view(k, i, j) = t(k, i, j);
                        }
                    }
                }
            }
            return ir::node_data<T>{std::move(data)};
        }
    };
}}}

#endif
