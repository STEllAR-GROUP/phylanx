//  Copyright (c) 2017 Alireza Kheirkhahan
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/file_write_csv.hpp>
#include <phylanx/ir/node_data.hpp>

#include <hpx/include/lcos.hpp>
#include <hpx/include/naming.hpp>
#include <hpx/include/util.hpp>
#include <hpx/throw_exception.hpp>

#include <cstddef>
#include <fstream>
#include <memory>
#include <string>
#include <vector>
#include <utility>

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace execution_tree { namespace primitives
{
    ///////////////////////////////////////////////////////////////////////////
    primitive create_file_write_csv(hpx::id_type const& locality,
        std::vector<primitive_argument_type>&& operands, std::string const& name)
    {
        static std::string type("file_write_csv");
        return create_primitive_component(
            locality, type, std::move(operands), name);
    }

    match_pattern_type const file_write_csv::match_data =
    {
        hpx::util::make_tuple("file_write_csv",
            std::vector<std::string>{"file_write_csv(_1, _2)"},
            &create_file_write_csv, &create_primitive<file_write_csv>)
    };

    ///////////////////////////////////////////////////////////////////////////
    file_write_csv::file_write_csv(
            std::vector<primitive_argument_type> && operands)
      : primitive_component_base(std::move(operands))
    {
    }

    namespace detail
    {
        struct file_write_csv : std::enable_shared_from_this<file_write_csv>
        {
            file_write_csv() = default;

        protected:
            void write_to_file_csv(ir::node_data<double> const& val)
            {
                std::ofstream outfile(
                    filename_.c_str(), std::ios::out | std::ios::trunc);
                if (!outfile.is_open())
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "phylanx::execution_tree::primitives::"
                            "file_write_csv::eval",
                        "couldn't open file: " + filename_);
                }

                outfile << std::setprecision(
                    std::numeric_limits<long double>::digits10 + 1);
                outfile << std::scientific;

                switch (val.num_dimensions())
                {
                case 0:
                    outfile << val.scalar() << '\n';
                    break;

                case 1:
                    {
                        auto v = val.vector();
                        for (std::size_t i = 0UL; i != v.size(); ++i)
                        {
                            if (i != 0)
                            {
                                outfile << ',';
                            }
                            outfile << v[i];
                        }
                        outfile << '\n';
                    }
                    break;

                case 2:
                    {
                        auto matrix = val.matrix();
                        for (std::size_t i = 0UL; i != matrix.rows(); ++i)
                        {
                            outfile << matrix(i, 0);
                            for (std::size_t j = 1UL; j != matrix.columns(); ++j)
                            {
                                outfile << ',' << matrix(i, j);
                            }
                            outfile << '\n';
                        }
                    }
                    break;
                }
            }

        public:
            hpx::future<primitive_argument_type> eval(
                std::vector<primitive_argument_type> const& operands,
                std::vector<primitive_argument_type> const& args)
            {
                if (operands.size() != 2)
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "phylanx::execution_tree::primitives::file_write::"
                            "file_write_csv",
                        "the file_write primitive requires exactly two operands");
                }

                if (!valid(operands[0]) || !valid(operands[1]))
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "phylanx::execution_tree::primitives::file_write::"
                            "file_write_csv",
                        "the file_write primitive requires that the given "
                            "operands are valid");
                }

                filename_ = string_operand_sync(operands[0], args);

                auto this_ = this->shared_from_this();
                return numeric_operand(operands[1], args)
                    .then(hpx::util::unwrapping(
                        [this_](ir::node_data<double> && val)
                        ->  primitive_argument_type
                        {
                            this_->write_to_file_csv(val);
                            return primitive_argument_type(std::move(val));
                        }));
            }

        private:
            std::string filename_;
            primitive_argument_type operand_;
        };
    }

    ///////////////////////////////////////////////////////////////////////////
    // write data to given file in CSV format and return content
    hpx::future<primitive_argument_type> file_write_csv::eval(
        std::vector<primitive_argument_type> const& args) const
    {
        if (operands_.empty())
        {
            return std::make_shared<detail::file_write_csv>()->eval(args, noargs);
        }

        return std::make_shared<detail::file_write_csv>()->eval(operands_, args);
    }
}}}
