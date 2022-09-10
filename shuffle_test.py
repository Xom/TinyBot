import numpy as np

z = [
    np.load('tb01m.npz'),
    np.load('tb01n.npz'),
    np.load('tb01o.npz'),
    np.load('tb01p.npz')
    ]

for i in range(4):
    print(z[i]['inputLocalNCHW'].shape)
    print(z[i]['inputGlobalNC'].shape)
    print(z[i]['outputPolicyNCHW'].shape)
    print(z[i]['outputValueNC'].shape)
    print(z[i]['outputOwnershipNCHW'].shape)
