//  Copyright (c) 2017 Alireza Kheirkhahan
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/file_write_csv.hpp>
#include <phylanx/ir/node_data.hpp>
#include <phylanx/util/serialization/ast.hpp>
#include <phylanx/util/serialization/execution_tree.hpp>
#include <phylanx/util/variant.hpp>

#include <hpx/include/components.hpp>
#include <hpx/include/lcos.hpp>
#include <hpx/include/util.hpp>

#include <cstddef>
#include <fstream>
#include <string>
#include <vector>

///////////////////////////////////////////////////////////////////////////////
typedef hpx::components::component<
    phylanx::execution_tree::primitives::file_write_csv>
    file_write_csv_type;
HPX_REGISTER_DERIVED_COMPONENT_FACTORY(file_write_csv_type,
    phylanx_file_write_csv_component, "phylanx_primitive_component",
    hpx::components::factory_enabled)
HPX_DEFINE_GET_COMPONENT_TYPE(file_write_csv_type::wrapped_type)

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace execution_tree { namespace primitives
{
    ///////////////////////////////////////////////////////////////////////////
    std::vector<match_pattern_type> const file_write_csv::match_data = {
        hpx::util::make_tuple("file_write_csv", "file_write_csv(_1, _2)",
            &create<file_write_csv>)};

    ///////////////////////////////////////////////////////////////////////////
    file_write_csv::file_write_csv(
        std::vector<primitive_argument_type> && operands)
    {
        if (operands.size() != 2)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "phylanx::execution_tree::primitives::file_write_csv::file_"
                "write_csv",
                "the file_write_csv primitive requires exactly two operands");
        }

        if (!valid(operands[0]) || !valid(operands[1]))
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "phylanx::execution_tree::primitives::file_write_csv::file_"
                "write_csv",
                "the file_write_csv primitive requires that the given operands "
                "are valid");
        }

        std::string* name = util::get_if<std::string>(&operands[0]);
        if (name == nullptr)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "phylanx::execution_tree::primitives::file_write_csv::file_"
                "write_csv",
                "the first literal argument must be a string representing a "
                "valid file name");
        }

        filename_ = std::move(*name);
        operand_ = std::move(operands[1]);
    }

    void write_to_file_csv(
        std::string const& filename, ir::node_data<double> const& val)
    {
        std::ofstream outfile(filename.c_str(), std::ios::out | std::ios::trunc);
        if (!outfile.is_open())
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "phylanx::execution_tree::primitives::file_write_csv::eval",
                "couldn't open file: " + filename);
        }
        outfile << std::setprecision(std::numeric_limits<long double>::digits10 + 1);
        outfile << std::scientific;
        const blaze::DynamicMatrix<double> & matrix = val.matrix();
        for (size_t i = 0UL; i < matrix.rows(); ++i) {
            outfile << matrix(i, 0);
            for (size_t j = 1UL; j < matrix.columns(); ++j) {
                outfile << ',' << matrix(i, j);
            }
            outfile << std::endl;
        }
    }

    // read data from given file and return content
    hpx::future<primitive_result_type> file_write_csv::eval(
        std::vector<primitive_argument_type> const& args) const
    {
        return numeric_operand(operand_, args)
            .then(hpx::util::unwrapping(
                [this](ir::node_data<double>&& val) -> primitive_result_type {
                    if (!valid(val))
                    {
                        HPX_THROW_EXCEPTION(hpx::bad_parameter,
                            "file_write_csv::eval",
                            "the file_write_csv primitive requires that the "
                            "argument"
                            " value given by the operand is non-empty");
                    }

                    write_to_file_csv(filename_, val);
                    return primitive_result_type(std::move(val));
                }));
    }
}}}
