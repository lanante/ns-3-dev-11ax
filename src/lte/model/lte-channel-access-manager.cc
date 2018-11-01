/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * lte-channel-access-manager.cc
 *
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

#include "lte-channel-access-manager.h"
#include "ns3/assert.h"
#include "ns3/log.h"
#include "ns3/simulator.h"
#include "ns3/double.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("LteChannelAccessManager");
NS_OBJECT_ENSURE_REGISTERED (LteChannelAccessManager);

TypeId
LteChannelAccessManager::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::LteChannelAccessManager")
    .SetParent<Object> ()
    .SetGroupName ("Lte")
    .AddConstructor<LteChannelAccessManager> ()
    .AddAttribute ("GrantDuration",
                   "Duration of grant for transmitting.",
                   TimeValue (Minutes(1)),
                   MakeTimeAccessor (&LteChannelAccessManager::SetGrantDuration,
                                     &LteChannelAccessManager::GetGrantDuration),
                   MakeTimeChecker ())
    .AddAttribute ("EnergyDetectionThreshold",
                   "CCA-ED threshold for channel sensing",
                   DoubleValue (-72.0),
                   MakeDoubleAccessor (&LteChannelAccessManager::m_edThreshold),
                   MakeDoubleChecker<double> (-100.0, -50.0))
    ;
  return tid;
}

void
LteChannelAccessManager::DoInitialize ()
{
  NS_LOG_FUNCTION (this);
  NS_ASSERT_MSG (true, "Initialize doesn't seem to be called");
}

LteChannelAccessManager::LteChannelAccessManager ()
{
  NS_LOG_FUNCTION (this);
}

LteChannelAccessManager::~LteChannelAccessManager ()
{
  NS_LOG_FUNCTION (this);
}

void LteChannelAccessManager::RequestAccess ()
{
  NS_LOG_FUNCTION (this);
  DoRequestAccess ();
}

void LteChannelAccessManager::SetAccessGrantedCallback (AccessGrantedCallback accessGrantedCallback)
{
  NS_LOG_FUNCTION (this);
  m_accessGrantedCallback = accessGrantedCallback;
}

void LteChannelAccessManager::SetGrantDuration (Time grantDuration)
{
  NS_LOG_FUNCTION (this);
  m_grantDuration = grantDuration;
  m_lastTxopStartTime = Simulator::Now ();
}

Time LteChannelAccessManager::GetGrantDuration () const
{
  NS_LOG_FUNCTION (this);
  return m_grantDuration;
}

Time LteChannelAccessManager::GetLastTxopStartTime () const
{
  NS_LOG_FUNCTION (this);
  return m_lastTxopStartTime;
}

void LteChannelAccessManager::DoRequestAccess ()
{
  NS_LOG_FUNCTION (this);
  NS_ASSERT_MSG (!m_accessGrantedCallback.IsNull (), "Access granted callback is not initialized!");
  // The approach taken here is that, according to this default channel access manager, the channel is always available,
  // so we immediately invoke the callback if one is set
  m_accessGrantedCallback (m_grantDuration);
}

}

