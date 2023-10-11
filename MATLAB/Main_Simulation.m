%% To generate dataset
% select appropriate channel model. Channel models are defined in Channel_functions.m file 

clc;clearvars;close all;
ch_func = Channel_functions();
est_func = Estimation_functions();
%% Simulation Parameters (IEEE 802.11p)
ChType   = 'RTV';                      % Channel model 
N_CH     = 1000;                       % Number of channel realizations
fs       = 64*156250;                  % Sampling frequency in Hz, here case of 802.11p with 64 subcarriers and 156250 Hz subcarrier spacing
fc       = 5.2e9;                      % Carrier Frequecy in Hz.
v        = 0;                          % Moving speed of user in km/h
c        = 3e8;                        % Speed of Light in m/s
fD       = (v/3.6)/c*fc;               % Doppler freq in Hz
rchan    = ch_func.GenFadingChannel(ChType, fD, fs); % Channel generation
ofdmBW                 = 10 * 10^6 ;     % OFDM bandwidth (Hz)
nFFT                   = 64;             % FFT size 
nDSC                   = 48;             % Number of data subcarriers
nPSC                   = 4;              % Number of pilot subcarriers
nZSC                   = 12;             % Number of zeros subcarriers
nUSC                   = nDSC + nPSC;    % Number of total used subcarriers
K                      = nUSC + nZSC;    % Number of total subcarriers
nSym                   = 10;            % Number of OFDM symbols within one frame
K_cp                   = K/4; 
qpskmod = comm.QPSKModulator;
qpskdemod = comm.QPSKDemodulator;

% Pre-defined preamble in frequency domain
dp       = [ 0  0 0 0 0 0 +1 +1 -1 -1 +1  +1 -1  +1 -1 +1 +1 +1 +1 +1 +1 -1 -1 +1 +1 -1 +1 -1 +1 +1 +1 +1 0 +1 -1 -1 +1 +1 -1 +1 -1 +1 -1 -1 -1 -1 -1 +1 +1 -1 -1 +1 -1 +1 -1 +1 +1 +1 +1 0 0 0 0 0];
dp                     = fftshift(dp); 
%%%%%%%%%%%%%%%%%%%%%%%%%%%%
Kset     = find(dp~=0);                % Active subcarriers
Kon      = length(Kset);               % Number of active subcarriers
Ep       = 1;                          % preamble power
dp       = sqrt(Ep)*dp.';              % Normalization
xp       = ifft(dp);                   % Frequency-Time conversion
xp_cp    = [xp(end-K_cp+1:end); xp];   % Adding CP 
SNR_p    = (-50:5:50)';                  % SNR range
N_SNR    = length(SNR_p);              % SNR length
N0       = Ep/K*10.^(-SNR_p/10);       % Noise power : snr_p = Ep/KN0 => N0 = Ep/(K*snr_p)
Err_Ls   = zeros(N_SNR,1);             % LS NMSE vector
Err_MMSE = zeros(N_SNR,1);             % MMSE NMSE vector
Phf      = zeros(N_SNR,1);             % Average channel power E(|hf|^2)

preamble_80211p        = repmat(xp_cp,1,2);         % IEEE 802.11p preamble symbols (tow symbols)
pilots_locations       = [8,22,44,58].'; % Pilot subcarriers positions
pilots                 = [1 1 1 -1].';
data_locations         = [2:7, 9:21, 23:27, 39:43, 45:57, 59:64].'; % Data subcarriers positions
ppositions             = [7,21, 32,46].';                           % Pilots positions in Kset
dpositions             = [1:6, 8:20, 22:31, 33:45, 47:52].';        % Data positions in Kset
nBitPerSym            = 2; 
M                     = 2 ^ nBitPerSym; % QAM Modulation Order   
Pow                   = mean(abs(qammod(0:(M-1),M)).^2); % Normalization factor for QAM 

%% Rh estimation
release(rchan);
init_seed   = 5;
rchan.Seed  = init_seed;
Rh          = est_func.Estimat_Rh(rchan, K_cp, K, Kset);
release(rchan);
rchan.Seed  = init_seed;

FileName=['.\results\Rh.txt'];
writematrix(Rh,FileName,'delimiter',',');

Ber_Ideal = zeros(N_SNR);
Ber_LS = zeros(N_SNR);
Ber_MMSE = zeros(N_SNR);

%% Main Simulation 
MMSE_Snr =zeros(1,N_SNR);
for n_snr = 1:N_SNR
    disp(['Running Simulation, SNR = ', num2str(SNR_p(n_snr))]);
    tic; 
    Hfe_LS = zeros(Kon, N_CH);
    Hfe_MMSE = zeros(Kon, N_CH);
    Hf =  zeros(Kon, N_CH);

    % MMSE filter varies with SNR
    W = est_func.MMSE_matrix (dp(Kset), Rh, Ep/N0(n_snr)/K);
    MMSE_Snr(:,n_snr) =Ep/N0(n_snr)/K;
    
    for n_ch = 1:N_CH % loop over channel realizations

        % Data generation
        N = nDSC * nSym;
        data = randi([0 1], N, 2);
        Data = reshape(data, [], 1);
        dataSym = bi2de(data);
        
        % QPSK modulator
        QPSK_symbol = qpskmod(dataSym);
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
        
        %MMSE estimation
        [hfe_mmse, err_mmse] = est_func.MMSE(yfp_r(Kset,1), W, hf(Kset,1));
        Err_MMSE (n_snr) = Err_MMSE (n_snr) + err_mmse;
        H_MMSE = repmat(hfe_mmse,1,nSym);
        
        % save the channels for further use
        Hf(:,n_ch) = hf(Kset,1);
        Hfe_LS(:,n_ch) = hfe_ls1;
        Hfe_MMSE(:,n_ch) = hfe_mmse; 
        
        % Equalization
        y_Ideal = yfp_r(data_locations ,3:end) ./ hf(data_locations,3:end); %Ideal
        y_LS = yfp_r(data_locations ,3:end)./ H_LS(dpositions,:); % LS
        y_MMSE = yfp_r(data_locations ,3:end)./ H_MMSE(dpositions,:);
        
        demod_y_ideal = reshape(y_Ideal, [], 1);
        demod_y_LS = reshape(y_LS, [], 1);
        demod_y_MMSE = reshape(y_MMSE, [], 1);
        
        % QAM - DeMapping
        
        De_Mapped_Ideal = qpskdemod(sqrt(Pow) * demod_y_ideal);
        release(qpskdemod);
        De_Mapped_LS = qpskdemod(sqrt(Pow) * demod_y_LS);
        release(qpskdemod);
        De_Mapped_MMSE = qpskdemod(sqrt(Pow) * demod_y_MMSE);
        release(qpskdemod);    
        sym_ideal = de2bi(De_Mapped_Ideal);
        sym_LS = de2bi(De_Mapped_LS);
        sym_MMSE = de2bi(De_Mapped_MMSE);
        
        bits_ideal = reshape(sym_ideal, [], 1);
        bits_LS = reshape(sym_LS, [], 1);
        bits_MMSE = reshape(sym_MMSE, [], 1);
        
        % Bits Extraction
        Bits_Ideal = zeros(nDSC,nSym,log2(M));
        Bits_LS     = zeros(nDSC,nSym,log2(M));
        Bits_MMSE     = zeros(nDSC,nSym,log2(M));
        
        % BER Calculation
        ber_Ideal  = biterr(Data,bits_ideal);       
        ber_LS     = biterr(Data,bits_LS);
        ber_MMSE     = biterr(Data,bits_MMSE);
        
        Ber_Ideal (n_snr) = Ber_Ideal (n_snr) + ber_Ideal;
        Ber_LS (n_snr)    = Ber_LS (n_snr) + ber_LS;
        Ber_MMSE (n_snr)    = Ber_MMSE (n_snr) + ber_MMSE;
    end
    toc; 
    X(1:Kon,:)     = real(Hfe_LS);
    X(Kon+1:2*Kon,:) = imag(Hfe_LS);
    
    Y(1:Kon,:)     = real(Hf);
    Y(Kon+1:2*Kon,:) = imag(Hf);
    
    Preamble_Error_Correction_Dataset.('X') =  X;
    Preamble_Error_Correction_Dataset.('Y') =  Y ;
    save(['.\results\Dataset_' num2str(n_snr)],  'Preamble_Error_Correction_Dataset','-v7.3');
end

%% Averaging over channel realizations
Phf       = Phf/N_CH;
Err_Ls    = Err_Ls/N_CH;
Err_MMSE  = Err_MMSE/N_CH;

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

%% Plotting NMSE Results
figure,
semilogy(SNR_p, Err_LSth ,'k--','LineWidth',2);
hold on;
semilogy(SNR_p,Err_LSsim,'k+','LineWidth',2);
hold on;
semilogy(SNR_p,Err_MMSEth,'k--','LineWidth',2);
hold on;
semilogy(SNR_p,Err_MMSEsim,'ko','LineWidth',2);
hold on;
grid on;
legend('Analytical-Ls', 'sim-LS', 'Analytical-MMSE', 'sim-MMSE')
xlabel('Preamble SNR')
ylabel('Average Error per subcarrier')

%% Bit Error Rate (BER)
BER_Ideal             = Ber_Ideal /(N_CH * nSym * nDSC * nBitPerSym);
BER_LS                = Ber_LS / (N_CH * nSym * nDSC * nBitPerSym);
BER_MMSE                = Ber_MMSE / (N_CH * nSym * nDSC * nBitPerSym);

figure,
p1 = semilogy(SNR_p, BER_Ideal,'k-o','LineWidth',2);
hold on;
p2 = semilogy(SNR_p, BER_LS,'r--o','LineWidth',2);
hold on;
p3 = semilogy(SNR_p, BER_MMSE,'y--o','LineWidth',2);
hold on;
grid on;
legend([p1(1),p2(1),p3(1)],{'Perfect Channel','LS','MMSE'});
xlabel('SNR(dB)');
ylabel('Bit Error Rate (BER)');

save(['.\results\ErrSet'],...
        'Err_LSth','Err_LSsim','Err_MMSEth','Err_MMSEsim','SNR_p','-v7.3');


