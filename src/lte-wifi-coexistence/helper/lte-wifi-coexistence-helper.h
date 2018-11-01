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

#ifndef LTE_WIFI_COEXISTENCE_HELPER_H
#define LTE_WIFI_COEXISTENCE_HELPER_H

#include <ns3/object.h>
#include <ns3/config.h>
#include <ns3/simulator.h>
#include <ns3/names.h>
#include <ns3/net-device.h>
#include <ns3/net-device-container.h>
#include <ns3/node.h>
#include <ns3/node-container.h>
#include <ns3/scenario-helper.h>

namespace ns3 {

enum Config_ChannelAccessManager
{
  Default,
  DutyCycle,
  BasicLbt,
  Lbt,
  OnlyListen
};

/**
 * \ingroup lte
 *
 * This class gathers methods necessary for configuring the LTE/Wi-Fi 
 * coexistance mode on a set of LTE eNodeB devices.
 * e.g configuring energy detection thresholds, configuring preamble 
 * detection thresholds, configuring a set of LteNetDevices for 
 * Listen Before Talk (LBT), configuring different backoff algorithms, etc.
 */
class LteWifiCoexistenceHelper : public Object
{
public:
  /**
   * Constructor
   */
  LteWifiCoexistenceHelper ();

  /**
   * Destructor
   */
  ~LteWifiCoexistenceHelper ();

  /**
   *  Register this type.
   *  \return The object TypeId.
   */
  static TypeId GetTypeId (void);

  virtual void DoDispose ();

  /**
   * Set channel access manager type
   * \param type The type of access manager type
   */
  void SetChannelAccessManagerType (std::string type);

  /*
   * Get channel access manager type.
   * \Returns a type of access manager.
   */
  std::string GetChannelAccessManagerType () const;

  /**
   * Configures LTE eNb devices to work in lte-wifi coexistance mode.
   * \param enbDevices The enbDevices to be configured in lte-wifi coexistence mode.
   * \param phyParams PhyParams to use in configuration
   */
  void ConfigureEnbDevicesForLbt (NetDeviceContainer enbDevices, struct PhyParams phyParams);


  void WifiRxBegin (Ptr< const Packet > packet);

private:
  /*
  *  Factory for channel access manager objects.
  */
  ObjectFactory m_channelAccessManagerFactory;
};

}

#endif // LTE_WIFI_COEXISTENCE_HELPER_H









