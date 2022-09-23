import sys
import numpy as np

buckets = int(sys.argv[1])
raw_range = range(int(sys.argv[2]), int(sys.argv[3]))

first = raw_range[0]
npy_input_local = np.load('shuffle/raw/' + str(first) + '/input_local.npy')
npy_input_global = np.load('shuffle/raw/' + str(first) + '/input_global.npy')
npy_output_policy = np.load('shuffle/raw/' + str(first) + '/output_policy.npy')
npy_output_value = np.load('shuffle/raw/' + str(first) + '/output_value.npy')
npy_output_land = np.load('shuffle/raw/' + str(first) + '/output_land.npy')

for i in raw_range[1:]:
    npy_input_local = np.concatenate((npy_input_local, np.load('shuffle/raw/' + str(i) + '/input_local.npy')))
    npy_input_global = np.concatenate((npy_input_global, np.load('shuffle/raw/' + str(i) + '/input_global.npy')))
    npy_output_policy = np.concatenate((npy_output_policy, np.load('shuffle/raw/' + str(i) + '/output_policy.npy')))
    npy_output_value = np.concatenate((npy_output_value, np.load('shuffle/raw/' + str(i) + '/output_value.npy')))
    npy_output_land = np.concatenate((npy_output_land, np.load('shuffle/raw/' + str(i) + '/output_land.npy')))

npy_rows = len(npy_output_value)
perm = np.random.permutation(npy_rows)
npy_input_local = np.array_split(npy_input_local[perm], buckets)
npy_input_global = np.array_split(npy_input_global[perm], buckets)
npy_output_policy = np.array_split(npy_output_policy[perm], buckets)
npy_output_value = np.array_split(npy_output_value[perm], buckets)
npy_output_land = np.array_split(npy_output_land[perm], buckets)

for i in range(buckets):
    with open('shuffle/bucket/' + str(i) + '/input_local_' + str(first) + '.npy', 'wb') as f:
        np.save(f, npy_input_local[i])
    with open('shuffle/bucket/' + str(i) + '/input_global_' + str(first) + '.npy', 'wb') as f:
        np.save(f, npy_input_global[i])
    with open('shuffle/bucket/' + str(i) + '/output_policy_' + str(first) + '.npy', 'wb') as f:
        np.save(f, npy_output_policy[i])
    with open('shuffle/bucket/' + str(i) + '/output_value_' + str(first) + '.npy', 'wb') as f:
        np.save(f, npy_output_value[i])
    with open('shuffle/bucket/' + str(i) + '/output_land_' + str(first) + '.npy', 'wb') as f:
        np.save(f, npy_output_land[i])
