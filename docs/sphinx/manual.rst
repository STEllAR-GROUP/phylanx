..
    Copyright (C) 2018 Mikael Simberg
    Copyright (C) 2018 Bibek Wagle

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

.. _manual:

=======
Manual
=======

A major challenge facing computer scientists in research and
industry today is the automatic analysis and pattern recognition
in large sets of data. To address this issue, a new field
known as machine learning has developed and produced
techniques and algorithms that enable machines to sort, categorize,
and make predictions based on provided datasets.

As machine learning tools have matured, so have the sizes of
data sets that scientist wish to analyze. To address the issue
researchers have turned to solutions provided by the
High Performance Computing (HPC) community to distribute
the execution of these applications to multiple nodes.
These techniques however require users be intimately familiar
with HPC in order to effectively make use of distributed
resources. In order to address this significant barrier
to domain scientist, our team has developed |phylanx|,
a machine learning platform which exposes a high level
Python API but provides HPC level performance.

In this document we describe |phylanx|, provide use case
examples, and document the components of the system.

.. toctree::
   :maxdepth: 2

   manual/about
