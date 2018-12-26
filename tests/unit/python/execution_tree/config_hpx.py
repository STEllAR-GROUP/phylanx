#  Copyright (c) 2018 R. Tohid
#
#  Distributed under the Boost Software License, Version 1.0. (See accompanying
#  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

from phylanx import Phylanx, PhylanxSession

cfg = [
    "hpx.run_hpx_main!=1", "hpx.commandline.allow_unknown!=1",
    "hpx.commandline.aliasing!=0", "hpx.os_threads!=4",
    "hpx.diagnostics_on_terminate!=0", "hpx.parcel.tcp.enable!=0"
]


@Phylanx
def foo():
    a = 2
    return a


def main():
    assert (foo() == 2)


if __name__ == "__main__":
    PhylanxSession.config(cfg)
    main()
