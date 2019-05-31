..
    Copyright (C) 2018 Hartmut Kaiser
    Copyright (C) 2018 Bibek Wagle
    Copyright (C) 2019 Adrain Serio


    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

============================
About |phylanx|
============================

Phylanx is a distribured Machine Learning platform
that aims to provide users a high level, Python interface
which delivers HPC performance. At its core Phylanx is
a general purpose system for computing large distributed
arrays often used on applied statistics problems.
Phylanx builds upon other solutions such as Spartan,
Theano, and Tensorflow in an effort to generalize array operations
specifically to support distributed computing. The system will
decompose array computations into a predefined set of parallel
operations and employ algorithms which optimize execution and data
layout from of a user provided expression graph. Using hints from
the user as well as applicaiton logic an expression graph is
created which is then passed to HPX, a distributed C++
runtime system used to schedule and execute the work on
commodity systems.

Phylanx improves upon previous solutions by adding more sophisticated
algorithms to optimize data layout, distribution, and tiling on
HPC systems, and by using cache-oblivious data layouts to improve the
overall performance and scalability of the provided expression evaluations.
