//  Copyright (c) 2018 Parsa Amini
//  Copyright (c) 2018 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/phylanx.hpp>

#include <hpx/hpx_main.hpp>
#include <hpx/include/agas.hpp>
#include <hpx/util/lightweight_test.hpp>

#include <cstddef>
#include <map>
#include <string>
#include <vector>

#include <blaze/Math.h>

///////////////////////////////////////////////////////////////////////////////
char const* const fib_code = R"(block(
    define(fib_test,
        block(
            define(x, 1.0),
            define(z, 0.0),
            define(y, 1.0),
            define(temp, 0.0),
            define(step, 2),
            while(
                step < 10,
                block(
                    store(z, x + y),
                    store(temp, y),
                    store(y, z),
                    store(x, temp),
                    store(step, step + 1)
                )
            ),
            z
        )
    ),
    fib_test
))";

std::map<std::string, std::size_t> expected_counts =
{
    { "/phylanx/__add$0/0$12$30", 8 },
    { "/phylanx/__add$1/0$16$33", 8 },
    { "/phylanx/__lt$0/0$10$17", 9 },
    { "/phylanx/access-variable$0$step/0$10$17", 9 },
    { "/phylanx/access-variable$1$z/0$12$27", 0 },
    { "/phylanx/access-variable$10$step/0$16$27", 0 },
    { "/phylanx/access-variable$11$step/0$16$33", 8 },
    { "/phylanx/access-variable$12$z/0$19$13", 1 },
    { "/phylanx/access-function$0$fib_test/0$22$5", 1 },
    { "/phylanx/access-variable$2$x/0$12$30", 8 },
    { "/phylanx/access-variable$3$y/0$12$34", 8 },
    { "/phylanx/access-variable$4$temp/0$13$27", 0 },
    { "/phylanx/access-variable$5$y/0$13$33", 8 },
    { "/phylanx/access-variable$6$y/0$14$27", 0 },
    { "/phylanx/access-variable$7$z/0$14$30", 8 },
    { "/phylanx/access-variable$8$x/0$15$27", 0 },
    { "/phylanx/access-variable$9$temp/0$15$30", 8 },
    { "/phylanx/block$0/0$1$1", 1 },
    { "/phylanx/block$1/0$3$9", 1 },
    { "/phylanx/block$2/0$11$17", 8 },
    { "/phylanx/define-variable$0$x/0$4$20", 1 },
    { "/phylanx/define-variable$1$z/0$5$20", 1 },
    { "/phylanx/define-variable$2$y/0$6$20", 1 },
    { "/phylanx/define-variable$3$temp/0$7$20", 1 },
    { "/phylanx/define-variable$4$step/0$8$20", 1 },
    { "/phylanx/define-variable$5$fib_test/0$2$12", 1 },
    { "/phylanx/store$0/0$12$21", 8 },
    { "/phylanx/store$1/0$13$21", 8 },
    { "/phylanx/store$2/0$14$21", 8 },
    { "/phylanx/store$3/0$15$21", 8 },
    { "/phylanx/store$4/0$16$21", 8 },
    { "/phylanx/variable$0$x/0$4$20", 1 },
    { "/phylanx/variable$1$z/0$5$20", 1 },
    { "/phylanx/variable$2$y/0$6$20", 1 },
    { "/phylanx/variable$3$temp/0$7$20", 1 },
    { "/phylanx/variable$4$step/0$8$20", 1 },
    { "/phylanx/function$0$fib_test/0$2$12", 1 },
    { "/phylanx/while$0/0$9$13", 1 },
};

const std::vector<std::string> performance_counter_name_last_part{
    "count/eval", "time/eval", "eval_direct"
};

int main()
{
    // Compile the given code
    phylanx::execution_tree::compiler::function_list snippets;

    auto const& code = phylanx::execution_tree::compile(
        phylanx::ast::generate_ast(fib_code), snippets);

    // enable collection of performance data, return list of existing primitives
    std::vector<std::string> existing_primitive_instances =
        phylanx::util::enable_measurements();

    auto const fibonacci = code.run();

    // Evaluate Fibonacci using the read data
    auto const result = fibonacci();

    for (auto const& entry :
        phylanx::util::retrieve_counter_data(existing_primitive_instances,
            performance_counter_name_last_part))
    {
        auto expected_value = expected_counts[entry.first];

        HPX_TEST_EQ(
            entry.second.size(), performance_counter_name_last_part.size());
        HPX_TEST_EQ(entry.second[0], expected_value);
        HPX_TEST_EQ(entry.second[1] != 0, expected_value != 0);
        HPX_TEST(entry.second[2] == -1 || entry.second[2] == 0 ||
            entry.second[2] == 1);
    }

    return hpx::util::report_errors();
}
