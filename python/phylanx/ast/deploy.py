from kubernetes import client, config, utils
from kubernetes.client.rest import ApiException

# uses this library
# https://github.com/litl/backoff
#
import backoff 

class deployer(object):
    '''
    data structure to manage posting physl code
    deployments
    '''
    def __init__(self, physl_str):
        self.physl = physl_str
        self.result = None

    def __call__(self):
        pass

    def set_result(self, result):
        self.result = result

    def result(self):
        return self.result

class kubernetes_deployer(deployer):
    '''
    data structure to manage posting physl code
    to a kubernetes cluster
    '''
    def __init__(self, physl_str, pod_manifest, base=10, cap=300):
        deployer.__init__(self, physl_str)
        self.manifest = pod_manifest
        self.name = pod_manifest['metadata']['name']
        self.base = 10
        self.cap = cap

    @backoff.on_predicate(backoff.expo, lambda x: x.status.phase != 'Pending'):
    def post_loop(self):
        '''
        posts using exponential backoff
        '''
        return api.read_namespaced_pod(name=self.name, namespace='default')

    @override
    def __call__(self):
        config.load_kube_config()
        c = Configuration()
        c.assert_hostname = False
        Configuration.set_default(c)
        api = core_v1_api.CoreV1Api()

        resp = None
        try:
            resp = self.post_loop()
        except ApiException as e:
            if e.status != 404:
                print('unknown error: %s' % (str(e),))

        self.set_result(resp)
