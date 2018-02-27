//  Copyright (c) 2018 Parsa Amini
//  Copyright (c) 2018 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/argmin.hpp>
#include <phylanx/ir/node_data.hpp>
#include <phylanx/util/matrix_iterators.hpp>

#include <hpx/include/lcos.hpp>
#include <hpx/include/naming.hpp>
#include <hpx/include/util.hpp>
#include <hpx/throw_exception.hpp>
#include <hpx/util/iterator_facade.hpp>

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include <blaze/Math.h>

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace execution_tree { namespace primitives
{
    ///////////////////////////////////////////////////////////////////////////
    primitive create_argmin(hpx::id_type const& locality,
        std::vector<primitive_argument_type>&& operands,
        std::string const& name, std::string const& codename)
    {
        static std::string type("argmin");
        return create_primitive_component(
            locality, type, std::move(operands), name, codename);
    }

    match_pattern_type const argmin::match_data =
    {
        hpx::util::make_tuple("argmin",
        std::vector<std::string>{"argmin(_1, _2)"},
        &create_argmin, &create_primitive<argmin>)
    };

    ///////////////////////////////////////////////////////////////////////////
    argmin::argmin(std::vector<primitive_argument_type>&& operands,
        std::string const& name, std::string const& codename)
      : primitive_component_base(std::move(operands), name, codename)
    {}

    ///////////////////////////////////////////////////////////////////////////
    namespace detail
    {
        struct argmin : std::enable_shared_from_this<argmin>
        {
            argmin(std::string const& name, std::string const& codename)
              : name_(name)
              , codename_(codename)
            {}

        protected:
            std::string name_;
            std::string codename_;

        protected:
            using val_type = double;
            using arg_type = ir::node_data<val_type>;
            using args_type = std::vector<arg_type>;

            primitive_argument_type argmin0d(args_type && args) const
            {
                // `axis` is optional
                if (args.size() == 2)
                {
                    // `axis` must be a scalar if provided
                    if (args[1].num_dimensions() != 0)
                    {
                        HPX_THROW_EXCEPTION(hpx::bad_parameter,
                            "argmin::argmin0d",
                            generate_error_message(
                                "operand axis must be a scalar", name_,
                                codename_));
                    }
                    const int axis = args[1].scalar();
                    // `axis` can only be -1 or 0
                    if (axis < -1 || axis > 0)
                    {
                        HPX_THROW_EXCEPTION(hpx::bad_parameter,
                            "argmin::argmin0d",
                            generate_error_message(
                                "operand axis can only between -1 and 0 for "
                                "an a operand that is 0d",
                                name_, codename_));
                    }
                }

                return 0ul;
            }

            ///////////////////////////////////////////////////////////////////////////
            primitive_argument_type argmin1d(args_type && args) const
            {
                // `axis` is optional
                if (args.size() == 2)
                {
                    // `axis` must be a scalar if provided
                    if (args[1].num_dimensions() != 0)
                    {
                        HPX_THROW_EXCEPTION(hpx::bad_parameter,
                            "argmin::argmin1d",
                            generate_error_message(
                                "operand axis must be a scalar", name_,
                                codename_));
                    }
                    const int axis = args[1].scalar();
                    // `axis` can only be -1 or 0
                    if (axis < -1 || axis > 0)
                    {
                        HPX_THROW_EXCEPTION(hpx::bad_parameter,
                            "argmin::argmin1d",
                            generate_error_message(
                                "operand axis can only between -1 and 0 for "
                                "an a operand that is 1d",
                                name_, codename_));
                    }
                }

                auto a = args[0].vector();

                // a should not be empty
                if (a.size() == 0)
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "argmin::argmin1d",
                        generate_error_message(
                            "attempt to get argmin of an empty sequence",
                            name_, codename_));
                }

                // Find the minimum value among the elements
                const auto min_it = std::min_element(a.begin(), a.end());

                // Return min's index
                return std::distance(a.begin(), min_it);
            }

            ///////////////////////////////////////////////////////////////////////////
            primitive_argument_type argmin2d_flatten(arg_type && arg_a) const
            {
                using phylanx::util::matrix_row_iterator;

                auto a = arg_a.matrix();

                const matrix_row_iterator<decltype(a)> a_begin(a);
                const matrix_row_iterator<decltype(a)> a_end(a, a.rows());

                val_type global_min = (std::numeric_limits<val_type>::max)();
                std::size_t global_index = 0ul;
                std::size_t passed_rows = 0ul;
                for (auto it = a_begin; it != a_end; ++it, ++passed_rows)
                {
                    const auto local_min = std::min_element(it->begin(), it->end());
                    const auto local_min_val = *local_min;

                    if (local_min_val < global_min)
                    {
                        global_min = local_min_val;
                        global_index = std::distance(it->begin(), local_min) +
                            passed_rows * it->size();
                    }
                }
                return global_index;
            }

            primitive_argument_type argmin2d_x_axis(arg_type && arg_a) const
            {
                using phylanx::util::matrix_row_iterator;

                auto a = arg_a.matrix();

                const matrix_row_iterator<decltype(a)> a_begin(a);
                const matrix_row_iterator<decltype(a)> a_end(a, a.rows());

                std::vector<primitive_argument_type> result;
                for (auto it = a_begin; it != a_end; ++it)
                {
                    const auto local_min = std::min_element(it->begin(), it->end());
                    std::int64_t index = std::distance(it->begin(), local_min);
                    result.emplace_back(primitive_argument_type(index));
                }
                return result;
            }
            primitive_argument_type argmin2d_y_axis(arg_type && arg_a) const
            {
                using phylanx::util::matrix_column_iterator;

                auto a = arg_a.matrix();

                const matrix_column_iterator<decltype(a)> a_begin(a);
                const matrix_column_iterator<decltype(a)> a_end(a, a.columns());

                std::vector<primitive_argument_type> result;
                for (auto it = a_begin; it != a_end; ++it)
                {
                    const auto local_min = std::min_element(it->begin(), it->end());
                    std::int64_t index = std::distance(it->begin(), local_min);
                    result.emplace_back(primitive_argument_type(index));
                }
                return result;
            }

            primitive_argument_type argmin2d(args_type && args) const
            {
                // a should not be empty
                if (args[0].matrix().rows() == 0 ||
                    args[0].matrix().columns() == 0)
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "argmax::argmin2d",
                        generate_error_message(
                            "attempt to get argmax of an empty sequence",
                            name_, codename_));
                }

                // `axis` is optional
                if (args.size() == 1)
                {
                    // Option 1: Flatten and find min
                    return argmin2d_flatten(std::move(args[0]));
                }

                // `axis` must be a scalar if provided
                if (args[1].num_dimensions() != 0)
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "argmin::argmin2d",
                        generate_error_message(
                            "operand axis must be a scalar", name_, codename_));
                }
                const int axis = args[1].scalar();
                // `axis` can only be -2, -1, 0, or 1
                if (axis < -2 || axis > 1)
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "argmin::argmin2d",
                        generate_error_message(
                            "operand axis can only between -2 and 1 for an a "
                            "operand that is 2d",
                            name_, codename_));
                }
                switch (axis)
                {
                // Option 2: Find min among rows
                case -2: HPX_FALLTHROUGH;
                case 0:
                    return argmin2d_x_axis(std::move(args[0]));

                // Option 3: Find min among columns
                case -1: HPX_FALLTHROUGH;
                case 1:
                    return argmin2d_y_axis(std::move(args[0]));

                default:
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "argmin::argmin2d",
                        generate_error_message(
                            "operand a has an invalid number of "
                            "dimensions",
                            name_, codename_));
                }


            }

        public:
            hpx::future<primitive_argument_type> eval(
                std::vector<primitive_argument_type> const& operands,
                std::vector<primitive_argument_type> const& args) const
            {
                if (operands.empty() || operands.size() > 2)
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "argmin::eval",
                        generate_error_message(
                            "the argmin primitive requires exactly one or two "
                            "operands",
                            name_, codename_));
                }

                for (auto const& i : operands)
                {
                    if (!valid(i))
                    {
                        HPX_THROW_EXCEPTION(hpx::bad_parameter,
                            "argmin::eval",
                            generate_error_message(
                                "the argmin primitive requires that the "
                                "arguments given by the operands array are "
                                "valid",
                                name_, codename_));
                    }
                }


                auto this_ = this->shared_from_this();
                return hpx::dataflow(hpx::util::unwrapping(
                    [this_](args_type&& args) -> primitive_argument_type
                    {
                        std::size_t a_dims = args[0].num_dimensions();
                        switch (a_dims)
                        {
                        case 0:
                            return this_->argmin0d(std::move(args));

                        case 1:
                            return this_->argmin1d(std::move(args));

                        case 2:
                            return this_->argmin2d(std::move(args));

                        default:
                            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                                "argmin::eval",
                                generate_error_message(
                                    "operand a has an invalid "
                                    "number of dimensions",
                                    this_->name_, this_->codename_));
                        }
                    }),
                    detail::map_operands(
                        operands, functional::numeric_operand{}, args,
                        name_, codename_));
            }
        };
    }

    hpx::future<primitive_argument_type> argmin::eval(
        std::vector<primitive_argument_type> const& args) const
    {
        if (operands_.empty())
        {
            return std::make_shared<detail::argmin>(name_, codename_)
                ->eval(args, noargs);
        }
        return std::make_shared<detail::argmin>(name_, codename_)
            ->eval(operands_, args);
    }
}}}
