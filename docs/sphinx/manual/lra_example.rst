..
    Copyright (C) 2018 Mikael Simberg
    Copyright (C) 2019 Adrian Serio

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

==============
LRA Example
==============

In this section we demonstrate using |phylanx| with the Python frontend.
In addition, power users can also interface with |phylanx| by writing
PhySL code or C++ directly. We will describe these interfaces in
following sections.

----------------------------------------
Using |phylanx| with the Python Frontend
----------------------------------------

In order to take advantage of |phylanx|, users can decorate their
Python code with Python decorators which will interpret and execute the
code block in |phylanx|. An example of this technique can be found
in the LRA (Linear Regression Algorithm) example below.

.. literalinclude:: ../../../../examples/algorithms/lra/phylanx_lra_csv_np.py
     :language: python

As shown in the example above, users import |phylanx| and
encapsulate their code in a @Phylanx decorator. The NumPy
commands will additionally be converted into |phylanx| primitives
when placed in a |phylanx| decorator.
When encapsulated, the code will be transformed into AST which
represents the operations to be performed.

.. Todo: Mention command to print out PhySL

--------------------------
Using |phylanx| with PhySL
--------------------------

.. note:: This section is for power users. We encourage
  our users to take advantage of the user friendly Python
  Frontend. All functionality made available through this
  interface is as available in our Python Frontend.

The |phylanx| AST can be directly constructed using PhySL,
or **Phy**\ lanx **S**\ pecification **L**\ anguage. This small language
allows users to compile a raw string which produces
an AST when it is compiled.

An example of using this interface can be found below:

.. literalinclude:: ../../../../examples/algorithms/lra/lra_csv.cpp
     :language: cpp

