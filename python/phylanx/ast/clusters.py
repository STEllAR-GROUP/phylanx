# Copyright (c) 2019 Christopher Taylor 
#
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

'''
see this documentation:

https://kubernetes.io/docs/tasks/administer-cluster/access-cluster-api/#accessing-the-cluster-api
https://kubernetes.io/docs/tasks/inject-data-application/define-environment-variable-container/
https://github.com/kubernetes-client/python/blob/master/examples/exec.py 
'''

import requests
from base64 import b64encode

class cluster(object):
    def __init_(self, args):
        self.args = args

    def send(self):
       raise NotImplementedError("cluster::send not implemented.")

class k8cluster(cluster):
    def __init__(self, args):
        cluster.__init__(self, args)

    @overrides(cluster)
    def sends(self, physl_ir_str):
        if 'jobconfig' not in self.args:
            raise Error("k8cluster jobconfig not defined.")
        if 'url' not in self.args:
            raise Error("k8cluster url not defined.")

        env_ir = [ {'name' : 'PHYSL_IR', 'value' : b64encode(physl_ir_str) } ]

        if 'env' not in self.args['jobconfig']['spec']['containers']:
            self.args['jobconfig']['spec']['containers']['env'] = list()
        
        self.args['jobconfig']['spec']['containers']['env'].append(env_ir)

        resp = requests.post(url=self.args['url'], args=self.args['jobconfig'])
        return resp.json()

def create_cluster(args):
    if 'type' not in args:
        raise Error("create_cluster type not defined.")

    cluster_type = args['type']

    if 'jobconfig' not in args:
        raise Error("create_cluster jobconfig not defined.")

    if cluster_type == 'k8' or cluster_type == 'kubernetes':
        return k8cluster(args['jobconfig'])
