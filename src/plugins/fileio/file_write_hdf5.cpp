//  Copyright (c) 2018 Alireza Kheirkhahan
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>

#if defined(PHYLANX_HAVE_HIGHFIVE)
#include <phylanx/ir/node_data.hpp>
#include <phylanx/plugins/fileio/file_write_hdf5.hpp>

#include <hpx/include/lcos.hpp>
#include <hpx/include/naming.hpp>
#include <hpx/include/util.hpp>
#include <hpx/throw_exception.hpp>

#include <highfive/H5File.hpp>
#include <highfive/H5DataSet.hpp>
#include <highfive/H5DataSpace.hpp>
#include <phylanx/util/detail/blaze-highfive.hpp>

#include <cstddef>
#include <fstream>
#include <memory>
#include <string>
#include <vector>
#include <utility>


///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace execution_tree { namespace primitives
{
    ///////////////////////////////////////////////////////////////////////////
    match_pattern_type const file_write_hdf5::match_data =
    {
        hpx::util::make_tuple("file_write_hdf5",
            std::vector<std::string>{"file_write_hdf5(_1, _2, _3)"},
            &create_file_write_hdf5, &create_primitive<file_write_hdf5>,
            R"(fname,dsetname,data
            Args:

                fname (string) : a file name
                dsetname (string) : a dataset name
                data (matrix or vector) : a data set

            Returns:

            Zero on success.)"
            )
    };

    ///////////////////////////////////////////////////////////////////////////
    file_write_hdf5::file_write_hdf5(
        primitive_arguments_type && operands,
            std::string const& name, std::string const& codename)
      : primitive_component_base(std::move(operands), name, codename)
    {
    }

    void file_write_hdf5::write_to_file_hdf5(ir::node_data<double> const& val,
        std::string const& filename, std::string const& dataset_name) const
    {
        HighFive::File outfile(filename,
            HighFive::File::ReadWrite | HighFive::File::Create |
                HighFive::File::Truncate);

        switch (val.num_dimensions())
        {
        case 0:
            {
                auto scalar = val.scalar();
                HighFive::DataSet dataSet =
                    outfile.createDataSet<double>(
                        dataset_name, HighFive::DataSpace::From(scalar));
                dataSet.write(scalar);
            }
            break;

        case 1:
            {
                auto vector = val.vector();
                std::vector<std::size_t> dims(1);
                dims[0] = vector.size();
                HighFive::DataSet dataSet =
                    outfile.createDataSet<double>(
                        dataset_name, HighFive::DataSpace(dims));
                dataSet.write(vector);
            }
            break;

        case 2:
            {
                auto matrix = val.matrix();
                std::vector<std::size_t> dims(2);
                dims[0] = matrix.rows();
                dims[1] = matrix.columns();
                HighFive::DataSet dataSet =
                    outfile.createDataSet<double>(
                        dataset_name, HighFive::DataSpace(dims));
                dataSet.write(matrix);
            }
            break;
        }
    }

    hpx::future<primitive_argument_type> file_write_hdf5::eval(
        primitive_arguments_type const& operands,
        primitive_arguments_type const& args, eval_context ctx) const
    {
        if (operands.size() != 3)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "phylanx::execution_tree::primitives::file_write::file_write_hdf5",
                generate_error_message(
                    "the file_write primitive requires exactly three operands"));
        }

        if (!valid(operands[0]) || !valid(operands[1]) ||
            !valid(operands[2]))
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "phylanx::execution_tree::primitives::file_write::file_write_hdf5",
                generate_error_message(
                    "the file_write primitive requires that the given operands "
                    "are valid"));
        }

        std::string filename =
            string_operand_sync(operands[0], args, name_, codename_, ctx);
        std::string dataset_name =
            string_operand_sync(operands[1], args, name_, codename_, ctx);

        auto this_ = this->shared_from_this();
        return numeric_operand(operands[2], args, name_, codename_, std::move(ctx))
            .then(hpx::launch::sync, hpx::util::unwrapping(
                [this_ = std::move(this_), filename = std::move(filename),
                    dataset_name = std::move(dataset_name)](
                    ir::node_data<double>&& val) mutable
                -> primitive_argument_type
                {
                    if (!valid(val))
                    {
                        HPX_THROW_EXCEPTION(hpx::bad_parameter,
                            "file_write_hdf5::eval",
                            this_->generate_error_message(
                                "the file_write_hdf5 primitive requires that "
                                "the argument value given by the operand is "
                                "non-empty"));
                    }

                    this_->write_to_file_hdf5(val, std::move(filename),
                        std::move(dataset_name));
                    return primitive_argument_type(std::move(val));
                }));
    }
}}}

#endif
