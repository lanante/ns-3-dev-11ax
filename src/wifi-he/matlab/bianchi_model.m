%--------------------------------------------------------------------------
N_min = 5; %number of stations (upstream!)
N_max = 40; %number of stations (upstream!)
N_step = 5; % step for the simulation loop
T_PROP = 0.000000; %propagation delay (s)
L_DATA = 1500 * 8; %payload size (bit)
RTS_mode = 0; % Basic mode (0) or Rts/Cts mode (1)
fer = 0.0; %frame error rate
standard = '11ac';
PLCP_mode = 'MIXED_MODE';
Data_Rate = 65000000; % bit/s
Basic_Rate = 24000000; % bit/s
Guard_Interval = 0.000000800; % guard interval in seconds
MIMO_streams = 1;
Aggregation_Type = 'A_MPDU'; %A_MPDU or A_MSDU (HYBRID not fully supported)
K_MSDU = 1;
K_MPDU = 2;
useExplicitBar = 1; %currently only supported for 11ac and 11ax
%--------------------------------------------------------------------------

fer = 1-((1-fer)^K_MSDU)

results=zeros(ceil((N_max-N_min)/N_step),3);
indice = 0;

for N=N_min:N_step:N_max

    %802.11b parameters
    if strcmp(standard, '11b') == 1
        Plcp_Header = 48; % bits
        if strcmp(PLCP_mode, 'DSSS_SHORT') == 1
            Plcp_Preamble = 72; % bits
            T_DSSS = (Plcp_Header/1000000) + (Plcp_Preamble/2000000); % microseconds
        else
            Plcp_Preamble = 144; % bits
            T_DSSS = ((Plcp_Header + Plcp_Preamble)/1000000); % microseconds
        end
        L_MAC = 28 * 8; % bits
        L_ACK = 14 * 8; % bits
        L_CTS = 14 * 8; % bits
        L_RTS = 20 * 8; % bits
        SLOT = 0.000020; % seconds
        SIFS = 0.000010; % seconds
        DIFS = SIFS + (2 * SLOT); % seconds
        CW_MIN = 31;
        m = 5;
        ACK_timeout = 0.000340; % seconds
        CTS_timeout = 0.000340; % seconds  
    end

    %802.11a parameters
    if strcmp(standard, '11a') == 1
        Symbol_Duration = 0.0000032 + Guard_Interval; % seconds
        Symbols_per_second = 1/Symbol_Duration;
        Data_bits_per_symbol = Data_Rate/Symbols_per_second; %data bits per OFDM symbol for a data packet
        Control_bits_per_symbol = Basic_Rate/Symbols_per_second; %data bits per OFDM symbol for a control packet
        if strcmp(PLCP_mode, 'LEGACY') == 1
            T_LPHY = 0.000016 + 0.000004; % seconds
        end
        L_SERVICE = 16; % bits
        L_TAIL = 6; % bits
        L_MAC = 28 * 8; % bits
        L_ACK = 14 * 8; % bits
        L_CTS = 14 * 8; % bits
        L_RTS = 20 * 8; % bits
        SLOT = 0.000009; % seconds
        SIFS = 0.000016; %sifs value (s)
        DIFS = SIFS + (2 * SLOT); % seconds
        CW_MIN = 15; %minimum contention window
        m = 6; %maximum backoff stage
        ACK_timeout = 0.000075; % seconds
        CTS_timeout = 0.000075; % seconds  
    end

    %802.11g parameters
    if strcmp(standard, '11g') == 1
        Symbol_Duration = 0.0000032 + Guard_Interval; % seconds
        Symbols_per_second = 1/Symbol_Duration;
        Data_bits_per_symbol = Data_Rate/Symbols_per_second; %data bits per OFDM symbol for a data packet
        Control_bits_per_symbol = Basic_Rate/Symbols_per_second; %data bits per OFDM symbol for a control packet
        if strcmp(PLCP_mode, 'LEGACY') == 1
            T_LPHY = 0.000016 + 0.000004; % seconds
        end
        Signal_Extension = 0.000006; % seconds
        L_SERVICE = 16; % bits
        L_TAIL = 6; % bits
        L_MAC = 28 * 8; % bits
        L_ACK = 14 * 8; % bits
        L_CTS = 14 * 8; % bits
        L_RTS = 20 * 8; % bits
        SLOT = 0.000009; % seconds
        SIFS = 0.000010; % seconds
        DIFS = SIFS + (2 * SLOT); % seconds
        CW_MIN = 15; %minimum contention window
        m = 6; %maximum backoff stage (CWmax = m * CWmin)
        ACK_timeout = 0.000340; % seconds
        CTS_timeout = 0.000340; % seconds  
    end

    %802.11n parameters
    if strcmp(standard, '11n_5GHz') == 1
        Legacy_Symbol_Duration = 0.000004;
        Symbol_Duration = 0.0000032 + Guard_Interval; % seconds
        Symbols_per_second = 1/Symbol_Duration;
        Data_bits_per_symbol = Data_Rate/Symbols_per_second; %data bits per OFDM symbol for a data packet
        Control_bits_per_symbol = Basic_Rate/Symbols_per_second; %data bits per OFDM symbol for a control packet
        T_LPHY = 0.000016 + 0.000004; % seconds
        if strcmp(PLCP_mode, 'LEGACY') == 1
            T_PHY = T_LPHY; %microseconds
        else
            if strcmp(PLCP_mode, 'MIXED_MODE') == 1
                T_PHY = T_LPHY + 0.000008 + 0.000004 + (0.000004 * MIMO_streams); %microseconds
            else
                if strcmp(PLCP_mode, 'GREENFIELD') == 1
                    T_PHY = 0.000008 + 0.000008 + 0.000008 + (0.000004 * MIMO_streams); %microseconds
                end
            end
        end
        L_SERVICE = 16; % bits
        L_TAIL = 6; % bits
        L_MAC = 30 * 8; % bits
        L_MSDU_HEADER = 14 * 8; % bits
        L_MPDU_HEADER = 4 * 8; % bits
        L_ACK = 14 * 8; % bits
        L_CTS = 14 * 8; % bits
        L_RTS = 20 * 8; % bits
        L_BACK = 32 * 8; %bits
        L_BAR = 24 * 8; %bits
        AIFSN = 3; %AIFSN value for AC_BE (Best Effort)
        SLOT = 0.000009; % seconds
        SIFS = 0.000016; %sifs value (s)
        DIFS = SIFS + (AIFSN * SLOT); % seconds
        CW_MIN = 15; %minimum contention window
        m = 6; %maximum backoff stage
        ACK_timeout = 0.000088; % seconds
        CTS_timeout = 0.000075; % seconds
    end

    if strcmp(standard, '11n_2_4GHz') == 1
        Legacy_Symbol_Duration = 0.000004;
        Symbol_Duration = 0.0000032 + Guard_Interval; % seconds
        Data_bits_per_symbol = (Data_Rate * Symbol_Duration); %data bits per OFDM symbol for a data packet
        Control_bits_per_symbol = (Basic_Rate * 0.000004); %data bits per OFDM symbol for a control packet
        T_LPHY = 0.000016 + 0.000004; % seconds
        if strcmp(PLCP_mode, 'LEGACY') == 1
            T_PHY = T_LPHY; %microseconds
        else
            if strcmp(PLCP_mode, 'MIXED_MODE') == 1
                T_PHY = T_LPHY + 0.000008 + 0.000004 + (0.000004 * MIMO_streams); %microseconds
            else
                if strcmp(PLCP_mode, 'GREENFIELD') == 1
                    T_PHY = 0.000008 + 0.000008 + 0.000008 + (0.000004 * MIMO_streams); %microseconds
                end
            end
        end
        Signal_Extension = 0.000006; % seconds
        L_SERVICE = 16; % bits
        L_TAIL = 6; % bits
        L_MAC = 30 * 8; % bits
        L_MSDU_HEADER = 14 * 8; % bits
        L_MPDU_HEADER = 4 * 8; % bits
        L_ACK = 14 * 8; % bits
        L_CTS = 14 * 8; % bits
        L_RTS = 20 * 8; % bits
        L_BACK = 32 * 8; %bits
        AIFSN = 3; %AIFSN value for AC_BE (Best Effort)
        SLOT = 0.000020 %0.000009; % seconds
        SIFS = 0.000010; %sifs value (s)
        DIFS = SIFS + (AIFSN * SLOT); % seconds
        CW_MIN = 15; %minimum contention window
        m = 6; %maximum backoff stage
        ACK_timeout = 0.000340; % seconds
        CTS_timeout = 0.000340; % seconds
    end

    %802.11ac parameters
    if strcmp(standard, '11ac') == 1
        Legacy_Symbol_Duration = 0.000004;
        Symbol_Duration = 0.0000032 + Guard_Interval; % seconds
        Symbols_per_second = 1/Symbol_Duration;
        Data_bits_per_symbol = Data_Rate/Symbols_per_second; %data bits per OFDM symbol for a data packet
        Control_bits_per_symbol = Basic_Rate/Symbols_per_second; %data bits per OFDM symbol for a control packet
        T_LPHY = 0.000016 + 0.000004; % seconds
        T_PHY = T_LPHY + 0.000008 + 0.000004 + (0.000004*MIMO_streams) + 0.000004; %microseconds
        Signal_Extension = 0.000000; % seconds
        L_SERVICE = 16; % bits
        L_TAIL = 6; % bits
        L_MAC = 30 * 8; %30 * 8; % bits
        L_MSDU_HEADER = 14 * 8; % bits
        L_MPDU_HEADER = 4 * 8; % bits
        L_ACK = 14 * 8; % bits
        L_CTS = 14 * 8; % bits
        L_RTS = 20 * 8; % bits
        L_BACK = 32 * 8; %bits
        L_BAR = 24 * 8; %bits
        AIFSN = 3; %AIFSN value for AC_BE (Best Effort)
        SLOT = 0.000009; % seconds
        SIFS = 0.000016; %sifs value (s)
        DIFS = SIFS + (AIFSN * SLOT); % seconds
        CW_MIN = 15; %minimum contention window
        m = 6; %maximum backoff stage
        BACK_timeout = 0.000101; % seconds
        ACK_timeout = 0.000088; % seconds
        CTS_timeout = 0.000075; % seconds  
    end

    %802.11ax parameters
    if strcmp(standard, '11ax_2_4GHz') == 1
        Legacy_Symbol_Duration = 0.000004;
        Symbol_Duration = 0.0000032 + Guard_Interval; % seconds
        Data_bits_per_symbol = (Data_Rate * Symbol_Duration); %data bits per OFDM symbol for a data packet
        Control_bits_per_symbol = (Basic_Rate * 0.000004); %data bits per OFDM symbol for a control packet
        T_LPHY = 0.000016 + 0.000004; % seconds
        T_PHY = T_LPHY + 0.000008 + 0.000004 + (0.000004 * MIMO_streams); %microseconds
        Signal_Extension = 0.000006; % seconds
        L_SERVICE = 16; % bits
        L_TAIL = 6; % bits
        L_MAC = 30 * 8; % bits
        L_MSDU_HEADER = 14 * 8; % bits
        L_MPDU_HEADER = 4 * 8; % bits
        L_ACK = 14 * 8; % bits
        L_CTS = 14 * 8; % bits
        L_RTS = 20 * 8; % bits
        L_BACK = 32 * 8; %bits
        AIFSN = 3; %AIFSN value for AC_BE (Best Effort)
        SLOT = 0.000020 %0.000009; % seconds
        SIFS = 0.000010; %sifs value (s)
        DIFS = SIFS + (AIFSN * SLOT); % seconds
        CW_MIN = 15; %minimum contention window
        m = 6; %maximum backoff stage
        ACK_timeout = 0.000340; % seconds
        CTS_timeout = 0.000340; % seconds
    end

    if strcmp(standard, '11ax_5GHz') == 1
        Legacy_Symbol_Duration = 0.000004;
        Symbol_Duration = 0.0000032 + Guard_Interval; % seconds
        Symbols_per_second = 1/Symbol_Duration;
        Data_bits_per_symbol = Data_Rate/Symbols_per_second; %data bits per OFDM symbol for a data packet
        Control_bits_per_symbol = Basic_Rate/Symbols_per_second; %data bits per OFDM symbol for a control packet
        T_LPHY = 0.000016 + 0.000004; % seconds
        if strcmp(PLCP_mode, 'LEGACY') == 1
            T_PHY = T_LPHY; %microseconds
        else
            if strcmp(PLCP_mode, 'MIXED_MODE') == 1
                T_PHY = T_LPHY + 0.000008 + 0.000004 + (0.000004*MIMO_streams) + 0.000004; %microseconds
            end
        end
        Signal_Extension = 0.000000; % seconds
        L_SERVICE = 16; % bits
        L_TAIL = 6; % bits
        L_MAC = 30 * 8; %30 * 8; % bits
        L_MSDU_HEADER = 14 * 8; % bits
        L_MPDU_HEADER = 4 * 8; % bits
        L_ACK = 14 * 8; % bits
        L_CTS = 14 * 8; % bits
        L_RTS = 20 * 8; % bits
        L_BACK = 32 * 8; %bits
        L_BAR = 24 * 8; %bits
        AIFSN = 3; %AIFSN value for AC_BE (Best Effort)
        SLOT = 0.000009; % seconds
        SIFS = 0.000016; %sifs value (s)
        DIFS = SIFS + (AIFSN * SLOT); % seconds
        CW_MIN = 15; %minimum contention window
        m = 6; %maximum backoff stage
        BACK_timeout = 0.000116; % seconds
        ACK_timeout = 0.000088; % seconds
        CTS_timeout = 0.000075; % seconds  
    end

    if strcmp(standard, '11b') == 1
        if RTS_mode == 0
            Ts = T_DSSS + ((L_MAC+L_DATA+(36*8))/Data_Rate) + SIFS + T_PROP + T_DSSS + (L_ACK/Basic_Rate) + DIFS + T_PROP; %time spent by a successful transmission
            Tc = T_DSSS + ((L_MAC+L_DATA+(36*8))/Data_Rate) + ACK_timeout + DIFS; %time spent in collision
            Tf = Tc;
        else
            Ts = T_DSSS + (L_RTS/Basic_Rate) + SIFS + T_PROP + T_DSSS + (L_CTS/Basic_Rate) + SIFS + T_PROP + T_DSSS + ((L_MAC+L_DATA+(36*8))/Data_Rate) + SIFS + T_PROP + T_DSSS + (L_ACK/Basic_Rate) + DIFS + T_PROP;
            Tc = T_DSSS + (L_RTS/Basic_Rate) + CTS_timeout + DIFS; %time spent in collision
            Tf = T_DSSS + ((L_MAC+L_DATA+(36*8))/Data_Rate) + ACK_timeout + DIFS; %time spent in collision
        end
    end

    if strcmp(standard, '11a') == 1
        if RTS_mode == 0
            Ts = T_LPHY + (Symbol_Duration * ceil((L_SERVICE + L_MAC + L_DATA + (36 * 8) + L_TAIL)/Data_bits_per_symbol)) + SIFS + T_PROP + T_LPHY + (Symbol_Duration * ceil((L_SERVICE + L_ACK + L_TAIL)/Control_bits_per_symbol)) + DIFS + T_PROP; %time spent by a successful transmission
            Tc = T_LPHY + (Symbol_Duration * ceil((L_SERVICE + L_MAC + L_DATA + (36 * 8) + L_TAIL)/Data_bits_per_symbol)) + ACK_timeout + DIFS; %time spent in collision
            Tf = Tc;
        else
            Ts = T_LPHY + (Symbol_Duration * ceil((L_SERVICE + L_RTS + L_TAIL)/Control_bits_per_symbol)) + SIFS + T_PROP + T_LPHY + (Symbol_Duration * ceil((L_SERVICE + L_CTS + L_TAIL)/Control_bits_per_symbol)) + SIFS + T_PROP + T_LPHY + (Symbol_Duration * ceil((L_SERVICE + L_MAC + L_DATA + (28 *8) + L_TAIL)/Data_bits_per_symbol)) + SIFS + T_PROP + T_LPHY + (Symbol_Duration * ceil((L_SERVICE + L_ACK + L_TAIL)/Control_bits_per_symbol)) + DIFS + T_PROP; %time spent by a successful transmission
            Tc = T_LPHY + (Symbol_Duration * ceil((L_SERVICE + L_RTS + L_TAIL)/Control_bits_per_symbol)) + CTS_timeout + DIFS; %time spent in collision
            Tf = T_LPHY + (Symbol_Duration * ceil((L_SERVICE + L_MAC + L_DATA + (36 * 8) + L_TAIL)/Data_bits_per_symbol)) + ACK_timeout + DIFS; %time spent in collision
        end
    end

    if strcmp(standard, '11g') == 1
        if RTS_mode == 0
            Ts = T_LPHY + (Symbol_Duration * ceil((L_SERVICE + L_MAC + L_DATA + (36*8) + L_TAIL)/Data_bits_per_symbol)) + Signal_Extension + SIFS + T_PROP + T_LPHY + (Symbol_Duration * ceil((L_SERVICE + L_ACK + L_TAIL)/Control_bits_per_symbol)) + Signal_Extension + DIFS + T_PROP; %time spent by a successful transmission
            Tc = T_LPHY + (Symbol_Duration * ceil((L_SERVICE + L_MAC + L_DATA + (36*8) + L_TAIL)/Data_bits_per_symbol)) + Signal_Extension + ACK_timeout + DIFS; %time spent in collision
            Tf = Tc; %time spent in erroneous transmission
        else
            Ts = T_LPHY + (Symbol_Duration * ceil((L_SERVICE + L_RTS + L_TAIL)/Control_bits_per_symbol)) + Signal_Extension + SIFS + T_PROP + T_LPHY + (Symbol_Duration * ceil((L_SERVICE + L_CTS + L_TAIL)/Control_bits_per_symbol)) + Signal_Extension + SIFS + T_PROP + T_LPHY + (Symbol_Duration * ceil((L_SERVICE + L_MAC + L_DATA + (28 *8) + L_TAIL)/Data_bits_per_symbol)) + Signal_Extension + SIFS + T_PROP + T_LPHY + (Symbol_Duration * ceil((L_SERVICE + L_ACK + L_TAIL)/Control_bits_per_symbol)) + Signal_Extension + DIFS + T_PROP; %time spent by a successful transmission
            Tc = T_LPHY + (Symbol_Duration * ceil((L_SERVICE + L_RTS + L_TAIL)/Control_bits_per_symbol)) + Signal_Extension + CTS_timeout + DIFS; %time spent in collision
            Tf = T_LPHY + (Symbol_Duration * ceil((L_SERVICE + L_MAC + L_DATA + (28 *8) + L_TAIL)/Data_bits_per_symbol)) + Signal_Extension + ACK_timeout + DIFS; %time spent in erroneous transmission
        end
    end

    if strcmp(standard, '11n_5GHz') == 1
        if (strcmp(Aggregation_Type, 'NONE') == 1)
            if RTS_mode == 0
                Ts = T_PHY + (Symbol_Duration * ceil((L_SERVICE + L_MAC + L_DATA + (36*8) + L_TAIL)/Data_bits_per_symbol)) + SIFS + T_PROP + T_LPHY + (Legacy_Symbol_Duration * ceil((L_SERVICE + L_ACK + L_TAIL)/Control_bits_per_symbol)) + DIFS + T_PROP; %time spent by a successful transmission
                Tc = T_PHY + (Symbol_Duration * ceil((L_SERVICE + L_MAC + L_DATA + (36*8) + L_TAIL)/Data_bits_per_symbol)) + T_PROP + ACK_timeout + DIFS; %time spent in collision
                Tf = Tc;
            else
                Ts = T_LPHY + (Legacy_Symbol_Duration * ceil((L_SERVICE + L_RTS + L_TAIL)/Control_bits_per_symbol)) + SIFS + T_PROP + T_LPHY + (Legacy_Symbol_Duration * ceil((L_SERVICE + L_CTS + L_TAIL)/Control_bits_per_symbol)) + SIFS + T_PROP + T_PHY + (Symbol_Duration * ceil((L_SERVICE + L_MAC + L_DATA + (36*8) + L_TAIL)/Data_bits_per_symbol)) + SIFS + T_PROP + T_LPHY + (Legacy_Symbol_Duration * ceil((L_SERVICE + L_ACK + L_TAIL)/Control_bits_per_symbol)) + DIFS + T_PROP; %time spent by a successful transmission
                Tc = T_LPHY + (Legacy_Symbol_Duration * ceil((L_SERVICE + L_RTS + L_TAIL)/Control_bits_per_symbol)) + DIFS + CTS_timeout; %time spent in collision
                Tf = Tc;
            end
        end
        if (strcmp(Aggregation_Type, 'A_MSDU') == 1)
            if RTS_mode == 0
                Ts = T_PHY + (Symbol_Duration * ceil((L_SERVICE + L_MAC + K_MSDU*(L_MSDU_HEADER + L_DATA + (36*8)) + L_TAIL)/Data_bits_per_symbol)) + SIFS + T_PROP + T_LPHY + (Legacy_Symbol_Duration * ceil((L_SERVICE + L_ACK + L_TAIL)/Control_bits_per_symbol)) + DIFS + T_PROP; %time spent by a successful transmission
                Tc = T_PHY + (Symbol_Duration * ceil((L_SERVICE + L_MAC + K_MSDU*(L_MSDU_HEADER + L_DATA + (36*8)) + L_TAIL)/Data_bits_per_symbol)) + T_PROP + ACK_timeout + DIFS; %time spent in collision
                Tf = Tc;
            else
                Ts = T_LPHY + (Legacy_Symbol_Duration * ceil((L_SERVICE + L_RTS + L_TAIL)/Control_bits_per_symbol)) + SIFS + T_PROP + T_LPHY + (Legacy_Symbol_Duration * ceil((L_SERVICE + L_CTS + L_TAIL)/Control_bits_per_symbol)) + SIFS + T_PROP + T_PHY + (Symbol_Duration * ceil((L_SERVICE + L_MAC + K_MSDU*(L_MSDU_HEADER + L_DATA + (36*8)) + L_TAIL)/Data_bits_per_symbol)) + SIFS + T_PROP + T_LPHY + (Legacy_Symbol_Duration * ceil((L_SERVICE + L_ACK + L_TAIL)/Control_bits_per_symbol)) + DIFS + T_PROP; %time spent by a successful transmission
                Tc = T_LPHY + (Legacy_Symbol_Duration * ceil((L_SERVICE + L_RTS + L_TAIL)/Control_bits_per_symbol)) + T_PROP + CTS_timeout + DIFS; %time spent in collision
                Tf = Tc;
            end
        end
        if strcmp(Aggregation_Type, 'A_MPDU') == 1
            T_BACKOFF = CW_MIN * SLOT/2;
            if RTS_mode == 0
                Ts = T_PHY + (Symbol_Duration * ceil((L_SERVICE + K_MPDU * (L_MAC + L_MPDU_HEADER + L_DATA + (36*8)) + L_TAIL)/Data_bits_per_symbol)) + SIFS + T_PROP + T_LPHY + (Legacy_Symbol_Duration * ceil((L_SERVICE + L_BACK + L_TAIL)/Control_bits_per_symbol)) + DIFS + T_PROP; %time spent by a successful transmission
                Tc = T_PHY + (Symbol_Duration * ceil((L_SERVICE + K_MPDU * (L_MAC + L_MPDU_HEADER + L_DATA + (36*8)) + L_TAIL)/Data_bits_per_symbol)) + T_PROP + ACK_timeout + DIFS + T_BACKOFF + T_LPHY + (Legacy_Symbol_Duration * ceil((L_SERVICE + L_BAR + L_TAIL)/Control_bits_per_symbol)) + SIFS + T_PROP + T_LPHY + (Legacy_Symbol_Duration * ceil((L_SERVICE + L_BACK + L_TAIL)/Control_bits_per_symbol)) + DIFS;
                Tf = Tc;
            else
                Ts = T_LPHY + (Legacy_Symbol_Duration * ceil((L_SERVICE + L_RTS + L_TAIL)/Control_bits_per_symbol)) + SIFS + T_PROP + T_LPHY + (Legacy_Symbol_Duration * ceil((L_SERVICE + L_CTS + L_TAIL)/Control_bits_per_symbol)) + SIFS + T_PROP + T_PHY + (Symbol_Duration * ceil((L_SERVICE + K_MPDU * (L_MAC + L_MPDU_HEADER + L_DATA + (36*8)) + L_TAIL)/Data_bits_per_symbol)) + SIFS + T_PROP + T_LPHY + (Legacy_Symbol_Duration * ceil((L_SERVICE + L_BACK + L_TAIL)/Control_bits_per_symbol)) + DIFS + T_PROP; %time spent by a successful transmission
                Tf = T_LPHY + (Legacy_Symbol_Duration * ceil((L_SERVICE + L_RTS + L_TAIL)/Control_bits_per_symbol)) + SIFS + T_PROP + T_LPHY + (Legacy_Symbol_Duration * ceil((L_SERVICE + L_CTS + L_TAIL)/Control_bits_per_symbol)) + SIFS + T_PROP + T_PHY + (Symbol_Duration * ceil((L_SERVICE + K_MPDU * (L_MAC + L_MPDU_HEADER + L_DATA + (36*8)) + L_TAIL)/Data_bits_per_symbol)) + T_PROP + ACK_timeout + DIFS + T_BACKOFF + T_LPHY + (Legacy_Symbol_Duration * ceil((L_SERVICE + L_BAR + L_TAIL)/Control_bits_per_symbol)) + SIFS + T_PROP + T_LPHY + (Legacy_Symbol_Duration * ceil((L_SERVICE + L_BACK + L_TAIL)/Control_bits_per_symbol)) + DIFS;
                Tc = T_LPHY + (Legacy_Symbol_Duration * ceil((L_SERVICE + L_RTS + L_TAIL)/Control_bits_per_symbol)) + T_PROP + CTS_timeout + DIFS; %time spent in collision
            end
        end
        if strcmp(Aggregation_Type, 'HYBRID') == 1
            T_BACKOFF = CW_MIN * SLOT/2;
            if RTS_mode == 0
                Ts = T_PHY + (Symbol_Duration * ceil((L_SERVICE + K_MPDU * (L_MAC + L_MPDU_HEADER + K_MSDU*(L_MSDU_HEADER + L_DATA + (36*8))) + L_TAIL)/Data_bits_per_symbol)) + SIFS + T_PROP + T_LPHY + (Legacy_Symbol_Duration * ceil((L_SERVICE + L_BACK + L_TAIL)/Control_bits_per_symbol)) + DIFS + T_PROP; %time spent by a successful transmission
                Tc = T_PHY + (Symbol_Duration * ceil((L_SERVICE + K_MPDU * (L_MAC + L_MPDU_HEADER + K_MSDU*(L_MSDU_HEADER + L_DATA + (36*8))) + L_TAIL)/Data_bits_per_symbol)) + T_PROP + ACK_timeout + DIFS + T_BACKOFF + T_LPHY + (Legacy_Symbol_Duration * ceil((L_SERVICE + L_BAR + L_TAIL)/Control_bits_per_symbol)) + SIFS + T_PROP + T_LPHY + (Legacy_Symbol_Duration * ceil((L_SERVICE + L_BACK + L_TAIL)/Control_bits_per_symbol)) + DIFS;
                Tf = Tc;
            else
                Ts = T_LPHY + (Legacy_Symbol_Duration * ceil((L_SERVICE + L_RTS + L_TAIL)/Control_bits_per_symbol)) + SIFS + T_PROP + T_LPHY + (Legacy_Symbol_Duration * ceil((L_SERVICE + L_CTS + L_TAIL)/Control_bits_per_symbol)) + SIFS + T_PROP + T_PHY + (Symbol_Duration * ceil((L_SERVICE + K_MPDU * (L_MAC + L_MPDU_HEADER + K_MSDU*(L_MSDU_HEADER + L_DATA + (36*8))) + L_TAIL)/Data_bits_per_symbol)) + SIFS + T_PROP + T_LPHY + (Legacy_Symbol_Duration * ceil((L_SERVICE + L_BACK + L_TAIL)/Control_bits_per_symbol)) + DIFS + T_PROP; %time spent by a successful transmission
                Tf = T_LPHY + (Legacy_Symbol_Duration * ceil((L_SERVICE + L_RTS + L_TAIL)/Control_bits_per_symbol)) + SIFS + T_PROP + T_LPHY + (Legacy_Symbol_Duration * ceil((L_SERVICE + L_CTS + L_TAIL)/Control_bits_per_symbol)) + SIFS + T_PROP + T_PHY + (Symbol_Duration * ceil((L_SERVICE + K_MPDU * (L_MAC + L_MPDU_HEADER + K_MSDU*(L_MSDU_HEADER + L_DATA + (36*8))) + L_TAIL)/Data_bits_per_symbol)) + T_PROP + ACK_timeout + DIFS + T_BACKOFF + T_LPHY + (Legacy_Symbol_Duration * ceil((L_SERVICE + L_BAR + L_TAIL)/Control_bits_per_symbol)) + SIFS + T_PROP + T_LPHY + (Legacy_Symbol_Duration * ceil((L_SERVICE + L_BACK + L_TAIL)/Control_bits_per_symbol)) + DIFS;
                Tc = T_LPHY + (Legacy_Symbol_Duration * ceil((L_SERVICE + L_RTS + L_TAIL)/Control_bits_per_symbol)) + T_PROP + CTS_timeout + DIFS; %time spent in collision
            end
        end
    end
    
    if strcmp(standard, '11n_2_4GHz') == 1
        if (strcmp(Aggregation_Type, 'NONE') == 1)
            if RTS_mode == 0
                Ts = T_PHY + (Symbol_Duration * ceil((L_SERVICE + L_MAC + L_DATA + (36*8) + L_TAIL)/Data_bits_per_symbol)) + Signal_Extension + SIFS + T_PROP + T_LPHY + (Legacy_Symbol_Duration * ceil((L_SERVICE + L_ACK + L_TAIL)/Control_bits_per_symbol)) + Signal_Extension + DIFS + T_PROP; %time spent by a successful transmission
                Tc = T_PHY + (Symbol_Duration * ceil((L_SERVICE + L_MAC + L_DATA + (36*8) + L_TAIL)/Data_bits_per_symbol)) + Signal_Extension + T_PROP + ACK_timeout + DIFS; %time spent in collision
                Tf = Tc;
            else
                Ts = T_LPHY + (Legacy_Symbol_Duration * ceil((L_SERVICE + L_RTS + L_TAIL)/Control_bits_per_symbol)) + Signal_Extension + SIFS + T_PROP + T_LPHY + (Legacy_Symbol_Duration * ceil((L_SERVICE + L_CTS + L_TAIL)/Control_bits_per_symbol)) + Signal_Extension + SIFS + T_PROP + T_PHY + (Symbol_Duration * ceil((L_SERVICE + L_MAC + K_MSDU*(L_DATA + (36*8)) + L_TAIL)/Data_bits_per_symbol)) + Signal_Extension + SIFS + T_PROP + T_LPHY + (Legacy_Symbol_Duration * ceil((L_SERVICE + L_ACK + L_TAIL)/Control_bits_per_symbol)) + Signal_Extension + DIFS + T_PROP; %time spent by a successful transmission
                Tc = T_LPHY + (Legacy_Symbol_Duration * ceil((L_SERVICE + L_RTS + L_TAIL)/Control_bits_per_symbol)) + Signal_Extension + DIFS + CTS_timeout; %time spent in collision
                Tf = T_LPHY + (Legacy_Symbol_Duration * ceil((L_SERVICE + L_RTS + L_TAIL)/Control_bits_per_symbol)) + Signal_Extension + SIFS + T_PROP + T_LPHY + (Legacy_Symbol_Duration * ceil((L_SERVICE + L_CTS + L_TAIL)/Control_bits_per_symbol)) + Signal_Extension + SIFS + T_PROP + T_PHY + (Symbol_Duration * ceil((L_SERVICE + L_MAC + L_DATA + (36*8) + L_TAIL)/Data_bits_per_symbol)) + Signal_Extension + T_PROP + ACK_timeout + DIFS;
            end
        end
        if (strcmp(Aggregation_Type, 'A_MSDU') == 1)
            if RTS_mode == 0
                Ts = T_PHY + (Symbol_Duration * ceil((L_SERVICE + L_MAC + K_MSDU*(L_MSDU_HEADER + L_DATA + (36*8)) + L_TAIL)/Data_bits_per_symbol)) + Signal_Extension + SIFS + T_PROP + T_LPHY + (Legacy_Symbol_Duration * ceil((L_SERVICE + L_ACK + L_TAIL)/Control_bits_per_symbol)) + Signal_Extension + DIFS + T_PROP; %time spent by a successful transmission
                Tc = T_PHY + (Symbol_Duration * ceil((L_SERVICE + L_MAC + K_MSDU*(L_MSDU_HEADER + L_DATA + (36*8)) + L_TAIL)/Data_bits_per_symbol)) + Signal_Extension + T_PROP + ACK_timeout + DIFS; %time spent in collision
                Tf = Tc; %Time spend in failure
            else
                Ts = T_LPHY + (Legacy_Symbol_Duration * ceil((L_SERVICE + L_RTS + L_TAIL)/Control_bits_per_symbol)) + Signal_Extension + SIFS + T_PROP + T_LPHY + (Legacy_Symbol_Duration * ceil((L_SERVICE + L_CTS + L_TAIL)/Control_bits_per_symbol)) + Signal_Extension + SIFS + T_PROP + T_PHY + (Symbol_Duration * ceil((L_SERVICE + L_MAC + K_MSDU*(L_MSDU_HEADER + L_DATA + (36*8)) + L_TAIL)/Data_bits_per_symbol)) + Signal_Extension + SIFS + T_PROP + T_LPHY + (Legacy_Symbol_Duration * ceil((L_SERVICE + L_ACK + L_TAIL)/Control_bits_per_symbol)) + Signal_Extension + DIFS + T_PROP; %time spent by a successful transmission
                Tc = T_LPHY + (Legacy_Symbol_Duration * ceil((L_SERVICE + L_RTS + L_TAIL)/Control_bits_per_symbol)) + Signal_Extension + T_PROP + CTS_timeout + DIFS; %time spent in collision
                Tf = Tc; %Time spend in failure
            end
        end
        if strcmp(Aggregation_Type, 'A_MPDU') == 1
            T_BACKOFF = CW_MIN * SLOT/2;
            if RTS_mode == 0
                Ts = T_PHY + (Symbol_Duration * ceil((L_SERVICE + K_MPDU * (L_MAC + L_MPDU_HEADER + L_DATA + (36*8)) + L_TAIL)/Data_bits_per_symbol)) + Signal_Extension + SIFS + T_PROP + T_LPHY + (Legacy_Symbol_Duration * ceil((L_SERVICE + L_BACK + L_TAIL)/Control_bits_per_symbol)) + Signal_Extension + DIFS + T_PROP; %time spent by a successful transmission
                Tf = T_PHY + (Symbol_Duration * ceil((L_SERVICE + K_MPDU * (L_MAC + L_MPDU_HEADER + L_DATA + (36*8)) + L_TAIL)/Data_bits_per_symbol)) + Signal_Extension + T_PROP + ACK_timeout + DIFS + T_BACKOFF + T_LPHY + (Legacy_Symbol_Duration * ceil((L_SERVICE + L_BAR + L_TAIL)/Control_bits_per_symbol)) + Signal_Extension + SIFS + T_PROP + T_LPHY + (Legacy_Symbol_Duration * ceil((L_SERVICE + L_BACK + L_TAIL)/Control_bits_per_symbol)) + Signal_Extension + DIFS;
                Tc = T_PHY + (Symbol_Duration * ceil((L_SERVICE + K_MPDU * (L_MAC + L_MPDU_HEADER + L_DATA + (36*8)) + L_TAIL)/Data_bits_per_symbol)) + Signal_Extension + T_PROP + ACK_timeout + DIFS;
            else
                Ts = T_LPHY + (Legacy_Symbol_Duration * ceil((L_SERVICE + L_RTS + L_TAIL)/Control_bits_per_symbol)) + Signal_Extension + SIFS + T_PROP + T_LPHY + (Legacy_Symbol_Duration * ceil((L_SERVICE + L_CTS + L_TAIL)/Control_bits_per_symbol)) + Signal_Extension + SIFS + T_PROP + T_PHY + (Symbol_Duration * ceil((L_SERVICE + K_MPDU * (L_MAC + L_MPDU_HEADER + L_DATA + (36*8)) + L_TAIL)/Data_bits_per_symbol)) + Signal_Extension + SIFS + T_PROP + T_LPHY + (Legacy_Symbol_Duration * ceil((L_SERVICE + L_BACK + L_TAIL)/Control_bits_per_symbol)) + Signal_Extension + DIFS + T_PROP; %time spent by a successful transmission
                Tf = T_LPHY + (Legacy_Symbol_Duration * ceil((L_SERVICE + L_RTS + L_TAIL)/Control_bits_per_symbol)) + Signal_Extension + SIFS + T_PROP + T_LPHY + (Legacy_Symbol_Duration * ceil((L_SERVICE + L_CTS + L_TAIL)/Control_bits_per_symbol)) + Signal_Extension + SIFS + T_PROP + T_PHY + (Symbol_Duration * ceil((L_SERVICE + K_MPDU * (L_MAC + L_MPDU_HEADER + L_DATA + (36*8)) + L_TAIL)/Data_bits_per_symbol)) + Signal_Extension + T_PROP + ACK_timeout + DIFS + T_BACKOFF + T_LPHY + (Legacy_Symbol_Duration * ceil((L_SERVICE + L_BAR + L_TAIL)/Control_bits_per_symbol)) + SIFS + T_PROP + T_LPHY + (Legacy_Symbol_Duration * ceil((L_SERVICE + L_BACK + L_TAIL)/Control_bits_per_symbol)) + Signal_Extension + DIFS;
                Tc = T_LPHY + (Legacy_Symbol_Duration * ceil((L_SERVICE + L_RTS + L_TAIL)/Control_bits_per_symbol)) + Signal_Extension + T_PROP + CTS_timeout + DIFS; %time spent in collision
            end
        end
        if strcmp(Aggregation_Type, 'HYBRID') == 1
            T_BACKOFF = CW_MIN * SLOT/2;
            if RTS_mode == 0
                Ts = T_PHY + (Symbol_Duration * ceil((L_SERVICE + K_MPDU * (L_MAC + L_MPDU_HEADER + K_MSDU*(L_MSDU_HEADER + L_DATA + (36*8))) + L_TAIL)/Data_bits_per_symbol)) + SIFS + T_PROP + T_LPHY + (Legacy_Symbol_Duration * ceil((L_SERVICE + L_BACK + L_TAIL)/Control_bits_per_symbol)) + DIFS + T_PROP; %time spent by a successful transmission
                Tc = T_PHY + (Symbol_Duration * ceil((L_SERVICE + K_MPDU * (L_MAC + L_MPDU_HEADER + K_MSDU*(L_MSDU_HEADER + L_DATA + (36*8))) + L_TAIL)/Data_bits_per_symbol)) + T_PROP + ACK_timeout + DIFS + T_BACKOFF + T_LPHY + (Legacy_Symbol_Duration * ceil((L_SERVICE + L_BAR + L_TAIL)/Control_bits_per_symbol)) + SIFS + T_PROP + T_LPHY + (Legacy_Symbol_Duration * ceil((L_SERVICE + L_BACK + L_TAIL)/Control_bits_per_symbol)) + DIFS;
                Tf = Tc;
            else
                Ts = T_LPHY + (Legacy_Symbol_Duration * ceil((L_SERVICE + L_RTS + L_TAIL)/Control_bits_per_symbol)) + SIFS + T_PROP + T_LPHY + (Legacy_Symbol_Duration * ceil((L_SERVICE + L_CTS + L_TAIL)/Control_bits_per_symbol)) + SIFS + T_PROP + T_PHY + (Symbol_Duration * ceil((L_SERVICE + K_MPDU * (L_MAC + L_MPDU_HEADER + K_MSDU*(L_MSDU_HEADER + L_DATA + (36*8))) + L_TAIL)/Data_bits_per_symbol)) + SIFS + T_PROP + T_LPHY + (Legacy_Symbol_Duration * ceil((L_SERVICE + L_BACK + L_TAIL)/Control_bits_per_symbol)) + DIFS + T_PROP; %time spent by a successful transmission
                Tf = T_LPHY + (Legacy_Symbol_Duration * ceil((L_SERVICE + L_RTS + L_TAIL)/Control_bits_per_symbol)) + SIFS + T_PROP + T_LPHY + (Legacy_Symbol_Duration * ceil((L_SERVICE + L_CTS + L_TAIL)/Control_bits_per_symbol)) + SIFS + T_PROP + T_PHY + (Symbol_Duration * ceil((L_SERVICE + K_MPDU * (L_MAC + L_MPDU_HEADER + K_MSDU*(L_MSDU_HEADER + L_DATA + (36*8))) + L_TAIL)/Data_bits_per_symbol)) + T_PROP + ACK_timeout + DIFS + T_BACKOFF + T_LPHY + (Legacy_Symbol_Duration * ceil((L_SERVICE + L_BAR + L_TAIL)/Control_bits_per_symbol)) + SIFS + T_PROP + T_LPHY + (Legacy_Symbol_Duration * ceil((L_SERVICE + L_BACK + L_TAIL)/Control_bits_per_symbol)) + DIFS;
                Tc = T_LPHY + (Legacy_Symbol_Duration * ceil((L_SERVICE + L_RTS + L_TAIL)/Control_bits_per_symbol)) + T_PROP + CTS_timeout + DIFS; %time spent in collision
            end
        end
    end

  if strcmp(standard, '11ac') == 1 || strcmp(standard, '11ax_5GHz') == 1 || strcmp(standard, '11ax_2_4GHz') == 1
      if (strcmp(Aggregation_Type, 'NONE') == 1)
          if RTS_mode == 0
              Ts = T_PHY + (Symbol_Duration * ceil((L_SERVICE + L_MAC + L_MPDU_HEADER + L_DATA + (36*8) + L_TAIL)/Data_bits_per_symbol)) + Signal_Extension + SIFS + T_PROP + T_LPHY + (Legacy_Symbol_Duration * ceil((L_SERVICE + L_ACK + L_TAIL)/Control_bits_per_symbol)) + Signal_Extension + DIFS + T_PROP; %time spent by a successful transmission
              Tc = T_PHY + (Symbol_Duration * ceil((L_SERVICE + L_MAC + L_MPDU_HEADER + L_DATA + (36*8) + L_TAIL)/Data_bits_per_symbol)) + Signal_Extension + T_PROP + ACK_timeout + DIFS;
              Tf = Tc;
          else
              Ts = T_LPHY + (Legacy_Symbol_Duration * ceil((L_SERVICE + L_RTS + L_TAIL)/Control_bits_per_symbol)) + Signal_Extension + SIFS + T_PROP + T_LPHY + (Legacy_Symbol_Duration * ceil((L_SERVICE + L_CTS + L_TAIL)/Control_bits_per_symbol)) + Signal_Extension + SIFS + T_PROP + T_PHY + (Symbol_Duration * ceil((L_SERVICE + L_MAC + L_MPDU_HEADER + L_DATA + (36*8) + L_TAIL)/Data_bits_per_symbol)) + SIFS + T_PROP + T_LPHY + (Legacy_Symbol_Duration * ceil((L_SERVICE + L_BACK + L_TAIL)/Control_bits_per_symbol)) + DIFS + T_PROP; %time spent by a successful transmission
              Tc = T_LPHY + (Legacy_Symbol_Duration * ceil((L_SERVICE + L_RTS + L_TAIL)/Control_bits_per_symbol)) + Signal_Extension + DIFS + CTS_timeout; %time spent in collision
              Tf = T_LPHY + (Legacy_Symbol_Duration * ceil((L_SERVICE + L_RTS + L_TAIL)/Control_bits_per_symbol)) + Signal_Extension + SIFS + T_PROP + T_LPHY + (Legacy_Symbol_Duration * ceil((L_SERVICE + L_CTS + L_TAIL)/Control_bits_per_symbol)) + Signal_Extension + SIFS + T_PROP + T_PHY + (Symbol_Duration * ceil((L_SERVICE + L_MAC + L_MPDU_HEADER + L_DATA + (36*8) + L_TAIL)/Data_bits_per_symbol)) + SIFS + T_PROP + ACK_timeout + DIFS;
          end
      end
      if ((strcmp(Aggregation_Type, 'A_MSDU') == 1) || (strcmp(Aggregation_Type, 'HYBRID')))
          if RTS_mode == 0
              Ts = T_PHY + (Symbol_Duration * ceil((L_SERVICE + K_MPDU * (L_MAC + L_MPDU_HEADER + K_MSDU*(L_MSDU_HEADER + L_DATA + (36*8))) + L_TAIL)/Data_bits_per_symbol)) + Signal_Extension + SIFS + T_PROP + T_LPHY + (Legacy_Symbol_Duration * ceil((L_SERVICE + L_ACK + L_TAIL)/Control_bits_per_symbol)) + Signal_Extension + DIFS + T_PROP; %time spent by a successful transmission
              Tc = T_PHY + (Symbol_Duration * ceil((L_SERVICE + K_MPDU * (L_MAC + L_MPDU_HEADER + K_MSDU*(L_MSDU_HEADER + L_DATA + (36*8))) + L_TAIL)/Data_bits_per_symbol)) + Signal_Extension + T_PROP + ACK_timeout + DIFS;
              Tf = Tc;
          else
              Ts = T_LPHY + (Legacy_Symbol_Duration * ceil((L_SERVICE + L_RTS + L_TAIL)/Control_bits_per_symbol)) + Signal_Extension + SIFS + T_PROP + T_LPHY + (Legacy_Symbol_Duration * ceil((L_SERVICE + L_CTS + L_TAIL)/Control_bits_per_symbol)) + Signal_Extension + SIFS + T_PROP + T_PHY + (Symbol_Duration * ceil((L_SERVICE + K_MPDU * (L_MAC + L_MPDU_HEADER + K_MSDU*(L_MSDU_HEADER + L_DATA + (36*8))) + L_TAIL)/Data_bits_per_symbol)) + Signal_Extension + SIFS + T_PROP + T_LPHY + (Legacy_Symbol_Duration * ceil((L_SERVICE + L_BACK + L_TAIL)/Control_bits_per_symbol)) + Signal_Extension + DIFS + T_PROP; %time spent by a successful transmission
              Tf = T_LPHY + (Legacy_Symbol_Duration * ceil((L_SERVICE + L_RTS + L_TAIL)/Control_bits_per_symbol)) + Signal_Extension + DIFS + CTS_timeout; %time spent in collision
          end
      end
      if strcmp(Aggregation_Type, 'A_MPDU') == 1
          if RTS_mode == 0
              Ts = T_PHY + (Symbol_Duration * ceil((L_SERVICE + K_MPDU * (L_MAC + L_MPDU_HEADER + L_DATA + (36*8)) + L_TAIL)/Data_bits_per_symbol)) + Signal_Extension + SIFS + T_PROP + T_LPHY + (Legacy_Symbol_Duration * ceil((L_SERVICE + L_BACK + L_TAIL)/Control_bits_per_symbol)) + Signal_Extension + DIFS + T_PROP; %time spent by a successful transmission
              if useExplicitBar == 1
                  T_BACKOFF = CW_MIN * SLOT/2;
                  Tc = T_PHY + (Symbol_Duration * ceil((L_SERVICE + K_MPDU * (L_MAC + L_MPDU_HEADER + L_DATA + (36*8)) + L_TAIL)/Data_bits_per_symbol)) + Signal_Extension + T_PROP + BACK_timeout + DIFS + T_BACKOFF + T_LPHY + (Legacy_Symbol_Duration * ceil((L_SERVICE + L_BAR + L_TAIL)/Control_bits_per_symbol)) + Signal_Extension + SIFS + T_PROP + T_LPHY + (Legacy_Symbol_Duration * ceil((L_SERVICE + L_BACK + L_TAIL)/Control_bits_per_symbol)) + Signal_Extension + DIFS + T_PROP;
              else
                  Tc = T_PHY + (Symbol_Duration * ceil((L_SERVICE + K_MPDU * (L_MAC + L_MPDU_HEADER + L_DATA + (36*8)) + L_TAIL)/Data_bits_per_symbol)) + Signal_Extension + T_PROP + BACK_timeout + DIFS;
              end
              Tf = Tc;
          else
              Ts = T_LPHY + (Legacy_Symbol_Duration * ceil((L_SERVICE + L_RTS + L_TAIL)/Control_bits_per_symbol)) + Signal_Extension + SIFS + T_PROP + T_LPHY + (Legacy_Symbol_Duration * ceil((L_SERVICE + L_CTS + L_TAIL)/Control_bits_per_symbol)) + Signal_Extension + SIFS + T_PROP + T_PHY + (Symbol_Duration * ceil((L_SERVICE + K_MPDU * (L_MAC + L_MPDU_HEADER + L_DATA + (36*8)) + L_TAIL)/Data_bits_per_symbol)) + Signal_Extension + SIFS + T_PROP + T_LPHY + (Legacy_Symbol_Duration * ceil((L_SERVICE + L_BACK + L_TAIL)/Control_bits_per_symbol)) + Signal_Extension + DIFS + T_PROP; %time spent by a successful transmission
              Tc = T_LPHY + (Legacy_Symbol_Duration * ceil((L_SERVICE + L_RTS + L_TAIL)/Control_bits_per_symbol)) + Signal_Extension + DIFS + CTS_timeout; %time spent in collision
          end
      end
  end
    
  if ((N >= 1) || (fer > 0))
      if strcmp(Aggregation_Type, 'A_MPDU') == 1 %TODO: hybrid!
          S=solve('x=(1-(((1-y)^(N-1))*(1-(fer^K_MPDU))))','y=((2-(4*x))/(((1-(2*x))*(CWmin+2))+(x*(CWmin+1)*(1-((2*x)^m)))))',strcat('K_MPDU=',num2str(K_MPDU)),strcat('fer=',num2str(fer)),strcat('N=',num2str(N)),strcat('CWmin=',num2str(CW_MIN)),strcat('m=',num2str(m))); %solving Bianchi's equations
      else
          S=solve('x=(1-(((1-y)^(N-1))*(1-fer)))','y=((2-(4*x))/(((1-(2*x))*(CWmin+2))+(x*(CWmin+1)*(1-((2*x)^m)))))',strcat('fer=',num2str(fer)),strcat('N=',num2str(N)),strcat('CWmin=',num2str(CW_MIN)),strcat('m=',num2str(m))); %solving Bianchi's equations
      end
    
      found = 0;
      for i=1:length(S.x)
          if ((isreal(S.x(i)) == 1) && (S.x(i)==(abs(S.x(i)))) && (S.x(i)<=1) && (found == 0))
              p=S.x(i);
              found = 1;
          end
          i = i + 1;
      end
      
      found = 0;
      for i=1:length(S.y)
          if ((isreal(S.y(i)) == 1) && (S.y(i)==(abs(S.y(i)))) && (S.y(i)<=1) && (found == 0))
              pi=S.y(i);
              found = 1;
          end
          i = i + 1;
      end    
  
      Ptr = (1-((1-pi)^N)); % at least one transmission in the considered slot time
      Tid = SLOT; % duration of idle period (= slot time)
      Pc = (1-((N*pi*(1-pi)^(N-1))/(1-(1-pi)^N))); % probability of a collision
      if strcmp(Aggregation_Type, 'A_MPDU') == 1 %TODO: hybrid!
          Ps = (((N*pi*(1-pi)^(N-1))/(1-(1-pi)^N))); %*(1-(fer^K_MPDU))); % probability of a successful transmission
          Pf = (((N*pi*(1-pi)^(N-1))/(1-(1-pi)^N))*(fer^K_MPDU)); % probability of a erroneous transmission
      else
          Ps = (((N*pi*(1-pi)^(N-1))/(1-(1-pi)^N))*(1-fer)); % probability of a successful transmission
          Pf = (((N*pi*(1-pi)^(N-1))/(1-(1-pi)^N))*fer); % probability of a erroneous transmission
      end
      
      if strcmp(Aggregation_Type, 'A_MPDU') == 1 %TODO: hybrid!
              Num = (1-fer)*Ps*Ptr*(L_DATA+(0*8))*K_MSDU*K_MPDU;
              Den = (1-(fer^K_MPDU))*Ps*Ptr*Ts+Ptr*Pc*Tc+Ptr*Pf*Tf+(1-Ptr)*Tid;
      else
              Num = Ps*Ptr*(L_DATA+(0*8))*K_MSDU*K_MPDU;
              Den = Ps*Ptr*Ts+Ptr*Pc*Tc+Ptr*Pf*Tf+(1-Ptr)*Tid;
      end
      
  else
      T_BACKOFF = CW_MIN * SLOT/2;
      Num = L_DATA*K_MSDU*K_MPDU;
      Den = Ts + T_BACKOFF;
  end
  
  indice = (indice + 1);
  
  N

  Goodput = Num/Den  % bit/s

  Efficiency = Goodput/Data_Rate
  
  results(indice,1) = N;
  results(indice,2) = Goodput / 1000000;
  results(indice,3) = Efficiency;
end

fid = fopen('Bianchi_Goodput.txt','at');
fprintf(fid,'%12.8f\n',results(:,2));
fclose(fid);

fid = fopen('Bianchi_efficiency.txt','at');
fprintf(fid,'%12.8f\n',results(:,3));
fclose(fid);

%--------------------------------------------------------------------------
