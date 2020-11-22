clear
clc


thresholdA=[-72 0];
Tput=zeros(length(thresholdA),100);

for ite=1:100
            
        filename=['Res_6_10_20_ConstantThreshold_-72_MaxMcs_' num2str(ite) '.csv'];
if isfile(filename)
        a=load(filename);
        b=a(1);
Tput(1,ite)=b;
else
Tput(1,ite)=Tput(1,ite-1);
end


    
            
        filename=['Res_6_10_20_ConstantThreshold_0_proposedMcs_' num2str(ite) '.csv'];
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

plot(edges(1:end-1),b1,'b',edges(1:end-1),b2,'r-.');
% legend('
