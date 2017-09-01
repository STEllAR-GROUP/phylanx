//  Copyright (c) 2001-2010 Joel de Guzman
//  Copyright (c) 2001-2017 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/phylanx.hpp>
#include <phylanx/ast/parser/expression_def.hpp>

#include <string>

using iterator_type = std::string::const_iterator;
template struct phylanx::ast::parser::expression<iterator_type>;
