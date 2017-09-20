//  Copyright (c) 2017 Bibek Wagle
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/sub_operation.hpp>
#include <phylanx/ir/node_data.hpp>
#include <phylanx/util/serialization/eigen.hpp>

#include <hpx/include/util.hpp>
#include <hpx/include/lcos.hpp>

#include <cstddef>
#include <numeric>
#include <utility>
#include <vector>

namespace phylanx { namespace execution_tree { namespace primitives
{
    ///////////////////////////////////////////////////////////////////////////
    sub_operation::sub_operation(std::vector<primitive> && operands)
      : operands_(std::move(operands))
    {
        if (operands_.size() < 2)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "sub_operation::eval",
                "the sub_operation primitive requires at least two operands");
        }
    }

    sub_operation::sub_operation(std::vector<primitive> const& operands)
      : operands_(operands)
    {
        if (operands_.size() < 2)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "sub_operation::eval",
                "the sub_operation primitive requires at least two operands");
        }
    }

    ///////////////////////////////////////////////////////////////////////////
    ir::node_data<double> sub_operation::sub0d(operands_type const& ops) const
    {
        std::size_t rhs_dims = ops[1].num_dimensions();
        switch(rhs_dims)
        {
            case 0:
            {
                if(ops.size()==2)
                {
                    auto result = [&]() -> double {
                        auto val1 = *ops.begin();
                        auto val2 = *(++ops.begin());
                        return val1[0] - val2[0];
                    };
                    return ir::node_data<double>(result());
                }
                else
                {
                    auto first = *ops.begin();
                    return ir::node_data<double>(
                            std::accumulate(ops.begin()+1, ops.end(), first[0],
                                            [](double result, ir::node_data<double> const& curr)
                                            {
                                                return result - curr[0];
                                            }));
                }
            }
            case 1: HPX_FALLTHROUGH;
            case 2: HPX_FALLTHROUGH;
            default:
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                                    "sub_operation::sub0d",
                                    "the operands have incompatible number of dimensions");
        }
    }

    ///////////////////////////////////////////////////////////////////////////
    ir::node_data<double> sub_operation::sub1d1d(operands_type const& ops) const
    {
        std::size_t lhs_size = ops[0].dimension(0);
        std::size_t rhs_size = ops[1].dimension(0);

        if(lhs_size  != rhs_size)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "sub_operation::sub1d1d",
                "the dimensions of the operands do not match");
        }

        using array_type = Eigen::Array<double, Eigen::Dynamic, 1>;
        using result_type = Eigen::Matrix<double, Eigen::Dynamic, 1>;
        using array_map_type = Eigen::Map<array_type const>;

        if(ops.size()==2)
        {
            auto lambda = [&]() -> result_type {
                auto val1 = *ops.begin();
                auto val2 = *(++ops.begin());
                array_map_type first(val1.data(), lhs_size);
                array_map_type second(val2.data(), lhs_size);
                return first - second;
            };
            result_type result = lambda();
            return ir::node_data<double>(std::move(result));
        }
        else
        {
            auto val1 = *ops.begin();
            array_map_type first(val1.data(), lhs_size);
            Eigen::Matrix<double, Eigen::Dynamic, 1> result =
                    std::accumulate(ops.begin()+1, ops.end(),
                                    array_type{first},
                                    [&](array_type const& result, ir::node_data<double> const& curr)
                                            ->  array_type
                                    {
                                        Eigen::Map<array_type const> lhs(curr.data(), lhs_size);
                                        return result - lhs;
                                    });

            return ir::node_data<double>(std::move(result));

        }
    }

    ir::node_data<double> sub_operation::sub1d(operands_type const& ops) const
    {
        std::size_t rhs_dims = ops[1].num_dimensions();
        switch(rhs_dims)
        {
        case 1:
            return sub1d1d(ops);

        case 0: HPX_FALLTHROUGH;
        case 2: HPX_FALLTHROUGH;
        default:
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "sub_operation::sub1d",
                "the operands have incompatible number of dimensions");
        }
    }

    ///////////////////////////////////////////////////////////////////////////
    ir::node_data<double> sub_operation::sub2d2d(operands_type const& ops) const
    {
        auto lhs_size = ops[0].dimensions();
        auto rhs_size = ops[1].dimensions();

        if (lhs_size != rhs_size)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "sub_operation::sub2d2d",
                "the dimensions of the operands do not match");
        }

        using array_type = Eigen::Array<double, Eigen::Dynamic, Eigen::Dynamic>;
        using result_type = Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic>;
        using array_map_type = Eigen::Map<array_type const>;

        if(ops.size()==2)
        {
            auto lambda = [&]() -> result_type {
                auto val1 = *ops.begin();
                auto val2 = *(++ops.begin());
                array_map_type first(val1.data(), lhs_size[0], lhs_size[1]);
                array_map_type second(val2.data(), lhs_size[0], lhs_size[1]);
                return first - second;
            };
            result_type result = lambda();
            return ir::node_data<double>(std::move(result));
        }
        else
        {
            auto val1 = *ops.begin();
            array_map_type first(val1.data(), lhs_size[0], lhs_size[1]);
            Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic> result =
                    std::accumulate(
                            ops.begin()+1, ops.end(),array_type{first},
                            [&](array_type const& result, ir::node_data<double> const& curr)
                                    ->  array_type
                            {
                                return result -
                                       array_map_type{curr.data(), lhs_size[0], lhs_size[1]};
                            });

            return ir::node_data<double>(std::move(result));
        }
    }

    ir::node_data<double> sub_operation::sub2d(operands_type const& ops) const
    {
        std::size_t rhs_dims = ops[1].num_dimensions();
        switch(rhs_dims)
        {
        case 2:
            return sub2d2d(ops);

        case 0: HPX_FALLTHROUGH;
        case 1: HPX_FALLTHROUGH;
        default:
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "sub_operation::sub2d",
                "the operands have incompatible number of dimensions");
        }
    }

    namespace detail
    {
        template <typename T, typename F>
        auto map(std::vector<T> const& in, F && f)
        ->  std::vector<decltype(hpx::util::invoke(f, std::declval<T>()))>
        {
            std::vector<decltype(hpx::util::invoke(f, std::declval<T>()))> out;
            out.reserve(in.size());
            for (auto const& v : in)
            {
                out.push_back(hpx::util::invoke(f, v));
            }
            return out;
        }
    }

    // implement '-' for all possible combinations of lhs and rhs
    hpx::future<ir::node_data<double>> sub_operation::eval() const
    {
        return hpx::dataflow(hpx::util::unwrapping(
            [this](std::vector<ir::node_data<double>> && ops)
            {
                std::size_t lhs_dims = ops[0].num_dimensions();
                switch (lhs_dims)
                {
                case 0:
                    return sub0d(ops);

                case 1:
                    return sub1d(ops);

                case 2:
                    return sub2d(ops);

                default:
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "sub_operation::eval",
                        "left hand side operand has unsupported number of "
                        "dimensions");
                }
            }),
            detail::map(operands_,
                [](primitive const& p) -> hpx::future<ir::node_data<double>>
                {
                    return p.eval();
                })
        );
    }
}}}
