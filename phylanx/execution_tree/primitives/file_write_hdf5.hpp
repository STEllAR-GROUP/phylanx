//  Copyright (c) 2017 Alireza Kheirkhahan
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_PRIMITIVES_FILE_WRITE_HDF5_JAN_26_2017_0129PM)
#define PHYLANX_PRIMITIVES_FILE_WRITE_hdf5_HDF5_JAN_2017_0129PM

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/base_primitive.hpp>
#include <phylanx/util/detail/blaze-highfive.hpp>

#include <hpx/include/components.hpp>

#include <string>
#include <vector>

namespace phylanx { namespace execution_tree { namespace primitives
{
    class HPX_COMPONENT_EXPORT file_write_hdf5
      : public base_primitive
      , public hpx::components::component_base<file_write_hdf5>
    {
    public:
        static match_pattern_type const match_data;

        file_write_hdf5() = default;

        file_write_hdf5(std::vector<primitive_argument_type>&& operands);

        hpx::future<primitive_result_type> eval(
            std::vector<primitive_argument_type> const& args) const override;

    private:
        std::vector<primitive_argument_type> operands_;
    };
}}}

#endif