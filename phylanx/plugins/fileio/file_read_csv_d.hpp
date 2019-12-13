//  Copyright (c) 2017 Alireza Kheirkhahan
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_PRIMITIVES_FILE_READ_CSV_D_OCT_31_2019_0334PM)
#define PHYLANX_PRIMITIVES_FILE_READ_CSV_D_OCT_31_2019_0334PM

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/base_primitive.hpp>
#include <phylanx/execution_tree/primitives/primitive_component_base.hpp>

#include <hpx/lcos/future.hpp>

#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace phylanx { namespace execution_tree { namespace primitives
{
    class file_read_csv_d
      : public primitive_component_base
      , public std::enable_shared_from_this<file_read_csv_d>
    {
    public:
        static match_pattern_type const match_data;

        file_read_csv_d() = default;

        file_read_csv_d(primitive_arguments_type&& operands,
            std::string const& name, std::string const& codename);

        primitive_argument_type read_scalar(std::string filename,
            std::pair<std::size_t, std::size_t> locs,
            std::pair<std::size_t, std::size_t> dims) const;

        primitive_argument_type read_vector(std::string filename,
            std::pair<std::size_t, std::size_t> loc_info,
            std::pair<std::size_t, std::size_t> dims) const;

        primitive_argument_type read_matrix(std::string filename,
            std::pair<std::size_t, std::size_t> locs,
            std::pair<std::size_t, std::size_t> dims) const;

        std::pair<std::size_t, std::size_t> get_dims(std::string filename) const;

        primitive_argument_type file_read_csv_d::read(std::string filename,
            phylanx::execution_tree::annotation&& locality_info) const;

        hpx::future<primitive_argument_type> eval(
            primitive_arguments_type const& operands,
            primitive_arguments_type const& args,
            eval_context ctx) const override;
    };

    inline primitive create_file_read_csv_d(hpx::id_type const& locality,
        primitive_arguments_type&& operands,
        std::string const& name = "", std::string const& codename = "")
    {
        return create_primitive_component(
            locality, "file_read_csv_d", std::move(operands), name, codename);
    }
}}}

#endif
