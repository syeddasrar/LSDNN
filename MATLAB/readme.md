Main_Simulation.m is used to generate datasets.
  Estimation_functions.m and Channel_functions.m are helper functions to Main_Simulation.m and main_dnn.m

LS_DNN_Training.py uses the datasets generated to train the DNN.
  The trained model is stored as .h5 file.
  Also mean and standard deviation values are stored to be used in normalization of input data.
  
main_dnn.m is the entire end-to-end transmitter receiver incorporating LS augmented DNN.
  It uses the trained model (.h5 file) and normalization parameters (mean and standard deviation values) for its operation. 
