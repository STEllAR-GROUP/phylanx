// Copyright (c) 2018 Tianyi Zhang
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>

#include <phylanx/plugins/arithmetics/generic_operation.hpp>
#include <phylanx/plugins/arithmetics/generic_operation_3d.hpp>

#include <cstdint>
#include <string>

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace execution_tree { namespace primitives
{
    GENERIC_OPERATION_3D_INSTANTIATION(log, std::int64_t);
    GENERIC_OPERATION_3D_INSTANTIATION(log2, std::int64_t);
    GENERIC_OPERATION_3D_INSTANTIATION(log10, std::int64_t);
    GENERIC_OPERATION_3D_INSTANTIATION(sin, std::int64_t);
    GENERIC_OPERATION_3D_INSTANTIATION(cos, std::int64_t);
    GENERIC_OPERATION_3D_INSTANTIATION(tan, std::int64_t);
    GENERIC_OPERATION_3D_INSTANTIATION(sinh, std::int64_t);
    GENERIC_OPERATION_3D_INSTANTIATION(cosh, std::int64_t);
    GENERIC_OPERATION_3D_INSTANTIATION(tanh, std::int64_t);
    GENERIC_OPERATION_3D_INSTANTIATION(asin, std::int64_t);
    GENERIC_OPERATION_3D_INSTANTIATION(acos, std::int64_t);
    GENERIC_OPERATION_3D_INSTANTIATION(atan, std::int64_t);
    GENERIC_OPERATION_3D_INSTANTIATION(asinh, std::int64_t);
    GENERIC_OPERATION_3D_INSTANTIATION(acosh, std::int64_t);
    GENERIC_OPERATION_3D_INSTANTIATION(atanh, std::int64_t);
    GENERIC_OPERATION_3D_INSTANTIATION(erf, std::int64_t);
    GENERIC_OPERATION_3D_INSTANTIATION(erfc, std::int64_t);
    GENERIC_OPERATION_3D_INSTANTIATION(square, std::int64_t);
    GENERIC_OPERATION_3D_INSTANTIATION(sign, std::int64_t);
}}}

