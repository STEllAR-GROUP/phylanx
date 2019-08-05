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
    GENERIC_OPERATION_3D_INSTANTIATION(log, double);
    GENERIC_OPERATION_3D_INSTANTIATION(log2, double);
    GENERIC_OPERATION_3D_INSTANTIATION(log10, double);
    GENERIC_OPERATION_3D_INSTANTIATION(sin, double);
    GENERIC_OPERATION_3D_INSTANTIATION(cos, double);
    GENERIC_OPERATION_3D_INSTANTIATION(tan, double);
    GENERIC_OPERATION_3D_INSTANTIATION(sinh, double);
    GENERIC_OPERATION_3D_INSTANTIATION(cosh, double);
    GENERIC_OPERATION_3D_INSTANTIATION(tanh, double);
    GENERIC_OPERATION_3D_INSTANTIATION(asin, double);
    GENERIC_OPERATION_3D_INSTANTIATION(acos, double);
    GENERIC_OPERATION_3D_INSTANTIATION(atan, double);
    GENERIC_OPERATION_3D_INSTANTIATION(asinh, double);
    GENERIC_OPERATION_3D_INSTANTIATION(acosh, double);
    GENERIC_OPERATION_3D_INSTANTIATION(atanh, double);
    GENERIC_OPERATION_3D_INSTANTIATION(erf, double);
    GENERIC_OPERATION_3D_INSTANTIATION(erfc, double);
    GENERIC_OPERATION_3D_INSTANTIATION(square, double);
    GENERIC_OPERATION_3D_INSTANTIATION(sign, double);
}}}

