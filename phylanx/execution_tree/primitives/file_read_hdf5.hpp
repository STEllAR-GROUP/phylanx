//  Copyright (c) 2018 Alireza Kheirkhahan
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_PRIMITIVES_FILE_READ_HDF5_JAN_26_2017_0129PM)
#define PHYLANX_PRIMITIVES_FILE_READ_HDF5_JAN_26_2017_0129PM

#include <phylanx/config.hpp>

#if defined(PHYLANX_HAVE_HIGHFIVE)
#include <phylanx/execution_tree/primitives/base_primitive.hpp>
#include <phylanx/execution_tree/primitives/primitive_component_base.hpp>

#include <hpx/lcos/future.hpp>

#include <string>
#include <vector>

namespace phylanx { namespace execution_tree { namespace primitives
{
    class file_read_hdf5 : public primitive_component_base
    {
    public:
        static match_pattern_type const match_data;

        file_read_hdf5() = default;

        file_read_hdf5(std::vector<primitive_argument_type>&& operands);

        hpx::future<primitive_argument_type> eval(
            std::vector<primitive_argument_type> const& args) const override;
    };

    PHYLANX_EXPORT primitive create_file_read_hdf5(hpx::id_type const& locality,
        std::vector<primitive_argument_type>&& operands,
        std::string const& name = "");
}}}

#endif
#endif
