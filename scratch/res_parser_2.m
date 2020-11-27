clear
clc
close all
%fig8
thresholdA=[-82 -72 -62 0];
MCSA=[0 4 8];
Tput=zeros(length(thresholdA),length(MCSA),100*10);
Tput1=zeros(length(MCSA),100*10);

for ite=1:200
    for k=1:length(MCSA)
    for i=1:length(thresholdA)
            
        filename=['../Sim2_res/Res_6_10_20_ConstantThreshold_' num2str(thresholdA(i)) '_VhtMcs' num2str(MCSA(k)) '_' num2str(ite) '.csv'];
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
plot(edges(1:end-1),b1,'b',edges(1:end-1),b2,'r-o',edges(1:end-1),b3,'m--',edges(1:end-1),b4,'k-+','LineWidth',1);
axis([0 4 0 1.2]);
grid
xlabel('BSS-1 DL Throughput (Mbps)','FontName','Times','FontSize',12);
ylabel('Cumulative Distribution','FontName','Times','FontSize',12);
legend('Sec. CCA Thresh. = -82','Sec. CCA Thresh. = -72','Sec. CCA Thresh. = -62','Proposed', 'Position',[0.55238095993797 0.121031782740638 0.34464284958584 0.155952376694906],'FontName','Times','FontSize',10);
annotation('ellipse',...
    [0.611909090909091 0.698564593301432 0.046272727272727 0.0406698564593291]);
annotation('ellipse',...
    [0.582818181818182 0.418660287081339 0.046272727272727 0.040669856459329]);
annotation('arrow',[0.738181818181818 0.647272727272727],...
    [0.587516746411483 0.712918660287081]);
annotation('arrow',[0.718181818181818 0.634545454545455],...
    [0.5 0.464114832535885]);
text('String',{'About the','same performance'},...
    'Position',[2.718779342723 0.643695014662757 0],'FontName','Times','FontSize',12);


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
plot(edges(1:end-1),b1,'b',edges(1:end-1),b2,'r-o',edges(1:end-1),b3,'m--',edges(1:end-1),b4,'k-+','LineWidth',1);
axis([0 10 0 1.2]);
grid
xlabel('BSS-1 DL Throughput (Mbps)','FontName','Times','FontSize',12);
ylabel('Cumulative Distribution','FontName','Times','FontSize',12);
legend('Sec. CCA Thresh. = -82','Sec. CCA Thresh. = -72','Sec. CCA Thresh. = -62','Proposed', 'Position',[0.55238095993797 0.121031782740638 0.34464284958584 0.155952376694906],'FontName','Times','FontSize',10);
text(6,0.8,['+60% Improvement'],'FontName','Times','FontSize',12);
text(3.7,0.5,['+13% Improvement'],'FontName','Times','FontSize',12);
annotation('doublearrow',[0.491228070175439 0.7],...
    [0.724118483412323 0.725118483412322]);
annotation('doublearrow',[0.350877192982456 0.403508771929825],...
    [0.451606635071091 0.45260663507109]);


k=3;
figure(k)
a=squeeze(Tput(:,k,:));
edges=[linspace(0,1,10) linspace(1,25,100)];
b=histogram(a(1,:),'Normalization','cdf','BinEdges', edges);
b1=b.Values;
b=histogram(a(2,:),'Normalization','cdf','BinEdges', edges);
b2=b.Values;
b=histogram(a(3,:),'Normalization','cdf','BinEdges', edges);
b3=b.Values;
plot(edges(1:end-1),b1,'b',edges(1:end-1),b2,'r-.',edges(1:end-1),b3,'g--');
b=histogram(a(4,:),'Normalization','cdf','BinEdges', edges);
b4=b.Values;
plot(edges(1:end-1),b1,'b',edges(1:end-1),b2,'r-o',edges(1:end-1),b3,'m--',edges(1:end-1),b4,'k-+','LineWidth',1);
axis([0 25 0 1.2]);
grid
legend('Sec. CCA Thresh. = -82','Sec. CCA Thresh. = -72','Sec. CCA Thresh. = -62','Proposed', 'Position',[0.55238095993797 0.121031782740638 0.34464284958584 0.155952376694906],'FontName','Times','FontSize',10);
xlabel('BSS-1 DL Throughput (Mbps)','FontName','Times','FontSize',12);
ylabel('Cumulative Distribution','FontName','Times','FontSize',12);
legend('Sec. CCA Thresh. = -82','Sec. CCA Thresh. = -72','Sec. CCA Thresh. = -62','Proposed', 'Position',[0.55238095993797 0.121031782740638 0.34464284958584 0.155952376694906],'FontName','Times','FontSize',10);
text(3,0.5,'+2100% Improvement','FontName','Times','FontSize',12);
text(7.5,0.9,'+74% Improvement','FontName','Times','FontSize',12);
annotation('doublearrow',[0.3375 0.241071428571429],...
    [0.710904761904762 0.70952380952381]);
annotation('doublearrow',[0.132142857142857 0.203571428571429],...
    [0.456142857142857 0.457142857142858]);

