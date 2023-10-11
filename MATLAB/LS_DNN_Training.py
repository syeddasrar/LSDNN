import numpy as np
from keras.initializers import TruncatedNormal
from keras.layers import Dense
from keras.models import Sequential
from sklearn.model_selection import train_test_split
from sklearn.preprocessing import StandardScaler
import h5py
from keras.callbacks import ModelCheckpoint
import math
import matplotlib.pyplot as plt

s = np.arange(1, 22)
for snr in s:
    # Load Matlab DataSets
    mat = h5py.File('./results/Dataset_{}.mat'.format(snr), 'r')
    X = np.array(mat['Preamble_Error_Correction_Dataset']['X'])
    Y = np.array(mat['Preamble_Error_Correction_Dataset']['Y'])
    print('Loaded Dataset Inputs: ', X.shape)
    print('Loaded Dataset Outputs: ', Y.shape)

    # Normalizing Datasets
    scalerx = StandardScaler()
    scalerx.fit(X)
    scalery = StandardScaler()
    scalery.fit(Y)
    XS = scalerx.transform(X)
    YS = scalery.transform(Y)
    # save the mean and standard deviation values to file
    # to be used in HLS
    a = np.zeros(104)
    b = np.zeros(104)
    for i in range(scalerx.mean_.shape[0]):
        a[i] = scalerx.mean_[i]
    np.savetxt('./results/DNN{}/mean.dat'.format(snr), a, fmt='%f', newline=',')
    for i in range(scalerx.mean_.shape[0]):
        b[i] = math.sqrt(scalerx.var_[i])
    np.savetxt('./results/DNN{}/std.dat'.format(snr), b, fmt='%f', newline=',')
    for i in range(scalery.mean_.shape[0]):
        a[i] = scalery.mean_[i]
    np.savetxt('./results/DNN{}/mean_o.dat'.format(snr), a, fmt='%f', newline=',')
    for i in range(scalery.mean_.shape[0]):
        b[i] = math.sqrt(scalery.var_[i])
    np.savetxt('./results/DNN{}/std_o.dat'.format(snr), b, fmt='%f', newline=',')


    # Split Data into train and test sets
    seed = 52
    train_X, test_X, train_Y, test_Y = train_test_split(XS, YS, test_size=0.02, random_state=seed)
    print('Training samples: ', train_X.shape[0])
    print('Testing samples: ', test_X.shape[0])

    # Build the model.
    init = TruncatedNormal(mean=0.0, stddev=0.05, seed=None)
    model = Sequential([
        Dense(units=52, activation='relu', input_dim=104,
              kernel_initializer=init,
              bias_initializer=init),
        Dense(units=104, kernel_initializer=init,
              bias_initializer=init)
    ])

    # Compile the model.
    model.compile(loss='mean_squared_error', optimizer='adam', metrics=['acc'])
    print(model.summary())
    model_path = './results/model/LS_DNN_{}.h5'.format(snr)

    # This check point saves the best DNN model with highest validation accuracy
    checkpoint = ModelCheckpoint(model_path, monitor='val_acc',
                                 verbose=1, save_best_only=True,
                                 mode='max')
    callbacks_list = [checkpoint]
    # Train the model.
    epoch = 10
    batch_size = 32

    model.fit(train_X, train_Y, epochs=epoch, batch_size=batch_size, verbose=2, validation_split=0.25,  callbacks=callbacks_list)

