#  Copyright (c) 2020 Hartmut Kaiser
#
#  Distributed under the Boost Software License, Version 1.0. (See accompanying
#  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

from phylanx import Phylanx, PhylanxSession
import time

###############################################################################
# initialize Phylanx
PhylanxSession.init(1)


# silence flake
def sleep(x):
    pass


@Phylanx
def call_sleep():
    sleep(1000)             # sleep for 1 second
    return


def call_sleep_async():
    # note: launch() requires for locality to be a named argument
    #       if locality=... is not specified it will execute the operation
    #       locally
    return call_sleep.launch(locality=0)  # run on locality 0


start = time.time()
f1 = call_sleep_async()      # asynchronously start call_sleep
f2 = call_sleep_async()      # asynchronously start call_sleep again

f1()    # wait for f1
f2()    # wait for f2
end = time.time()

# the overall execution time should be smaller than 2 seconds
assert((end - start) < 2.0)
