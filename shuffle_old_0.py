import numpy as np

npy_input_local = np.load('tmp2/input_local.npy')
npy_input_global = np.load('tmp2/input_global.npy')
npy_output_policy = np.load('tmp2/output_policy.npy')
npy_output_value = np.load('tmp2/output_value.npy')
npy_output_land = np.load('tmp2/output_land.npy')

npy_input_local = np.concatenate((npy_input_local, np.load('tmp3/input_local.npy')))
npy_input_global = np.concatenate((npy_input_global, np.load('tmp3/input_global.npy')))
npy_output_policy = np.concatenate((npy_output_policy, np.load('tmp3/output_policy.npy')))
npy_output_value = np.concatenate((npy_output_value, np.load('tmp3/output_value.npy')))
npy_output_land = np.concatenate((npy_output_land, np.load('tmp3/output_land.npy')))

npy_input_local = np.concatenate((npy_input_local, np.load('tmp4/input_local.npy')))
npy_input_global = np.concatenate((npy_input_global, np.load('tmp4/input_global.npy')))
npy_output_policy = np.concatenate((npy_output_policy, np.load('tmp4/output_policy.npy')))
npy_output_value = np.concatenate((npy_output_value, np.load('tmp4/output_value.npy')))
npy_output_land = np.concatenate((npy_output_land, np.load('tmp4/output_land.npy')))


npy_rows = len(npy_output_value)
perm = np.random.permutation(npy_rows)
npy_input_local = np.array_split(npy_input_local[perm], 4)
npy_input_global = np.array_split(npy_input_global[perm], 4)
npy_output_policy = np.array_split(npy_output_policy[perm], 4)
npy_output_value = np.array_split(npy_output_value[perm], 4)
npy_output_land = np.array_split(npy_output_land[perm], 4)


with open('input_local_10.npy', 'wb') as f:
    np.save(f, npy_input_local[0])

with open('input_global_10.npy', 'wb') as f:
    np.save(f, npy_input_global[0])

with open('output_policy_10.npy', 'wb') as f:
    np.save(f, npy_output_policy[0])

with open('output_value_10.npy', 'wb') as f:
    np.save(f, npy_output_value[0])

with open('output_land_10.npy', 'wb') as f:
    np.save(f, npy_output_land[0])


with open('input_local_11.npy', 'wb') as f:
    np.save(f, npy_input_local[1])

with open('input_global_11.npy', 'wb') as f:
    np.save(f, npy_input_global[1])

with open('output_policy_11.npy', 'wb') as f:
    np.save(f, npy_output_policy[1])

with open('output_value_11.npy', 'wb') as f:
    np.save(f, npy_output_value[1])

with open('output_land_11.npy', 'wb') as f:
    np.save(f, npy_output_land[1])


with open('input_local_12.npy', 'wb') as f:
    np.save(f, npy_input_local[2])

with open('input_global_12.npy', 'wb') as f:
    np.save(f, npy_input_global[2])

with open('output_policy_12.npy', 'wb') as f:
    np.save(f, npy_output_policy[2])

with open('output_value_12.npy', 'wb') as f:
    np.save(f, npy_output_value[2])

with open('output_land_12.npy', 'wb') as f:
    np.save(f, npy_output_land[2])


with open('input_local_13.npy', 'wb') as f:
    np.save(f, npy_input_local[3])

with open('input_global_13.npy', 'wb') as f:
    np.save(f, npy_input_global[3])

with open('output_policy_13.npy', 'wb') as f:
    np.save(f, npy_output_policy[3])

with open('output_value_13.npy', 'wb') as f:
    np.save(f, npy_output_value[3])

with open('output_land_13.npy', 'wb') as f:
    np.save(f, npy_output_land[3])
