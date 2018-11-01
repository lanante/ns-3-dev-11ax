/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
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
 *  Author: Biljana Bojovic <bbojovic@cttc.es> and Tom Henderson <tomh@tomh.org>
 */

#include <ns3/object.h>
#include <ns3/nstime.h>
#include <ns3/callback.h>
#include "ns3/wifi-mac.h"
#include "ns3/mac-low.h"

#include "ns3/lte-channel-access-manager.h"
#include "ns3/lbt-access-manager.h"

#ifndef BASIC_LBT_ACCESS_MANAGER_H_
#define BASIC_LBT_ACCESS_MANAGER_H_

namespace ns3 {

class SpectrumWifiPhy;

class BasicLbtAccessManager : public LbtAccessManager
{

public:
  static TypeId GetTypeId (void);
  BasicLbtAccessManager ();

  void SetWifiPhy (Ptr<SpectrumWifiPhy> phy);

  virtual ~BasicLbtAccessManager ();
  /*
   * Set duration of channel access grant.
   * \param onTime - duration
   */
  void SetTxTime (Time txTime);
  /*
* Set waitTime variable that determines how much to wait before checking again if channel is IDLE
   * \param onTime - duration
  */
  void SetWaitTime (Time waitTime);
  /*
   * Get duration of channel access grant.
   * \returns duration
   */
  Time GetTxTime () const;
  /*
   * Get duration of wait time before checking again if channel is IDLE
   * \returns duration
   */
  Time GetWaitTime () const;

protected:
  virtual void DoInitialize ();
  virtual void DoRequestAccess ();

  /*
   * Set next timeout, which is relative time to current simulation time.
   * \param timeout time in milliseconds.
   * */
  void SetAccessGranted (bool accessGranted);

  void Wait ();

  bool IsWaiting ();

  Ptr<SpectrumWifiPhy> m_wifiPhy;
  Time m_txTime;
  Time m_waitTime;
  Time m_timeout;
  Time m_waitTimeout;
};

}




#endif /* BASIC_LBT_ACCESS_MANAGER_H_ */
