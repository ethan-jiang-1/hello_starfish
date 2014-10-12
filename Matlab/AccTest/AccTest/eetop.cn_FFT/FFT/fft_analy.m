
function [SNR,SNDR]=fft_analy(y,fs,fin,M,N,w)
	%fs=2/16;
	%cycle=13;
	%M=128;
	%fin=0.0126953125;
	%k=M/cycle;
	d=0;

	t(1:1:M)=(1*(1:1:M)/(fs*1e6));
	% Dither inserted
	%fin=((M/k)-1*d)*fs/M;
	% Find Harmonic  Signals/Images
	%N=input('input number of Harmonic = ');
	fh(1:N)=-1;
	h(1:N)=0;

% Find input and harmonic signal/image
	for hom=1:1:N	
		while (fh(hom)==-1)& (h(hom)<100)
 			fa1=abs(h(hom)*fs+(hom)*fin);
 			fa2=abs(h(hom)*fs-(hom)*fin);
 				if (fa1<fs/2)
  			 		fh(hom)=fa1;
 				end
	 			if (fa2<=fs/2)
     					fh(hom)=fa2;
  				end
      		h(hom)=h(hom)+1;
		end
	end

% Find the centers for signal and harmonic signals
	nfh(1:N)=(fh(1:N)+1*d)/fs*M;

% Preset
	ptst = 1;
	ptfi = M;
	pts = M;

	nb=1:1:pts;
	if w==1
	kb=0.355768-0.487396*cos(2*pi*nb/(pts))+0.144232*cos(4*pi*nb/(pts))-0.012604*cos(6*pi*nb/(pts));
	kb=kb';
	fprintf('hamaning window \n');
	fprintf('  \n');
	else
	kb=ones(M,1);
	fprintf('NO window \n');
	fprintf('  \n');
	end
	win_gain=sum(kb)/pts;
	kb=kb/win_gain;
	%kb=kb';
	%yb=y(1:pts);
	yb=kb.*y(1:pts);
	yk=fft(yb(ptst:ptfi));
 
	no=length(yk);
	f=fs*10^6*(0:no-1)/no;
	Pxx=yk.*conj(yk)/((no/2)^2);

	% normalized to signal
	%Pxxn=Pxx/Pxx(ceil(nfh(1)));
	Pxxn=Pxx/max(Pxx(2:no));
	%points contribute signal and homonic power
	signal_pwr=sum(Pxxn(ceil(nfh(1))-1:ceil(nfh(1))+1));
	harmonic_pwr(2:N)=sum(Pxxn(ceil(nfh(2:N))-2:ceil(nfh(2:N))+2));
	harmonic_pwrc(2:N)=sum(Pxxn(ceil(nfh(2:N))-2:4:ceil(nfh(2:N))+2));

	%noise power & noise+THD
	noise_pwr=sum(Pxxn(5:no/2))-signal_pwr-sum(harmonic_pwr(2:N)+harmonic_pwrc(2:N));
	noise_hd_pwr=sum(Pxxn(5:no/2))-signal_pwr;

	%SNR & SNDR
	snr=10*log10(signal_pwr/noise_pwr);
	sndr=10*log10(signal_pwr/noise_hd_pwr);
  
	clf;
	hold off;	
	%subplot(2,1,1);
	figure
		plot(t,y,'r-*');
 		title('Time Domain Waveform');
 		xlabel('Times (sec) ');
 		ylabel('output voltage (v)');
 		grid;
	figure
	%subplot(2,1,2);
		plot(f(1:no/2),10*log10(Pxxn(1:no/2)),'b');
  		title('10BIT RADC Fs=1MHz, Fin= 30.2734375kHz');
		xlabel('Frequency response (Hz)');
  		ylabel('fft (dB)');
		%axis([1e1 (fs/2)*1e6  -130   10]);
  		grid;
	hold on;
	%db(1:10)=10*log10(Pxxn(nfh(1:10)));
	%plot (fh(1:10)*1e6 ,db(1:10),'*r');

	%plot (fh(1:10)*1e6 ,10*log10(Pxxn(ceil(nfh(1:10)))),'*r');
	for ppst=1:1:N
	fprintf('The center frequency	of fh(%f) = %f  MHz \n', ppst,fh(ppst));
	fprintf('The center power	of fh(%f) = %f  dB \n', ppst,10*log10(Pxxn(ceil(nfh(ppst)))));
	fprintf('  \n');
	end


	hold off;
	fprintf('SNR	is	%f	dB\n',snr);
	fprintf('SNDR	is	%f	dB\n',sndr);



