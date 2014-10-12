function []=ACCTEST()
% basic DNA data logging with primary measurement data, secondary control
% data and pixel images stored for analysis
%%%zga only for test
%% average and deviation

X=[];
%[a]=textread('d:\first.txt','%d');
[a1,a2,a3]=textread('e:\1.txt','%d%s%d','headerlines',3);
X=[a1]
N=length(X);
% for i=1:10:N
%     Y=[X(i),X(i+1),X(i+2),X(i+3),X(i+4),X(i+5),X(i+6),X(i+7),X(i+8),X(i+9)];
for j=1:5:N
    Y=[X(j),X(j+1),X(j+2),X(j+3),X(j+4)];
    c1=mean(Y);
    c2=var(Y);
    if c1>10 && c2<0.5
        break;
    end
end

for i=N:-5:1
    Y=[X(i),X(i-9),X(i-8),X(i-7),X(i-6)];
    c3=mean(Y);
    c4=var(Y);
    if c3>10 && c4<1 && c3>c1
        break;
    end
end
%N=3000;         %2500????           %?????????????
%load 1.txt -ascii
%start = 2;
%step  = 1;
%X=mat27_old1(start:step:(start+M*step-step));
a=-1000;
a=a*0.00025;
b=acos(a)*180/pi;
aa=[];
diary off;
diary('d:\acos.h');
diary on;
for i=1:1:3999
    x=i;
    x=x*0.00025;
    y=acos(x)*180/pi;
   tt=uint8(y);
    aa(tt)=i;
end
for i=1:1:90
    fprintf('%d%s\r\n',aa(i),',');
end
diary off;
minput=-1000;
if minput>=0
    if minput>3999
        minput=3999;
    else 
        for i=1:1:89
            if (minput<=aa(i)&& minput>aa(i+1))
                moutput=i;
                break;
            end
        end
    end
else
    minput=abs(minput);
        if minput>3999
        minput=3999;
    else 
        for i=1:1:90
              if (minput<=aa(i)&& minput>aa(i+1))
                moutput=i;
                moutput=180-moutput;
                break;
            end
        end
        end
end
    
Fs=10;        %????
X=[];
[a]=textread('d:\1.txt','%f');
X=[a];

N=length(X);
t=(0:N-1)/Fs;
Y=fft(X,N);     %???????
f=(0:N-1)*Fs/N; %?????
subplot(2,1,1);
plot(f,abs(Y))  %???
subplot(2,1,2);
plot(f,angle(Y)) %???

%%%%%%%%%%%%%delete




