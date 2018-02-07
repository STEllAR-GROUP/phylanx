//  Copyright (c) 2018 Alireza Kheirkhahan
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/file_read_hdf5.hpp>
#include <phylanx/ir/node_data.hpp>

#include <hpx/include/components.hpp>
#include <hpx/include/lcos.hpp>

#include <highfive/H5File.hpp>
#include <highfive/H5DataSet.hpp>
#include <highfive/H5DataSpace.hpp>

#include <cstddef>
#include <fstream>
#include <string>
#include <utility>
#include <vector>

///////////////////////////////////////////////////////////////////////////////
typedef hpx::components::component<
phylanx::execution_tree::primitives::file_read_hdf5>
        file_read_hdf5_type;
HPX_REGISTER_DERIVED_COMPONENT_FACTORY(file_read_hdf5_type,
        phylanx_file_read_hdf5_component, "phylanx_primitive_component",
hpx::components::factory_enabled)
HPX_DEFINE_GET_COMPONENT_TYPE(file_read_hdf5_type::wrapped_type)

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace execution_tree { namespace primitives
{
    ///////////////////////////////////////////////////////////////////////////
    match_pattern_type const file_read_hdf5::match_data = {
        hpx::util::make_tuple("file_read_hdf5",
            std::vector<std::string>{"file_read_hdf5(_1, _2)"},
            &create<file_read_hdf5>)};

    ///////////////////////////////////////////////////////////////////////////
    file_read_hdf5::file_read_hdf5(
        std::vector<primitive_argument_type> && operands)
      : base_primitive(std::move(operands))
    {
    }

    // read data from given file and return content
    hpx::future<primitive_result_type> file_read_hdf5::eval(
        std::vector<primitive_argument_type> const& args) const
    {
        if (operands_.size() != 2)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "phylanx::execution_tree::primitives::file_read_hdf5::"
                "eval",
                "the file_read_hdf5 primitive requires exactly twe literal "
                "arguments");
        }

        if (!valid(operands_[0]) || !valid(operands_[1]))
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "phylanx::execution_tree::primitives::file_read_hdf5::"
                "eval",
                "the file_read_hdf5 primitive requires that the given operand "
                "is valid");
        }

        std::string filename = string_operand_sync(operands_[0], args);
        std::string datasetName = string_operand_sync(operands_[1], args);

        HighFive::File infile(filename, HighFive::File::ReadOnly);
        HighFive::DataSet dataSet = infile.getDataSet(datasetName);
        HighFive::DataSpace dataSpace = dataSet.getSpace();
        std::vector<std::size_t> dims = dataSpace.getDimensions();

        switch(dims.size())
        {
            case 0: {
                // scalar value
                double scalar;
                dataSet.read(scalar);
                return hpx::make_ready_future(primitive_result_type{
                        ir::node_data<double>{scalar}});
            }
            case 1: {
                // vector
                blaze::DynamicVector<double> vector(dims[0]);
                dataSet.read(vector);
                return hpx::make_ready_future(primitive_result_type{
                        ir::node_data<double>{std::move(vector)}});
            }
            case 2: {

                // matrix
                blaze::DynamicMatrix<double> matrix(
                        dims[0], dims[1]);
                dataSet.read(matrix);
                return hpx::make_ready_future(
                        primitive_result_type{ir::node_data<double>{std::move(matrix)}});
            }
            default:
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                                    "phylanx::execution_tree::primitives::file_read_hdf5::"
                                            "eval",
                                    "the input file has incompatible number of dimensions");
        }

    }
}}}
