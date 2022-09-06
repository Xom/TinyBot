import numpy as np

z = [
    np.load('tb00m.npz'),
    np.load('tb00n.npz'),
    np.load('tb00o.npz'),
    np.load('tb00p.npz')
    ]

for i in range(4):
    print(z[i]['inputLocalNCHW'].shape)
    print(z[i]['inputGlobalNC'].shape)
    print(z[i]['outputPolicyNCHW'].shape)
    print(z[i]['outputValueNC'].shape)
    print(z[i]['outputOwnershipNCHW'].shape)
