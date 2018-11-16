/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "wifi-he-helper.h"
#include "ns3/obss-pd-algorithm.h"
#include "ns3/ap-wifi-mac.h"
#include "ns3/log.h"

namespace ns3 {

WifiHeHelper::~WifiHeHelper ()
{
}

WifiHeHelper::WifiHeHelper ()
{
  SetObssPdAlgorithm ("ns3::ConstantObssPdAlgorithm");
  m_heConfiguration = ObjectFactory ();
  m_heConfiguration.SetTypeId ("ns3::HeConfiguration");
}

void
WifiHeHelper::SetObssPdAlgorithm (std::string type,
                                  std::string n0, const AttributeValue &v0,
                                  std::string n1, const AttributeValue &v1,
                                  std::string n2, const AttributeValue &v2,
                                  std::string n3, const AttributeValue &v3,
                                  std::string n4, const AttributeValue &v4,
                                  std::string n5, const AttributeValue &v5,
                                  std::string n6, const AttributeValue &v6,
                                  std::string n7, const AttributeValue &v7)
{
  m_algorithm = ObjectFactory ();
  m_algorithm.SetTypeId (type);
  m_algorithm.Set (n0, v0);
  m_algorithm.Set (n1, v1);
  m_algorithm.Set (n2, v2);
  m_algorithm.Set (n3, v3);
  m_algorithm.Set (n4, v4);
  m_algorithm.Set (n5, v5);
  m_algorithm.Set (n6, v6);
  m_algorithm.Set (n7, v7);
}

void
WifiHeHelper::SetHeConfigurationAttribute (std::string n0, const AttributeValue &v0)
{
  m_heConfiguration.Set (n0, v0);
}

void
WifiHeHelper::Install (NodeContainer c) const
{
  // for each node
  for (uint32_t i = 0; i < c.GetN (); i++)
  {
    Ptr<Node> node = c.Get (i);
    Ptr<WifiNetDevice> wifiNetDevice = DynamicCast<WifiNetDevice> (node->GetDevice (0));
    if (wifiNetDevice)
      {
        // HE Configuration for each AP device
        Ptr<RegularWifiMac> wifiMac = wifiNetDevice->GetMac ()->GetObject<RegularWifiMac> ();
        if (wifiMac)
          {
            Ptr<HeConfiguration> heConfiguration = m_heConfiguration.Create<HeConfiguration> ();
            wifiMac->SetHeConfiguration (heConfiguration);
          }
        else
          {
            NS_LOG_UNCOND ("Node does not have a WifiMac.  HE Configuration was not set.");
          }
        // create also the OBSS PD algorithm object and aggregate it
        Ptr<ObssPdAlgorithm> obssPdAlgorithm = m_algorithm.Create<ObssPdAlgorithm> ();
        wifiNetDevice->AggregateObject (obssPdAlgorithm);
        // set up the callbacks that the OBSS PD algorithm needs
        obssPdAlgorithm->SetupCallbacks ();
      }
    else
      {
        NS_LOG_UNCOND ("Node does not have a WifiNetDevice.  HE Configuration was not set.");
      }
  }
}

}

