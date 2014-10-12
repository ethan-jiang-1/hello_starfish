
% SNR TEST PROGRAM
figure
clf;
clear;
format long;

fs=1000; % *10e6 Hz

cycle=1024;
fin=27;% *10e6 Hz
M=round(cycle*fs/fin);
k=M/cycle;


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%



load mat27_old1.txt -ascii





start = 2;
step  = 1;


%PLL
y=mat27_old1(start:step:(start+M*step-step),2)/1.8;
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%ADC
%d9=round(AD96(start:step:(start+M*step-step),2)/1.8);
%d8=round(AD96(start:step:(start+M*step-step),3)/1.8);
%d7=round(AD96(start:step:(start+M*step-step),4)/1.8);
%d6=round(AD96(start:step:(start+M*step-step),5)/1.8);
%d5=round(AD52(start:step:(start+M*step-step),2)/1.8);
%d4=round(AD52(start:step:(start+M*step-step),3)/1.8);
%d3=round(AD52(start:step:(start+M*step-step),4)/1.8);
%d2=round(AD52(start:step:(start+M*step-step),5)/1.8);
%d1=round(AD10(start:step:(start+M*step-step),2)/1.8);
%d0=round(AD10(start:step:(start+M*step-step),3)/1.8);
%y=(2^9*d9+2^8*d8+2^7*d7+2^6*d6+2^5*d5+2^4*d4+2^3*d3+2^2*d2+2^1*d1+2^0*d0)/2^10;
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
k
N=3;
[SNR,SNDR]=fft_analy(y,fs,fin,M,N,2)