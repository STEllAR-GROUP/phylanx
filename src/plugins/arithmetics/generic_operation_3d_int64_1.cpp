// Copyright (c) 2018 Tianyi Zhang
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>

#include <phylanx/plugins/arithmetics/generic_operation.hpp>
#include <phylanx/plugins/arithmetics/generic_operation_3d.hpp>
#include <phylanx/plugins/arithmetics/generic_operation_3d_definitions.hpp>

#include <cstdint>

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace execution_tree { namespace primitives
{
    GENERIC_OPERATION_3D_INSTANTIATION(abs, std::int64_t);
    GENERIC_OPERATION_3D_INSTANTIATION(floor, std::int64_t);
    GENERIC_OPERATION_3D_INSTANTIATION(ceil, std::int64_t);
    GENERIC_OPERATION_3D_INSTANTIATION(trunc, std::int64_t);
    GENERIC_OPERATION_3D_INSTANTIATION(round, std::int64_t);
    GENERIC_OPERATION_3D_INSTANTIATION(conj, std::int64_t);
    GENERIC_OPERATION_3D_INSTANTIATION(real, std::int64_t);
    GENERIC_OPERATION_3D_INSTANTIATION(imag, std::int64_t);
    GENERIC_OPERATION_3D_INSTANTIATION(sqrt, std::int64_t);
    GENERIC_OPERATION_3D_INSTANTIATION(invsqrt, std::int64_t);
    GENERIC_OPERATION_3D_INSTANTIATION(cbrt, std::int64_t);
    GENERIC_OPERATION_3D_INSTANTIATION(invcbrt, std::int64_t);
    GENERIC_OPERATION_3D_INSTANTIATION(exp, std::int64_t);
    GENERIC_OPERATION_3D_INSTANTIATION(exp2, std::int64_t);
    GENERIC_OPERATION_3D_INSTANTIATION(exp10, std::int64_t);
}}}

