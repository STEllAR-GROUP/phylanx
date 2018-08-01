//  Copyright (c) 2018 Alireza Kheirkhahan
//  Copyright (c) 2018 Chris Taylor 
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>

#if defined(PHYLANX_HAVE_HIGHFIVE)
#include <phylanx/ir/node_data.hpp>
#include <phylanx/plugins/fileio/csc_write_hdf5.hpp>

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
    void serialize(HighFive::File & file
       , std::string const& hdf5_group_name
       , blaze::CompressedMatrix<ValueType, SO> const& data);

    template<typename ValueType>
    void serialize(HighFive::File & file
        , std::string const& hdf5_group_name
        , blaze::CompressedMatrix<ValueType, blaze::columnMajor> const& data) {

        using namespace HighFive;

        Group grp = file.createGroup(hdf5_group_name);
        std::string const desc("csc");
        std::string const fmtkey("h5sparse_format");
        Attribute fmt = grp.createAttribute< std::string >(fmtkey, DataSpace::From(desc));
        fmt.write(desc);

        std::string const shapekey("h5sparse_shape");
        std::vector< std::int64_t > const dims{
            static_cast<std::int64_t>(data.rows())
            , static_cast<std::int64_t>(data.columns())
        };

        Attribute shape = grp.createAttribut<std::int64_t>(shapekey, DataSpace::From(dims));
        shape.write(dims);

        std::vector<ValueType> values;
        std::vector<std::int64_t> indices;
        std::vector<std::int64_t> indptr{0};

        for(std::int64_t i = 0; i < dims[0]; ++i) {
            for(std::int64_t j = 0; j < dims[1]; ++j) {
                if(data(i,j) != 0) {
                    values.push_back(data(i,j));
                    indices.push_back(j);
                }

                indptr.push_back(indices.size());
            }
        }
   
        std::string const indiceskey("indices");
        DataSet dset_indices = grp.createDataSet<std::int64_t>(indiceskey, DataSpace::From(indices));

        std::string const datakey("data");
        DataSet dset_data = grp.createDataSet<ValueType>(datakey, DataSpace::From(values));

        std::string const indptrkey("indptr");
        DataSet dset_indptr = grp.createDataSet<std::int64_t>(indptrkey, DataSpace::From(indptr));

        dset_indices.write(indices);
        dset_data.write(values);
        dset_indptr.write(indptr);

        file.flush();
    }

    ///////////////////////////////////////////////////////////////////////////
    match_pattern_type const csc_write_hdf5::match_data =
    {
        hpx::util::make_tuple("csc_write_hdf5",
            std::vector<std::string>{"csc_write_hdf5(_1, _2, _3)"},
            &create_csc_write_hdf5, &create_primitive<csc_write_hdf5>)
    };

    ///////////////////////////////////////////////////////////////////////////
    csc_write_hdf5::csc_write_hdf5(
        std::vector<primitive_argument_type> && operands,
            std::string const& name, std::string const& codename)
      : primitive_component_base(std::move(operands), name, codename)
    {
    }

    void csc_write_hdf5::write_to_file_hdf5(ir::node_data<double> const& val,
        std::string const& filename, std::string const& dataset_name) const
    {
        HighFive::File outfile(filename,
            HighFive::File::ReadWrite | HighFive::File::Create |
                HighFive::File::Truncate);

        auto const dims = val.num_dimensions();
        if(dims > 0 && dims < 3)
        {
            auto matrix = val.matrix();
            serialize(filename, dataset_name, matrix);
        }
    }

    hpx::future<primitive_argument_type> csc_write_hdf5::eval(
        std::vector<primitive_argument_type> const& operands,
        std::vector<primitive_argument_type> const& args) const
    {
        if (operands.size() != 3)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "phylanx::execution_tree::primitives::file_write::csc_write_hdf5",
                util::generate_error_message(
                    "the file_write primitive requires exactly three operands",
                    name_, codename_));
        }

        if (!valid(operands[0]) || !valid(operands[1]) ||
            !valid(operands[2]))
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "phylanx::execution_tree::primitives::file_write::csc_write_hdf5",
                util::generate_error_message(
                    "the file_write primitive requires that the given operands "
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
                            "csc_write_hdf5::eval",
                            util::generate_error_message(
                                "the csc_write_hdf5 primitive requires that "
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
    hpx::future<primitive_argument_type> csc_write_hdf5::eval(
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
