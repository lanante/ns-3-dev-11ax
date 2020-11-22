clear
clc

%fig8
thresholdA=[-82 -72 -62 0];
MCSA=[0 4 8];
Tput=zeros(length(thresholdA),length(MCSA),100*10);
Tput1=zeros(length(MCSA),100*10);

for ite=1:100
    for k=1:length(MCSA)
    for i=1:length(thresholdA)
            
        filename=['Res_6_10_20_ConstantThreshold_' num2str(thresholdA(i)) '_VhtMcs' num2str(MCSA(k)) '_' num2str(ite) '.csv'];
if isfile(filename)
        a=load(filename);
        b=a(7:16);
Tput(i,k,(ite-1)*10+1:ite*10)=b;
else
Tput(i,k,ite)=Tput(i,k,ite-1);
end

    end
   
     end
ite
end

k=1;
figure(k)
a=squeeze(Tput(:,k,:));
edges=linspace(0,4,100);
b=histogram(a(1,:),'Normalization','cdf','BinEdges', edges);
b1=b.Values;
b=histogram(a(2,:),'Normalization','cdf','BinEdges', edges);
b2=b.Values;
b=histogram(a(3,:),'Normalization','cdf','BinEdges', edges);
b3=b.Values;
b=histogram(a(4,:),'Normalization','cdf','BinEdges', edges);
b4=b.Values;
plot(edges(1:end-1),b1,'b',edges(1:end-1),b2,'r-.',edges(1:end-1),b3,'y--',edges(1:end-1),b4,'m-+');


k=2;
figure(k)
a=squeeze(Tput(:,k,:));
edges=linspace(0,10,100);
b=histogram(a(1,:),'Normalization','cdf','BinEdges', edges);
b1=b.Values;
b=histogram(a(2,:),'Normalization','cdf','BinEdges', edges);
b2=b.Values;
b=histogram(a(3,:),'Normalization','cdf','BinEdges', edges);
b3=b.Values;
b=histogram(a(4,:),'Normalization','cdf','BinEdges', edges);
b4=b.Values;
plot(edges(1:end-1),b1,'b',edges(1:end-1),b2,'r-.',edges(1:end-1),b3,'y--',edges(1:end-1),b4,'m-+');






k=3;
figure(k)
a=squeeze(Tput(:,k,:));
edges=linspace(0,25,100);
b=histogram(a(1,:),'Normalization','cdf','BinEdges', edges);
b1=b.Values;
b=histogram(a(2,:),'Normalization','cdf','BinEdges', edges);
b2=b.Values;
b=histogram(a(3,:),'Normalization','cdf','BinEdges', edges);
b3=b.Values;
plot(edges(1:end-1),b1,'b',edges(1:end-1),b2,'r-.',edges(1:end-1),b3,'y--');
b=histogram(a(4,:),'Normalization','cdf','BinEdges', edges);
b4=b.Values;
plot(edges(1:end-1),b1,'b',edges(1:end-1),b2,'r-.',edges(1:end-1),b3,'y--',edges(1:end-1),b4,'m-+');



