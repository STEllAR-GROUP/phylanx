//  Copyright (c) 2017 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/phylanx.hpp>

#include <hpx/hpx_main.hpp>
#include <hpx/modules/testing.hpp>

#include <cstdint>

int main(int argc, char* argv[])
{
    HPX_TEST_EQ(
        std::uint8_t(PHYLANX_VERSION_MAJOR),
        phylanx::major_version());
    HPX_TEST_EQ(
        std::uint8_t(PHYLANX_VERSION_MINOR),
        phylanx::minor_version());
    HPX_TEST_EQ(
        std::uint8_t(PHYLANX_VERSION_SUBMINOR),
        phylanx::subminor_version());

    return hpx::util::report_errors();
}
