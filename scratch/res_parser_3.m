clear
clc
close all

thresholdA=[-72 0];
Tput=zeros(length(thresholdA),100);

for ite=1:200
            
        filename=['../Sim3_res/Res_6_10_20_ConstantThreshold_-72_MaxMcs_' num2str(ite) '.csv'];
if isfile(filename)
        a=load(filename);
        b=a(1);
Tput(1,ite)=b;
else
Tput(1,ite)=Tput(1,ite-1);
end


    
            
        filename=['../Sim3_res/Res_6_10_20_ConstantThreshold_0_proposedMcs_' num2str(ite) '.csv'];
if isfile(filename)
        a=load(filename);
        b=a(1);
Tput(2,ite)=b;
else
Tput(2,ite)=Tput(2,ite-1);
end

    
   
ite
end


a=squeeze(Tput);
edges=linspace(0,200,100);
b=histogram(a(1,:),'Normalization','cdf','BinEdges', edges);
b1=b.Values;
b=histogram(a(2,:),'Normalization','cdf','BinEdges', edges);
b2=b.Values;

plot(edges(1:end-1),b1,'r-o',edges(1:end-1),b2,'k-+');
axis([0 120 0 1.2])
xlabel('BSS-1 UL Throughput (Mbps)','FontName','Times','FontSize',12);
ylabel('Cumulative Distribution','FontName','Times','FontSize',12);
grid;
legend('Sec. CCA Thresh. = -72','Proposed','FontName','Times','FontSize',10)
annotation('doublearrow',[0.343859649122809 0.457894736842105],...
    [0.439758293838866 0.438388625592417]);
annotation('doublearrow',[0.587719298245614 0.664912280701754],...
    [0.731227488151659 0.732227488151659]);
text(82,0.9,'+15% Improvement','FontName','Times','FontSize',12);
text(52,0.5,'+49% Improvement','FontName','Times','FontSize',12);
