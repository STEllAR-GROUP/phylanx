//  Copyright (c) 2018 Alireza Kheirkhahan
//  Copyright (c) 2018 Chris Taylor 
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>

#if defined(PHYLANX_HAVE_HIGHFIVE)
#include <phylanx/ir/node_data.hpp>
#include <phylanx/plugins/fileio/csc_read_hdf5.hpp>

#include <hpx/include/lcos.hpp>
#include <hpx/include/naming.hpp>
#include <hpx/include/util.hpp>
#include <hpx/throw_exception.hpp>

#include <phylanx/util/detail/blaze-highfive.hpp>
#include <blaze/Blaze.h>
#include <highfive/H5File.hpp>
#include <highfive/H5DataSet.hpp>
#include <highfive/H5DataSpace.hpp>

#include <cstddef>
#include <fstream>
#include <memory>
#include <string>
#include <vector>
#include <utility>

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace execution_tree { namespace primitives
{

    template<typename ValueType, bool SO>
    void marshal(HighFive::File & file
       , std::string const& hdf5_group_name
       , blaze::CompressedMatrix<ValueType, SO> & data);

    template<typename ValueType>
    void marshal(HighFive::File & file
        , std::string const& hdf5_group_name
        , blaze::CompressedMatrix<ValueType, blaze::columnMajor> & data) {

        using namespace HighFive;

        Group grp = file.getGroup(hdf5_group_name);
        std::string const desc("csc");

        std::string const fmtkey("h5sparse_format");
        Attribute fmt = grp.getAttribute(fmtkey);
        std::string smat_fmt;
        fmt.read(smat_fmt);

        if(smat_fmt.compare(desc) == 0) {
           //TODO: how does error handling work?
        }

        std::string const shapekey("h5sparse_shape");
        std::vector< std::int64_t > dims(2);
        Attribute shape = grp.getAttribute(shapekey);
        shape.read(dims);

        data.resize(dims[0], dims[1], false);

        std::string const indiceskey("indices");
        DataSet dset_indices = grp.getDataSet(indiceskey);
        std::string const datakey("data");
        DataSet dset_data = grp.getDataSet(datakey);
        std::string const indptrkey("indptr");
        DataSet dset_indptr = grp.getDataSet(indptrkey);

        std::vector<ValueType> values;
        std::vector<std::int64_t> indices;
        std::vector<std::int64_t> indptr;

        dset_data.read(values);
        dset_indices.read(indices);
        dset_indptr.read(indptr);

        for(std::int64_t i = 0; i < dims[1]; ++i) {
            for(std::int64_t j = indptr[i]; j < indptr[i+1]; ++j) {
                data(i, indices[j]) = values[j];
            }
        }
   }

    ///////////////////////////////////////////////////////////////////////////
    match_pattern_type const csc_read_hdf5::match_data =
    {
        hpx::util::make_tuple("csc_read_hdf5",
            std::vector<std::string>{"csc_read_hdf5(_1, _2, _3)"},
            &create_csc_read_hdf5, &create_primitive<csc_read_hdf5>)
    };

    ///////////////////////////////////////////////////////////////////////////
    csc_read_hdf5::csc_read_hdf5(
        std::vector<primitive_argument_type> && operands,
            std::string const& name, std::string const& codename)
      : primitive_component_base(std::move(operands), name, codename)
    {
    }

    hpx::future<primitive_argument_type> csc_read_hdf5::eval(
        std::vector<primitive_argument_type> const& args) const
    {
        if (operands_.size() != 2)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "phylanx::execution_tree::primitives::file_read_hdf5::eval",
                util::generate_error_message(
                    "the file_read_hdf5 primitive requires exactly two "
                        "literal arguments",
                    name_, codename_));
        }

        if (!valid(operands_[0]) || !valid(operands_[1]))
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "phylanx::execution_tree::primitives::file_read_hdf5::eval",
                util::generate_error_message(
                    "the file_read_hdf5 primitive requires that the given "
                        "operand is valid",
                    name_, codename_));
        }

        std::string filename =
            string_operand_sync(operands_[0], args, name_, codename_);
        std::string datasetName =
            string_operand_sync(operands_[1], args, name_, codename_);

        HighFive::File infile(filename, HighFive::File::ReadOnly);
        // matrix
        blaze::CompressedMatrix<double, blaze::columnMajor> matrix;
        marshal(infile, datasetName, matrix);

        return hpx::make_ready_future(primitive_argument_type{
            ir::node_data< blaze::CompressedMatrix<double, blaze::columnMajor> >{std::move(matrix)}});
        };
    }
}}}

#endif
