import sys
import numpy as np

bucket_path = 'shuffle/bucket/' + sys.argv[1] + '/'

first = sys.argv[2]
npy_input_local = np.load(bucket_path + 'input_local_' + first + '.npy')
npy_input_global = np.load(bucket_path + 'input_global_' + first + '.npy')
npy_output_policy = np.load(bucket_path + 'output_policy_' + first + '.npy')
npy_output_value = np.load(bucket_path + 'output_value_' + first + '.npy')
npy_output_land = np.load(bucket_path + 'output_land_' + first + '.npy')

for s in sys.argv[3:]:
    npy_input_local = np.concatenate((npy_input_local, np.load(bucket_path + 'input_local_' + s + '.npy')))
    npy_input_global = np.concatenate((npy_input_global, np.load(bucket_path + 'input_global_' + s + '.npy')))
    npy_output_policy = np.concatenate((npy_output_policy, np.load(bucket_path + 'output_policy_' + s + '.npy')))
    npy_output_value = np.concatenate((npy_output_value, np.load(bucket_path + 'output_value_' + s + '.npy')))
    npy_output_land = np.concatenate((npy_output_land, np.load(bucket_path + 'output_land_' + s + '.npy')))

npy_rows = len(npy_output_value)
perm = np.random.permutation(npy_rows)

npy_input_local = npy_input_local[perm]
npy_input_global = npy_input_global[perm]
npy_output_policy = npy_output_policy[perm]
npy_output_value = npy_output_value[perm]
npy_output_land = npy_output_land[perm]

with open('shuffle/tb04_' + sys.argv[1] + '.npz', 'wb') as f:
    np.savez(f, inputLocalNCHW=npy_input_local, inputGlobalNC=npy_input_global, outputPolicyNCHW=npy_output_policy, outputValueNC=npy_output_value, outputOwnershipNCHW=npy_output_land)

with open('shuffle/tb04_' + sys.argv[1] + '.json', 'w') as f:
    f.write('{"num_rows":' + str(npy_rows) + '}')
