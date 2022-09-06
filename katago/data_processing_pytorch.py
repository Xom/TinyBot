import logging
import os

import numpy as np

import torch
import torch.nn.functional

import modelconfigs

def read_npz_training_data(
    npz_files,
    batch_size: int,
    #world_size: int,
    rank: int,
    pos_len: int,
    device,
    model_config: modelconfigs.ModelConfig,
):
    rand = np.random.default_rng(seed=list(os.urandom(12)))
    num_bin_features = modelconfigs.get_num_bin_input_features(model_config)
    num_global_features = modelconfigs.get_num_global_input_features(model_config)

    for npz_file in npz_files:
        with np.load(npz_file) as npz:
            inputLocalNCHW = npz["inputLocalNCHW"]
            inputGlobalNC = npz["inputGlobalNC"]
            outputPolicyNCHW = npz["outputPolicyNCHW"]
            outputValueNC = npz["outputValueNC"]
            outputOwnershipNCHW = npz["outputOwnershipNCHW"]
            #scoreDistrN = npz["scoreDistrN"].astype(np.float32)
            #valueTargetsNCHW = npz["valueTargetsNCHW"].astype(np.float32)
        del npz

        assert inputLocalNCHW.shape[1] == num_bin_features
        assert inputGlobalNC.shape[1] == num_global_features
        assert outputPolicyNCHW.shape[1] == 649
        assert outputValueNC.shape[1] == 8
        assert outputOwnershipNCHW.shape[1] == 1

        num_samples = inputLocalNCHW.shape[0]
        # Just discard stuff that doesn't divide evenly
        num_whole_steps = num_samples // batch_size

        logging.info(f"Beginning {npz_file} with {num_whole_steps} usable batches, my rank is {rank}")
        for n in range(num_whole_steps):
            start = (n + rank) * batch_size
            end = start + batch_size

            batch_inputLocalNCHW = torch.from_numpy(inputLocalNCHW[start:end]).to(device)
            batch_inputGlobalNC = torch.from_numpy(inputGlobalNC[start:end]).to(device)
            batch_outputPolicyNCHW = torch.from_numpy(outputPolicyNCHW[start:end]).to(device)
            batch_outputValueNC = torch.from_numpy(outputValueNC[start:end]).to(device)
            batch_outputOwnershipNCHW = torch.from_numpy(outputOwnershipNCHW[start:end]).to(device)

            #(batch_binaryInputNCHW, batch_globalInputNC) = apply_history_matrices(
                #model_config, batch_binaryInputNCHW, batch_globalInputNC, batch_globalTargetsNC, h_base, h_builder
            #)

            batch_inputLocalNCHW = batch_inputLocalNCHW.contiguous()
            batch_inputGlobalNC = batch_inputGlobalNC.contiguous()
            batch_outputPolicyNCHW = batch_outputPolicyNCHW.contiguous()
            batch_outputValueNC = batch_outputValueNC.contiguous()
            batch_outputOwnershipNCHW = batch_outputOwnershipNCHW.contiguous()

            batch = dict(
                inputLocalNCHW = batch_inputLocalNCHW,
                inputGlobalNC = batch_inputGlobalNC,
                outputPolicyNCHW = batch_outputPolicyNCHW,
                outputValueNC = batch_outputValueNC,
                outputOwnershipNCHW = batch_outputOwnershipNCHW,
            )
            yield batch
