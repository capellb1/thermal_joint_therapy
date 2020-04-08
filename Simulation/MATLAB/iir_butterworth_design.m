%% ELC433 Final Project / ELC495 Senior Project
% IIR Discrete Butterworth Filter Design with Prewarping and Bilinear Transform

%Filter Specifications
fs = 100;              %Hz, used for prewarping, 10x closed loop system bw 
gp = 0.01;             %Minimum Passband (dB)
gs = 80;               %Max Stopband (dB)

w_sys = 2.26;
wp = w_sys*10;
ws = w_sys*15;

%Nyquist Normalized Frequencies
Wp = 2*(wp/(2*pi*fs));
Ws = 2*(ws/(2*pi*fs));

%Prewarping
Wp_warped = (2/(1/fs))*tan(Wp*((1/fs)/2)); %rad/s
Ws_warped = (2/(1/fs))*tan(Ws*((1/fs)/2)); %rad/s

%Determine "n" order required for Butterworth Filter
n = ceil(log10((10^(-gs/10)-1)/(10^(-gp/10)-1)...
    /(2*log10(Ws_warped/Wp_warped))));

%Calculate the range of possible cutoff frequencies
Wc_prewarp_1 = abs(Wp_warped/((10^(-gp/10)-1)^(1/(2*n))));
Wc_prewarp_2 = abs(Ws_warped/((10^(-gs/10)-1)^(1/(2*n))));

% Butterworth filter prototype centered at 1 rad/s
[z,p,k] = buttap(n);

% Convert to transfer function form
[num,den] = zp2tf(z,p,k); 

%Change cutoff frequency
[bt,at] = lp2lp(num,den,(Wc_prewarp_1*(pi*fs)));

%Visualize Analog Filter
freqs(bt,at)
title("Analog Filter Magnitude Response (dB)")

%Analog to digital via Bilinear Transformation
[bd,ad] = bilinear(bt,at,fs);

%Visualize Digital Filter
freqz(bd,ad)
title("Discrete Filter Magnitude Response (dB)")

fvtool(bd,ad,'Fs',fs); 

%% MATLAB "Butter" Digital Filter Algorithms

%Order and Cutoff Freq Function
[n,Wn] = buttord(Wp,Ws,gp,gs)
%Filter Synthesis; Coeffecient Look-up
[b,a] = butter(n,Wn);
%Filter Visualization
freqz(b,a)

%% MATLAB General Filter Design Algorithm

%Single Line Example using MATLAB's dsp Toolbox
%No prewarping
LP_IIR = dsp.LowpassFilter('SampleRate',fs,'FilterType','IIR',...
    'DesignForMinimumOrder',true,...
    'PassbandFrequency',wp/(2*pi),'StopbandFrequency',ws/(2*pi),...
    'PassbandRipple',gp,'StopbandAttenuation',gs);
Nlp = order(LP_IIR);
fvtool(LP_IIR,'Fs',fs); 

