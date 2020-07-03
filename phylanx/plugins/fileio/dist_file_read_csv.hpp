//  Copyright (c) 2017 Alireza Kheirkhahan
//  Copyright (c) 2020 Bita Hasheminezhad
//  Copyright (c) 2020 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_PRIMITIVES_DIST_FILE_READ_CSV)
#define PHYLANX_PRIMITIVES_DIST_FILE_READ_CSV

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/base_primitive.hpp>
#include <phylanx/execution_tree/primitives/primitive_component_base.hpp>

#include <hpx/futures/future.hpp>

#include <fstream>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace phylanx { namespace execution_tree { namespace primitives
{
    class dist_file_read_csv
      : public primitive_component_base
      , public std::enable_shared_from_this<dist_file_read_csv>
    {
    public:
        static match_pattern_type const match_data;

        dist_file_read_csv() = default;

        dist_file_read_csv(primitive_arguments_type&& operands,
            std::string const& name, std::string const& codename);

    protected:
        hpx::future<primitive_argument_type> eval(
            primitive_arguments_type const& operands,
            primitive_arguments_type const& args,
            eval_context ctx) const override;
    };

    inline primitive create_dist_file_read_csv(hpx::id_type const& locality,
        primitive_arguments_type&& operands,
        std::string const& name = "", std::string const& codename = "")
    {
        return create_primitive_component(
            locality, "file_read_csv_d", std::move(operands), name, codename);
    }
}}}

#endif
