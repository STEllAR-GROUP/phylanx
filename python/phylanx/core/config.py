# Copyright (c) 2018 R. Tohid
#
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

from phylanx.core import init_hpx_runtime
from phylanx.exceptions import RuntimeAlreadyInitializedError


class PhylanxSession:
    cfg = [
        # make sure hpx_main is always executed
        "hpx.run_hpx_main!=1",
        # allow for unknown command line options
        "hpx.commandline.allow_unknown!=1",
        # disable HPX' short options
        "hpx.commandline.aliasing!=0",
        # by default run one thread only (for now)
        "hpx.os_threads!=1",
        # don't print diagnostics during forced terminate
        "hpx.diagnostics_on_terminate!=0",
        # disable the TCP parcelport
        "hpx.parcel.tcp.enable!=0"
    ]

    is_initialized = False

    @staticmethod
    def init(num_threads=1):
        if not PhylanxSession.is_initialized:
            hpx_thread = "hpx.os_threads!=%s" % num_threads
            PhylanxSession.cfg[3] = hpx_thread
            init_hpx_runtime(PhylanxSession.cfg)
            PhylanxSession.is_initialized = True
        else:
            raise RuntimeAlreadyInitializedError(
                "Phylanx is already initialized with %s" % PhylanxSession.cfg)

    @staticmethod
    def config(cfg):
        if not PhylanxSession.is_initialized:
            PhylanxSession.cfg = cfg
            init_hpx_runtime(cfg)
            PhylanxSession.is_initialized = True
        else:
            raise RuntimeAlreadyInitializedError(
                "Phylanx is already initialized with %s" % PhylanxSession.cfg)
