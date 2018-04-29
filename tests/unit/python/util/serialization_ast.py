#  Copyright (c) 2018 R. Tohid
#
#  Distributed under the Boost Software License, Version 1.0. (See accompanying
#  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

import phylanx
from phylanx.ast import *

@Phylanx("PhySL")
def foo(a):
    a += 2
    return a

foo_ast = generate_ast(foo.__src__)
foo_serialized = phylanx.util.serialize(foo_ast)
foo_unserialized = phylanx.util.unserialize(foo_serialized)
assert foo_ast == foo_unserialized
