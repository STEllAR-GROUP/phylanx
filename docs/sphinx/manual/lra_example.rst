..
    Copyright (C) 2018 Mikael Simberg
    Copyright (C) 2019 Adrian Serio

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

==============
LRA Example
==============

In this section we demonstrate an example using the |phylanx| frontend.
As mentioned in the previous section, users can decorate their Pyton
code with |phylanx| decorators which will interpret and execute the
code block in |phylanx|. An example of this technique can be found
in the LRA (Linnear Reression Algorithm) example below.

.. literalinclude:: ../../../../examples/algorithms/lra/phylanx_lra_csv_np.py
     :language: python

As you can see in the example above, by importing |phylanx| and
encapulating the desired functionality in the @phylanx decorator
users can gain all the benefits of |phylanx|.
