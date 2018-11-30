//  Copyright (c) 2017 Alireza Kheirkhahan
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_PRIMITIVES_FILE_WRITE_HDF5_JAN_26_2017_0129PM)
#define PHYLANX_PRIMITIVES_FILE_WRITE_HDF5_JAN_26_2017_0129PM

#include <phylanx/config.hpp>

#if defined(PHYLANX_HAVE_HIGHFIVE)
#include <phylanx/execution_tree/primitives/base_primitive.hpp>
#include <phylanx/execution_tree/primitives/primitive_component_base.hpp>

#include <hpx/lcos/future.hpp>

#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace phylanx { namespace execution_tree { namespace primitives
{
    class file_write_hdf5
      : public primitive_component_base
      , public std::enable_shared_from_this<file_write_hdf5>
    {
    protected:
        hpx::future<primitive_argument_type> eval(
            primitive_arguments_type const& operands,
            primitive_arguments_type const& args, eval_context ctx) const;

    public:
        static match_pattern_type const match_data;

        file_write_hdf5() = default;

        file_write_hdf5(primitive_arguments_type&& operands,
            std::string const& name, std::string const& codename);

    private:
        void write_to_file_hdf5(ir::node_data<double> const& val,
            std::string const& filename, std::string const& dataset_name) const;
    };

    inline primitive create_file_write_hdf5(hpx::id_type const& locality,
        primitive_arguments_type&& operands,
        std::string const& name = "", std::string const& codename = "")
    {
        return create_primitive_component(
            locality, "file_write_hdf5", std::move(operands), name, codename);
    }
}}}

#endif
#endif
