import numpy as np

npy_input_local = np.load('input_local.npy')
npy_input_global = np.load('input_global.npy')
npy_output_policy = np.load('output_policy.npy')
npy_output_value = np.load('output_value.npy')
npy_output_land = np.load('output_land.npy')

#npy_input_local = np.load('tb03v/input_local.npy')
#npy_input_global = np.load('tb03v/input_global.npy')
#npy_output_policy = np.load('tb03v/output_policy.npy')
#npy_output_value = np.load('tb03v/output_value.npy')
#npy_output_land = np.load('tb03v/output_land.npy')

#npy_input_local = np.concatenate((npy_input_local, np.load('input_local_00.npy')))
#npy_input_global = np.concatenate((npy_input_global, np.load('input_global_00.npy')))
#npy_output_policy = np.concatenate((npy_output_policy, np.load('output_policy_00.npy')))
#npy_output_value = np.concatenate((npy_output_value, np.load('output_value_00.npy')))
#npy_output_land = np.concatenate((npy_output_land, np.load('output_land_00.npy')))

#npy_input_local = np.concatenate((npy_input_local, np.load('input_local_20.npy')))
#npy_input_global = np.concatenate((npy_input_global, np.load('input_global_20.npy')))
#npy_output_policy = np.concatenate((npy_output_policy, np.load('output_policy_20.npy')))
#npy_output_value = np.concatenate((npy_output_value, np.load('output_value_20.npy')))
#npy_output_land = np.concatenate((npy_output_land, np.load('output_land_20.npy')))

#npy_input_local = np.concatenate((npy_input_local, np.load('input_local_30.npy')))
#npy_input_global = np.concatenate((npy_input_global, np.load('input_global_30.npy')))
#npy_output_policy = np.concatenate((npy_output_policy, np.load('output_policy_30.npy')))
#npy_output_value = np.concatenate((npy_output_value, np.load('output_value_30.npy')))
#npy_output_land = np.concatenate((npy_output_land, np.load('output_land_30.npy')))

npy_rows = len(npy_output_value)
perm = np.random.permutation(npy_rows)

npy_input_local = npy_input_local[perm]
npy_input_global = npy_input_global[perm]
npy_output_policy = npy_output_policy[perm]
npy_output_value = npy_output_value[perm]
npy_output_land = npy_output_land[perm]

#npy_input_local = np.array_split(npy_input_local, 4)
#npy_input_global = np.array_split(npy_input_global, 4)
#npy_output_policy = np.array_split(npy_output_policy, 4)
#npy_output_value = np.array_split(npy_output_value, 4)
#npy_output_land = np.array_split(npy_output_land, 4)

with open('tb09v.npz', 'wb') as f:
    np.savez(f, inputLocalNCHW=npy_input_local, inputGlobalNC=npy_input_global, outputPolicyNCHW=npy_output_policy, outputValueNC=npy_output_value, outputOwnershipNCHW=npy_output_land)

#with open('tb02b.npz', 'wb') as f:
    #np.savez(f, inputLocalNCHW=npy_input_local[1], inputGlobalNC=npy_input_global[1], outputPolicyNCHW=npy_output_policy[1], outputValueNC=npy_output_value[1], outputOwnershipNCHW=npy_output_land[1])

#with open('tb02c.npz', 'wb') as f:
    #np.savez(f, inputLocalNCHW=npy_input_local[2], inputGlobalNC=npy_input_global[2], outputPolicyNCHW=npy_output_policy[2], outputValueNC=npy_output_value[2], outputOwnershipNCHW=npy_output_land[2])

#with open('tb02d.npz', 'wb') as f:
    #np.savez(f, inputLocalNCHW=npy_input_local[3], inputGlobalNC=npy_input_global[3], outputPolicyNCHW=npy_output_policy[3], outputValueNC=npy_output_value[3], outputOwnershipNCHW=npy_output_land[3])
