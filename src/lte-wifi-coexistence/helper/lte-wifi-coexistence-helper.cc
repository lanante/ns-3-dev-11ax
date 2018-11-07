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
 * Authors: Biljana Bojovic <bbojovic@cttc.es> and Tom Henderson <tomh@tomh.org>
 */

#include <ns3/lte-wifi-coexistence-helper.h>
#include <ns3/log.h>

#include <ns3/lte-enb-net-device.h>
#include <ns3/core-module.h>
#include <ns3/network-module.h>
#include <ns3/internet-module.h>
#include <ns3/point-to-point-module.h>
#include <ns3/lte-module.h>
#include <ns3/wifi-module.h>
#include <ns3/spectrum-module.h>
#include <ns3/applications-module.h>
#include <ns3/internet-module.h>
#include <ns3/propagation-module.h>
#include <ns3/config-store-module.h>
#include <ns3/flow-monitor-module.h>
#include <ns3/basic-lbt-access-manager.h>
#include <ns3/duty-cycle-access-manager.h>
#include <ns3/lbt-access-manager.h>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("LteWifiCoexistenceHelper");

NS_OBJECT_ENSURE_REGISTERED (LteWifiCoexistenceHelper);

LteWifiCoexistenceHelper::LteWifiCoexistenceHelper ()
{
  NS_LOG_FUNCTION (this);
}

LteWifiCoexistenceHelper::~LteWifiCoexistenceHelper ()
{
  NS_LOG_FUNCTION (this);
}

TypeId
LteWifiCoexistenceHelper::GetTypeId (void)
{

  static TypeId tid = TypeId ("ns3::LteWifiCoexistenceHelper")
    .SetParent<Object> ()
    .SetGroupName ("lte-wifi-coexistence")
    .AddAttribute ("ChannelAccessManagerType",
                   "The type of channel access manager to be used for eNBs. "
                   "The allowed values for this attributes are the type names "
                   "of any class inheriting from ns3::ChannelAccessManager. Default value"
                   "is ns3::ChannelAccessManager",
                   StringValue ("ns3::ChannelAccessManager"),
                   MakeStringAccessor (&LteWifiCoexistenceHelper::SetChannelAccessManagerType,
                                       &LteWifiCoexistenceHelper::GetChannelAccessManagerType),
                   MakeStringChecker ());
  return tid;

}

void
LteWifiCoexistenceHelper::SetChannelAccessManagerType (std::string type)
{
  NS_LOG_FUNCTION (this << type);
  m_channelAccessManagerFactory = ObjectFactory ();
  m_channelAccessManagerFactory.SetTypeId (type);
}

std::string
LteWifiCoexistenceHelper::GetChannelAccessManagerType () const
{
  return m_channelAccessManagerFactory.GetTypeId ().GetName ();
}

void
LteWifiCoexistenceHelper::DoDispose ()
{
  NS_LOG_FUNCTION (this);
  Object::DoDispose ();
}

void
LteWifiCoexistenceHelper::ConfigureEnbDevicesForLbt (NetDeviceContainer enbDevices, struct PhyParams phyParams)
{

  NS_LOG_FUNCTION (this);
  NS_ASSERT_MSG (enbDevices.GetN () > 0, "empty enb device container");
  NetDeviceContainer monitorDevices;

  for (NetDeviceContainer::Iterator i = enbDevices.Begin (); i != enbDevices.End (); ++i)
    {
      Ptr<Node> node = (*i)->GetNode ();
      // we need a spectrum channel in order to install wifi device on the same instance of spectrum channel
      Ptr<LteEnbNetDevice> lteEnbNetDevice = (*i)->GetObject<LteEnbNetDevice> ();
      Ptr<SpectrumChannel> downlinkSpectrumChannel = lteEnbNetDevice->GetPhy ()->GetDownlinkSpectrumPhy ()->GetChannel ();
      SpectrumWifiPhyHelper spectrumPhy = SpectrumWifiPhyHelper::Default ();
      spectrumPhy.SetChannel (downlinkSpectrumChannel);

      WifiHelper wifi;
      wifi.SetStandard (WIFI_PHY_STANDARD_80211n_5GHZ);
      WifiMacHelper mac;
      spectrumPhy.Set ("ChannelWidth", UintegerValue (20));
      spectrumPhy.Set ("TxGain", DoubleValue (phyParams.m_bsTxGain));
      spectrumPhy.Set ("RxGain", DoubleValue (phyParams.m_bsRxGain));
      spectrumPhy.Set ("TxPowerStart", DoubleValue (phyParams.m_bsTxPower));
      spectrumPhy.Set ("TxPowerEnd", DoubleValue (phyParams.m_bsTxPower));
      spectrumPhy.Set ("RxNoiseFigure", DoubleValue (phyParams.m_bsNoiseFigure));
      spectrumPhy.Set ("Antennas", UintegerValue (2));

      wifi.SetRemoteStationManager ("ns3::IdealWifiManager");
      //which implements a Wi-Fi MAC that does not perform any kind of beacon generation, probing, or association
      mac.SetType ("ns3::AdhocWifiMac");

      //uint32_t channelNumber = 36 + 4 * (i%4);
      uint32_t channelNumber = 36;
      spectrumPhy.Set ("ChannelNumber", UintegerValue(channelNumber));

      // wifi device that is doing monitoring
      Ptr<NetDevice> monitor = (wifi.Install (spectrumPhy, mac, node)).Get (0);
      Ptr<WifiPhy> wifiPhy = monitor->GetObject<WifiNetDevice> ()->GetPhy ();
      Ptr<SpectrumWifiPhy> spectrumWifiPhy = DynamicCast<SpectrumWifiPhy> (wifiPhy);
      //Ptr<MacLow> macLow = monitor->GetObject<WifiNetDevice>()->GetMac();

      Ptr<LteEnbPhy> ltePhy = (*i)->GetObject<LteEnbNetDevice> ()->GetPhy ();
      Ptr<LteEnbMac> lteMac = (*i)->GetObject<LteEnbNetDevice> ()->GetMac ();

      if (m_channelAccessManagerFactory.GetTypeId ().GetName () == "ns3::BasicLbtAccessManager")
        {
          Ptr<BasicLbtAccessManager> basicLbtAccessManager = m_channelAccessManagerFactory.Create<BasicLbtAccessManager> ();
          basicLbtAccessManager->SetWifiPhy (spectrumWifiPhy);
          // set channel access manager to lteEnbPhy
          ltePhy->SetChannelAccessManager (basicLbtAccessManager);
        }
      else if (m_channelAccessManagerFactory.GetTypeId ().GetName () == "ns3::LbtAccessManager")
        {
          Ptr<LbtAccessManager> lbtAccessManager = m_channelAccessManagerFactory.Create<LbtAccessManager> ();
          lbtAccessManager->SetWifiPhy (spectrumWifiPhy);
          lbtAccessManager->SetLteEnbMac(lteMac);
          lbtAccessManager->SetLteEnbPhy(ltePhy);
          //lbtAccessManager->SetupLowListener(macLow);
          ltePhy->SetChannelAccessManager (lbtAccessManager);
        }
      else if (m_channelAccessManagerFactory.GetTypeId ().GetName () == "ns3::DutyCycleAccessManager")
      {
         Ptr<DutyCycleAccessManager> dutyCycleAccessManager = m_channelAccessManagerFactory.Create<DutyCycleAccessManager> ();
         ltePhy->SetChannelAccessManager (dutyCycleAccessManager);
      }
      // set callbacks
      //wifiPhy->TraceConnectWithoutContext ("PhyRxBegin", MakeCallback (&ns3::LteWifiCoexistenceHelper::WifiRxBegin, this));
    }
}

void LteWifiCoexistenceHelper::WifiRxBegin (Ptr< const Packet > packet)
{
  NS_LOG_DEBUG ("Packet:" << packet->GetUid ());
}

} // namespace ns3
