This directory contains scripts for producing plots from coexistence
simulation results for downlink transmissions, according to 3GPP TSG
RAN1 WG1 contribution R1-156621 (November 2015).

http://www.3gpp.org/ftp/tsg_ran/WG1_RL1/TSGR1_83/Docs/R1-156621.zip

The plotting scripts rely on the gnuplot program being installed on the 
user's path.  The ns-3 libraries with examples enabled should also be
built before trying these scripts (and for faster execution, build
the optimized version of the libraries).

It also contains scripts for producing plots from subsequent publications.

1. Code status

The code in this repository is undergoing further development.  If one
wishes to reproduce more faithfully the plots depicted in the
above 3GPP contribution, one must work with a previous version of this
simulator (dated 29 November 2015), available from the repository at 
this link:

http://code.nsnam.org/laa/ns-3-lbt/archive/ee4c3f5f6064.zip

Please visit the main repository to look at various changesets
that have been contributed since that time, including changes
reflecting agreements that were reached at the 3GPP RAN1 December 
2015 meeting, as well as the most recent code revisions.

http://code.nsnam.org/laa

Over time, the models in this special ns-3 distribution are planned
to be migrated to mainline ns-3 releases.

2. How to reproduce

The scripts are written in bash and work as follows.

There are three sets of figures in the document.  The first is for
FTP over UDP, with a lambda FTP model 1 arrival rate of 2.5 (see 
3GPP TR36.814 for definition of this traffic model), and a maximum
TXOP time for LBT of 8 ms.

The figures show a progression of simulation cases, with throughput
(Figure 3-1) and latency (Figure 3-2) plotted.  These values are
observed using the ns-3 "flow monitor" tool which records statistics
on every flow it observes, at the IP layer.  Although 3GPP TR36.814 
FTP traffic is defined in the absence of IP (i.e., a raw file transfer
on top of Wi-Fi, not using IP), we use an IP traffic generator running
over UDP to obtain the benefit of using the IP-based ns-3 flow monitor.

The first directory 'laa-wifi-indoor-ftp' contains two scripts.  The
script 'run-laa-wifi-indoor-ed-ftp-wifi.sh' will run a number of 
simulations for different values of FTP lambda (over UDP) for the 'step 1' 
configuration of having two operators both configured for Wi-Fi, and
will run a number of simulations for different values of FTP lambda
and different energy detection thresholds for LAA (the 'step 2' 
configuration).  The results of the scripts are placed into a 'results'
directory.  The data used for the plots will be in files named with
file suffix '_operatorA' and '_operatorB'.  Each of these files contains
a line reporting file transfer statistics for each FTP flow.  e.g.

  laa_wifi_indoor_eD_-62.0_ftpLambda_2.5_cellA_Wifi_operatorA
  laa_wifi_indoor_eD_-62.0_ftpLambda_2.5_cellA_Wifi_operatorB

Each line in the file contains a record of statistics for an individual
flow in operator A or B network.  These space-delimited files are then
parsed by the plotting scripts described next; each line becomes a 
data point in the plots produced.

The next step is to convert these results into CDF plots as shown
in the document.  This is automated with the script 
'plot-laa-wifi-indoor-ed-ftp.sh'.  Image results (images produced
in multiple file formats) are placed in the 'images' directory.
A thumbnail of each figure can be browsed by opening the file
'images/thumbnails/index.html' in a web browser.

The simulation duration is scaled to account for differences in 
FTP arrival rate; lower values of lambda run for longer duration to 
try to equalize the number of data points across different traffic
intensities.

The second directory 'laa-wifi-indoor-tcp' runs a similar FTP but
on top of TCP transport rather than UDP.  The effects of TCP 
congestion control contribute to the performance.  The data produced
by these scripts correspond to Figures 3-3 and 3-4 in the contribution,
and while FTP transfers are able to achieve 80 Mbps in 80% of the 
cases for Wi-Fi on Wi-Fi step 1 scenario, the LAA on Wi-Fi is much
worse to Wi-Fi.  The underlying reason is due to the channel occupancy
due to LAA not providing Wi-Fi flows with enough of an opportunity
to build the congestion window to a high value.  We have mainly
experimented with TXOP values of 4 ms and 5 ms; the plots in the 
contribution are from a TXOP of 4 ms (the smallest tested).

Similar to the first directory, the programs 'run-laa-wifi-indoor-ftp-tcp.sh'
and 'plot-laa-wifi-indoor-ftp-tcp.sh' can be used to genenerate the
data and images, respectively.

The data presented at the end of section 3.2 corresponds to the
scripts run with some extra output (due to the patch 'newreno.patch') whose
output is captured and post processed with shell tools such as 'awk'
and 'sed'.  The patch prints out occurrences of window changes due
to slow start, congestion avoidance, fast retransmit, and hard
retransmit.

The third directory 'laa-wifi-indoor-udp-full-buffer' contains scripts
to generate results similar to those of Table 3-1 and of Figures 3-5 to
3-8.  Rather than use the FTP model 1 traffic generator, UDP traffic
is streamed at a constant bit rate (according to the parameter 'udpRate')
from the basestation/AP to all client devices.  Aside from the usual
CDF plots, the Table 3-1 draws statistics from the other logs that are
generated.  As an example, we have already discussed that the main
IP flow monitor results are found in the two log files:

  laa_wifi_indoor_eD_-82.0_udpRate_75Kbps_cellA_Laa_operatorA
  laa_wifi_indoor_eD_-82.0_udpRate_75Kbps_cellA_Laa_operatorB

In addition, for each run, several other log files are generated:

  laa_wifi_indoor_eD_-82.0_udpRate_75Kbps_cellA_Laa_backoff_log
  laa_wifi_indoor_eD_-82.0_udpRate_75Kbps_cellA_Laa_cw_log
  laa_wifi_indoor_eD_-82.0_udpRate_75Kbps_cellA_Laa_fail_retries_log
  laa_wifi_indoor_eD_-82.0_udpRate_75Kbps_cellA_Laa_harq_feedback_log
  laa_wifi_indoor_eD_-82.0_udpRate_75Kbps_cellA_Laa_phy_log
  laa_wifi_indoor_eD_-82.0_udpRate_75Kbps_cellA_Laa_retries_log
  laa_wifi_indoor_eD_-82.0_udpRate_75Kbps_cellA_Laa_txop_log

These logs are generated by the LAA scenario helper by listening to
various trace sources in the code.

- 'backoff_log' will log each backoff value drawn by either wifi or LAA
- 'cw_log' will log each change to the contention window
- 'fail_retries_log' will log each Wi-Fi failure of frame transmission
- 'harq_feedback_log' will log LTE HARQ feedback statistics
- 'phy_log' will log each LTE and Wi-Fi signal on the channel
- 'retries_log' will log each Wi-Fi retransmission
- 'txop_log' will log each LAA TXOP

Additional directories
------------------------
The directory 'laa-wifi-indoor-ftp-bs-placement' is similar to
'laa-wifi-indoor-ftp' but with the base stations placed in the corner
of the room.  This leads to reduced performance due to an increase
in average distance between UE and base station, as well as more
hidden terminals.  This alternative BS placement is controlled by
the global value "bsCornerPlacement".  As a result, the scripts experiment
with configuring a different energy detection threshold for Wi-Fi,
to improve LAA performance (make Wi-Fi more sensitive to LAA transmissions).

The directory 'laa-wifi-indoor-ftp-vary-drs' produces a plot that
contrasts the FTP performance when a DRS interval of 40 ms is used
instead of a DRS interval of 160 ms.

These scripts are for a simulation duration of over 300 seconds, so
