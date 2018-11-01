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

#include "duty-cycle-access-manager.h"
#include "ns3/assert.h"
#include "ns3/log.h"
#include "ns3/simulator.h"
#include "ns3/nstime.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("DutyCycleAccessManager");

NS_OBJECT_ENSURE_REGISTERED (DutyCycleAccessManager);

TypeId
DutyCycleAccessManager::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::DutyCycleAccessManager")
    .SetParent<LteChannelAccessManager> ()
    .SetGroupName ("lte")
    .AddConstructor<DutyCycleAccessManager> ()
    .AddAttribute ("OnDuration",
                   "Duration of ON state.",
                   TimeValue (MilliSeconds (20)),
                   MakeTimeAccessor (&DutyCycleAccessManager::SetOnDuration,
                                     &DutyCycleAccessManager::GetOnDuration),
                   MakeTimeChecker ())
    .AddAttribute ("OnStartTime",
                   "Time in which starts ON state. Used to place ON state inside of duty cycle "
                   "period, e.g. if is equal to 0, then ON state starts at the beginning of duty cycle.",
                   TimeValue (MilliSeconds (0)),
                   MakeTimeAccessor (&DutyCycleAccessManager::SetOnStartTime,
                                     &DutyCycleAccessManager::GetOnStartTime),
                   MakeTimeChecker ())
    .AddAttribute ("DutyCyclePeriod",
                   "Duration of duty cycle period which consists of ON and OFF state.",
                   TimeValue (MilliSeconds (60)),
                   MakeTimeAccessor (&DutyCycleAccessManager::SetDutyCyclePeriod,
                                     &DutyCycleAccessManager::GetDutyCyclePeriod),
                   MakeTimeChecker ());
  return tid;

}

void DutyCycleAccessManager::SetOnDuration (Time onTime)
{
  m_onDuration = onTime;
}

void DutyCycleAccessManager::SetOnStartTime (Time onStartTime)
{
  m_onStartTime = onStartTime;
}
void DutyCycleAccessManager::SetDutyCyclePeriod (Time dutyCycle)
{
  m_dutyCyclePeriod = dutyCycle;
}

Time DutyCycleAccessManager::GetOnDuration () const
{
  return m_onDuration;
}

Time DutyCycleAccessManager::GetOnStartTime () const
{
  return m_onStartTime;
}

Time DutyCycleAccessManager::GetDutyCyclePeriod () const
{
  return m_dutyCyclePeriod;
}

void
DutyCycleAccessManager::DoInitialize ()
{
	m_waitingForChannel = false;
}

DutyCycleAccessManager::DutyCycleAccessManager ()
{
  NS_LOG_FUNCTION (this);
}

DutyCycleAccessManager::~DutyCycleAccessManager ()
{
  NS_LOG_FUNCTION (this);
}


void DutyCycleAccessManager::DoRequestAccess ()
{
  NS_LOG_FUNCTION (this);
  NS_ASSERT_MSG (!m_accessGrantedCallback.IsNull (), "Access granted callback is not initialized!");

  // The approach taken here is that there is a simple two-state machine 
  // (ON/OFF). During OFF time, the LTE device will not be allowed to transmit,
  // while during the ON time it will be allowed to do so.
  Time tInt = Time (Simulator::Now ().GetNanoSeconds () % m_dutyCyclePeriod.GetNanoSeconds ());
  if (tInt >= m_onStartTime && tInt < m_onStartTime + m_onDuration)
    {
      // allow channel access until the end of ON period, notify that the channel access is granted
      m_accessGrantedCallback (m_onStartTime + m_onDuration - tInt);
    }
  else if (tInt < m_onStartTime)  // wait until the end of OFF period
    {
      Simulator::Schedule (m_onStartTime - tInt, &DutyCycleAccessManager::DelayedAccessGrantedCallback, this);
    }
  else // wait until the end of OFF period
    {
      Simulator::Schedule (m_dutyCyclePeriod - tInt, &DutyCycleAccessManager::DelayedAccessGrantedCallback, this);
    }
}


void DutyCycleAccessManager::DelayedAccessGrantedCallback()
{
	NS_LOG_FUNCTION (this);
	m_accessGrantedCallback(m_onDuration);
}


}

