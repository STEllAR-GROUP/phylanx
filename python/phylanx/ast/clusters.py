# Copyright (c) 2018 Christopher Taylor 
#
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
import requests

class cluster(object):
    def __init_(self, args):
        self.args = args

    def send(self):
       raise NotImplementedError("cluster::send not impl error.")

class k8cluster(cluster):
    def __init__(self, args):
        cluster.__init__(self, args)

    @overrides(cluster)
    def sends(self):
        resp = requests.post(url=self.args['url'], args=self.args)
        data = resp.json()

def create_cluster(args):
    cluster_type = args['type']
    if cluster_type == 'k8' or cluster_type == 'kubernetes':
        if 'config' not in args:
            raise NotImplementedError("k8cluster config missing.")
        return k8cluster(args['config'])
