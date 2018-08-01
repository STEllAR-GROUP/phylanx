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
        Attribute fmt = grp.getAttribute< std::string >(fmtkey, DataSpace::From(desc));
        std:string smat_fmt;
        fmt.read(smat_fmt);

        if(!smat_fmt.equals(desc)) {
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

    void csc_read_hdf5::read_to_file_hdf5(ir::node_data<double> const& val,
        std::string const& filename, std::string const& dataset_name) const
    {
        HighFive::File infile(filename, HighFive::File::ReadOnly);

        auto const dims = val.num_dimensions();
        if(dims > 0 && dims < 3)
        {
            blaze::DynamicMatrix<double, blaze::columnMajor> matrix;
            marshal(infile, dataset_name, matrix);
        }
        // TODO: error handling?
    }

    hpx::future<primitive_argument_type> csc_read_hdf5::eval(
        std::vector<primitive_argument_type> const& operands,
        std::vector<primitive_argument_type> const& args) const
    {
        if (operands.size() != 3)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "phylanx::execution_tree::primitives::file_read::csc_read_hdf5",
                util::generate_error_message(
                    "the csc_read_hdf5 primitive requires exactly three operands",
                    name_, codename_));
        }

        if (!valid(operands[0]) || !valid(operands[1]) ||
            !valid(operands[2]))
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "phylanx::execution_tree::primitives::csc_read_hdf5::csc_read_hdf5",
                util::generate_error_message(
                    "the csc_read_hdf5 primitive requires that the given operands "
                    "are valid",
                    name_, codename_));
        }

        std::string filename =
            string_operand_sync(operands[0], args, name_, codename_);
        std::string dataset_name =
            string_operand_sync(operands[1], args, name_, codename_);

        auto this_ = this->shared_from_this();
        return numeric_operand(operands[2], args, name_, codename_)
            .then(hpx::launch::sync, hpx::util::unwrapping(
                [this_, filename = std::move(filename),
                    dataset_name = std::move(dataset_name)](
                    ir::node_data<double>&& val) -> primitive_argument_type
                {
                    if (!valid(val))
                    {
                        HPX_THROW_EXCEPTION(hpx::bad_parameter,
                            "csc_read_hdf5::eval",
                            util::generate_error_message(
                                "the csc_read_hdf5 primitive requires that "
                                "the argument value given by the operand is "
                                "non-empty",
                                this_->name_, this_->codename_));
                    }

                    this_->write_to_file_hdf5(val, std::move(filename),
                        std::move(dataset_name));
                    return primitive_argument_type(std::move(val));
                }));
    }

    ///////////////////////////////////////////////////////////////////////////
    // write data to given file in hdf5 format and return content
    hpx::future<primitive_argument_type> csc_read_hdf5::eval(
        std::vector<primitive_argument_type> const& args) const
    {
        if (this->no_operands())
        {
            return eval(args, noargs);
        }
        return eval(this->operands(), args);
    }
}}}

#endif
