//  Copyright (c) 2018 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/phylanx.hpp>

#include <hpx/hpx_main.hpp>
#include <hpx/runtime/naming_fwd.hpp>
#include <hpx/modules/testing.hpp>

#include <string>

namespace pec = phylanx::execution_tree::compiler;

void test_primitive_name(pec::primitive_name_parts const& expected_parts,
    std::string const& expected_name)
{
    std::string name = pec::compose_primitive_name(expected_parts);
    HPX_TEST_EQ(name, expected_name);

    pec::primitive_name_parts parts = pec::parse_primitive_name(name);
    HPX_TEST(parts == expected_parts);
}

int main(int argc, char* argv[])
{
    pec::primitive_name_parts parts;
    parts.locality = hpx::naming::invalid_locality_id;
    parts.primitive = "add";
    parts.sequence_number = 1;
    parts.instance = "";
    parts.compile_id = 2;
    parts.tag1 = 3;
    parts.tag2 = -1;

    test_primitive_name(parts, "/phylanx/add$1/2$3");

    parts.locality = 0;
    parts.instance = "test";
    test_primitive_name(parts, "/phylanx$0/add$1$test/2$3");

    parts.locality = 1;
    parts.tag2 = 4;
    test_primitive_name(parts, "/phylanx$1/add$1$test/2$3$4");

    parts.locality = hpx::naming::invalid_locality_id;
    parts.instance = "";
    test_primitive_name(parts, "/phylanx/add$1/2$3$4");

    return hpx::util::report_errors();
}
