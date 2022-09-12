import numpy as np

z = [
    np.load('tb02a.npz'),
    np.load('tb02b.npz'),
    np.load('tb02c.npz'),
    np.load('tb02d.npz')
    ]

for i in range(4):
    print(z[i]['inputLocalNCHW'].shape)
    print(z[i]['inputGlobalNC'].shape)
    print(z[i]['outputPolicyNCHW'].shape)
    print(z[i]['outputValueNC'].shape)
    print(z[i]['outputOwnershipNCHW'].shape)
