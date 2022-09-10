import numpy as np

npy_input_local = np.load('tmp6/input_local.npy')
npy_input_global = np.load('tmp6/input_global.npy')
npy_output_policy = np.load('tmp6/output_policy.npy')
npy_output_value = np.load('tmp6/output_value.npy')
npy_output_land = np.load('tmp6/output_land.npy')

npy_input_local = np.concatenate((npy_input_local, np.load('tmp7/input_local.npy')))
npy_input_global = np.concatenate((npy_input_global, np.load('tmp7/input_global.npy')))
npy_output_policy = np.concatenate((npy_output_policy, np.load('tmp7/output_policy.npy')))
npy_output_value = np.concatenate((npy_output_value, np.load('tmp7/output_value.npy')))
npy_output_land = np.concatenate((npy_output_land, np.load('tmp7/output_land.npy')))

npy_input_local = np.concatenate((npy_input_local, np.load('tmp8/input_local.npy')))
npy_input_global = np.concatenate((npy_input_global, np.load('tmp8/input_global.npy')))
npy_output_policy = np.concatenate((npy_output_policy, np.load('tmp8/output_policy.npy')))
npy_output_value = np.concatenate((npy_output_value, np.load('tmp8/output_value.npy')))
npy_output_land = np.concatenate((npy_output_land, np.load('tmp8/output_land.npy')))


npy_rows = len(npy_output_value)
perm = np.random.permutation(npy_rows)
npy_input_local = np.array_split(npy_input_local[perm], 4)
npy_input_global = np.array_split(npy_input_global[perm], 4)
npy_output_policy = np.array_split(npy_output_policy[perm], 4)
npy_output_value = np.array_split(npy_output_value[perm], 4)
npy_output_land = np.array_split(npy_output_land[perm], 4)


with open('input_local_20.npy', 'wb') as f:
    np.save(f, npy_input_local[0])

with open('input_global_20.npy', 'wb') as f:
    np.save(f, npy_input_global[0])

with open('output_policy_20.npy', 'wb') as f:
    np.save(f, npy_output_policy[0])

with open('output_value_20.npy', 'wb') as f:
    np.save(f, npy_output_value[0])

with open('output_land_20.npy', 'wb') as f:
    np.save(f, npy_output_land[0])


with open('input_local_21.npy', 'wb') as f:
    np.save(f, npy_input_local[1])

with open('input_global_21.npy', 'wb') as f:
    np.save(f, npy_input_global[1])

with open('output_policy_21.npy', 'wb') as f:
    np.save(f, npy_output_policy[1])

with open('output_value_21.npy', 'wb') as f:
    np.save(f, npy_output_value[1])

with open('output_land_21.npy', 'wb') as f:
    np.save(f, npy_output_land[1])


with open('input_local_22.npy', 'wb') as f:
    np.save(f, npy_input_local[2])

with open('input_global_22.npy', 'wb') as f:
    np.save(f, npy_input_global[2])

with open('output_policy_22.npy', 'wb') as f:
    np.save(f, npy_output_policy[2])

with open('output_value_22.npy', 'wb') as f:
    np.save(f, npy_output_value[2])

with open('output_land_22.npy', 'wb') as f:
    np.save(f, npy_output_land[2])


with open('input_local_23.npy', 'wb') as f:
    np.save(f, npy_input_local[3])

with open('input_global_23.npy', 'wb') as f:
    np.save(f, npy_input_global[3])

with open('output_policy_23.npy', 'wb') as f:
    np.save(f, npy_output_policy[3])

with open('output_value_23.npy', 'wb') as f:
    np.save(f, npy_output_value[3])

with open('output_land_23.npy', 'wb') as f:
    np.save(f, npy_output_land[3])
