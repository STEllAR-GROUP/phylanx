//   Copyright (c) 2017 Hartmut Kaiser
//
//   Distributed under the Boost Software License, Version 1.0. (See accompanying
//   file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/phylanx.hpp>
#include <hpx/hpx_main.hpp>

#include <fstream>
#include <iostream>
#include <iterator>

struct traverse_ast : phylanx::ast::static_visitor
{
    traverse_ast(std::stringstream& strm)
      : strm_(strm)
    {
    }

    template <typename T>
    bool operator()(T const& val) const
    {
        strm_ << val << '\n';
        return true;
    }

    std::stringstream& strm_;
};

void generate(std::istream& in, bool print_source_code)
{
    std::string source_code;
    in.unsetf(std::ios::skipws);    // disable white space skipping
    std::copy(
        std::istream_iterator<char>(in),
        std::istream_iterator<char>(),
        std::back_inserter(source_code));

    try {
        phylanx::ast::expression ast = phylanx::ast::generate_ast(source_code);

        std::stringstream strm;
        phylanx::ast::traverse(ast, traverse_ast{strm});

        if (print_source_code)
            std::cout << "source code:\n" << source_code << std::endl;
        std::cout << "generated ast:\n" << strm.str() << std::endl;
    }
    catch (hpx::exception const& e) {
        if (print_source_code)
            std::cout << "source code:\n" << source_code << std::endl;
        std::cout << "generated error:\n" << hpx::get_error_what(e) << std::endl;
    }
}

int main(int argc, char* argv[])
{
    if (argc > 1)
    {
        std::ifstream in(argv[1], std::ios_base::in);
        if (!in)
        {
            std::cerr
                << "Error: Could not open input file: "
                << argv[1] << std::endl;
            return -1;
        }
        generate(in, true);
    }
    else
    {
        generate(std::cin, false);
    }
    return 0;
}

