/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2015 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
 * Copyright (c) 2015 University of Washington
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Authors: Nicola Baldo <nbaldo@cttc.es> and Tom Henderson <tomh@tomh.org>
 */

#ifndef LAA_SCENARIO_HELPER_H
#define LAA_SCENARIO_HELPER_H

#include <ns3/core-module.h>
#include <ns3/lte-module.h>
#include <ns3/wifi-module.h>
#include <ns3/network-module.h>
#include <ns3/spectrum-module.h>
#include <ns3/flow-monitor-module.h>

using namespace ns3;

enum Config_e
{
  WIFI,
  LTE,
  LAA
};

enum Transport_e
{
  FTP,
  TCP,
  UDP
};

enum Standard_e
{
  St80211a,
  St80211n,
  St80211nFull
};

struct PhyParams
{
  double m_bsTxGain; // dBi
  double m_bsRxGain; // dBi
  double m_bsTxPower; // dBm
  double m_bsNoiseFigure; // dB
  double m_ueTxGain; // dB
  double m_ueRxGain; // dB
  double m_ueTxPower; // dBm
  double m_ueNoiseFigure; // dB
};


void
PrintFlowMonitorStats (Ptr<FlowMonitor> monitor, FlowMonitorHelper& flowmonHelper, double duration);

void
ConfigureLte (Ptr<LteHelper> lteHelper, Ptr<PointToPointEpcHelper> epcHelper, Ipv4AddressHelper& internetIpv4Helper, NodeContainer bsNodes, NodeContainer ueNodes, NodeContainer clientNodes, NetDeviceContainer& bsDevices, NetDeviceContainer& ueDevices, struct PhyParams phyParams, std::vector<LteSpectrumValueCatcher>& lteDlSinrCatcherVector, std::bitset<40> absPattern, Transport_e transport);

NetDeviceContainer
ConfigureWifiAp (NodeContainer bsNodes, struct PhyParams phyParams, Ptr<SpectrumChannel> channel, Ssid ssid);

NetDeviceContainer
ConfigureWifiSta (NodeContainer ueNodes, struct PhyParams phyParams, Ptr<SpectrumChannel> channel, Ssid ssid);

void
ConfigureMonitor (NodeContainer bsNodesA, struct PhyParams phyParams, Ptr<SpectrumChannel> channel);

ApplicationContainer
ConfigureUdpServers (NodeContainer servers, Time startTime, Time stopTime);

ApplicationContainer
ConfigureUdpClients (NodeContainer client, Ipv4InterfaceContainer servers, Time startTime, Time stopTime, Time interval);

ApplicationContainer
ConfigureTcpServers (NodeContainer servers, Time startTime);

ApplicationContainer
ConfigureTcpClients (NodeContainer client, Ipv4InterfaceContainer servers, Time startTime);

void
StartFileTransfer (Ptr<ExponentialRandomVariable> ftpArrivals, ApplicationContainer clients, uint32_t nextClient, Time stopTime);

void
ConfigureAndRunScenario (Config_e cellConfigA,
                         Config_e cellConfigB,
                         NodeContainer bsNodesA,
                         NodeContainer bsNodesB,
                         NodeContainer ueNodesA,
                         NodeContainer ueNodesB,
                         struct PhyParams phyParams,
                         Time duration,
                         Transport_e transport,
                         std::string propagationLossModel,
                         bool disableApps,
                         double lteDutyCycle,
                         bool generateRem,
                         std::string outFileName,
                         std::string simulationParams);


#endif

