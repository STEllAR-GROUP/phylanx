//  Copyright (c) 2017 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/phylanx.hpp>
#include <pybind11/pybind11.h>

#include <hpx/util/detail/pp/stringize.hpp>

#if defined(_DEBUG)
#define PHYLANX_MODULE_NAME phylanxd
#else
#define PHYLANX_MODULE_NAME phylanx
#endif

PYBIND11_PLUGIN(PHYLANX_MODULE_NAME)
{
    pybind11::module m(
        HPX_PP_STRINGIZE(PHYLANX_MODULE_NAME),
        "Phylanx plugin module");

    // expose version functions
    m.def("major_version", &phylanx::major_version);
    m.def("minor_version", &phylanx::minor_version);
    m.def("subminor_version", &phylanx::subminor_version);
    m.def("full_version", &phylanx::full_version);
    m.def("full_version_as_string", &phylanx::full_version_as_string);

    return m.ptr();
}
