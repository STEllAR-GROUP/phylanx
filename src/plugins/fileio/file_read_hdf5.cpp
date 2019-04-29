//  Copyright (c) 2018 Alireza Kheirkhahan
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>

#if defined(PHYLANX_HAVE_HIGHFIVE)
#include <phylanx/ir/node_data.hpp>
#include <phylanx/plugins/fileio/file_read_hdf5.hpp>

#include <hpx/include/lcos.hpp>
#include <hpx/include/naming.hpp>
#include <hpx/include/util.hpp>
#include <hpx/throw_exception.hpp>

#include <phylanx/util/detail/blaze-highfive.hpp>
#include <highfive/H5DataSet.hpp>
#include <highfive/H5DataSpace.hpp>
#include <highfive/H5File.hpp>

#include <cstddef>
#include <fstream>
#include <string>
#include <utility>
#include <vector>

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace execution_tree { namespace primitives
{
    ///////////////////////////////////////////////////////////////////////////
    match_pattern_type const file_read_hdf5::match_data =
    {
        hpx::util::make_tuple("file_read_hdf5",
            std::vector<std::string>{"file_read_hdf5(_1, _2)"},
            &create_file_read_hdf5, &create_primitive<file_read_hdf5>,
            R"(fname,dsetname
            Args:

                fname (string) : a file name
                dsetname (string) : a dataset name

            Returns:

            The dataset, either a matrix or vector.)"
            )
    };

    ///////////////////////////////////////////////////////////////////////////
    file_read_hdf5::file_read_hdf5(
            primitive_arguments_type && operands,
            std::string const& name, std::string const& codename)
      : primitive_component_base(std::move(operands), name, codename)
    {
    }

    // read data from given file and return content
    hpx::future<primitive_argument_type> file_read_hdf5::eval(
        primitive_arguments_type const& operands,
        primitive_arguments_type const& args, eval_context ctx) const
    {
        if (operands.size() != 2)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "phylanx::execution_tree::primitives::file_read_hdf5::eval",
                generate_error_message(
                    "the file_read_hdf5 primitive requires exactly two "
                        "literal arguments"));
        }

        if (!valid(operands[0]) || !valid(operands[1]))
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "phylanx::execution_tree::primitives::file_read_hdf5::eval",
                generate_error_message(
                    "the file_read_hdf5 primitive requires that the given "
                        "operand is valid"));
        }

        std::string filename =
            string_operand_sync(operands[0], args, name_, codename_, ctx);
        std::string datasetName = string_operand_sync(
            operands[1], args, name_, codename_, std::move(ctx));

        HighFive::File infile(filename, HighFive::File::ReadOnly);
        HighFive::DataSet dataSet = infile.getDataSet(datasetName);
        HighFive::DataSpace dataSpace = dataSet.getSpace();

        switch (dataSpace.getNumberDimensions())
        {
        case 0:
            {
                // scalar value
                double scalar;
                dataSet.read(scalar);
                return hpx::make_ready_future(
                    primitive_argument_type{ir::node_data<double>{scalar}});
            }

        case 1:
            {
                // vector
                std::vector<std::size_t> dims = dataSpace.getDimensions();
                blaze::DynamicVector<double> vector(dims[0]);
                dataSet.read(vector);
                return hpx::make_ready_future(primitive_argument_type{
                    ir::node_data<double>{std::move(vector)}});
            }

        case 2:
            {
                // matrix
                std::vector<std::size_t> dims = dataSpace.getDimensions();
                blaze::DynamicMatrix<double> matrix(dims[0], dims[1]);
                dataSet.read(matrix);
                return hpx::make_ready_future(primitive_argument_type{
                    ir::node_data<double>{std::move(matrix)}});
            }

        default:
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "phylanx::execution_tree::primitives::file_read_hdf5::eval",
                generate_error_message(
                    "the input file has incompatible number of dimensions"));
        }
    }
}}}

#endif
