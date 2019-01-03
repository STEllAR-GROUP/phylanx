//  Copyright (c) 2017-2018 Hartmut Kaiser
//  Copyright (c) 2018 R. Tohid
//  Copyright (c) 2018 Steven R. Brandt
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/include/version.hpp>

#include <init_hpx.hpp>
#include <bindings/binding_helpers.hpp>

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include <hpx/util/detail/pp/stringize.hpp>

///////////////////////////////////////////////////////////////////////////////
#if defined(_DEBUG)
PYBIND11_MODULE(_phylanxd, m)
#else
PYBIND11_MODULE(_phylanx, m)
#endif
{
    m.doc() = "Phylanx plugin module";

    m.attr("__version__") = pybind11::str(
        HPX_PP_STRINGIZE(PHYLANX_VERSION_MAJOR) "."
        HPX_PP_STRINGIZE(PHYLANX_VERSION_MINOR) "."
        HPX_PP_STRINGIZE(PHYLANX_VERSION_SUBMINOR) "."
        PHYLANX_HAVE_GIT_COMMIT);

    ///////////////////////////////////////////////////////////////////////////
    // expose version functions
    m.def("major_version", &phylanx::major_version);
    m.def("minor_version", &phylanx::minor_version);
    m.def("subminor_version", &phylanx::subminor_version);
    m.def("full_version", &phylanx::full_version);
    m.def("full_version_as_string", &phylanx::full_version_as_string);

    m.def("init_hpx_runtime", &phylanx::bindings::init_hpx_runtime);
    m.def("stop_hpx_runtime", &phylanx::bindings::stop_hpx_runtime);

    ///////////////////////////////////////////////////////////////////////////
    // expose the other modules
    phylanx::bindings::bind_ast(m);
    phylanx::bindings::bind_execution_tree(m);
    phylanx::bindings::bind_util(m);

    ///////////////////////////////////////////////////////////////////////////
    // make sure HPX is unloaded at module unload

    // Register a callback function that is invoked when the BaseClass object
    // is collected
    pybind11::cpp_function cleanup_callback(
        [](pybind11::handle weakref)
        {
            phylanx::bindings::stop_hpx_runtime();
            weakref.dec_ref();    // release weak reference
        });

    // Create a weak reference with a cleanup callback and initially leak it
    (void) pybind11::weakref(m, cleanup_callback).release();
}
