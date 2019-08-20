// Copyright (c) 2018 Tianyi Zhang
// Copyright (c) 2018 Bita Hasheminezhad
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef PHYLANX_GENERIC_OPERATION_3D_DEFINITIONS_HPP
#define PHYLANX_GENERIC_OPERATION_3D_DEFINITIONS_HPP

#include <phylanx/config.hpp>

#include <phylanx/execution_tree/compiler/primitive_name.hpp>
#include <phylanx/execution_tree/primitives/node_data_helpers.hpp>
#include <phylanx/ir/node_data.hpp>
#include <phylanx/plugins/arithmetics/generic_operation.hpp>
#include <phylanx/plugins/arithmetics/generic_operation_3d.hpp>

#include <hpx/assertion.hpp>
#include <hpx/errors/throw_exception.hpp>

#include <cmath>
#include <cstddef>
#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include <blaze/Math.h>
#include <blaze_tensor/Math.h>

////////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace execution_tree { namespace primitives
{
    GENERIC_OPERATION_3D(abs);
    GENERIC_OPERATION_3D(floor);
    GENERIC_OPERATION_3D(ceil);
    GENERIC_OPERATION_3D(trunc);
    GENERIC_OPERATION_3D(round);
    GENERIC_OPERATION_3D(conj);
    GENERIC_OPERATION_3D(real);
    GENERIC_OPERATION_3D(imag);
    GENERIC_OPERATION_3D(sqrt);
    GENERIC_OPERATION_3D(invsqrt);
    GENERIC_OPERATION_3D(cbrt);
    GENERIC_OPERATION_3D(invcbrt);
    GENERIC_OPERATION_3D(exp);
    GENERIC_OPERATION_3D(exp2);
    GENERIC_OPERATION_3D(exp10);
    GENERIC_OPERATION_3D(log);
    GENERIC_OPERATION_3D(log2);
    GENERIC_OPERATION_3D(log10);
    GENERIC_OPERATION_3D(sin);
    GENERIC_OPERATION_3D(cos);
    GENERIC_OPERATION_3D(tan);
    GENERIC_OPERATION_3D(sinh);
    GENERIC_OPERATION_3D(cosh);
    GENERIC_OPERATION_3D(tanh);
    GENERIC_OPERATION_3D(asin);
    GENERIC_OPERATION_3D(acos);
    GENERIC_OPERATION_3D(atan);
    GENERIC_OPERATION_3D(asinh);
    GENERIC_OPERATION_3D(erf);
    GENERIC_OPERATION_3D(erfc);
    GENERIC_OPERATION_3D(sign);

#undef GENERIC_OPERATION_3D

    ////////////////////////////////////////////////////////////////////////////
    template <typename T>
    ir::node_data<T> acosh_3d(ir::node_data<T>&& t)
    {
#if defined(PHYLANX_DEBUG)
        for (size_t k = 0UL; k < t.tensor().pages(); ++k)
        {
            for (size_t i = 0UL; i < t.tensor().rows(); ++i)
            {
                for (size_t j = 0UL; j < t.tensor().columns(); ++j)
                {
                    if (t.tensor()(k, i, j) < 1)
                    {
                        HPX_THROW_EXCEPTION(hpx::bad_parameter,
                            "arccosh", "tensor arccosh: domain error");
                    }
                }
            }
        }
#endif
        if (t.is_ref())
        {
            t = blaze::acosh(t.tensor());
        }
        else
        {
            t.tensor() = blaze::acosh(t.tensor());
        }
        return ir::node_data<T>(std::move(t));
    }

    template <typename T>
    ir::node_data<T> atanh_3d(ir::node_data<T>&& t)
    {
#if defined(PHYLANX_DEBUG)
        for (size_t k = 0UL; k < t.tensor().pages(); ++k)
        {
            for (size_t i = 0UL; i < t.tensor().rows(); ++i)
            {
                for (size_t j = 0UL; j < t.tensor().columns(); ++j)
                {
                    if (t.tensor()(k, i, j) <= -1 ||
                        t.tensor()(k, i, j) >= 1)
                    {
                        HPX_THROW_EXCEPTION(hpx::bad_parameter,
                            "arctanh", "tensor arctanh: domain error");
                    }
                }
            }
        }
#endif
        if (t.is_ref())
        {
            t = blaze::atanh(t.tensor());
        }
        else
        {
            t.tensor() = blaze::atanh(t.tensor());
        }
        return ir::node_data<T>(std::move(t));
    }

    template <typename T>
    ir::node_data<T> square_3d(ir::node_data<T>&& t)
    {
        if (t.is_ref())
        {
            t = t.tensor() % t.tensor();
        }
        else
        {
            t.tensor() %= t.tensor();
        }
        return ir::node_data<T>(std::move(t));
    }
}}}

#endif
