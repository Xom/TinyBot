from typing import Any, Dict, List
import math

from model_pytorch import Model, compute_gain

import torch
import torch.nn
import torch.nn.functional

def cross_entropy(pred_logits, target_probs, dim):
    return -torch.sum(target_probs * torch.nn.functional.log_softmax(pred_logits, dim=dim), dim=dim)

def huber_loss(x, y, delta):
    abs_diff = torch.abs(x - y)
    return torch.where(
        abs_diff > delta,
        (0.5 * delta * delta) + delta * (abs_diff - delta),
        0.5 * abs_diff * abs_diff,
    )

def constant_like(data, other_tensor):
    return torch.tensor(data, dtype=other_tensor.dtype, device=other_tensor.device, requires_grad=False)

class Metrics:
    def __init__(self, batch_size: int, raw_model: Model):
        self.n = batch_size
        #self.world_size = world_size
        self.pos_len = raw_model.pos_len
        self.pos_area = raw_model.pos_len * raw_model.pos_len
        self.policy_len = raw_model.pos_len * raw_model.pos_len * 8 + 1

    def state_dict(self):
        return dict()

    def load_state_dict(self, state_dict: Dict[str,Any]):
        pass

    def loss_policy_player_samplewise(self, pred_logits, target_probs):
        assert pred_logits.shape == (self.n, self.policy_len)
        assert target_probs.shape == (self.n, self.policy_len)
        loss = cross_entropy(pred_logits, target_probs, dim=1)
        return loss

    def loss_tiny_samplewise(self, pred, target):
        assert pred.shape == (self.n, 8)
        assert target.shape == (self.n, 8)
        loss = torch.sum(torch.square(pred - target), dim=1)
        return loss * 64

    def loss_ownership_samplewise(self, pred_pretanh, target):
        assert pred_pretanh.shape == (self.n, 1, 9, 9)
        assert target.shape == (self.n, 1, 9, 9)
        loss = torch.sum(torch.nn.functional.binary_cross_entropy_with_logits(pred_pretanh, target, reduction='none'), dim=[1,2,3])
        return loss * 0.015625

    def target_entropy(self, target_probs):
        return -torch.sum(torch.sum(target_probs * torch.log(target_probs + 1e-30), dim=-1))

    # Returns 0.5 times the sum of squared model weights, for each reg group of model weights
    def get_model_norms(self,raw_model):
        reg_dict : Dict[str,List] = {}
        raw_model.add_reg_dict(reg_dict)

        device = reg_dict["normal"][0].device
        dtype = torch.float32

        modelnorm_normal = torch.zeros([],device=device,dtype=dtype)
        modelnorm_normal_gamma = torch.zeros([],device=device,dtype=dtype)
        modelnorm_output = torch.zeros([],device=device,dtype=dtype)
        modelnorm_noreg = torch.zeros([],device=device,dtype=dtype)
        modelnorm_output_noreg = torch.zeros([],device=device,dtype=dtype)
        for tensor in reg_dict["normal"]:
            modelnorm_normal += torch.sum(tensor * tensor)
        for tensor in reg_dict["normal_gamma"]:
            modelnorm_normal_gamma += torch.sum(tensor * tensor)
        for tensor in reg_dict["output"]:
            modelnorm_output += torch.sum(tensor * tensor)
        for tensor in reg_dict["noreg"]:
            modelnorm_noreg += torch.sum(tensor * tensor)
        for tensor in reg_dict["output_noreg"]:
            modelnorm_output_noreg += torch.sum(tensor * tensor)
        modelnorm_normal *= 0.5
        modelnorm_normal_gamma *= 0.5
        modelnorm_output *= 0.5
        modelnorm_noreg *= 0.5
        modelnorm_output_noreg *= 0.5
        return (modelnorm_normal, modelnorm_normal_gamma, modelnorm_output, modelnorm_noreg, modelnorm_output_noreg)

    def get_specific_norms_and_gradient_stats(self,raw_model):
        with torch.no_grad():
            params = {}
            for name, param in raw_model.named_parameters():
                params[name] = param

            stats = {}
            def add_norm_and_grad_stats(name):
                param = params[name]
                if name.endswith(".weight"):
                    fanin = param.shape[1]
                elif name.endswith(".gamma"):
                    fanin = 1
                elif name.endwith(".beta"):
                    fanin = 1
                else:
                    assert False, "unimplemented case to compute stats on parameter"

                # 1.0 means that the average squared magnitude of a parameter in this tensor is around where
                # it would be at initialization, assuming it uses the activation that the model generally
                # uses (e.g. relu or mish)
                param_scale = torch.sqrt(torch.mean(torch.square(param))) / compute_gain(raw_model.activation) * math.sqrt(fanin)
                stats[f"{name}.SCALE_batch"] = param_scale

                # How large is the gradient, on the same scale?
                stats[f"{name}.GRADSC_batch"] = torch.sqrt(torch.mean(torch.square(param.grad))) / compute_gain(raw_model.activation) * math.sqrt(fanin)

                # And how large is the component of the gradient that is orthogonal to the overall magnitude of the parameters?
                orthograd = param.grad - param * (torch.sum(param.grad * param) / (1e-20 + torch.sum(torch.square(param))))
                stats[f"{name}.OGRADSC_batch"] = torch.sqrt(torch.mean(torch.square(orthograd))) / compute_gain(raw_model.activation) * math.sqrt(fanin)

            add_norm_and_grad_stats("blocks.1.normactconvp.conv.weight")
            add_norm_and_grad_stats("blocks.1.blockstack.0.normactconv1.conv.weight")
            add_norm_and_grad_stats("blocks.1.blockstack.0.normactconv2.conv.weight")
            add_norm_and_grad_stats("blocks.1.blockstack.1.normactconv2.norm.gamma")
            add_norm_and_grad_stats("blocks.1.normactconvq.conv.weight")
            add_norm_and_grad_stats("blocks.1.normactconvq.norm.gamma")

            add_norm_and_grad_stats("blocks.6.normactconvp.conv.weight")
            add_norm_and_grad_stats("blocks.6.blockstack.0.normactconv1.conv.weight")
            add_norm_and_grad_stats("blocks.6.blockstack.0.normactconv2.conv.weight")
            add_norm_and_grad_stats("blocks.6.blockstack.1.normactconv2.norm.gamma")
            add_norm_and_grad_stats("blocks.6.normactconvq.conv.weight")
            add_norm_and_grad_stats("blocks.6.normactconvq.norm.gamma")

            add_norm_and_grad_stats("blocks.10.normactconvp.conv.weight")
            add_norm_and_grad_stats("blocks.10.blockstack.0.normactconv1.conv.weight")
            add_norm_and_grad_stats("blocks.10.blockstack.0.normactconv2.conv.weight")
            add_norm_and_grad_stats("blocks.10.blockstack.1.normactconv2.norm.gamma")
            add_norm_and_grad_stats("blocks.10.normactconvq.conv.weight")
            add_norm_and_grad_stats("blocks.10.normactconvq.norm.gamma")

            add_norm_and_grad_stats("blocks.16.normactconvp.conv.weight")
            add_norm_and_grad_stats("blocks.16.blockstack.0.normactconv1.conv.weight")
            add_norm_and_grad_stats("blocks.16.blockstack.0.normactconv2.conv.weight")
            add_norm_and_grad_stats("blocks.16.blockstack.1.normactconv2.norm.gamma")
            add_norm_and_grad_stats("blocks.16.normactconvq.conv.weight")
            add_norm_and_grad_stats("blocks.16.normactconvq.norm.gamma")

            add_norm_and_grad_stats("policy_head.conv1p.weight")
            add_norm_and_grad_stats("value_head.conv1.weight")

        return stats

    def metrics_dict_batchwise(
        self,
        raw_model,
        model_outputs,
        batch,
        is_training,
        main_loss_scale,
    ):
        results = self.metrics_dict_batchwise_single_heads_output(
            raw_model,
            model_outputs[0],
            batch,
            is_training=is_training,
        )
        if main_loss_scale is not None:
            results["loss_sum"] = main_loss_scale * results["loss_sum"]

        return results


    def metrics_dict_batchwise_single_heads_output(
        self,
        raw_model,
        model_outputs,
        batch,
        is_training,
    ):
        (
            out_policy,
            out_miscvalue,
            out_ownership,
        ) = model_outputs

        outputPolicyNCHW = batch["outputPolicyNCHW"]
        outputValueNC = batch["outputValueNC"]
        outputOwnershipNCHW = batch["outputOwnershipNCHW"]

        loss_policy_player = self.loss_policy_player_samplewise(
            out_policy,
            outputPolicyNCHW,
        ).sum()

        loss_tiny = self.loss_tiny_samplewise(
            out_miscvalue,
            outputValueNC,
        ).sum()

        loss_ownership = self.loss_ownership_samplewise(
            out_ownership,
            outputOwnershipNCHW,
        ).sum()

        loss_sum = (
            loss_policy_player
            + loss_tiny
            + loss_ownership
        )

        results = {
            "p0loss_sum": loss_policy_player,
            "tinyloss_sum": loss_tiny,
            "oloss_sum": loss_ownership,
            "loss_sum": loss_sum,
        }

        nsamples = int(outputValueNC.shape[0])
        #policy_target_entropy = self.target_entropy(out_policy)

        (modelnorm_normal, modelnorm_normal_gamma, modelnorm_output, modelnorm_noreg, modelnorm_output_noreg) = self.get_model_norms(raw_model)

        extra_results = {
            "nsamp": nsamples,
            #"ptentr_sum": policy_target_entropy,
            "norm_normal_batch": modelnorm_normal,
            "norm_normal_gamma_batch": modelnorm_normal_gamma,
            "norm_output_batch": modelnorm_output,
            "norm_noreg_batch": modelnorm_noreg,
            "norm_output_noreg_batch": modelnorm_output_noreg,
        }
        for key,value in extra_results.items():
            results[key] = value
        return results
