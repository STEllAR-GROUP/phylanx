// Copyright (c) 2017-2018 Weile Wei
// Copyright (c) 2018 Parsa Amini
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/ir/dictionary.hpp>

#include <hpx/include/lcos.hpp>
#include <hpx/include/naming.hpp>
#include <hpx/include/util.hpp>
#include <hpx/throw_exception.hpp>

#include <memory>
#include <string>
#include <utility>
#include <vector>

#include <phylanx/execution_tree/primitives/primitive_argument_type.hpp>
#include <phylanx/util/variant.hpp>
#include <boost/functional/hash.hpp>

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace execution_tree { namespace primitives {

}}}

namespace std {
    std::size_t hash<phylanx::util::recursive_wrapper<
        phylanx::execution_tree::primitive_argument_type>>::
    operator()(argument_type const& s) const noexcept
    {
        phylanx::execution_tree::primitive_argument_type const& val = s.get();
        std::size_t result;
        switch (val.index())
        {
        case 0:    // ast::nil
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "phylanx::ir::dictionary",
                "unsupported nil data type");

        case 1:    // phylanx::ir::node_data<std::uint8_t>
        {
            phylanx::ir::node_data<std::uint8_t> val_temp =
                phylanx::util::get<1>(val);
            switch (val_temp.num_dimensions())
            {
            case 0:
                boost::hash<uint8_t> hash_uint8_t;
                return hash_uint8_t(val_temp[0]);
            case 1:
                HPX_FALLTHROUGH;
            case 2:
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "phylanx::ir::dictionary",
                    "unsupported none 0-dimension node data type");
            }
        }

        case 2:    // phylanx::ir::node_data<std::int64_t>
        {
            phylanx::ir::node_data<std::int64_t> val_temp =
                phylanx::util::get<2>(val);
            switch (val_temp.num_dimensions())
            {
            case 0:
                boost::hash<__int64_t> hash_int64_t;
                return hash_int64_t(val_temp[0]);
            case 1:
                HPX_FALLTHROUGH;
            case 2:
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "phylanx::ir::dictionary",
                    "unsupported none 0-dimension node data type");
            }
        }

        case 3:    // std::string
        {
            std::string val_temp = phylanx::util::get<3>(val);
            boost::hash<std::string> hash_string;
            return hash_string(val_temp);
        }

        case 4:    // phylanx::ir::node_data<double>
        {
            phylanx::ir::node_data<double> val_temp =
                phylanx::util::get<4>(val);
            switch (val_temp.num_dimensions())
            {
            case 0:
                boost::hash<double> hash_double;
                return hash_double(val_temp[0]);
            case 1:
                HPX_FALLTHROUGH;
            case 2:
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "phylanx::ir::dictionary",
                    "unsupported none 0-dimension node data type");
            }
        }

        case 5:    // primitive
        {
            hash hash_primitive;
            return hash_primitive(val);
        }

        case 6:    // std::vector<ast::expression>
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "phylanx::ir::dictionary",
                "unsupported std::vector<ast::expression> data type");
        }

        case 7:    // ir::range
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "phylanx::ir::dictionary",
                "unsupported ir::range data type");
        }

        case 8:    // phylanx::ir::dictionary
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "phylanx::ir::dictionary",
                "unsupported dictionary data type");
        }
        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "phylanx::ir::dictionary",
            "unsupported data type");
    }
}
