# Copyright (c) 2019 Christopher Taylor
#
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

# uses this library: https://github.com/kubernetes-client/python
#
import kubernetes.client
from kubernetes.client import Configuration
from kubernetes.client.apis import core_v1_api
from kubernetes.client.rest import ApiException

# uses this library: https://github.com/litl/backoff
#
import backoff


class deployer(object):
    '''
    data structure to manage posting physl code
    deployments
    '''
    def __init__(self, physl_str):
        self.physl = physl_str

    def __call__(self):
        pass


class deployedresult(object):
    '''
    data structure to manage posting physl code
    deployments
    '''
    def __init__(self):
        pass

    def get(self):
        pass


class kubernetes_deployer_result(deployedresult):
    '''
    data structure to manage getting result of
    physl code deployed to a kubernetes cluster
    '''
    def __init__(self, kubernetes_pod_deploy_response,
                 kubernetes_config, log_path):
        deployedresult.__init__(self)
        self.kpod_response = kubernetes_pod_deploy_response
        self.kpod_config = kubernetes_config
        self.success = False
        self.log_path = log_path

        if self.kpod_response is not None:
            self.success = True

    def get(self):
        api_instance = kubernetes.client.LogsApi(
             kubernetes.client.ApiClient(self.kpod_config))

        thread = None
        try:
            thread = api_instance.log_file_handler_with_http_info(self.logpath)
        except ApiException as e:
            print("Exception when calling LogsApi->log_file_handler: %s\n" % e)

        if thread is not None:
            return thread.get()

        return None


class kubernetes_deployer(deployer):
    '''
    data structure to manage posting physl code
    to a kubernetes cluster
    '''
    def __init__(self, physl_str, pod_manifest,
                 kubernetes_config, base=10, cap=300):
        deployer.__init__(self, physl_str)
        self.manifest = pod_manifest
        self.kpod_config = kubernetes_config
        self.name = pod_manifest['metadata']['name']
        self.base = base
        self.cap = cap

    @backoff.on_predicate(backoff.expo, lambda x: x.status.phase != 'Pending')
    def post_pod_loop(self):
        '''
        posts using exponential backoff
        '''
        return self.api.read_namespaced_pod(
                   name=self.name, namespace='default')

    def __call__(self):
        Configuration.set_default(self.kpod_config)
        self.api = core_v1_api.CoreV1Api()

        resp = None
        try:
            resp = self.post_pod_loop()
        except ApiException as e:
            if e.status != 404:
                print('unknown error: %s' % (str(e),))
                return None

        return kubernetes_deployer_result(resp)
