//  Copyright (c) 2017 Alireza Kheirkhahan
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/ir/node_data.hpp>
#include <phylanx/plugins/fileio/file_write_csv.hpp>

#include <hpx/include/lcos.hpp>
#include <hpx/include/naming.hpp>
#include <hpx/include/util.hpp>
#include <hpx/throw_exception.hpp>
#include <hpx/runtime/threads/run_as_os_thread.hpp>

#include <cstddef>
#include <fstream>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>
#include <utility>

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace execution_tree { namespace primitives
{
    ///////////////////////////////////////////////////////////////////////////
    match_pattern_type const file_write_csv::match_data =
    {
        hpx::util::make_tuple("file_write_csv",
            std::vector<std::string>{"file_write_csv(_1, _2)"},
            &create_file_write_csv, &create_primitive<file_write_csv>,
            R"(fname, m
            Args:

                fname (string): a file name
                m (array or matrix): an object to store in the file.

            Returns:

            The matrix written.)"
            )
    };

    ///////////////////////////////////////////////////////////////////////////
    file_write_csv::file_write_csv(
            primitive_arguments_type && operands,
            std::string const& name, std::string const& codename)
      : primitive_component_base(std::move(operands), name, codename)
    {
    }

    hpx::future<primitive_argument_type>  file_write_csv::write_to_file_csv(
        ir::node_data<double> && val, std::string && filename) const
    {
        auto this_ = this->shared_from_this();
        return hpx::threads::run_as_os_thread(
            [this_ = std::move(this_)](
                ir::node_data<double> && val, std::string && filename)
            -> primitive_argument_type
            {
                std::ofstream outfile(
                    filename.c_str(), std::ios::out | std::ios::trunc);
                if (!outfile.is_open())
                {
                    throw std::runtime_error(this_->generate_error_message(
                        "couldn't open file: " + filename));
                }

                outfile << std::setprecision(
                    std::numeric_limits<long double>::digits10 + 1);
                outfile << std::scientific;

                switch (val.num_dimensions())
                {
                case 0:
                    outfile << val.scalar() << '\n';
                    break;

                case 1:
                    {
                        auto v = val.vector();
                        for (std::size_t i = 0UL; i != v.size(); ++i)
                        {
                            if (i != 0)
                            {
                                outfile << ',';
                            }
                            outfile << v[i];
                        }
                        outfile << '\n';
                    }
                    break;

                case 2:
                    {
                        auto matrix = val.matrix();
                        for (std::size_t i = 0UL; i != matrix.rows(); ++i)
                        {
                            outfile << matrix(i, 0);
                            for (std::size_t j = 1UL; j != matrix.columns(); ++j)
                            {
                                outfile << ',' << matrix(i, j);
                            }
                            outfile << '\n';
                        }
                    }
                    break;
                }

                return primitive_argument_type{std::move(val)};
            },
            std::move(val), std::move(filename));
    }

    hpx::future<primitive_argument_type> file_write_csv::eval(
        primitive_arguments_type const& operands,
        primitive_arguments_type const& args, eval_context ctx) const
    {
        if (operands.size() != 2)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "phylanx::execution_tree::primitives::file_write::"
                    "file_write_csv",
                generate_error_message(
                    "the file_write primitive requires exactly two "
                        "operands"));
        }

        if (!valid(operands[0]) || !valid(operands[1]))
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "phylanx::execution_tree::primitives::file_write::"
                    "file_write_csv",
                generate_error_message("the file_write primitive "
                    "requires that the given operands are valid"));
        }

        std::string filename =
            string_operand_sync(operands[0], args, name_, codename_, ctx);

        auto this_ = this->shared_from_this();
        return numeric_operand(operands[1], args, name_, codename_, std::move(ctx))
            .then(hpx::launch::sync, hpx::util::unwrapping(
                [this_ = std::move(this_), filename = std::move(filename)](
                        ir::node_data<double> && val) mutable
                ->  hpx::future<primitive_argument_type>
                {
                    return this_->write_to_file_csv(
                        std::move(val), std::move(filename));
                }));
    }
}}}
