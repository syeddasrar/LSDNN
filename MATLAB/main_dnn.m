%% To test trained model stored as .h5 file
clc;clearvars;close all;
ch_func = Channel_functions();
est_func = Estimation_functions();
%% Simulation Parameters (IEEE 802.11p)
modelIndex = 13;                    % Training set SNR index
modelfile = ['./results/model/LS_DNN_',num2str(modelIndex),'.h5'];

net = importKerasNetwork(modelfile);
mui = readmatrix(['.\results\dnn',num2str(modelIndex),'\mean.dat'],'delimiter',',');
sigi = readmatrix(['.\results\dnn',num2str(modelIndex),'\std.dat'],'delimiter',',');
muo = readmatrix(['.\results\dnn',num2str(modelIndex),'\mean_o.dat'],'delimiter',',');
sigo = readmatrix(['.\results\dnn',num2str(modelIndex),'\std_o.dat'],'delimiter',',');

mui=mui';
sigi=sigi';
muo=muo';
sigo=sigo';

N_CH     = 1000;                        % Number of channel realizations. Keep it 10000
ChType   = 'RTV';                       % Channel model   
fs       = 64*156250;                   % Sampling frequency in Hz, here case of 802.11p with 64 subcarriers and 156250 Hz subcarrier spacing
fc       = 5.2e9;                       % Carrier Frequecy in Hz.
v        = 0;                           % Moving speed of user in km/h
c        = 3e8;                         % Speed of Light in m/s
fD       = (v/3.6)/c*fc;                % Doppler freq in Hz
rchan    = ch_func.GenFadingChannel(ChType, fD, fs); % Channel generation
ofdmBW   = 10 * 10^6 ;                  % OFDM bandwidth (Hz)
nFFT     = 64;                          % FFT size 
nDSC     = 48;                          % Number of data subcarriers
nPSC     = 4;                           % Number of pilot subcarriers
nZSC     = 12;                          % Number of zeros subcarriers
nUSC     = nDSC + nPSC;                 % Number of total used subcarriers
K        = nUSC + nZSC;                 % Number of total subcarriers
nSym     = 10;                          % Number of OFDM symbols within one frame
K_cp     = K/4; 
qpskmod = comm.QPSKModulator;
qpskdemod = comm.QPSKDemodulator;

% Pre-defined preamble in frequency domain
dp       = [ 0  0 0 0 0 0 +1 +1 -1 -1 +1  +1 -1  +1 -1 +1 +1 +1 +1 +1 +1 -1 -1 +1 +1 -1 +1 -1 +1 +1 +1 +1 0 +1 -1 -1 +1 +1 -1 +1 -1 +1 -1 -1 -1 -1 -1 +1 +1 -1 -1 +1 -1 +1 -1 +1 +1 +1 +1 0 0 0 0 0];
dp       = fftshift(dp); 
Kset     = find(dp~=0);                % Active subcarriers
Kon      = length(Kset);               % Number of active subcarriers
Ep       = 1;                          % preamble power
dp       = sqrt(Ep)*dp.';              % Normalization
xp       = ifft(dp);                   % Frequency-Time conversion
xp_cp    = [xp(end-K_cp+1:end); xp];   % Adding CP 
SNR_p    = (-50:5:50)';                  % SNR range
N_SNR    = length(SNR_p);              % SNR length
N0       = Ep/K*10.^(-SNR_p/10);       % Noise power : snr_p = Ep/KN0 => N0 = Ep/(K*snr_p)

preamble_80211p        = repmat(xp_cp,1,2);         % IEEE 802.11p preamble symbols (tow symbols)
pilots_locations       = [8,22,44,58].'; % Pilot subcarriers positions
pilots                 = [1 1 1 -1].';
data_locations         = [2:7, 9:21, 23:27, 39:43, 45:57, 59:64].'; % Data subcarriers positions
ppositions             = [7,21, 32,46].';                           % Pilots positions in Kset
dpositions             = [1:6, 8:20, 22:31, 33:45, 47:52].';        % Data positions in Kset

Err_Ls   = zeros(N_SNR,1);             
Err_MMSE = zeros(N_SNR,1);             
Err_MMSE_1 = zeros(N_SNR,1);
Err_MMSE_2 = zeros(N_SNR,1);          
Err_MMSE_3 = zeros(N_SNR,1);
Err_Dnn = zeros(N_SNR,1);              
Phf      = zeros(N_SNR,1);             % Average channel power E(|hf|^2)

%% ------ Bits Modulation Technique------------------------------------------
mod                       = 'QPSK';
if(strcmp(mod,'QPSK') == 1)
     nBitPerSym            = 2; 
elseif (strcmp(mod,'16QAM') == 1)
     nBitPerSym            = 4; 
elseif (strcmp(mod,'64QAM') == 1)
     nBitPerSym            = 6; 
end
M                     = 2 ^ nBitPerSym; % QAM Modulation Order   
Pow                   = mean(abs(qammod(0:(M-1),M)).^2); % Normalization factor for QAM  

%% Rh estimation
release(rchan);
init_seed   = 5;
rchan.Seed  = init_seed;
Rh          = est_func.Estimat_Rh(rchan, K_cp, K, Kset);
release(rchan);

rchan1    = ch_func.GenFadingChannel('VTV-UC', fD, fs);
release(rchan1);
init_seed   = 5;
rchan1.Seed  = init_seed;
Rh1          = est_func.Estimat_Rh(rchan1, K_cp, K, Kset);
release(rchan1);

rchan2    = ch_func.GenFadingChannel('VTV', fD, fs);
release(rchan2);
init_seed   = 5;
rchan2.Seed  = init_seed;
Rh2          = est_func.Estimat_Rh(rchan2, K_cp, K, Kset);
release(rchan2);

rchan3    = ch_func.GenFadingChannel('RTV-UC', fD, fs);
release(rchan3);
init_seed   = 5;
rchan3.Seed  = init_seed;
Rh3          = est_func.Estimat_Rh(rchan3, K_cp, K, Kset);
release(rchan3);

rchan.Seed  = init_seed;
rchan1.Seed  = init_seed;
rchan2.Seed  = init_seed;
rchan3.Seed  = init_seed;

Ber_Ideal = zeros(N_SNR,1);
Ber_LS = zeros(N_SNR,1);
Ber_DNN = zeros(N_SNR,1);
Ber_DNN1 = zeros(N_SNR,1);
Ber_MMSE = zeros(N_SNR);
Ber_MMSE_1 = zeros(N_SNR);
Ber_MMSE_2 = zeros(N_SNR);
Ber_MMSE_3 = zeros(N_SNR);

%% Main Simulation 
MMSE_Snr =zeros(1,N_SNR,Kon);
Hfe_LSDNN = zeros(N_SNR, N_CH,Kon);
Hfe_LS = zeros(N_SNR, N_CH ,Kon);
Hf =  zeros(N_SNR , N_CH, Kon);
Hfe_MMSE = zeros(N_SNR, N_CH, Kon);
Hfe_MMSE_1 = zeros(N_SNR, N_CH, Kon);
Hfe_MMSE_2 = zeros(N_SNR, N_CH, Kon);
Hfe_MMSE_3 = zeros(N_SNR, N_CH, Kon);

for n_snr = 1:N_SNR
    disp(['Running Simulation, SNR = ', num2str(SNR_p(n_snr))]);
    tic; 
    % MMSE filter varies with SNR
    W = est_func.MMSE_matrix (dp(Kset), Rh, Ep/N0(n_snr)/K);
    W1 = est_func.MMSE_matrix (dp(Kset), Rh1, Ep/N0(n_snr)/K);
    W2 = est_func.MMSE_matrix (dp(Kset), Rh2, Ep/N0(n_snr)/K);
    W3 = est_func.MMSE_matrix (dp(Kset), Rh3, Ep/N0(n_snr)/K);

    MMSE_Snr(:,n_snr) =Ep/N0(n_snr)/K; 
    for n_ch = 1:N_CH % loop over channel realizations
        % Data generation
        N = nDSC * nSym;
        data = randi([0 1], N, nBitPerSym);
        Data = reshape(data, [], 1);
        dataSym = bi2de(data);
        
        % QPSK modulator
        QPSK_symbol =1/sqrt(Pow) * qammod(dataSym,M);
        Modulated_Bits = reshape(QPSK_symbol, nDSC, nSym);
        
        % OFDM Frame Generation
        OFDM_Frame = zeros(K,nSym);
        OFDM_Frame(data_locations,:) = Modulated_Bits;
        OFDM_Frame(pilots_locations,:) = repmat(pilots,1,nSym);
        % Taking FFT, the term (nFFT/sqrt(nDSC)) is for normalizing the power of transmit symbol to 1 
        IFFT_Data = (K/sqrt(nUSC))*ifft(OFDM_Frame);
        
        % Appending cylic prefix
        CP_Coded = IFFT_Data((K - K_cp +1):K,:);
        IFFT_Data_CP = [CP_Coded; IFFT_Data];
        % Appending preamble symbol 
        IFFT_Data_CP_Preamble = [ preamble_80211p IFFT_Data_CP];
        
        % ideal preamble estimation
        [ h, y ] = ch_func.ApplyChannel( rchan, IFFT_Data_CP_Preamble, K_cp);
        release(rchan);
        rchan.Seed = rchan.Seed+1;
        
        yp = y((K_cp+1):end,:);
        h = h((K_cp+1):end,:);
        yfp = fft(yp); % FD preamble
        hf = fft(h); % Fd channel
        Phf(n_snr)  = Phf(n_snr)  + norm(hf(Kset))^2;
        
        %add noise
        yfp_r = yfp+ sqrt(K*N0(n_snr))*ch_func.GenRandomNoise([1,1], 1);        
        
        %LS estimation
        [hfe_ls1, err_ls] = est_func.LS(yfp_r(Kset,1), dp(Kset), hf(Kset,1));
        Err_Ls (n_snr) = Err_Ls (n_snr) + err_ls;
        
        [hfe_ls2, err_ls] = est_func.LS(yfp_r(Kset,2), dp(Kset), hf(Kset,2));
        hfe_ls = (hfe_ls1+hfe_ls2)/2;
        H_LS = repmat(hfe_ls,1,nSym);
        
        %LSDNN Channel Estimation
        X1(1:52,:)     = real(hfe_ls1);
        X1(52+1:2*52,:) = imag(hfe_ls1);
        X11 =((X1-mui)./sigi)';
        DNN_output = predict(net,X11).';
        DNN_output_denorm = ((DNN_output.*sigo)+muo);
        testing_real_Y = DNN_output_denorm(1:52,:);
        testing_imag_Y = DNN_output_denorm(53:104,:);
        he_DNN_Preamble1 = testing_real_Y + 1i *  testing_imag_Y; 
        err_dnn = norm(he_DNN_Preamble1-hf(Kset,1))^2;
        
        Err_Dnn (n_snr) = Err_Dnn (n_snr) + err_dnn;
        
        X2(1:52,:)     = real(hfe_ls2);
        X2(52+1:2*52,:) = imag(hfe_ls2);
        X22 = ((X2-mui)./sigi)';
        DNN_output = predict(net,X22).';
        DNN_output_denorm = ((DNN_output.*sigo)+muo);
        testing_real_Y = DNN_output_denorm(1:52,:);
        testing_imag_Y = DNN_output_denorm(53:104,:);
        he_DNN_Preamble2 = testing_real_Y + 1i *  testing_imag_Y; 
        
        hfe_lsdnn = (he_DNN_Preamble1+he_DNN_Preamble2)./2;
        H_DNN = repmat(hfe_lsdnn,1,nSym);
 
        %MMSE estimation
        [hfe_mmse, err_mmse] = est_func.MMSE(yfp_r(Kset,1), W, hf(Kset,1));
        Err_MMSE (n_snr) = Err_MMSE (n_snr) + err_mmse;
        H_MMSE = repmat(hfe_mmse,1,nSym);

        %MMSE_1 estimation
        [hfe_mmse1, err_mmse1] = est_func.MMSE(yfp_r(Kset,1), W1, hf(Kset,1));
        Err_MMSE_1 (n_snr) = Err_MMSE_1 (n_snr) + err_mmse1;
        H_MMSE_1 = repmat(hfe_mmse1,1,nSym);

        %MMSE_2 estimation
        [hfe_mmse2, err_mmse2] = est_func.MMSE(yfp_r(Kset,1), W2, hf(Kset,1));
        Err_MMSE_2 (n_snr) = Err_MMSE_2 (n_snr) + err_mmse2;
        H_MMSE_2 = repmat(hfe_mmse2,1,nSym);

        %MMSE_3 estimation
        [hfe_mmse3, err_mmse3] = est_func.MMSE(yfp_r(Kset,1), W3, hf(Kset,1));
        Err_MMSE_3 (n_snr) = Err_MMSE_3 (n_snr) + err_mmse3;
        H_MMSE_3 = repmat(hfe_mmse3,1,nSym);
        
        % save the channels for further use
        Hf(n_snr ,n_ch, :) = hf(Kset,1).';
        Hfe_LSDNN(n_snr,n_ch, :) = hfe_lsdnn.';
        Hfe_LS(n_snr,n_ch, :) = hfe_ls.';
        Hfe_MMSE(n_snr,n_ch, :) = hfe_mmse;
        Hfe_MMSE_1(n_snr,n_ch, :) = hfe_mmse1;
        Hfe_MMSE_2(n_snr,n_ch, :) = hfe_mmse2;
        Hfe_MMSE_3(n_snr,n_ch, :) = hfe_mmse3;
        
        % Equalization
        y_Ideal = yfp_r(data_locations ,3:end) ./ hf(data_locations,3:end); %Ideal
        y_LS = yfp_r(data_locations ,3:end)./ H_LS(dpositions,:); % LS
        y_MMSE = yfp_r(data_locations ,3:end)./ H_MMSE(dpositions,:);
        y_DNN = yfp_r(data_locations ,3:end)./ H_DNN(dpositions,:);        
        y_MMSE_1 = yfp_r(data_locations ,3:end)./ H_MMSE_1(dpositions,:);
        y_MMSE_2 = yfp_r(data_locations ,3:end)./ H_MMSE_2(dpositions,:);
        y_MMSE_3 = yfp_r(data_locations ,3:end)./ H_MMSE_3(dpositions,:);
        
        demod_y_ideal = reshape(y_Ideal, [], 1);
        demod_y_LS = reshape(y_LS, [], 1);
        demod_y_MMSE = reshape(y_MMSE, [], 1);
        demod_y_DNN = reshape(y_DNN, [], 1);
        demod_y_MMSE_1 = reshape(y_MMSE_1, [], 1);
        demod_y_MMSE_2 = reshape(y_MMSE_2, [], 1);
        demod_y_MMSE_3 = reshape(y_MMSE_3, [], 1);
        
        %demodulation
        De_Mapped_Ideal = qamdemod(sqrt(Pow) * demod_y_ideal,M);
        De_Mapped_LS = qamdemod(sqrt(Pow) * demod_y_LS,M);
        De_Mapped_MMSE = qamdemod(sqrt(Pow) * demod_y_MMSE,M);
        De_Mapped_DNN = qamdemod(sqrt(Pow) * demod_y_DNN,M);
        De_Mapped_MMSE_1 = qamdemod(sqrt(Pow) * demod_y_MMSE_1,M);
        De_Mapped_MMSE_2 = qamdemod(sqrt(Pow) * demod_y_MMSE_2,M);
        De_Mapped_MMSE_3 = qamdemod(sqrt(Pow) * demod_y_MMSE_3,M);
        
        sym_ideal = de2bi(De_Mapped_Ideal);
        sym_LS = de2bi(De_Mapped_LS);
        sym_MMSE = de2bi(De_Mapped_MMSE);
        sym_DNN = de2bi(De_Mapped_DNN);
        sym_MMSE_1 = de2bi(De_Mapped_MMSE_1);
        sym_MMSE_2 = de2bi(De_Mapped_MMSE_2);
        sym_MMSE_3 = de2bi(De_Mapped_MMSE_3);
        
        bits_ideal = reshape(sym_ideal, [], 1);
        bits_LS = reshape(sym_LS, [], 1);
        bits_MMSE = reshape(sym_MMSE, [], 1);
        bits_DNN = reshape(sym_DNN, [], 1);
        bits_MMSE_1 = reshape(sym_MMSE_1, [], 1);
        bits_MMSE_2 = reshape(sym_MMSE_2, [], 1);
        bits_MMSE_3 = reshape(sym_MMSE_3, [], 1);
        
        % Bits Extraction
        Bits_Ideal = zeros(nDSC,nSym,log2(M));
        Bits_LS     = zeros(nDSC,nSym,log2(M));
        Bits_MMSE     = zeros(nDSC,nSym,log2(M));
        Bits_DNN     = zeros(nDSC,nSym,log2(M));
        Bits_MMSE_1     = zeros(nDSC,nSym,log2(M));
        Bits_MMSE_2     = zeros(nDSC,nSym,log2(M));
        Bits_MMSE_3     = zeros(nDSC,nSym,log2(M));
        
        % BER Calculation
        ber_Ideal  = biterr(Data,bits_ideal);       
        ber_LS     = biterr(Data,bits_LS);
        ber_MMSE     = biterr(Data,bits_MMSE);
        ber_DNN     = biterr(Data,bits_DNN);
        ber_MMSE_1     = biterr(Data,bits_MMSE_1);
        ber_MMSE_2     = biterr(Data,bits_MMSE_2);
        ber_MMSE_3     = biterr(Data,bits_MMSE_3);
        
        Ber_Ideal (n_snr) = Ber_Ideal (n_snr) + ber_Ideal;
        Ber_LS (n_snr)    = Ber_LS (n_snr) + ber_LS;
        Ber_MMSE (n_snr)    = Ber_MMSE (n_snr) + ber_MMSE;
        Ber_DNN (n_snr)    = Ber_DNN (n_snr) + ber_DNN;
        Ber_MMSE_1 (n_snr)    = Ber_MMSE_1 (n_snr) + ber_MMSE_1;
        Ber_MMSE_2 (n_snr)    = Ber_MMSE_2 (n_snr) + ber_MMSE_2;
        Ber_MMSE_3 (n_snr)    = Ber_MMSE_3 (n_snr) + ber_MMSE_3; 
    end
    toc; 
end

%% Averaging over channel realizations
Phf       = Phf/N_CH;
Err_Ls    = Err_Ls/N_CH;
Err_MMSE  = Err_MMSE/N_CH;
Err_DNN  = Err_Dnn/N_CH;
Err_MMSE_1  = Err_MMSE_1/N_CH;
Err_MMSE_2  = Err_MMSE_2/N_CH;
Err_MMSE_3  = Err_MMSE_3/N_CH;

%% Theorectical LS NMSE Calculation
Err_Ls_th = Kon*K*N0/Ep;

%% Theorectical MMSE NMSE Calculation
release(rchan);
init_seed = 22;
rchan.Seed = init_seed;
Rh = est_func.Estimat_Rh(rchan, K_cp, K, Kset);
release(rchan);
rchan.Seed = init_seed;
Sig = real(eig(Rh));
Err_MMSE_th = zeros(N_SNR,1);
for n_snr = 1:N_SNR
    Err_MMSE_th(n_snr) = Err_MMSE_th(n_snr) + sum(Sig./(Sig+K*N0(n_snr)./Ep));
end
Err_MMSE_th = K*N0/Ep .* Err_MMSE_th;

%% Normalization by Channel Power 
Err_LSth          = Err_Ls_th./Phf;
Err_LSsim         = Err_Ls./Phf;
Err_MMSEth        = Err_MMSE_th./Phf;
Err_MMSEsim       = Err_MMSE./Phf;
Err_dnnsim       = Err_DNN./Phf;

Err_MMSEsim_1       = Err_MMSE_1./Phf;
Err_MMSEsim_2       = Err_MMSE_2./Phf;
Err_MMSEsim_3       = Err_MMSE_3./Phf;

%% Plotting NMSE Results
%Fig. 4(a)
figure(1),
semilogy(SNR_p,Err_LSsim,'c--','LineWidth',2);
hold on;
semilogy(SNR_p, Err_LSth ,'ko','LineWidth',2);
semilogy(SNR_p,Err_MMSEsim,'m--','LineWidth',2);
semilogy(SNR_p,Err_MMSEth,'kx','LineWidth',2);
semilogy(SNR_p,Err_MMSEsim_1,'g-s','LineWidth',2);
semilogy(SNR_p,Err_dnnsim,'b-p','LineWidth',2);
grid on;
legend('sim-LS','Analytical-Ls', 'sim-MMSE', 'Analytical-MMSE', 'sim-MMSE_erroneous','LSDNN')
xlabel('SNR(dB)')
ylabel('NMSE')


%% Bit Error Rate (BER)
BER_Ideal             = Ber_Ideal ./(N_CH * nSym * nDSC * nBitPerSym);
BER_LS                = Ber_LS ./ (N_CH * nSym * nDSC * nBitPerSym);
BER_MMSE                = Ber_MMSE./ (N_CH * nSym * nDSC * nBitPerSym);
BER_DNN                = Ber_DNN ./ (N_CH * nSym * nDSC * nBitPerSym);
BER_MMSE_1                = Ber_MMSE_1 / (N_CH * nSym * nDSC * nBitPerSym);
BER_MMSE_2                = Ber_MMSE_2 / (N_CH * nSym * nDSC * nBitPerSym);
BER_MMSE_3                = Ber_MMSE_3 / (N_CH * nSym * nDSC * nBitPerSym);

%Fig. 4(b)
figure(2),
semilogy(SNR_p, BER_Ideal,'k-o','LineWidth',2);
hold on
semilogy(SNR_p, BER_LS,'c-','LineWidth',2);
semilogy(SNR_p, BER_MMSE(:,1),'m-','LineWidth',2);
semilogy(SNR_p, BER_MMSE_1(:,1),'g-s','LineWidth',2);
semilogy(SNR_p, BER_DNN(:,1),'b-p','LineWidth',2);
grid on;
legend('Perfect Channel','LS','MMSE','Erroneous MMSE','LSDNN1');
xlabel('SNR(dB)');
ylabel('Bit Error Rate (BER)');

%%  MAGNITUDE PLOTS

subcarriers = 0:51;
psnr = 15;
hf_plot(:) = Hf(psnr,10,:);
Hfe_LS_plot(:) = Hfe_LS(psnr,10,:);
Hfe_LSDNN_plot(:) = Hfe_LSDNN(psnr,10,:);
Hfe_MMSE_plot(:) = Hfe_MMSE(psnr,10,:);
Hfe_MMSE_plot_1(:) = Hfe_MMSE_1(psnr,10,:);
Hfe_MMSE_plot_2(:) = Hfe_MMSE_2(psnr,10,:);
Hfe_MMSE_plot_3(:) = Hfe_MMSE_3(psnr,10,:);

%Fig. 5
figure(3),
plot(subcarriers,abs(hf_plot),'k-d','LineWidth',2);
hold on;
plot(subcarriers,abs(Hfe_LS_plot),'r-o','LineWidth',2);
plot(subcarriers,abs(Hfe_LSDNN_plot),'g-s','LineWidth',2);
plot(subcarriers,abs(Hfe_MMSE_plot),'y-d','LineWidth',2);
plot(subcarriers,abs(Hfe_MMSE_plot_1),'m-p','LineWidth',2);
xlabel('Preamble subcarriers')
ylabel('Magnitude')
xlim([1 52])
legend('CIR','LS',['LSDNN ',num2str(SNR_p(modelIndex)),'dB'],'MMSE','MMSE_1');
title(['Magnitude Plots @', num2str(SNR_p(psnr)),'dB SNR']);




