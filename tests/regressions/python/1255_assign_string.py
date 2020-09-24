#  Copyright (c) 2020 Patrick Diehl
#
#  Distributed under the Boost Software License, Version 1.0. (See accompanying
#  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

# #1255: Passing a dict to Phylanx is not possible

from phylanx import Phylanx


dict_nn = {
    "key " : "value"
}


@Phylanx(debug=True)
def change(data):
    data["key"] = 42.0
    data["key"] = 42
    data["key"] = "new value"
    return data


result = change(dict_nn)
assert result['key'] == 'new value', result
