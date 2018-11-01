clear all;
close all;

%% LTE from theory to practice
%% Table 22.7 Reference sensitivity.

fdl = 2120e6;  % DL carrier freq Hz, EARFCN = 100
ful = 1930e6;  % UL carrier freq Hz, EARFCN = 18100 
nrbs = 25; % tx bandwidth configuration in number of RBs
bw = nrbs * 180000; % bandwidth in Hz, note that this is smaller than
		    % the nominal Channel Bandwdith, see TS 36.101 fig 5.6-1
kT = -174; % noise PSD in dBm / Hz
ndBm = kT + 10*log10(bw);  % noise power dBm

dlpdBm = 30;  % tx power dBm in DL
dlp = 10.^((dlpdBm - 30)/10); %% tx pow in W in DL
dlnf = 9; % receiver noise figure in dB in DL
dln = 10.^((ndBm-30+dlnf)/10); %% noise in W in DL

ulpdBm = 10;  % tx power dBm in UL
ulp = 10.^((ulpdBm - 30)/10); %% tx pow in W in UL
ulnf = 5; % receiver noise figure in dB in UL
uln = 10.^((ndBm-30+ulnf)/10); %% noise in W in UL

ber = 0.00005;
gamma = -log (5*ber)./1.5;



%% distances
%%         d2  
%%  UE1-----------eNB2
%%   |             |
%% d1|             |d1
%%   |     d2      |
%%  eNB1----------UE2
%%

d1 = 50;

d2v = [50 200 500];


%% to get the MCS, you need to do a manual lookup into 3GPP R1-081483
%% starting from the spectral efficiency value.
%% See the Testing section in the LTE module documentation for more info
%% on how this is done. You might as well look into lte_amc.m


%% MCS corresponding to d1 and d2 for the overlapping case
overlapMcsv = [2 14 22];
%% MCS is always 28 for the orthogonal case and the considered value of D1
orthogonalMcs = 28;

for ii = 1:3

  d2 = d2v(ii);
  overlapMcs = overlapMcsv(ii);
  
  %% propagation gains (linear)
  %%             g21dl  
  %%      UE1<----------eNB2
  %%      ^ |              
  %%g11dl1| |g11ul          
  %%      | v            
  %%      eNB1<---------UE2
  %%             g21ul

  g11dl = gain_freespace (d1, fdl);
  g11ul = gain_freespace (d1, ful);
  g21dl = gain_freespace (d2, fdl);
  g21ul = gain_freespace (d2, ful);

  %% SINR (linear)
  dlsinr = dlp*g11dl / (dlp*g21dl + dln);
  dlsinrabs = dlp*g11dl / (dln);
  ulsinr = ulp*g11ul / (ulp*g21ul + uln);
  ulsinrabs = ulp*g11ul / (uln);

  %% SINR (dB)
  dlsinrdB = 10.*log10(dlsinr);
  dlsinrabsdB = 10.*log10(dlsinrabs);
  ulsinrdB = 10.*log10(ulsinr);
  ulsinrdBabs = 10.*log10(ulsinrabs);

  %% Spectal Efficiency
  dlse = log2(1 + dlsinr./gamma);
  dlseabs = log2(1 + dlsinrabs./gamma);
  ulse = log2(1 + ulsinr./gamma);
  ulseabs = log2(1 + ulsinrabs./gamma);

  for repetition=1:3

    ## overlapping ABS patterns
    x = round (rand(1,40));


    ################################################################################
    ## WATCH OUT! std::bitset uses inverse order for string initialization
    ################################################################################
    
    ## make sure we can transmit MIB    
    if (x(40) && x(30) && x(20) && x(10))
      x(40) = 0;
    endif
    
    ## make sure this  can send SIB1
    if (x(25) && x(5))
      if (rand(1,1) < 0.5)
	x (25) = 0;
      else
	x (5) = 0;
      endif
    endif
      
           
    printf("AddTestCase (new LteInterferenceAbsTestCase (\"d1=%d, d2=%d\", % f, %f, \"",\
	   d1, d2, d1, d2);
    for ii=1:40
      printf("%d", x(ii));
    endfor
    printf ("\", \""); 
    for ii=1:40
      printf("%d", x(ii));
    endfor   
    printf("\", %f, %f, % f, %f, %d, %d), TestCase::EXTENSIVE);\n", \
		dlsinr, ulsinr, dlse, ulse, overlapMcs, overlapMcs);
    
  endfor

  
  for repetition=1:3

    ## orthogonal ABS patterns
    x = round (rand(1,40));

    
    ################################################################################
    ## WATCH OUT! std::bitset uses inverse order for string initialization
    ################################################################################
    
        
    ## make sure we can transmit MIB
    if (x(40) && x(30) && x(20) && x(10))
      x(10) = 0;
    endif
    ## make sure orthogonal one can transmit MIB
    if (!x(40) && !x(30) && !x(20) && !x(10))
      x(40) = 1;
    endif
    
    ## make sure this and orthogonal can send SIB1
    if (rand(1,1) < 0.5)
      x (25) = 0;
      x (5) = 1;
    else
      x (25) = 1;
      x (5) = 0;
    endif      
      
    
    y = !x;
    
    printf("AddTestCase (new LteInterferenceAbsTestCase (\"d1=%d, d2=%d\", % f, %f, \"",\
	   d1, d2, d1, d2);
    for ii=1:40
      printf("%d", x(ii));
    endfor
    printf ("\", \""); 
    for ii=1:40
      printf("%d", y(ii));
    endfor   
    printf("\", %f, %f, % f, %f, %d, %d), TestCase::EXTENSIVE);\n", \
		dlsinrabs, ulsinrabs, dlseabs, ulseabs, orthogonalMcs, orthogonalMcs);
    
  endfor
  
endfor
