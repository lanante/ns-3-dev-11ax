clear
clc
close all
set(gca,'fontname','times')
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
plot(thresholdA,mean(Tput(:,1,:),3),'b',thresholdA,mean(Tput(:,2,:),3),'r-.',thresholdA,mean(Tput(:,3,:),3),'m--','LineWidth',1)
legend('MCS0','MCS4','MCS8','FontName','Times','FontSize',10);
text(-70,70,'Proposed','FontName','Times','FontSize',12);
annotation('ellipse',...
    [0.176438596491228 0.665876777251185 0.0165438596491228 0.0269803656059581]);
annotation('ellipse',...
    [0.287152882205513 0.389686301060714 0.0165438596491228 0.0269803656059581]);
annotation('ellipse',...
    [0.421081453634084 0.180162491536907 0.0165438596491228 0.0269803656059581]);
annotation('arrow',[0.301785714285714 0.203571428571429],...
    [0.677571428571429 0.683333333333333]);
annotation('arrow',[0.342857142857143 0.303571428571429],...
    [0.644238095238095 0.428571428571429]);
annotation('arrow',[0.373214285714286 0.430357142857143],...
    [0.632333333333333 0.20952380952381]);
grid;
axis([-87 -57 0 100])
xlabel('Secondary CCA Threshold (dBm)','FontName','Times','FontSize',12);
ylabel('Throughput (Mbps)','FontName','Times','FontSize',12);

subplot(1,2,2)
plot(thresholdA,mean(Tput(:,1,:),3)/6.5,'b',thresholdA,mean(Tput(:,2,:),3)/39,'r-.',thresholdA,mean(Tput(:,3,:),3)/78,'m--','LineWidth',1)
legend('MCS0','MCS4','MCS8','FontName','Times','FontSize',10);
grid;
axis([-87 -57 0 2])
text(-70,1.1,'Proposed','FontName','Times','FontSize',12);
annotation('ellipse',...
    [0.622867167919793 0.461114872489285 0.016543859649123 0.0269803656059581]);
annotation('ellipse',...
    [0.86215288220551 0.737305348679766 0.016543859649123 0.0269803656059581]);
annotation('ellipse',...
    [0.72822431077694 0.473019634394047 0.016543859649123 0.0269803656059581]);
annotation('arrow',[0.828571428571429 0.866071428571428],...
    [0.577571428571429 0.728571428571429]);
annotation('arrow',[0.807142857142857 0.75],...
    [0.525190476190476 0.504761904761905]);
annotation('arrow',[0.739285714285714 0.646428571428571],...
    [0.544238095238095 0.492857142857143]);
xlabel('Secondary CCA Threshold (dBm)','FontName','Times','FontSize',12);
ylabel('Normalized Throughput','FontName','Times','FontSize',12);

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
plot(MCSA,mean(Tput(1,:,:),3),'b',MCSA,mean(Tput(2,:,:),3),'r-o',MCSA,mean(Tput(3,:,:),3),'k-+','LineWidth',1)
legend('Sec. CCA Thresh. = -82dBm','Sec. CCA Thresh. = -62dBm','Proposed','FontName','Times','FontSize',10);
grid;
xlabel('MCS','FontName','Times','FontSize',12);
ylabel('Throughput (Mbps)','FontName','Times','FontSize',12);
axis([0 8 0 100])
subplot(1,2,2)
plot(MCSA,mean(Tput(1,:,:),3)./DR,'b',MCSA,mean(Tput(2,:,:),3)./DR,'r-o',MCSA,mean(Tput(3,:,:),3)./DR,'k-+','LineWidth',1)
legend('Sec. CCA Thresh. = -82dBm','Sec. CCA Thresh. = -62dBm','Proposed','FontName','Times','FontSize',10);
grid;
axis([0 8 0 2])
xlabel('MCS','FontName','Times','FontSize',12);
ylabel('Normalized Throughput','FontName','Times','FontSize',12);
