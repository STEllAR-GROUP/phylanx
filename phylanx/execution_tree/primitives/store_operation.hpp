// Copyright (c) 2017 Alireza kheirkhahan
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_PRIMITIVES_STORE_PRIMITIVE_OCT_05_2017_0204PM)
#define PHYLANX_PRIMITIVES_STORE_PRIMITIVE_OCT_05_2017_0204PM

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/base_primitive.hpp>
#include <phylanx/execution_tree/primitives/primitive_component_base.hpp>

#include <hpx/lcos/future.hpp>

#include <memory>
#include <string>
#include <vector>

namespace phylanx {namespace execution_tree { namespace primitives
{
    class store_operation
      : public primitive_component_base
      , public std::enable_shared_from_this<store_operation>
    {
    public:
        static match_pattern_type const match_data;

        store_operation() = default;

        store_operation(std::vector<primitive_argument_type>&& operands,
            std::string const& name, std::string const& codename);

        hpx::future<primitive_argument_type> eval(
            std::vector<primitive_argument_type> const& args) const override;

    protected:
        hpx::future<primitive_argument_type> eval(
            std::vector<primitive_argument_type> const& operands,
            std::vector<primitive_argument_type> const& args) const;
        bool check_set_parameters(int64_t start, int64_t stop, int64_t step,
            size_t array_length) const;
        std::vector<int64_t> create_list_set(int64_t start,
            int64_t stop,
            int64_t step,
            size_t array_length) const;
        int64_t extract_integer_value(
            const primitive_argument_type& val, int64_t default_value) const;
        std::vector<int64_t> extract_slicing(
            primitive_argument_type&& arg, size_t arg_size) const;
        primitive_argument_type set0d(
            std::vector<primitive_argument_type>&& args) const;
        void set1d(std::vector<primitive_argument_type>&& args,
            ir::node_data<double>& data,
            std::vector<int64_t>& init_list) const;
        void set2d(std::vector<primitive_argument_type>&& args,
            ir::node_data<double>& data,
            std::vector<int64_t>& init_list_row,
            std::vector<int64_t>& init_list_col) const;
    };

    PHYLANX_EXPORT primitive create_store_operation(
        hpx::id_type const& locality,
        std::vector<primitive_argument_type>&& operands,
        std::string const& name = "", std::string const& codename = "");
}}}

#endif
