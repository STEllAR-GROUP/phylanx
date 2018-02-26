//  Copyright (c) 2018 Parsa Amini
//  Copyright (c) 2018 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/argmax.hpp>
#include <phylanx/ir/node_data.hpp>

#include <hpx/include/lcos.hpp>
#include <hpx/include/naming.hpp>
#include <hpx/include/util.hpp>
#include <hpx/throw_exception.hpp>
#include <hpx/util/iterator_facade.hpp>

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <numeric>
#include <string>
#include <utility>
#include <vector>

#include <blaze/Math.h>

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace execution_tree { namespace primitives
{
    ///////////////////////////////////////////////////////////////////////////
    primitive create_argmax(hpx::id_type const& locality,
        std::vector<primitive_argument_type>&& operands,
        std::string const& name, std::string const& codename)
    {
        static std::string type("argmax");
        return create_primitive_component(
            locality, type, std::move(operands), name, codename);
    }

    match_pattern_type const argmax::match_data =
    {
        hpx::util::make_tuple("argmax",
            std::vector<std::string>{"argmax(_1, _2)"},
            &create_argmax, &create_primitive<argmax>)
    };

    ///////////////////////////////////////////////////////////////////////////
    argmax::argmax(
            std::vector<primitive_argument_type> && operands,
            std::string const& name, std::string const& codename)
      : primitive_component_base(std::move(operands), name, codename)
    {}

    ///////////////////////////////////////////////////////////////////////////
    namespace detail
    {
        template <typename T>
        class matrix_row_iterator
          : public hpx::util::iterator_facade<
                matrix_row_iterator<T>,
                blaze::Row<T>,
                std::random_access_iterator_tag,
                blaze::Row<T>>
        {
        public:
            matrix_row_iterator(T& t, std::size_t index = 0)
              : data_(t)
              , index_(index)
            {
            }

        private:
            friend class hpx::util::iterator_core_access;

            void increment()
            {
                ++index_;
            }
            void decrement()
            {
                --index_;
            }
            void advance(std::size_t n)
            {
                index_ += n;
            }
            bool equal(matrix_row_iterator const& other) const
            {
                return index_ == other.index_;
            }
            blaze::Row<T> dereference() const
            {
                return blaze::row(data_, index_);
            }

        private:
            T& data_;
            std::size_t index_;
        };

        template <typename T>
        class matrix_column_iterator
            : public hpx::util::iterator_facade<
                matrix_column_iterator<T>,
                blaze::Column<T>,
                std::random_access_iterator_tag,
                blaze::Column<T>>
        {
        public:
            matrix_column_iterator(T& t, std::size_t index = 0)
              : data_(t)
              , index_(index)
            {
            }

        private:
            friend class hpx::util::iterator_core_access;

            void increment()
            {
                ++index_;
            }
            void decrement()
            {
                --index_;
            }
            void advance(std::size_t n)
            {
                index_ += n;
            }
            bool equal(matrix_column_iterator const& other) const
            {
                return index_ == other.index_;
            }
            blaze::Column<T> dereference() const
            {
                return blaze::column(data_, index_);
            }

        private:
            T& data_;
            std::size_t index_;
        };


        struct argmax : std::enable_shared_from_this<argmax>
        {
            argmax(std::string const& name, std::string const& codename)
              : name_(name)
              , codename_(codename)
            {}

        protected:
            std::string name_;
            std::string codename_;

        protected:
            using arg_type = ir::node_data<double>;
            using args_type = std::vector<arg_type>;

            primitive_argument_type argmax0d(args_type && args) const
            {
                return 0ul;
            }

            ///////////////////////////////////////////////////////////////////////////
            primitive_argument_type argmax1d(args_type && args) const
            {
                auto a = args[0].vector();
                auto max_it = std::max_element(a.begin(), a.end());

                return std::distance(a.begin(), max_it);
            }

            ///////////////////////////////////////////////////////////////////////////
            primitive_argument_type argmax2d_flatten(arg_type && arg_a) const
            {
                auto a = arg_a.matrix();

                matrix_row_iterator<decltype(a)> a_begin(a);
                matrix_row_iterator<decltype(a)> a_end(a, a.rows());

                double global_max = 0.;
                std::size_t global_index = 0ul;
                std::size_t passed_rows = 0ul;
                for (auto it = a_begin; it != a_end; ++it, ++passed_rows)
                {
                    auto local_max = std::max_element(it->begin(), it->end());
                    auto local_max_val = *local_max;
                    
                    if (local_max_val > global_max)
                    {
                        global_max = local_max_val;
                        auto index = std::distance(it->begin(), local_max) +
                            passed_rows * it->size();
                    }
                }
                return global_index;
            }

            primitive_argument_type argmax2d_x_axis(arg_type && arg_a) const
            {
                auto a = arg_a.matrix();

                matrix_row_iterator<decltype(a)> a_begin(a);
                matrix_row_iterator<decltype(a)> a_end(a, a.rows());

                std::vector<primitive_argument_type> result;
                for (auto it = a_begin; it != a_end; ++it)
                {
                    auto local_max = std::max_element(it->begin(), it->end());
                    auto index = std::distance(it->begin(), local_max);
                    result.emplace_back(std::move(index));
                }
                return result;
            }
            primitive_argument_type argmax2d_y_axis(arg_type && arg_a) const
            {

                auto a = arg_a.matrix();

                matrix_column_iterator<decltype(a)> a_begin(a);
                matrix_column_iterator<decltype(a)> a_end(a, a.columns());

                std::vector<primitive_argument_type> result;
                for (auto it = a_begin; it != a_end; ++it)
                {
                    auto local_max = std::max_element(it->begin(), it->end());
                    //auto index = std::distance(it->begin(), local_max);
                    //result.emplace_back(std::move(index));
                }
                return result;
            }

            primitive_argument_type argmax2d(args_type && args) const
            {
                // `axis` is optional
                if (args.size() == 1)
                {
                    // Option 1: Flatten and find max
                    return argmax2d_flatten(std::move(args[0]));
                }

                // `axis` must be a scalar if provided
                if (args[1].num_dimensions() != 0)
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "argmax::argmax2d",
                        generate_error_message(
                            "operand axis must be a scalar", name_, codename_));
                }
                int axis = args[1].scalar();
                // `axis` can only be -2, -1, 0, or 1
                if (axis < -2 || axis > 1)
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "argmax::argmax2d",
                        generate_error_message(
                            "operand axis can only be -2, -1, 0, or 1 for "
                            "an a operand that is 2d",
                            name_, codename_));
                }
                switch (axis)
                {
                // Option 2: Find max among rows
                case -2: HPX_FALLTHROUGH;
                case -0:
                    return argmax2d_x_axis(std::move(args[0]));

                // Option 3: Find max among columns
                case -1: HPX_FALLTHROUGH;
                case 1:
                    return argmax2d_y_axis(std::move(args[0]));

                default:
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "argmax::argmax2d",
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
                        "argmax::eval",
                        generate_error_message(
                            "the argmax primitive requires exactly one or two "
                                "operands",
                            name_, codename_));
                }

                for (auto const& i : operands)
                {
                    if (!valid(i))
                    {
                        HPX_THROW_EXCEPTION(hpx::bad_parameter,
                            "argmax::eval",
                            generate_error_message(
                                "the argmax primitive requires that the "
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
                            return this_->argmax0d(std::move(args));

                        case 1:
                            return this_->argmax1d(std::move(args));

                        case 2:
                            return this_->argmax2d(std::move(args));

                        default:
                            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                                "argmax::eval",
                                generate_error_message(
                                    "operand a has an invalid "
                                    "number of dimensions",
                                this_->name_, this_->codename_));
                        }
                    }),
                    // TODO: Check what value -1 is going to turn into.
                    // node_data of doubles?
                    detail::map_operands(
                        operands, functional::numeric_operand{}, args,
                        name_, codename_));
            }
        };
    }

    hpx::future<primitive_argument_type> argmax::eval(
        std::vector<primitive_argument_type> const& args) const
    {
        if (operands_.empty())
        {
            return std::make_shared<detail::argmax>(name_, codename_)
                ->eval(args, noargs);
        }
        return std::make_shared<detail::argmax>(name_, codename_)
            ->eval(operands_, args);
    }
}}}
