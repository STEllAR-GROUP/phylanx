//  Copyright (c) 2015 Anton Bikineev
//  Copyright (c) 2017 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/util/serialization/optional.hpp>

#include <hpx/hpx_main.hpp>
#include <hpx/include/serialization.hpp>
#include <hpx/util/lightweight_test.hpp>

#include <string>
#include <vector>

template <typename T>
struct A
{
    A() {}

    A(T t) : t_(t) {}
    T t_;

    A & operator=(T t) { t_ = t; return *this; }

    friend bool operator==(A a, A b)
    {
        return a.t_ == b.t_;
    }

    friend std::ostream& operator<<(std::ostream& os, A a)
    {
        os << a.t_;
        return os;
    }

    template <typename Archive>
    void serialize(Archive & ar, unsigned)
    {
        ar & t_;
    }
};

template <typename T>
void test_serialization(T value)
{
    std::vector<char> buf;
    hpx::serialization::output_archive oar(buf);
    hpx::serialization::input_archive iar(buf);

    phylanx::util::optional<T> oo = value;
    phylanx::util::optional<T> io;
    oar << oo;
    iar >> io;

    HPX_TEST(io.has_value());
    HPX_TEST(*io == *oo);
    HPX_TEST(*io == value);
}

int main()
{
    {
        std::vector<char> buf;
        hpx::serialization::output_archive oar(buf);
        hpx::serialization::input_archive iar(buf);

        phylanx::util::optional<std::string> oo;
        phylanx::util::optional<std::string> io;
        oar << oo;
        iar >> io;

        HPX_TEST(!io.has_value());
    }

    test_serialization(std::string("dfsdf"));
    test_serialization(2.5);
    test_serialization(42);
    test_serialization(A<int>(2));

    return hpx::util::report_errors();
}
