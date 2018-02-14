//  Copyright (c) 2018 Alireza Kheirkhahan
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>

#if defined(PHYLANX_HAVE_HIGHFIVE)
#include <phylanx/execution_tree/primitives/file_write_hdf5.hpp>
#include <phylanx/ir/node_data.hpp>
#include <phylanx/util/variant.hpp>

#include <hpx/include/components.hpp>
#include <hpx/include/lcos.hpp>
#include <hpx/include/util.hpp>

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
typedef hpx::components::component<
phylanx::execution_tree::primitives::file_write_hdf5>
        file_write_hdf5_type;
HPX_REGISTER_DERIVED_COMPONENT_FACTORY(file_write_hdf5_type,
        phylanx_file_write_hdf5_component, "phylanx_primitive_component",
hpx::components::factory_enabled)
HPX_DEFINE_GET_COMPONENT_TYPE(file_write_hdf5_type::wrapped_type)

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace execution_tree { namespace primitives
{
    ///////////////////////////////////////////////////////////////////////////
    match_pattern_type const file_write_hdf5::match_data = {
        hpx::util::make_tuple("file_write_hdf5",
            std::vector<std::string>{"file_write_hdf5(_1, _2, _3)"},
            &create<file_write_hdf5>)};

    ///////////////////////////////////////////////////////////////////////////
    file_write_hdf5::file_write_hdf5(
        std::vector<primitive_argument_type> && operands)
      : operands_(operands)
    {
    }

    namespace detail
    {
        struct file_write_hdf5 : std::enable_shared_from_this<file_write_hdf5>
        {
            file_write_hdf5() = default;

        protected:
            void write_to_file_hdf5(ir::node_data<double> const& val)
            {
                HighFive::File outfile(filename_,
                    HighFive::File::ReadWrite | HighFive::File::Create |
                        HighFive::File::Truncate);

                switch (val.num_dimensions()) {
                    case 0: {
                        auto scalar = val.scalar();
                        HighFive::DataSet dataSet =
                            outfile.createDataSet<double>(datasetName_,
                                HighFive::DataSpace::From(scalar));
                        dataSet.write(scalar);
                    }
                        break;
                    case 1: {
                        auto vector = val.vector();
                        std::vector<std::size_t> dims(1);
                        dims[0] = vector.size();
                        HighFive::DataSet dataSet =
                            outfile.createDataSet<double>(
                                datasetName_, HighFive::DataSpace(dims));
                        dataSet.write(vector);
                    }
                        break;

                    case 2: {
                        auto matrix = val.matrix();
                        std::vector<std::size_t> dims(2);
                        dims[0] = matrix.rows();
                        dims[1] = matrix.columns();
                        HighFive::DataSet dataSet =
                            outfile.createDataSet<double>(
                                datasetName_, HighFive::DataSpace(dims));
                        dataSet.write(matrix);
                    }
                        break;
                }
            }

        public:
            hpx::future<primitive_result_type> eval(
                std::vector<primitive_argument_type> const& operands,
                std::vector<primitive_argument_type> const& args)
            {
                if (operands.size() != 3)
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "phylanx::execution_tree::primitives::file_write::"
                        "file_write_hdf5",
                        "the file_write primitive requires exactly three "
                        "operands");
                }

                if (!valid(operands[0]) || !valid(operands[1]) ||
                    !valid(operands[2]))
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "phylanx::execution_tree::primitives::file_write::"
                        "file_write_hdf5",
                        "the file_write primitive requires that the given "
                        "operands are valid");
                }

                filename_ = string_operand_sync(operands[0], args);
                datasetName_ = string_operand_sync(operands[1], args);

                auto this_ = this->shared_from_this();
                return numeric_operand(operands[2], args)
                    .then(hpx::util::unwrapping(
                        [this_](ir::node_data<double>&& val)
                        -> primitive_result_type
                        {
                            if (!valid(val))
                            {
                                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                                    "file_write_hdf5::eval",
                                    "the file_write_hdf5 primitive requires that "
                                    "the argument value given by the "
                                    "operand is non-empty");
                            }

                            this_->write_to_file_hdf5(val);
                            return primitive_result_type(std::move(val));
                        }));
            }

        private:
            std::string filename_;
            std::string datasetName_;
            primitive_argument_type operand_;
        };
    }

    ///////////////////////////////////////////////////////////////////////////
    // write data to given file in hdf5 format and return content
    hpx::future<primitive_result_type> file_write_hdf5::eval(
        std::vector<primitive_argument_type> const& args) const
    {
        if (operands_.empty())
        {
            return std::make_shared<detail::file_write_hdf5>()->eval(
                args, noargs);
        }

        return std::make_shared<detail::file_write_hdf5>()->eval(
            operands_, args);
    }
}}}

#endif
