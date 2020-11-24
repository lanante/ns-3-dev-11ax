clear
clc

% %fig8
thresholdA=[-87:2:-57];
MCSA=[0 4 8];
Tput=zeros(length(thresholdA),length(MCSA),200);
for ite=1:200
    for i=1:length(thresholdA)
            for k=1:length(MCSA)
        filename=['../Sim1_res/Res_2_10_20_ConstantThreshold_' num2str(thresholdA(i)) '_VhtMcs' num2str(MCSA(k)) '_' num2str(ite) '.csv'];
if isfile(filename)
        a=load(filename);
Tput(i,k,ite)=sum(a(3:12));
else
Tput(i,k,ite)=Tput(i,k,ite-1);
end

    end
    end
ite
end
subplot(1,2,1)
plot(thresholdA,mean(Tput(:,1,:),3),'b-.',thresholdA,mean(Tput(:,2,:),3),'r--',thresholdA,mean(Tput(:,3,:),3),'y','LineWidth',1)
legend('MCS0','MCS4','MCS8');
grid;
axis([-87 -57 0 100])
subplot(1,2,2)
plot(thresholdA,mean(Tput(:,1,:),3)/6.5,'b-.',thresholdA,mean(Tput(:,2,:),3)/39,'r--',thresholdA,mean(Tput(:,3,:),3)/78,'y','LineWidth',1)
legend('MCS0','MCS4','MCS8');
grid;
axis([-87 -57 0 2])




%fig9
figure
thresholdA=[-82 -62 0];
MCSA=[0:8];
Tput=zeros(length(thresholdA),length(MCSA),100);
Tput1=zeros(length(MCSA),100);
PropThresh=[-62 -62 -62 -62 -82 -82 -82 -82 -82];
DR=[6.5 13 19.5 26 39 52 58.5 65 78];
for ite=1:200
    for i=1:length(thresholdA)
            for k=1:length(MCSA)
        filename=['../Sim1_res/Res_2_10_20_ConstantThreshold_' num2str(thresholdA(i)) '_VhtMcs' num2str(MCSA(k)) '_' num2str(ite) '.csv'];
if isfile(filename)
        a=load(filename);
Tput(i,k,ite)=sum(a(3:12));
else
Tput(i,k,ite)=Tput(i,k,ite-1);
end

    end
    end
    

    
    
ite
end
subplot(1,2,1)
plot(MCSA,mean(Tput(1,:,:),3),'b-+',MCSA,mean(Tput(2,:,:),3),'r--+',MCSA,mean(Tput(3,:,:),3),'y-o','LineWidth',1)
legend('Sec. CCA Thresh.=-82dBm','Sec. CCA Thresh.=-62dBm','Proposed');
grid;
xlabel('MCS');
ylabel('Throughput (Mbps)');
axis([0 8 0 100])
subplot(1,2,2)
plot(MCSA,mean(Tput(1,:,:),3)./DR,'b-+',MCSA,mean(Tput(2,:,:),3)./DR,'r--+',MCSA,mean(Tput(3,:,:),3)./DR,'y-o','LineWidth',1)
legend('Sec. CCA Thresh.=-82dBm','Sec. CCA Thresh.=-62dBm','Proposed');
grid;
axis([0 8 0 2])
xlabel('MCS');
ylabel('Normalized Throughput');
