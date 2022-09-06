import numpy as np

npy_input_local = np.load('tmp0/input_local.npy')
npy_input_global = np.load('tmp0/input_global.npy')
npy_output_policy = np.load('tmp0/output_policy.npy')
npy_output_value = np.load('tmp0/output_value.npy')
npy_output_land = np.load('tmp0/output_land.npy')

npy_input_local = np.concatenate((npy_input_local, np.load('tmp1/input_local.npy')))
npy_input_global = np.concatenate((npy_input_global, np.load('tmp1/input_global.npy')))
npy_output_policy = np.concatenate((npy_output_policy, np.load('tmp1/output_policy.npy')))
npy_output_value = np.concatenate((npy_output_value, np.load('tmp1/output_value.npy')))
npy_output_land = np.concatenate((npy_output_land, np.load('tmp1/output_land.npy')))

npy_input_local = np.concatenate((npy_input_local, np.load('tmp2/input_local.npy')))
npy_input_global = np.concatenate((npy_input_global, np.load('tmp2/input_global.npy')))
npy_output_policy = np.concatenate((npy_output_policy, np.load('tmp2/output_policy.npy')))
npy_output_value = np.concatenate((npy_output_value, np.load('tmp2/output_value.npy')))
npy_output_land = np.concatenate((npy_output_land, np.load('tmp2/output_land.npy')))


npy_rows = len(npy_output_value)
perm = np.random.permutation(npy_rows)
npy_input_local = np.array_split(npy_input_local[perm], 4)
npy_input_global = np.array_split(npy_input_global[perm], 4)
npy_output_policy = np.array_split(npy_output_policy[perm], 4)
npy_output_value = np.array_split(npy_output_value[perm], 4)
npy_output_land = np.array_split(npy_output_land[perm], 4)


with open('input_local_00.npy', 'wb') as f:
    np.save(f, npy_input_local[0])

with open('input_global_00.npy', 'wb') as f:
    np.save(f, npy_input_global[0])

with open('output_policy_00.npy', 'wb') as f:
    np.save(f, npy_output_policy[0])

with open('output_value_00.npy', 'wb') as f:
    np.save(f, npy_output_value[0])

with open('output_land_00.npy', 'wb') as f:
    np.save(f, npy_output_land[0])


with open('input_local_01.npy', 'wb') as f:
    np.save(f, npy_input_local[1])

with open('input_global_01.npy', 'wb') as f:
    np.save(f, npy_input_global[1])

with open('output_policy_01.npy', 'wb') as f:
    np.save(f, npy_output_policy[1])

with open('output_value_01.npy', 'wb') as f:
    np.save(f, npy_output_value[1])

with open('output_land_01.npy', 'wb') as f:
    np.save(f, npy_output_land[1])


with open('input_local_02.npy', 'wb') as f:
    np.save(f, npy_input_local[2])

with open('input_global_02.npy', 'wb') as f:
    np.save(f, npy_input_global[2])

with open('output_policy_02.npy', 'wb') as f:
    np.save(f, npy_output_policy[2])

with open('output_value_02.npy', 'wb') as f:
    np.save(f, npy_output_value[2])

with open('output_land_02.npy', 'wb') as f:
    np.save(f, npy_output_land[2])


with open('input_local_03.npy', 'wb') as f:
    np.save(f, npy_input_local[3])

with open('input_global_03.npy', 'wb') as f:
    np.save(f, npy_input_global[3])

with open('output_policy_03.npy', 'wb') as f:
    np.save(f, npy_output_policy[3])

with open('output_value_03.npy', 'wb') as f:
    np.save(f, npy_output_value[3])

with open('output_land_03.npy', 'wb') as f:
    np.save(f, npy_output_land[3])
