/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2005,2006 INRIA
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
 * Author: Mathieu Lacage <mathieu.lacage@sophia.inria.fr>
 */

#include <algorithm>
#include "ns3/log.h"
#include "ns3/simulator.h"
#include "ns3/packet.h"
#include "wifi-phy-state-helper.h"
#include "wifi-tx-vector.h"
#include "wifi-phy-listener.h"
#include "wifi-psdu.h"
#include "wifi-phy.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("WifiPhyStateHelper");

NS_OBJECT_ENSURE_REGISTERED (WifiPhyStateHelper);

TypeId
WifiPhyStateHelper::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::WifiPhyStateHelper")
    .SetParent<Object> ()
    .SetGroupName ("Wifi")
    .AddConstructor<WifiPhyStateHelper> ()
    .AddTraceSource ("State",
                     "The state of the PHY layer",
                     MakeTraceSourceAccessor (&WifiPhyStateHelper::m_stateLogger),
                     "ns3::WifiPhyStateHelper::StateTracedCallback")
    .AddTraceSource ("RxOk",
                     "A packet has been received successfully.",
                     MakeTraceSourceAccessor (&WifiPhyStateHelper::m_rxOkTrace),
                     "ns3::WifiPhyStateHelper::RxOkTracedCallback")
    .AddTraceSource ("RxError",
                     "A packet has been received unsuccessfully.",
                     MakeTraceSourceAccessor (&WifiPhyStateHelper::m_rxErrorTrace),
                     "ns3::WifiPhyStateHelper::RxEndErrorTracedCallback")
    .AddTraceSource ("Tx", "Packet transmission is starting.",
                     MakeTraceSourceAccessor (&WifiPhyStateHelper::m_txTrace),
                     "ns3::WifiPhyStateHelper::TxTracedCallback")
  ;
  return tid;
}

WifiPhyStateHelper::WifiPhyStateHelper ()
  : m_sleeping (false),
    m_isOff (false),
    m_endTx (Seconds (0)),
    m_endRx (Seconds (0)),
    m_endSwitching (Seconds (0)),
    m_startTx (Seconds (0)),
    m_startRx (Seconds (0)),
    m_startSwitching (Seconds (0)),
    m_startSleep (Seconds (0)),
    m_previousStateChangeTime (Seconds (0))
{
  NS_LOG_FUNCTION (this);
}

WifiPhyStateHelper::~WifiPhyStateHelper ()
{
  NS_LOG_FUNCTION (this);
}

void
WifiPhyStateHelper::DoInitialize (void)
{
  NS_LOG_FUNCTION (this);
  m_startCcaBusy.clear ();
  m_endCcaBusy.clear ();
}

void
WifiPhyStateHelper::DoDispose (void)
{
  NS_LOG_FUNCTION (this);
  m_startCcaBusy.clear ();
  m_endCcaBusy.clear ();
}

void
WifiPhyStateHelper::SetReceiveOkCallback (RxOkCallback callback)
{
  m_rxOkCallback = callback;
}

void
WifiPhyStateHelper::SetReceiveErrorCallback (RxErrorCallback callback)
{
  m_rxErrorCallback = callback;
}

void
WifiPhyStateHelper::RegisterListener (WifiPhyListener *listener)
{
  m_listeners.push_back (listener);
}

void
WifiPhyStateHelper::UnregisterListener (WifiPhyListener *listener)
{
  ListenersI i = find (m_listeners.begin (), m_listeners.end (), listener);
  if (i != m_listeners.end ())
    {
      m_listeners.erase (i);
    }
}

bool
WifiPhyStateHelper::IsStateIdle (WifiSpectrumBand band, double ccaThreshold) const
{
  return (GetState (band, ccaThreshold) == WifiPhyState::IDLE);
}

bool
WifiPhyStateHelper::IsStateCcaBusy (WifiSpectrumBand band, double ccaThreshold) const
{
  return (GetState (band, ccaThreshold) == WifiPhyState::CCA_BUSY);
}

bool
WifiPhyStateHelper::IsStateRx (WifiSpectrumBand primaryBand, double primaryCcaThreshold) const
{
  return (GetState (primaryBand, primaryCcaThreshold) == WifiPhyState::RX);
}

bool
WifiPhyStateHelper::IsStateTx (WifiSpectrumBand primaryBand, double primaryCcaThreshold) const
{
  return (GetState (primaryBand, primaryCcaThreshold) == WifiPhyState::TX);
}

bool
WifiPhyStateHelper::IsStateSwitching (WifiSpectrumBand primaryBand, double primaryCcaThreshold) const
{
  return (GetState (primaryBand, primaryCcaThreshold) == WifiPhyState::SWITCHING);
}

bool
WifiPhyStateHelper::IsStateSleep (WifiSpectrumBand primaryBand, double primaryCcaThreshold) const
{
  return (GetState (primaryBand, primaryCcaThreshold) == WifiPhyState::SLEEP);
}

bool
WifiPhyStateHelper::IsStateOff (WifiSpectrumBand primaryBand, double primaryCcaThreshold) const
{
  return (GetState (primaryBand, primaryCcaThreshold) == WifiPhyState::OFF);
}

Time
WifiPhyStateHelper::GetDelayUntilIdle (WifiSpectrumBand band, double ccaThreshold) const
{
  Time retval;
  switch (GetState (band, ccaThreshold))
    {
    case WifiPhyState::RX:
      retval = m_endRx - Simulator::Now ();
      break;
    case WifiPhyState::TX:
      retval = m_endTx - Simulator::Now ();
      break;
    case WifiPhyState::CCA_BUSY:
    {
      auto it = m_endCcaBusy.find (std::make_pair (band, ccaThreshold));
      NS_ASSERT (it != m_endCcaBusy.end ());
      retval = it->second - Simulator::Now ();
      break;
    }
    case WifiPhyState::SWITCHING:
      retval = m_endSwitching - Simulator::Now ();
      break;
    case WifiPhyState::IDLE:
      retval = Seconds (0);
      break;
    case WifiPhyState::SLEEP:
      NS_FATAL_ERROR ("Cannot determine when the device will wake up.");
      retval = Seconds (0);
      break;
    case WifiPhyState::OFF:
      NS_FATAL_ERROR ("Cannot determine when the device will be switched on.");
      retval = Seconds (0);
      break;
    default:
      NS_FATAL_ERROR ("Invalid WifiPhy state.");
      retval = Seconds (0);
      break;
    }
  retval = Max (retval, Seconds (0));
  return retval;
}

Time
WifiPhyStateHelper::GetDelaySinceIdle (WifiSpectrumBand band, double ccaThreshold) const
{
  auto it = m_endCcaBusy.find (std::make_pair (band, ccaThreshold));
  Time idleStart = Max (m_endTx, m_endRx);
  idleStart = Max (idleStart, m_endSwitching);
  if (it != m_endCcaBusy.end ())
    {
      idleStart = Max (idleStart, it->second);
    }
  return Simulator::Now () - idleStart;
}

Time
WifiPhyStateHelper::GetLastRxStartTime (void) const
{
  return m_startRx;
}

WifiPhyState
WifiPhyStateHelper::GetState (WifiSpectrumBand band, double ccaThreshold) const
{
  Time now = Simulator::Now ();
  auto it = m_endCcaBusy.find (std::make_pair (band, ccaThreshold));
  if (m_isOff)
    {
      return WifiPhyState::OFF;
    }
  if (m_sleeping)
    {
      return WifiPhyState::SLEEP;
    }
  else if (m_endTx > now)
    {
      return WifiPhyState::TX;
    }
  else if (m_endRx > now)
    {
      return WifiPhyState::RX;
    }
  else if (m_endSwitching > now)
    {
      return WifiPhyState::SWITCHING;
    }
  else if ((it != m_endCcaBusy.end ()) && (it->second > now))
    {
      return WifiPhyState::CCA_BUSY;
    }
  else
    {
      return WifiPhyState::IDLE;
    }
}

void
WifiPhyStateHelper::NotifyTxStart (Time duration, double txPowerDbm)
{
  NS_LOG_FUNCTION (this);
  for (Listeners::const_iterator i = m_listeners.begin (); i != m_listeners.end (); i++)
    {
      (*i)->NotifyTxStart (duration, txPowerDbm);
    }
}

void
WifiPhyStateHelper::NotifyRxStart (Time duration)
{
  NS_LOG_FUNCTION (this);
  for (Listeners::const_iterator i = m_listeners.begin (); i != m_listeners.end (); i++)
    {
      (*i)->NotifyRxStart (duration);
    }
}

void
WifiPhyStateHelper::NotifyRxEndOk (void)
{
  NS_LOG_FUNCTION (this);
  for (Listeners::const_iterator i = m_listeners.begin (); i != m_listeners.end (); i++)
    {
      (*i)->NotifyRxEndOk ();
    }
}

void
WifiPhyStateHelper::NotifyRxEndError (void)
{
  NS_LOG_FUNCTION (this);
  for (Listeners::const_iterator i = m_listeners.begin (); i != m_listeners.end (); i++)
    {
      (*i)->NotifyRxEndError ();
    }
}

void
WifiPhyStateHelper::NotifyMaybeCcaBusyStart (Time duration)
{
  NS_LOG_FUNCTION (this << duration);
  for (Listeners::const_iterator i = m_listeners.begin (); i != m_listeners.end (); i++)
    {
      (*i)->NotifyMaybeCcaBusyStart (duration);
    }
}

void
WifiPhyStateHelper::NotifySwitchingStart (Time duration)
{
  NS_LOG_FUNCTION (this << duration);
  for (Listeners::const_iterator i = m_listeners.begin (); i != m_listeners.end (); i++)
    {
      (*i)->NotifySwitchingStart (duration);
    }
}

void
WifiPhyStateHelper::NotifySleep (void)
{
  NS_LOG_FUNCTION (this);
  for (Listeners::const_iterator i = m_listeners.begin (); i != m_listeners.end (); i++)
    {
      (*i)->NotifySleep ();
    }
}

void
WifiPhyStateHelper::NotifyOff (void)
{
  NS_LOG_FUNCTION (this);
  for (Listeners::const_iterator i = m_listeners.begin (); i != m_listeners.end (); i++)
    {
      (*i)->NotifyOff ();
    }
}

void
WifiPhyStateHelper::NotifyWakeup (void)
{
  NS_LOG_FUNCTION (this);
  for (Listeners::const_iterator i = m_listeners.begin (); i != m_listeners.end (); i++)
    {
      (*i)->NotifyWakeup ();
    }
}

void
WifiPhyStateHelper::NotifyOn (void)
{
  NS_LOG_FUNCTION (this);
  for (Listeners::const_iterator i = m_listeners.begin (); i != m_listeners.end (); i++)
    {
      (*i)->NotifyOn ();
    }
}

void
WifiPhyStateHelper::LogPreviousIdleAndCcaBusyStates (WifiSpectrumBand primaryBand, double primaryCcaThreshold)
{
  NS_LOG_FUNCTION (this);
  Time now = Simulator::Now ();
  Time endCcaBusy = Seconds (0);
  auto itEndCca = m_endCcaBusy.find (std::make_pair(primaryBand, primaryCcaThreshold));
  if (itEndCca != m_endCcaBusy.end ())
    {
      endCcaBusy = itEndCca->second;
    }
  Time idleStart = Max (endCcaBusy, m_endRx);
  idleStart = Max (idleStart, m_endTx);
  idleStart = Max (idleStart, m_endSwitching);
  NS_ASSERT (idleStart <= now);
  if (endCcaBusy > m_endRx
      && endCcaBusy > m_endSwitching
      && endCcaBusy > m_endTx)
    {
      Time ccaBusyStart = Max (m_endTx, m_endRx);
      auto itStartCca = m_startCcaBusy.find (std::make_pair(primaryBand, primaryCcaThreshold));
        if (itStartCca != m_startCcaBusy.end ())
          {
            ccaBusyStart = Max (ccaBusyStart, itStartCca->second);
          }
      ccaBusyStart = Max (ccaBusyStart, m_endSwitching);
      m_stateLogger (ccaBusyStart, idleStart - ccaBusyStart, WifiPhyState::CCA_BUSY);
    }
  m_stateLogger (idleStart, now - idleStart, WifiPhyState::IDLE);
}

void
WifiPhyStateHelper::SwitchToTx (Time txDuration, WifiPsduMap psdus, double txPowerDbm, WifiTxVector txVector, WifiSpectrumBand primaryBand, double primaryCcaThreshold)
{
  NS_LOG_FUNCTION (this << txDuration << psdus << txPowerDbm << txVector);
  for (auto const& psdu : psdus)
    {
      m_txTrace (psdu.second->GetPacket (), txVector.GetMode (psdu.first), txVector.GetPreambleType (), txVector.GetTxPowerLevel ());
    }
  Time now = Simulator::Now ();
  switch (GetState (primaryBand, primaryCcaThreshold))
    {
    case WifiPhyState::RX:
      /* The packet which is being received as well
       * as its endRx event are cancelled by the caller.
       */
      m_stateLogger (m_startRx, now - m_startRx, WifiPhyState::RX);
      m_endRx = now;
      break;
    case WifiPhyState::CCA_BUSY:
      {
        Time ccaStart = Max (m_endRx, m_endTx);
        auto it = m_startCcaBusy.find (std::make_pair (primaryBand, primaryCcaThreshold));
        if (it != m_startCcaBusy.end ())
          {
            ccaStart = Max (ccaStart, it->second);
          }
        ccaStart = Max (ccaStart, m_endSwitching);
        m_stateLogger (ccaStart, now - ccaStart, WifiPhyState::CCA_BUSY);
      } break;
    case WifiPhyState::IDLE:
      LogPreviousIdleAndCcaBusyStates (primaryBand, primaryCcaThreshold);
      break;
    default:
      NS_FATAL_ERROR ("Invalid WifiPhy state.");
      break;
    }
  m_stateLogger (now, txDuration, WifiPhyState::TX);
  m_previousStateChangeTime = now;
  m_endTx = now + txDuration;
  m_startTx = now;
  for (auto & startCcaBusy : m_startCcaBusy)
    {
      if (startCcaBusy.first.first == primaryBand)
        {
          continue;
        }
      if (now < startCcaBusy.second)
        {
          startCcaBusy.second = now;
        }
    }
  for (auto & endCcaBusy : m_endCcaBusy)
    {
      if (endCcaBusy.first.first == primaryBand)
        {
          continue;
        }
      endCcaBusy.second = std::max (endCcaBusy.second, now + txDuration);
    }
  NotifyTxStart (txDuration, txPowerDbm);
}

void
WifiPhyStateHelper::SwitchToRx (Time rxDuration, WifiSpectrumBand primaryBand, double primaryCcaThreshold)
{
  NS_LOG_FUNCTION (this << rxDuration);
  Time now = Simulator::Now ();
  switch (GetState (primaryBand, primaryCcaThreshold))
    {
    case WifiPhyState::IDLE:
      LogPreviousIdleAndCcaBusyStates (primaryBand, primaryCcaThreshold);
      break;
    case WifiPhyState::CCA_BUSY:
      {
        Time ccaStart = Max (m_endRx, m_endTx);
        auto it = m_startCcaBusy.find (std::make_pair (primaryBand, primaryCcaThreshold));
        if (it != m_startCcaBusy.end ())
          {
            ccaStart = Max (ccaStart, it->second);
          }
        ccaStart = Max (ccaStart, m_endSwitching);
        m_stateLogger (ccaStart, now - ccaStart, WifiPhyState::CCA_BUSY);
      }
      break;
    default:
      NS_FATAL_ERROR ("Invalid WifiPhy state");
      break;
    }
  m_previousStateChangeTime = now;
  m_startRx = now;
  m_endRx = now + rxDuration;
  for (auto & startCcaBusy : m_startCcaBusy)
    {
      if (startCcaBusy.first.first == primaryBand)
        {
          continue;
        }
      if (now < startCcaBusy.second)
        {
          startCcaBusy.second = now;
        }
    }
  for (auto & endCcaBusy : m_endCcaBusy)
    {
      if (endCcaBusy.first.first == primaryBand)
        {
          continue;
        }
      endCcaBusy.second = std::max (endCcaBusy.second, now + rxDuration);
    }
  NotifyRxStart (rxDuration);
  NS_ASSERT (IsStateRx (primaryBand, primaryCcaThreshold));
}

void
WifiPhyStateHelper::SwitchToChannelSwitching (Time switchingDuration, WifiSpectrumBand primaryBand, double primaryCcaThreshold)
{
  NS_LOG_FUNCTION (this << switchingDuration);
  Time now = Simulator::Now ();
  switch (GetState (primaryBand, primaryCcaThreshold))
    {
    case WifiPhyState::RX:
      /* The packet which is being received as well
       * as its endRx event are cancelled by the caller.
       */
      m_stateLogger (m_startRx, now - m_startRx, WifiPhyState::RX);
      m_endRx = now;
      break;
    case WifiPhyState::CCA_BUSY:
      {
        Time ccaStart = Max (m_endRx, m_endTx);
        auto it = m_startCcaBusy.find (std::make_pair (primaryBand, primaryCcaThreshold));
        if (it != m_startCcaBusy.end ())
          {
            ccaStart = Max (ccaStart, it->second);
          }
        ccaStart = Max (ccaStart, m_endSwitching);
        m_stateLogger (ccaStart, now - ccaStart, WifiPhyState::CCA_BUSY);
      } break;
    case WifiPhyState::IDLE:
      LogPreviousIdleAndCcaBusyStates (primaryBand, primaryCcaThreshold);
      break;
    default:
      NS_FATAL_ERROR ("Invalid WifiPhy state.");
      break;
    }

  for (auto & endCcaBusy : m_endCcaBusy)
    {
      if (now < endCcaBusy.second)
        {
          endCcaBusy.second = now;
        }
    }

  m_stateLogger (now, switchingDuration, WifiPhyState::SWITCHING);
  m_previousStateChangeTime = now;
  m_startSwitching = now;
  m_endSwitching = now + switchingDuration;
  NotifySwitchingStart (switchingDuration);
  NS_ASSERT (IsStateSwitching (primaryBand, primaryCcaThreshold));
}

void
WifiPhyStateHelper::ContinueRxNextMpdu (Ptr<WifiPsdu> psdu, RxSignalInfo rxSignalInfo, WifiTxVector txVector)
{
  NS_LOG_FUNCTION (this << *psdu << rxSignalInfo << txVector);
  std::vector<bool> statusPerMpdu;
  if (!m_rxOkCallback.IsNull ())
    {
      m_rxOkCallback (psdu, rxSignalInfo, txVector, statusPerMpdu);
    }
}

void
WifiPhyStateHelper::SwitchFromRxEndOk (Ptr<WifiPsdu> psdu, RxSignalInfo rxSignalInfo, WifiTxVector txVector,
                                       uint16_t staId, std::vector<bool> statusPerMpdu)
{
  NS_LOG_FUNCTION (this << *psdu << rxSignalInfo << txVector << staId <<
                   statusPerMpdu.size () << std::all_of (statusPerMpdu.begin (), statusPerMpdu.end(), [] (bool v) { return v; })); //returns true if all true
  NS_ASSERT (statusPerMpdu.size () != 0);
  NS_ASSERT (Abs (m_endRx - Simulator::Now ()) < MicroSeconds (1)); //1us corresponds to the maximum propagation delay (delay spread)
  //TODO: a better fix would be to call the function once all HE TB PPDUs are received
  m_rxOkTrace (psdu->GetPacket (), rxSignalInfo.snr, txVector.GetMode (staId), txVector.GetPreambleType ());
  NotifyRxEndOk ();
  DoSwitchFromRx ();
  if (!m_rxOkCallback.IsNull ())
    {
      m_rxOkCallback (psdu, rxSignalInfo, txVector, statusPerMpdu);
    }
}

void
WifiPhyStateHelper::SwitchFromRxEndError (Ptr<WifiPsdu> psdu, double snr)
{
  NS_LOG_FUNCTION (this << *psdu << snr);
  NS_ASSERT (Abs (m_endRx - Simulator::Now ()) < MicroSeconds (1)); //1us corresponds to the maximum propagation delay (delay spread)
  //TODO: a better fix would be to call the function once all HE TB PPDUs are received
  m_rxErrorTrace (psdu->GetPacket (), snr);
  NotifyRxEndError ();
  DoSwitchFromRx ();
  if (!m_rxErrorCallback.IsNull ())
    {
      m_rxErrorCallback (psdu);
    }
}

void
WifiPhyStateHelper::DoSwitchFromRx (void)
{
  NS_LOG_FUNCTION (this);
  Time now = Simulator::Now ();
  m_stateLogger (m_startRx, now - m_startRx, WifiPhyState::RX);
  m_previousStateChangeTime = now;
  m_endRx = Simulator::Now ();
}

void
WifiPhyStateHelper::SwitchMaybeToCcaBusy (Time duration, WifiSpectrumBand band, bool isPrimaryChannel, double ccaThreshold)
{
  NS_LOG_FUNCTION (this << duration << band.first << band.second << isPrimaryChannel << ccaThreshold);
  Time now = Simulator::Now ();
  if (isPrimaryChannel && GetState (band, ccaThreshold) != WifiPhyState::RX)
    {
      NotifyMaybeCcaBusyStart (duration);
    }
  auto itEndCca = m_endCcaBusy.find (std::make_pair (band, ccaThreshold));
  if (itEndCca == m_endCcaBusy.end ())
    {
      m_endCcaBusy.insert ({std::make_pair (band, ccaThreshold), now + duration});
    }
  else
    {
      itEndCca->second = std::max (itEndCca->second, now + duration);
    }
  switch (GetState (band, ccaThreshold))
    {
    case WifiPhyState::IDLE:
      if (isPrimaryChannel)
        {
          LogPreviousIdleAndCcaBusyStates (band, ccaThreshold);
        }
      break;
    case WifiPhyState::RX:
      return;
    default:
      break;
    }
  if (GetState (band, ccaThreshold) != WifiPhyState::CCA_BUSY)
    {
      auto itStartCca = m_startCcaBusy.find (std::make_pair (band, ccaThreshold));
      if (itStartCca == m_startCcaBusy.end ())
        {
          m_startCcaBusy.insert ({std::make_pair (band, ccaThreshold), now});
        }
      else
        {
          itStartCca->second = now;
        }
    }
  if (isPrimaryChannel)
    {
      m_stateLogger (now, duration, WifiPhyState::CCA_BUSY);
    }
}

void
WifiPhyStateHelper::SwitchToSleep (WifiSpectrumBand primaryBand, double primaryCcaThreshold)
{
  NS_LOG_FUNCTION (this);
  Time now = Simulator::Now ();
  switch (GetState (primaryBand, primaryCcaThreshold))
    {
    case WifiPhyState::IDLE:
      LogPreviousIdleAndCcaBusyStates (primaryBand, primaryCcaThreshold);
      break;
    case WifiPhyState::CCA_BUSY:
      {
        Time ccaStart = Max (m_endRx, m_endTx);
        auto it = m_startCcaBusy.find (std::make_pair (primaryBand, primaryCcaThreshold));
        if (it != m_startCcaBusy.end ())
          {
            ccaStart = Max (ccaStart, it->second);
          }
        ccaStart = Max (ccaStart, m_endSwitching);
        m_stateLogger (ccaStart, now - ccaStart, WifiPhyState::CCA_BUSY);
      } break;
    default:
      NS_FATAL_ERROR ("Invalid WifiPhy state.");
      break;
    }
  m_previousStateChangeTime = now;
  m_sleeping = true;
  m_startSleep = now;
  NotifySleep ();
  NS_ASSERT (IsStateSleep (primaryBand, primaryCcaThreshold));
}

void
WifiPhyStateHelper::SwitchFromSleep (Time duration, WifiSpectrumBand band, bool isPrimaryChannel, double ccaThreshold)
{
  NS_LOG_FUNCTION (this << duration);
  NS_ASSERT (IsStateSleep (band, ccaThreshold));
  Time now = Simulator::Now ();
  m_sleeping = false;
  if (isPrimaryChannel)
    {
      m_stateLogger (m_startSleep, now - m_startSleep, WifiPhyState::SLEEP);
      m_previousStateChangeTime = now;
      NotifyWakeup ();
    }
  //update endCcaBusy after the sleep period
  auto it = m_endCcaBusy.find (std::make_pair (band, ccaThreshold));
  Time endCca;
  if (it == m_endCcaBusy.end ())
    {
      endCca = now + duration;
      m_endCcaBusy.insert ({std::make_pair (band, ccaThreshold), endCca});
    }
  else
    {
      endCca = std::max (it->second, now + duration);
      it->second = endCca;
    }
  if (isPrimaryChannel && (endCca > now))
    {
      NotifyMaybeCcaBusyStart (endCca - now);
    }
}

void
WifiPhyStateHelper::SwitchFromRxAbort (bool failure)
{
  NS_LOG_FUNCTION (this);
  //NS_ASSERT (IsStateRx ()); //FIXME: we need band and threshold info, this seems an overkill here so implementation should be improved
  if (failure)
    {
      NotifyRxEndError ();
    }
  else
    {
      NotifyRxEndOk ();
    }
  DoSwitchFromRx ();
  //NS_ASSERT (!IsStateRx ()); //FIXME: we need band and threshold info, this seems an overkill here so implementation should be improved
}

void
WifiPhyStateHelper::SwitchToOff (WifiSpectrumBand primaryBand, double primaryCcaThreshold)
{
  NS_LOG_FUNCTION (this);
  Time now = Simulator::Now ();
  switch (GetState (primaryBand, primaryCcaThreshold))
    {
    case WifiPhyState::RX:
      /* The packet which is being received as well
       * as its endRx event are cancelled by the caller.
       */
      m_stateLogger (m_startRx, now - m_startRx, WifiPhyState::RX);
      m_endRx = now;
      break;
    case WifiPhyState::TX:
      /* The packet which is being transmitted as well
       * as its endTx event are cancelled by the caller.
       */
      m_stateLogger (m_startTx, now - m_startTx, WifiPhyState::TX);
      m_endTx = now;
      break;
    case WifiPhyState::IDLE:
      LogPreviousIdleAndCcaBusyStates (primaryBand, primaryCcaThreshold);
      break;
    case WifiPhyState::CCA_BUSY:
      {
        Time ccaStart = Max (m_endRx, m_endTx);
        auto it = m_startCcaBusy.find (std::make_pair (primaryBand, primaryCcaThreshold));
        if (it != m_startCcaBusy.end ())
          {
            ccaStart = Max (ccaStart, it->second);
          }
        ccaStart = Max (ccaStart, m_endSwitching);
        m_stateLogger (ccaStart, now - ccaStart, WifiPhyState::CCA_BUSY);
      } break;
    default:
      NS_FATAL_ERROR ("Invalid WifiPhy state.");
      break;
    }
  m_previousStateChangeTime = now;
  m_isOff = true;
  NotifyOff ();
  NS_ASSERT (IsStateOff (primaryBand, primaryCcaThreshold));
}

void
WifiPhyStateHelper::SwitchFromOff (Time duration, WifiSpectrumBand band, bool isPrimaryChannel, double ccaThreshold)
{
  NS_LOG_FUNCTION (this << duration);
  NS_ASSERT (IsStateOff (band, ccaThreshold));
  Time now = Simulator::Now ();
  m_isOff = false;
  if (isPrimaryChannel)
    {
      m_previousStateChangeTime = now;
      NotifyWakeup ();
    }
  //update endCcaBusy after the off period
  auto it = m_endCcaBusy.find (std::make_pair (band, ccaThreshold));
  Time endCca;
  if (it == m_endCcaBusy.end ())
    {
      endCca = now + duration;
      m_endCcaBusy.insert ({std::make_pair (band, ccaThreshold), endCca});
    }
  else
    {
      endCca = std::max (it->second, now + duration);
      it->second = endCca;
    }
  if (isPrimaryChannel && (endCca > now))
    {
      NotifyMaybeCcaBusyStart (endCca - now);
    }
}

} //namespace ns3
