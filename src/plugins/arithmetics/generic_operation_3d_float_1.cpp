// Copyright (c) 2018 Tianyi Zhang
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>

#include <phylanx/plugins/arithmetics/generic_operation.hpp>
#include <phylanx/plugins/arithmetics/generic_operation_3d.hpp>

#include <string>

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace execution_tree { namespace primitives
{
    GENERIC_OPERATION_3D_INSTANTIATION(abs, double);
    GENERIC_OPERATION_3D_INSTANTIATION(floor, double);
    GENERIC_OPERATION_3D_INSTANTIATION(ceil, double);
    GENERIC_OPERATION_3D_INSTANTIATION(trunc, double);
    GENERIC_OPERATION_3D_INSTANTIATION(round, double);
    GENERIC_OPERATION_3D_INSTANTIATION(conj, double);
    GENERIC_OPERATION_3D_INSTANTIATION(real, double);
    GENERIC_OPERATION_3D_INSTANTIATION(imag, double);
    GENERIC_OPERATION_3D_INSTANTIATION(sqrt, double);
    GENERIC_OPERATION_3D_INSTANTIATION(invsqrt, double);
    GENERIC_OPERATION_3D_INSTANTIATION(cbrt, double);
    GENERIC_OPERATION_3D_INSTANTIATION(invcbrt, double);
    GENERIC_OPERATION_3D_INSTANTIATION(exp, double);
    GENERIC_OPERATION_3D_INSTANTIATION(exp2, double);
    GENERIC_OPERATION_3D_INSTANTIATION(exp10, double);
}}}

