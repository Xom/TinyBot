import numpy as np

npy_input_local = np.load('input_local_03.npy')
npy_input_global = np.load('input_global_03.npy')
npy_output_policy = np.load('output_policy_03.npy')
npy_output_value = np.load('output_value_03.npy')
npy_output_land = np.load('output_land_03.npy')

npy_input_local = np.concatenate((npy_input_local, np.load('input_local_13.npy')))
npy_input_global = np.concatenate((npy_input_global, np.load('input_global_13.npy')))
npy_output_policy = np.concatenate((npy_output_policy, np.load('output_policy_13.npy')))
npy_output_value = np.concatenate((npy_output_value, np.load('output_value_13.npy')))
npy_output_land = np.concatenate((npy_output_land, np.load('output_land_13.npy')))

npy_input_local = np.concatenate((npy_input_local, np.load('input_local_23.npy')))
npy_input_global = np.concatenate((npy_input_global, np.load('input_global_23.npy')))
npy_output_policy = np.concatenate((npy_output_policy, np.load('output_policy_23.npy')))
npy_output_value = np.concatenate((npy_output_value, np.load('output_value_23.npy')))
npy_output_land = np.concatenate((npy_output_land, np.load('output_land_23.npy')))

#npy_input_local = np.concatenate((npy_input_local, np.load('input_local_33.npy')))
#npy_input_global = np.concatenate((npy_input_global, np.load('input_global_33.npy')))
#npy_output_policy = np.concatenate((npy_output_policy, np.load('output_policy_33.npy')))
#npy_output_value = np.concatenate((npy_output_value, np.load('output_value_33.npy')))
#npy_output_land = np.concatenate((npy_output_land, np.load('output_land_33.npy')))

npy_rows = len(npy_output_value)
perm = np.random.permutation(npy_rows)

npy_input_local = npy_input_local[perm]
npy_input_global = npy_input_global[perm]
npy_output_policy = npy_output_policy[perm]
npy_output_value = npy_output_value[perm]
npy_output_land = npy_output_land[perm]

npy_input_local = np.array_split(npy_input_local, 4)
npy_input_global = np.array_split(npy_input_global, 4)
npy_output_policy = np.array_split(npy_output_policy, 4)
npy_output_value = np.array_split(npy_output_value, 4)
npy_output_land = np.array_split(npy_output_land, 4)

with open('tb01m.npz', 'wb') as f:
    np.savez(f, inputLocalNCHW=npy_input_local[0], inputGlobalNC=npy_input_global[0], outputPolicyNCHW=npy_output_policy[0], outputValueNC=npy_output_value[0], outputOwnershipNCHW=npy_output_land[0])

with open('tb01n.npz', 'wb') as f:
    np.savez(f, inputLocalNCHW=npy_input_local[1], inputGlobalNC=npy_input_global[1], outputPolicyNCHW=npy_output_policy[1], outputValueNC=npy_output_value[1], outputOwnershipNCHW=npy_output_land[1])

with open('tb01o.npz', 'wb') as f:
    np.savez(f, inputLocalNCHW=npy_input_local[2], inputGlobalNC=npy_input_global[2], outputPolicyNCHW=npy_output_policy[2], outputValueNC=npy_output_value[2], outputOwnershipNCHW=npy_output_land[2])

with open('tb01p.npz', 'wb') as f:
    np.savez(f, inputLocalNCHW=npy_input_local[3], inputGlobalNC=npy_input_global[3], outputPolicyNCHW=npy_output_policy[3], outputValueNC=npy_output_value[3], outputOwnershipNCHW=npy_output_land[3])
