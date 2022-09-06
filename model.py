import math
import numpy as np
import tensorflow as tf
from tensorflow.keras.models import Model
from tensorflow.keras.layers import Input, Reshape, Activation, Layer
from tensorflow.keras.layers import Conv2D, Dense, GlobalAveragePooling2D, GlobalMaxPooling2D
from tensorflow.keras.layers import add, Concatenate, Flatten
from tensorflow.keras.initializers import TruncatedNormal
from tensorflow.keras.losses import CategoricalCrossentropy, MeanSquaredError
#from keras.utils.vis_utils import plot_model

import tf2onnx
#import onnxruntime as rt

NUM_FEATURE_LOCAL = 21
NUM_INPUT_LOCAL = 81 * NUM_FEATURE_LOCAL
NUM_INPUT_GLOBAL = 11 #12
SHAPE_INPUT_LOCAL = (9, 9, NUM_FEATURE_LOCAL)
NUM_TARGET_POLICY = 8
NUM_OUTPUT_NOPASS = 81 * NUM_TARGET_POLICY
NUM_OUTPUT_POLICY = NUM_OUTPUT_NOPASS + 1
NUM_TARGET_SCORE = 8
#SHAPE_OUTPUT_SCORE = (NUM_TARGET_SCORE, 2)
#SHAPE_OUTPUT_LAND = (81,)

DEWEIGHT_INPUT_GLOBAL = 2 # without comment in KataGo source
DEWEIGHT_G2 = 4
DEWEIGHT_PASS = 8 # without comment in KataGo source
#LOSS_WEIGHT_SCORE_MEAN = 1
#LOSS_WEIGHT_SCORE_SQERR = 1
#LOSS_WEIGHT_LAND = 1

# 6-Block
NUM_CHANNEL_TRUNK = 96
NUM_CHANNEL_GPOOL = 32
NUM_CHANNEL_REGULAR = NUM_CHANNEL_TRUNK - NUM_CHANNEL_GPOOL
NUM_CHANNEL_HEAD = 32
NUM_CHANNEL_G2 = NUM_CHANNEL_HEAD * 2
NUM_CHANNEL_V2 = 48
BLOCKS = [
  ["rconv1", "regular"],
  ["rconv2", "regular"],
  ["rconv3", "gpool"],
  ["rconv4", "regular"],
  ["rconv5", "gpool"],
  ["rconv6", "regular"],
]

NUM_BLOCK = len(BLOCKS)
FIXUP_SCALE_2 = 1.0 / math.sqrt(NUM_BLOCK)
FIXUP_SCALE_4 = 1.0 / (NUM_BLOCK ** 0.25)

# https://github.com/Zelgunn/CustomKerasLayers/blob/8894328932cfc3805d93852c499c0b41e668cff3/layers/ResBlock.py

class ResidualMultiplier(Layer):
    def __init__(self, **kwargs):
        super(ResidualMultiplier, self).__init__(**kwargs)
        self.multiplier = None

    def build(self, input_shape):
        self.multiplier = self.add_weight(name="multiplier", shape=[], dtype=tf.float32, initializer=tf.ones_initializer)

    def call(self, inputs, **kwargs):
        return inputs * self.multiplier

    def compute_output_signature(self, input_signature):
        return input_signature

class ResidualBias(Layer):
    def __init__(self, **kwargs):
        super(ResidualBias, self).__init__(**kwargs)
        self.bias = None

    def build(self, input_shape):
        self.bias = self.add_weight(name="bias", shape=[], dtype=tf.float32, initializer=tf.zeros_initializer)

    def call(self, inputs, **kwargs):
        return inputs + self.bias

    def compute_output_signature(self, input_signature):
        pass

# Some internet rando claims standalone layers like above have performance impact, therefore wherever bias always follows convolution, I rely on use_bias instead

def herangzhen(n):
    return math.sqrt(2.0 / n)

def block_regular(layer_in):
    result = Activation('relu')(layer_in)
    first_conv_initializer = TruncatedNormal(stddev=(herangzhen(NUM_CHANNEL_TRUNK * 9) * FIXUP_SCALE_2))
    result = Conv2D(NUM_CHANNEL_TRUNK, (3, 3), padding='same', use_bias=False, kernel_initializer=first_conv_initializer)(result)
    result = Activation('relu')(ResidualBias()(ResidualMultiplier()(result)))
    result = Conv2D(NUM_CHANNEL_TRUNK, (3, 3), padding='same', use_bias=True, kernel_initializer='zeros')(result)
    return result

def block_gpool(layer_in):
    result = Activation('relu')(layer_in)
    regular_conv_initializer = TruncatedNormal(stddev=(herangzhen(NUM_CHANNEL_TRUNK * 9) * FIXUP_SCALE_2))
    regular_conv = Conv2D(NUM_CHANNEL_REGULAR, (3, 3), padding='same', use_bias=False, kernel_initializer=regular_conv_initializer)(result)
    gpool_conv_initializer = TruncatedNormal(stddev=(herangzhen(NUM_CHANNEL_TRUNK * 9) * FIXUP_SCALE_4))
    gpool_conv = Conv2D(NUM_CHANNEL_GPOOL, (3, 3), padding='same', use_bias=True, kernel_initializer=gpool_conv_initializer)(result)
    gpool_conv = Activation('relu')(gpool_conv)
    gpool_mean = GlobalAveragePooling2D()(gpool_conv)
    gpool_max = GlobalMaxPooling2D()(gpool_conv)
    result = Concatenate(axis=1)([gpool_mean, gpool_max])
    gpool_dense_initializer = TruncatedNormal(stddev=(herangzhen(NUM_CHANNEL_GPOOL * 2) * FIXUP_SCALE_4 * 0.5))
    result = Dense(NUM_CHANNEL_REGULAR, use_bias=False, kernel_initializer=gpool_dense_initializer)(result)
    result = add([regular_conv, result])
    result = Activation('relu')(ResidualBias()(ResidualMultiplier()(result)))
    result = Conv2D(NUM_CHANNEL_TRUNK, (3, 3), padding='same', use_bias=True, kernel_initializer='zeros')(result)
    return result

input_zlocal = Input(shape=(NUM_INPUT_LOCAL,))
input_local = Reshape(SHAPE_INPUT_LOCAL)(input_zlocal)
input_global = Input(shape=(NUM_INPUT_GLOBAL,))

initial_conv_initializer = TruncatedNormal(stddev=herangzhen(NUM_INPUT_LOCAL * 25))
trunk = Conv2D(NUM_CHANNEL_TRUNK, (5, 5), padding='same', use_bias=True, kernel_initializer=initial_conv_initializer)(input_local)

initial_global_initializer = TruncatedNormal(stddev=herangzhen(NUM_INPUT_GLOBAL * DEWEIGHT_INPUT_GLOBAL))
initial_global = Dense(NUM_CHANNEL_TRUNK, use_bias=False, kernel_initializer=initial_global_initializer)(input_global)
trunk = add([trunk, initial_global])

for i in range(NUM_BLOCK):
    if BLOCKS[i][1] == "gpool":
        trunk = add([trunk, block_gpool(trunk)])
    else:
        trunk = add([trunk, block_regular(trunk)])

trunk = Activation('relu')(trunk)

p1_initializer = TruncatedNormal(stddev=herangzhen(NUM_CHANNEL_TRUNK))
p1_conv = Conv2D(NUM_CHANNEL_HEAD, (1, 1), use_bias=True, kernel_initializer=p1_initializer)(trunk)
g1_initializer = TruncatedNormal(stddev=herangzhen(NUM_CHANNEL_TRUNK))
g1_conv = Conv2D(NUM_CHANNEL_HEAD, (1, 1), use_bias=True, kernel_initializer=g1_initializer)(trunk)
g1_conv = Activation('relu')(g1_conv)
g1_mean = GlobalAveragePooling2D()(g1_conv)
g1_max = GlobalMaxPooling2D()(g1_conv)
g2_layer = Concatenate(axis=1)([g1_mean, g1_max])
g3_initializer = TruncatedNormal(stddev=herangzhen(NUM_CHANNEL_G2 * DEWEIGHT_G2))
g3_layer = Dense(NUM_CHANNEL_HEAD, use_bias=False, kernel_initializer=g3_initializer)(g2_layer)
p1_sum = add([p1_conv, g3_layer])
p1_sum = Activation('relu')(p1_sum)
p2_initializer = TruncatedNormal(stddev=(herangzhen(NUM_CHANNEL_HEAD) * 0.3)) # yet another magic deweight
p2_conv = Conv2D(NUM_TARGET_POLICY, (1, 1), use_bias=False, kernel_initializer=p2_initializer)(p1_sum)
pass_initializer = TruncatedNormal(stddev=(herangzhen(NUM_CHANNEL_G2 * DEWEIGHT_PASS) * 0.3)) # XD
output_pass = Dense(1, use_bias=False, kernel_initializer=pass_initializer)(g2_layer)
output_policy = Reshape((NUM_OUTPUT_NOPASS,))(p2_conv)
output_policy = Concatenate(axis=1)([output_policy, output_pass])
output_policy = Activation('softmax', name='p')(output_policy)

v1_initializer = TruncatedNormal(stddev=herangzhen(NUM_CHANNEL_TRUNK))
v1_conv = Conv2D(NUM_CHANNEL_HEAD, (1, 1), use_bias=True, kernel_initializer=v1_initializer)(trunk)
v1_conv = Activation('relu')(v1_conv)
v1_mean = GlobalAveragePooling2D()(v1_conv)
v1_max = GlobalMaxPooling2D()(v1_conv)
v1_pool = Concatenate(axis=1)([v1_mean, v1_max])
v2_initializer = TruncatedNormal(stddev=herangzhen(NUM_CHANNEL_G2))
v2_layer = Dense(NUM_CHANNEL_V2, use_bias=True, kernel_initializer=v2_initializer)(v1_pool)
v2_layer = Activation('relu')(v2_layer)
v3_initializer = TruncatedNormal(stddev=herangzhen(NUM_CHANNEL_V2))
output_value = Dense(NUM_TARGET_SCORE, use_bias=True, kernel_initializer=v3_initializer, name='v')(v2_layer)

model = Model(inputs=[input_zlocal, input_global], outputs=[output_policy, output_value])

optimizer = tf.keras.optimizers.SGD(
    learning_rate=0.00006,
    momentum=0.9,
    nesterov=True,
    clipnorm=1.0)

model.compile(optimizer=tf.keras.optimizers.Adadelta(clipnorm=0.1), loss={'p': CategoricalCrossentropy(), 'v': MeanSquaredError()})

#rii = np.random.rand(1000, NUM_INPUT_LOCAL)
#rig = np.random.rand(1000, NUM_INPUT_GLOBAL)
#rop = np.zeros((1000, NUM_OUTPUT_POLICY))
#for i in range (1000):
    #rop[i][np.random.randint(0, NUM_OUTPUT_POLICY)] = 1
#rov = np.random.rand(1000, NUM_TARGET_SCORE)

#npy_input_local = np.load('input_local.npy')
#npy_input_global = np.load('input_global.npy')
#npy_output_policy = np.load('output_policy.npy')
#npy_output_value = np.load('output_value.npy')

#npy_input_local = np.concatenate((npy_input_local, np.load('legacy_data/input_local.npy')))
#npy_input_global = np.concatenate((npy_input_global, np.load('legacy_data/input_global.npy')))
#npy_output_policy = np.concatenate((npy_output_policy, np.load('legacy_data/output_policy.npy')))
#npy_output_value = np.concatenate((npy_output_value, np.load('legacy_data/output_value.npy')))

#npy_rows = len(npy_output_value)
#perm = np.random.permutation(npy_rows)
#npy_input_local = npy_input_local[perm]
#npy_input_global = npy_input_global[perm]
#npy_output_policy = npy_output_policy[perm]
#npy_output_value = npy_output_value[perm]

ds_feature = {'input_1': tf.io.FixedLenFeature((NUM_INPUT_LOCAL), tf.float32),
    'input_2': tf.io.FixedLenFeature((NUM_INPUT_GLOBAL), tf.float32),
    'p': tf.io.FixedLenFeature((NUM_OUTPUT_POLICY), tf.float32),
    'v': tf.io.FixedLenFeature((8), tf.float32),}
def decode(serialized_example):
  features = tf.io.parse_single_example(serialized_example, features=ds_feature)
  inputs = {}
  inputs['input_1'] = tf.cast(features['input_1'], tf.float32)
  inputs['input_2'] = tf.cast(features['input_2'], tf.float32)
  labels = {}
  labels['p'] = tf.cast(features['p'], tf.float32)
  labels['v'] = tf.cast(features['v'], tf.float32)
  return inputs, labels

def load_dataset(filenames):
    ignore_order = tf.data.Options()
    ignore_order.experimental_deterministic = False  # disable order, increase speed
    dataset = tf.data.TFRecordDataset(filenames).with_options(ignore_order)  # uses data as soon as it streams in, rather than in its original order
    dataset = dataset.map(decode, num_parallel_calls=tf.data.AUTOTUNE).prefetch(buffer_size=tf.data.AUTOTUNE).batch(256)
    # returns a dataset of (image, label) pairs if labeled=True or just images if labeled=False
    return dataset

dataset = load_dataset(['tb00x.tfrecord'])

model_checkpoint_callback = tf.keras.callbacks.ModelCheckpoint(
    filepath='checkpoint/cheapModel2_{loss}',
    save_weights_only=True,
    monitor='loss',
    mode='min',
    save_best_only=True,
    save_freq=16)

#plot_model(model, to_file='model_plot.png', show_shapes=True, show_layer_names=True)
#history = model.fit([npy_input_local, npy_input_global], [npy_output_policy, npy_output_value], epochs=100, batch_size=256, callbacks=[model_checkpoint_callback])
history = model.fit(x=dataset, epochs=100, callbacks=[model_checkpoint_callback])
#model.save('cheapModel2')

spec = (tf.TensorSpec((None, NUM_INPUT_LOCAL), tf.float32, name="input_1"), tf.TensorSpec((None, NUM_INPUT_GLOBAL), tf.float32, name="input_2"))
model_proto, _ = tf2onnx.convert.from_keras(model, input_signature=spec, opset=13, output_path='cheapModel2.onnx')
output_names = [n.name for n in model_proto.graph.output]
print(output_names)
