clear
clc
CWmin=31;
CWmax=1023;
nA=1:5:40;
nBss=4;
for j=1:length(nA)
    n=nA(j)*1;
W=CWmin+1;
m=log2((CWmax+1)/(CWmin+1));
tau1=linspace(0,1,1e4);
p=1-(1-tau1).^(n-1);
ps=p*0;

for i=0:m-1
ps=ps+(2*p).^i;
end
taup=2./(1+W+p.*W.*ps);
[a,b]=min(abs(tau1-taup));
tau_bianchi=taup(b);

T_ACK=44e-6;
T_AMPDU=1.972e-3;
T_SIFS=16e-6;
AIFSn=3;
T_SLOT=9e-6;
T_AIFS=T_SIFS+T_SLOT*AIFSn;
T_EIFS=T_ACK+T_SIFS+T_AIFS;

T_S=T_AMPDU+T_SIFS+T_ACK+T_AIFS;
T_C=T_AMPDU+T_AIFS;

tau=tau_bianchi;
if n>1
Ptr=(1-(1-tau)^(1*n))/1;
Ps=n*tau*(1-tau)^(1*n-1)/Ptr;
else
Ptr=(1-(1-tau)^(1*1))/1;
Ps=1*tau*(1-tau)^(1*1-1)/Ptr; 
end
EP=1500*8;

% c=nBss;
PT=(1-Ptr)^nBss*T_SLOT+(1-(1-Ptr)^nBss)*(Ps*T_S+(1-Ps)*T_C);


S_bianchi(j)=Ps*Ptr*EP/PT/1e6*nBss;
% S_bianchi(j)=(Ps*Ptr*EP*2/((1-Ptr)^2*T_SLOT*1+(1-(1-Ptr)^2)*T_S))/1e6;
% 
end


T_AVE_BACKOFF=(CWmin+1)/2*T_SLOT+T_SIFS+AIFSn*T_SLOT;
T_AVE_S=T_AVE_BACKOFF+T_AMPDU+T_SIFS+T_ACK+T_SIFS;
S_ave=EP/T_AVE_S;
S_ns3=[ 6.18 7.35 7.85 8.18 8.45 7.66 8.40 6.06];%nBss=4 d=5m
% S_ns3=[ 6.01 6.57 6.80 6.96 6.89 6.98 6.55 5.68];%nBss=3 d=5m
% S_ns3=[ 5.76 5.83 5.72 5.62 5.64 5.51 5.50 5.41];%nBss=2 d=5m
% S_ns3=[ 5.41 5.07 4.79 4.58 4.42 4.25 4.23 4.08];%nBss=1 d=5m

plot(nA(1:length(S_ns3)),S_bianchi(1:length(S_ns3)),'-*',nA(1:length(S_ns3)),S_ns3,'-o');
grid;
axis([0 40 0 10])
% 
% 
% fpname='backofftest10.txt';
% fid=fopen(fpname);
% 
% linenum=9;
% n=10;
% for i=1:1e6
%     C = textscan(fid,'%s',1,'delimiter','\n', 'headerlines',linenum-1);
% A=C{1}{1};
% if strcmp(A,'Spatial Reuse Statistics')
%    break; 
% end
% ind1=strfind(A,' ');
% Time(i)=str2double(A(ind1(1)+1:ind1(2)-1));
% STA(i)=str2double(A(ind1(2)+1:ind1(3)-1));
% if strcmp((A(1:ind1(1)-1)),'BO')
% BO(i)=str2double(A(ind1(3)+1:end));
% elseif strcmp((A(1:ind1(1)-1)),'CW')
%     CW(i)=str2double(A(ind1(3)+1:end));
% end
% linenum=1;
% end
% fclose(fid)
% n=1;
% tau=1;
% p=0;
% 
% 
% 
% 
% 
