//  Copyright (c) 2018 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/phylanx.hpp>

#include <hpx/hpx_main.hpp>

#include <iostream>

int main(int argc, char* argv[])
{
    std::cout << "PHYLANX_VERSION_MAJOR:    " << phylanx::major_version()
              << "\n";
    std::cout << "PHYLANX_VERSION_MINOR:    " << phylanx::minor_version()
              << "\n";
    std::cout << "PHYLANX_VERSION_SUBMINOR: " << phylanx::subminor_version()
              << "\n";

    return 0;
}
