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
 * Authors: Mathieu Lacage <mathieu.lacage@sophia.inria.fr>
 *          Sébastien Deronne <sebastien.deronne@gmail.com>
 */

#include <numeric>
#include "ns3/packet.h"
#include "ns3/simulator.h"
#include "ns3/log.h"
#include "interference-helper.h"
#include "wifi-phy.h"
#include "error-rate-model.h"
#include "wifi-utils.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("InterferenceHelper");

/****************************************************************
 *       Phy event class
 ****************************************************************/

Event::Event (Ptr<const Packet> packet, WifiTxVector txVector, Time duration, RxPowerWattPerChannelBand rxPower)
  : m_packet (packet),
    m_txVector (txVector),
    m_startTime (Simulator::Now ()),
    m_endTime (m_startTime + duration),
    m_rxPowerW (rxPower)
{
}

Event::~Event ()
{
}

Ptr<const Packet>
Event::GetPacket (void) const
{
  return m_packet;
}

Time
Event::GetStartTime (void) const
{
  return m_startTime;
}

Time
Event::GetEndTime (void) const
{
  return m_endTime;
}

double
Event::GetRxPowerW (void) const
{
  NS_ASSERT (m_rxPowerW.size () > 0);
  double sum = std::accumulate (std::begin (m_rxPowerW), std::end (m_rxPowerW), 0.0,
                               [](const double previous, const std::pair<std::pair<uint16_t, uint16_t>, double> &p)
                               { return (p.first.second < 40) ? (previous + p.second) : 0.0; });
  return sum;
}

double
Event::GetRxPowerW (FrequencyWidthPair band) const
{
  auto it = m_rxPowerW.find (band);
  NS_ASSERT (it != m_rxPowerW.end ());
  return it->second;
}

RxPowerWattPerChannelBand
Event::GetRxPowerWPerBand (void) const
{
  return m_rxPowerW;
}

void
Event::SetTxVector (WifiTxVector txVector)
{
  m_txVector = txVector;
}

WifiTxVector
Event::GetTxVector (void) const
{
  return m_txVector;
}

WifiMode
Event::GetPayloadMode (void) const
{
  return m_txVector.GetMode ();
}


/****************************************************************
 *       Class which records SNIR change events for a
 *       short period of time.
 ****************************************************************/

InterferenceHelper::NiChange::NiChange (double power, Ptr<Event> event)
  : m_power (power),
    m_event (event)
{
}

double
InterferenceHelper::NiChange::GetPower (void) const
{
  return m_power;
}

void
InterferenceHelper::NiChange::AddPower (double power)
{
  m_power += power;
}

Ptr<Event>
InterferenceHelper::NiChange::GetEvent (void) const
{
  return m_event;
}


/****************************************************************
 *       The actual InterferenceHelper
 ****************************************************************/

InterferenceHelper::InterferenceHelper ()
  : m_errorRateModel (0),
    m_numRxAntennas (1),
    m_rxing (false)
{
}

InterferenceHelper::~InterferenceHelper ()
{
  EraseEvents ();
  m_errorRateModel = 0;
}

Ptr<Event>
InterferenceHelper::Add (Ptr<const Packet> packet, WifiTxVector txVector, Time duration, RxPowerWattPerChannelBand rxPowerW)
{
  Ptr<Event> event = Create<Event> (packet, txVector, duration, rxPowerW);
  AppendEvent (event);
  return event;
}

void
InterferenceHelper::AddForeignSignal (Time duration, RxPowerWattPerChannelBand rxPowerW)
{
  // Parameters other than duration and rxPowerW are unused for this type
  // of signal, so we provide dummy versions
  WifiTxVector fakeTxVector;
  Ptr<const Packet> packet (0);
  Add (packet, fakeTxVector, duration, rxPowerW);
}

void
InterferenceHelper::SetFrequencyBands (uint16_t firstCenterFrequency, uint16_t channelWidth)
{
  if (firstCenterFrequency == 0 || channelWidth == 0)
    {
      //if at least one of the two is not initialized, do not add frequency bands yet
      return;
    }
  if (channelWidth == 22)
  {
    channelWidth = 20;
  }
  NS_LOG_FUNCTION (this << firstCenterFrequency << channelWidth);
  for (uint8_t i = 0; i < std::max (1, channelWidth / 20); i++)
    {
      uint16_t centerFrequency = firstCenterFrequency + (i * 20);
      auto band = std::make_pair (centerFrequency, 20);
      auto it = m_niChangesPerBand.find (band);
      if (it == m_niChangesPerBand.end ())
        {
          NiChanges NiChanges;
          m_niChangesPerBand.insert ({band, NiChanges});
          AddNiChangeEvent (Time (0), NiChange (0.0, 0), band);
          m_firstPowerPerBand.insert ({band, 0.0});
        }
    }
  for (uint8_t i = 0; i < channelWidth / 40; i++)
    {
      uint16_t centerFrequency = firstCenterFrequency + (i * 40);
      auto band = std::make_pair (centerFrequency, 40);
      auto it = m_niChangesPerBand.find (band);
      if (it == m_niChangesPerBand.end ())
        {
          NiChanges NiChanges;
          m_niChangesPerBand.insert ({band, NiChanges});
          AddNiChangeEvent (Time (0), NiChange (0.0, 0), band);
          m_firstPowerPerBand.insert ({band, 0.0});
        }
    }
  for (uint8_t i = 0; i < channelWidth / 80; i++)
    {
      uint16_t centerFrequency = firstCenterFrequency + (i * 80);
      auto band = std::make_pair (centerFrequency, 80);
      auto it = m_niChangesPerBand.find (band);
      if (it == m_niChangesPerBand.end ())
        {
          NiChanges NiChanges;
          m_niChangesPerBand.insert ({band, NiChanges});
          AddNiChangeEvent (Time (0), NiChange (0.0, 0), band);
          m_firstPowerPerBand.insert ({band, 0.0});
        }
    }
  for (uint8_t i = 0; i < channelWidth / 160; i++)
    {
      uint16_t centerFrequency = firstCenterFrequency + (i * 160);
      auto band = std::make_pair (centerFrequency, 160);
      auto it = m_niChangesPerBand.find (band);
      if (it == m_niChangesPerBand.end ())
        {
          NiChanges NiChanges;
          m_niChangesPerBand.insert ({band, NiChanges});
          AddNiChangeEvent (Time (0), NiChange (0.0, 0), band);
          m_firstPowerPerBand.insert ({band, 0.0});
        }
    }
}

void
InterferenceHelper::SetNoiseFigure (double value)
{
  m_noiseFigure = value;
}

void
InterferenceHelper::SetErrorRateModel (const Ptr<ErrorRateModel> rate)
{
  m_errorRateModel = rate;
}

Ptr<ErrorRateModel>
InterferenceHelper::GetErrorRateModel (void) const
{
  return m_errorRateModel;
}

void
InterferenceHelper::SetNumberOfReceiveAntennas (uint8_t rx)
{
  m_numRxAntennas = rx;
}

Time
InterferenceHelper::GetEnergyDuration (double energyW, uint16_t centerFrequency, uint16_t channelWidth) const
{
  NS_LOG_FUNCTION (this << centerFrequency);
  Time now = Simulator::Now ();
  auto band = std::make_pair (centerFrequency, channelWidth == 22 ? 20 : channelWidth);
  auto i = GetPreviousPosition (now, band);
  Time end = i->first;
  NS_ASSERT (m_niChangesPerBand.find (band) != m_niChangesPerBand.end ());
  for (; i != m_niChangesPerBand.find (band)->second.end (); ++i)
    {
      double noiseInterferenceW = i->second.GetPower ();
      end = i->first;
      if (noiseInterferenceW < energyW)
        {
          break;
        }
    }
  return end > now ? end - now : MicroSeconds (0);
}

void
InterferenceHelper::AppendEvent (Ptr<Event> event)
{
  NS_LOG_FUNCTION (this);
  RxPowerWattPerChannelBand rxPowerWattPerChannelBand = event->GetRxPowerWPerBand ();
  for (auto it : rxPowerWattPerChannelBand)
    {
      uint16_t centerFrequency = it.first.first;
      uint16_t channelWidth = it.first.second;
      auto band = std::make_pair (centerFrequency, channelWidth == 22 ? 20 : channelWidth);
      auto ni = m_niChangesPerBand.find (band);
      NS_ASSERT (ni != m_niChangesPerBand.end ());
      double previousPowerStart = 0;
      double previousPowerEnd = 0;
      previousPowerStart = GetPreviousPosition (event->GetStartTime (), band)->second.GetPower ();
      previousPowerEnd = GetPreviousPosition (event->GetEndTime (), band)->second.GetPower ();
      if (!m_rxing)
        {
          m_firstPowerPerBand.find (band)->second = previousPowerStart;
          // Always leave the first zero power noise event in the list
          ni->second.erase (++(ni->second.begin ()), GetNextPosition (event->GetStartTime (), band));
        }
      auto first = AddNiChangeEvent (event->GetStartTime (), NiChange (previousPowerStart, event), band);
      auto last = AddNiChangeEvent (event->GetEndTime (), NiChange (previousPowerEnd, event), band);
      for (auto i = first; i != last; ++i)
        {
          i->second.AddPower (it.second);
        }
    }
}

double
InterferenceHelper::CalculateSnr (double signal, double noiseInterference, uint16_t channelWidth) const
{
  NS_LOG_FUNCTION (this << signal << noiseInterference << channelWidth);
  //thermal noise at 290K in J/s = W
  static const double BOLTZMANN = 1.3803e-23;
  //Nt is the power of thermal noise in W
  double Nt = BOLTZMANN * 290 * channelWidth * 1e6;
  //receiver noise Floor (W) which accounts for thermal noise and non-idealities of the receiver
  double noiseFloor = m_noiseFigure * Nt;
  double noise = noiseFloor + noiseInterference;
  double snr = signal / noise; //linear scale
  NS_LOG_DEBUG ("bandwidth(MHz)=" << channelWidth << ", signal(W)= " << signal << ", noise(W)=" << noiseFloor << ", interference(W)=" << noiseInterference << ", snr=" << RatioToDb(snr) << "dB");
  return snr;
}

double
InterferenceHelper::CalculateNoiseInterferenceW (Ptr<Event> event, NiChangesPerBand *nis, FrequencyWidthPair band) const
{
  NS_LOG_FUNCTION (this << band.first << band.second);
  double noiseInterferenceW = 0.0;
  NiChanges ni;
  auto ni_it = m_niChangesPerBand.find (band);
  NS_ASSERT (ni_it != m_niChangesPerBand.end ());
  auto firstPower = m_firstPowerPerBand.find (band);
  NS_ASSERT (firstPower != m_firstPowerPerBand.end ());
  double noiseInterferencePerBand = firstPower->second;
  auto it = ni_it->second.find (event->GetStartTime ());
  for (; it != ni_it->second.end () && it->first < Simulator::Now (); ++it)
    {
      noiseInterferencePerBand = it->second.GetPower () - event->GetRxPowerW (band);
    }
  it = ni_it->second.find (event->GetStartTime ());
  NS_ASSERT (it != ni_it->second.end ());
  for (; it != ni_it->second.end () && it->second.GetEvent () != event; ++it);
  ni.emplace (event->GetStartTime (), NiChange (0, event));
  while (++it != ni_it->second.end () && it->second.GetEvent () != event)
    {
      ni.insert (*it);
    }
  ni.emplace (event->GetEndTime (), NiChange (0, event));
  nis->insert ({band, ni});
  noiseInterferenceW += noiseInterferencePerBand;
  NS_ASSERT_MSG (noiseInterferenceW >= 0, "CalculateNoiseInterferenceW returns negative value " << noiseInterferenceW);
  return noiseInterferenceW;
}

double
InterferenceHelper::CalculateChunkSuccessRate (double snir, Time duration, WifiMode mode, WifiTxVector txVector) const
{
  if (duration.IsZero ())
    {
      return 1.0;
    }
  uint64_t rate = mode.GetPhyRate (txVector);
  uint64_t nbits = static_cast<uint64_t> (rate * duration.GetSeconds ());
  if (txVector.GetMode ().GetModulationClass () == WIFI_MOD_CLASS_HT || txVector.GetMode ().GetModulationClass () == WIFI_MOD_CLASS_VHT || txVector.GetMode ().GetModulationClass () == WIFI_MOD_CLASS_HE)
    {
      nbits /= txVector.GetNss (); //divide effective number of bits by NSS to achieve same chunk error rate as SISO for AWGN
      double gain = (txVector.GetNTx () * m_numRxAntennas); //compute gain offered by MIMO, SIMO or MISO compared to SISO for AWGN
      NS_LOG_DEBUG ("TX=" << +txVector.GetNTx () <<
                    ", RX=" << +m_numRxAntennas <<
                    ", SNIR improvement=+" << 10 * std::log10 (gain) << "dB");
      snir *= gain;
    }
  double csr = m_errorRateModel->GetChunkSuccessRate (mode, txVector, snir, nbits);
  return csr;
}

double
InterferenceHelper::CalculatePayloadPer (Ptr<const Event> event, NiChangesPerBand *nis, FrequencyWidthPair band, std::pair<Time, Time> window) const
{
  NS_LOG_FUNCTION (this << window.first << window.second);
  const WifiTxVector txVector = event->GetTxVector ();
  double psr = 1.0; /* Packet Success Rate */
  auto ni = nis->find (band)->second;
  auto j = ni.begin ();
  Time previous = j->first;
  WifiMode payloadMode = event->GetPayloadMode ();
  WifiPreamble preamble = txVector.GetPreambleType ();
  Time plcpHeaderStart = j->first + WifiPhy::GetPlcpPreambleDuration (txVector); //packet start time + preamble
  Time plcpHsigHeaderStart = plcpHeaderStart + WifiPhy::GetPlcpHeaderDuration (txVector); //packet start time + preamble + L-SIG
  Time plcpTrainingSymbolsStart = plcpHsigHeaderStart + WifiPhy::GetPlcpHtSigHeaderDuration (preamble) + WifiPhy::GetPlcpSigA1Duration (preamble) + WifiPhy::GetPlcpSigA2Duration (preamble); //packet start time + preamble + L-SIG + HT-SIG or SIG-A
  Time plcpPayloadStart = plcpTrainingSymbolsStart + WifiPhy::GetPlcpTrainingSymbolDuration (txVector) + WifiPhy::GetPlcpSigBDuration (preamble); //packet start time + preamble + L-SIG + HT-SIG or SIG-A + Training + SIG-B
  Time windowStart = plcpPayloadStart + window.first;
  Time windowEnd = plcpPayloadStart + window.second;
  double noiseInterferenceW = m_firstPowerPerBand.begin ()->second;
  double powerW = event->GetRxPowerW ();
  while (++j != ni.end ())
    {
      Time current = j->first;
      NS_LOG_DEBUG ("previous= " << previous << ", current=" << current);
      NS_ASSERT (current >= previous);
      //Case 1: Both previous and current point to the windowed payload
      if (previous >= windowStart)
        {
          psr *= CalculateChunkSuccessRate (CalculateSnr (powerW,
                                                          noiseInterferenceW,
                                                          txVector.GetChannelWidth ()),
                                            current - previous,
                                            payloadMode, txVector);
          NS_LOG_DEBUG ("Both previous and current point to the windowed payload: mode=" << payloadMode << ", psr=" << psr);
        }
      //Case 2: previous is before windowed payload and current is in the windowed payload
      else if (current >= windowStart)
        {
          psr *= CalculateChunkSuccessRate (CalculateSnr (powerW,
                                                          noiseInterferenceW,
                                                          txVector.GetChannelWidth ()),
                                            current - windowStart,
                                            payloadMode, txVector);
          NS_LOG_DEBUG ("previous is before windowed payload and current is in the windowed payload: mode=" << payloadMode << ", psr=" << psr);
        }
      noiseInterferenceW = j->second.GetPower () - powerW;
      previous = j->first;
      if (previous > windowEnd)
        {
          NS_LOG_DEBUG ("Stop: new previous=" << previous << " after time window end=" << windowEnd);
          break;
        }

    }
  double per = 1 - psr;
  return per;
}

double
InterferenceHelper::CalculateLegacyPhyHeaderPer (Ptr<const Event> event, NiChangesPerBand *nis) const
{
  NS_LOG_FUNCTION (this);
  const WifiTxVector txVector = event->GetTxVector ();
  double psr = 1.0; /* Packet Success Rate */
  auto ni = nis->begin()->second; //assume first band is primary channel
  auto j = ni.begin ();
  Time previous = j->first;
  WifiPreamble preamble = txVector.GetPreambleType ();
  WifiMode headerMode = WifiPhy::GetPlcpHeaderMode (txVector);
  Time plcpHeaderStart = j->first + WifiPhy::GetPlcpPreambleDuration (txVector); //packet start time + preamble
  Time plcpHsigHeaderStart = plcpHeaderStart + WifiPhy::GetPlcpHeaderDuration (txVector); //packet start time + preamble + L-SIG
  Time plcpTrainingSymbolsStart = plcpHsigHeaderStart + WifiPhy::GetPlcpHtSigHeaderDuration (preamble) + WifiPhy::GetPlcpSigA1Duration (preamble) + WifiPhy::GetPlcpSigA2Duration (preamble); //packet start time + preamble + L-SIG + HT-SIG or SIG-A
  Time plcpPayloadStart = plcpTrainingSymbolsStart + WifiPhy::GetPlcpTrainingSymbolDuration (txVector) + WifiPhy::GetPlcpSigBDuration (preamble); //packet start time + preamble + L-SIG + HT-SIG or SIG-A + Training + SIG-B
  double noiseInterferenceW = m_firstPowerPerBand.begin ()->second;
  double powerW = event->GetRxPowerW ();
  while (++j != ni.end ())
    {
      Time current = j->first;
      NS_LOG_DEBUG ("previous= " << previous << ", current=" << current);
      NS_ASSERT (current >= previous);
      //Case 1: previous and current after playload start
      if (previous >= plcpPayloadStart)
        {
          psr *= 1;
          NS_LOG_DEBUG ("Case 1 - previous and current after playload start: nothing to do");
        }
      //Case 2: previous is in training or in SIG-B: legacy will not enter here since it didn't enter in the last two and they are all the same for legacy
      else if (previous >= plcpTrainingSymbolsStart)
        {
          NS_ASSERT ((preamble != WIFI_PREAMBLE_LONG) && (preamble != WIFI_PREAMBLE_SHORT));
          psr *= 1;
          NS_LOG_DEBUG ("Case 2 - previous is in training or in SIG-B: nothing to do");
        }
      //Case 3: previous is in HT-SIG or SIG-A: legacy will not enter here since it didn't enter in the last two and they are all the same for legacy
      else if (previous >= plcpHsigHeaderStart)
        {
          NS_ASSERT ((preamble != WIFI_PREAMBLE_LONG) && (preamble != WIFI_PREAMBLE_SHORT));
          psr *= 1;
          NS_LOG_DEBUG ("Case 3cii - previous is in HT-SIG or SIG-A: nothing to do");
        }
      //Case 4: previous in L-SIG: HT GF will not reach here because it will execute the previous if and exit
      else if (previous >= plcpHeaderStart)
        {
          NS_ASSERT (preamble != WIFI_PREAMBLE_HT_GF);
          //Case 4a: current after payload start
          if (current >= plcpPayloadStart)
            {
              psr *= CalculateChunkSuccessRate (CalculateSnr (powerW,
                                                              noiseInterferenceW,
                                                              txVector.GetChannelWidth ()),
                                                plcpHsigHeaderStart - previous,
                                                headerMode, txVector);
              NS_LOG_DEBUG ("Case 4a - previous in L-SIG and current after payload start: mode=" << headerMode << ", psr=" << psr);
            }
          //Case 4b: current is in training or in SIG-B. legacy will not come here since it went in previous if or if the previous if is not true this will be not true
          else if (current >= plcpTrainingSymbolsStart)
            {
              NS_ASSERT ((preamble != WIFI_PREAMBLE_LONG) && (preamble != WIFI_PREAMBLE_SHORT));
              psr *= CalculateChunkSuccessRate (CalculateSnr (powerW,
                                                              noiseInterferenceW,
                                                              txVector.GetChannelWidth ()),
                                                plcpHsigHeaderStart - previous,
                                                headerMode, txVector);
              NS_LOG_DEBUG ("Case 4a - previous in L-SIG and current is in training or in SIG-B: mode=" << headerMode << ", psr=" << psr);
            }
          //Case 4c: current in HT-SIG or in SIG-A. Legacy will not come here since it went in previous if or if the previous if is not true this will be not true
          else if (current >= plcpHsigHeaderStart)
            {
              NS_ASSERT ((preamble != WIFI_PREAMBLE_LONG) && (preamble != WIFI_PREAMBLE_SHORT));
              psr *= CalculateChunkSuccessRate (CalculateSnr (powerW,
                                                              noiseInterferenceW,
                                                              txVector.GetChannelWidth ()),
                                                plcpHsigHeaderStart - previous,
                                                headerMode, txVector);
              NS_LOG_DEBUG ("Case 4ci - previous is in L-SIG and current in HT-SIG or in SIG-A: mode=" << headerMode << ", psr=" << psr);
            }
          //Case 4d: current with previous in L-SIG
          else
            {
              psr *= CalculateChunkSuccessRate (CalculateSnr (powerW,
                                                              noiseInterferenceW,
                                                              txVector.GetChannelWidth ()),
                                                current - previous,
                                                headerMode, txVector);
              NS_LOG_DEBUG ("Case 4d - current with previous in L-SIG: mode=" << headerMode << ", psr=" << psr);
            }
        }
      //Case 5: previous is in the preamble works for all cases
      else
        {
          //Case 5a: current after payload start
          if (current >= plcpPayloadStart)
            {
              psr *= CalculateChunkSuccessRate (CalculateSnr (powerW,
                                                              noiseInterferenceW,
                                                              txVector.GetChannelWidth ()),
                                                plcpHsigHeaderStart - plcpHeaderStart,
                                                headerMode, txVector);
              NS_LOG_DEBUG ("Case 5aii - previous is in the preamble and current is after payload start: mode=" << headerMode << ", psr=" << psr);
            }
          //Case 5b: current is in training or in SIG-B. Legacy will not come here since it went in previous if or if the previous if is not true this will be not true
          else if (current >= plcpTrainingSymbolsStart)
            {
              NS_ASSERT ((preamble != WIFI_PREAMBLE_LONG) && (preamble != WIFI_PREAMBLE_SHORT));
              psr *= CalculateChunkSuccessRate (CalculateSnr (powerW,
                                                              noiseInterferenceW,
                                                              txVector.GetChannelWidth ()),
                                                plcpHsigHeaderStart - plcpHeaderStart,
                                                headerMode, txVector);
              NS_LOG_DEBUG ("Case 5b - previous is in the preamble and current is in training or in SIG-B: mode=" << headerMode << ", psr=" << psr);
            }
          //Case 5c: current in HT-SIG or in SIG-A. Legacy will not come here since it went in previous if or if the previous if is not true this will be not true
          else if (current >= plcpHsigHeaderStart)
            {
              NS_ASSERT ((preamble != WIFI_PREAMBLE_LONG) && (preamble != WIFI_PREAMBLE_SHORT));
              psr *= CalculateChunkSuccessRate (CalculateSnr (powerW,
                                                              noiseInterferenceW,
                                                              txVector.GetChannelWidth ()),
                                                plcpHsigHeaderStart - plcpHeaderStart,
                                                headerMode, txVector);
              NS_LOG_DEBUG ("Case 5b - previous is in the preamble and current in HT-SIG or in SIG-A: mode=" << headerMode << ", psr=" << psr);
            }
          //Case 5d: current is in L-SIG.
          else if (current >= plcpHeaderStart)
            {
              NS_ASSERT (preamble != WIFI_PREAMBLE_HT_GF);
              psr *= CalculateChunkSuccessRate (CalculateSnr (powerW,
                                                              noiseInterferenceW,
                                                              txVector.GetChannelWidth ()),
                                                current - plcpHeaderStart,
                                                headerMode, txVector);
              NS_LOG_DEBUG ("Case 5d - previous is in the preamble and current is in L-SIG: mode=" << headerMode << ", psr=" << psr);
            }
        }

      noiseInterferenceW = j->second.GetPower () - powerW;
      previous = j->first;
    }

  double per = 1 - psr;
  return per;
}

double
InterferenceHelper::CalculateNonLegacyPhyHeaderPer (Ptr<const Event> event, NiChangesPerBand *nis) const
{
  NS_LOG_FUNCTION (this);
  const WifiTxVector txVector = event->GetTxVector ();
  double psr = 1.0; /* Packet Success Rate */
  auto ni = nis->begin()->second; //assume first band is primary channel
  auto j = ni.begin ();
  Time previous = j->first;
  WifiPreamble preamble = txVector.GetPreambleType ();
  WifiMode mcsHeaderMode;
  if (preamble == WIFI_PREAMBLE_HT_MF || preamble == WIFI_PREAMBLE_HT_GF)
    {
      //mode for PLCP header fields sent with HT modulation
      mcsHeaderMode = WifiPhy::GetHtPlcpHeaderMode ();
    }
  else if (preamble == WIFI_PREAMBLE_VHT_SU || preamble == WIFI_PREAMBLE_VHT_MU)
    {
      //mode for PLCP header fields sent with VHT modulation
      mcsHeaderMode = WifiPhy::GetVhtPlcpHeaderMode ();
    }
  else if (preamble == WIFI_PREAMBLE_HE_SU || preamble == WIFI_PREAMBLE_HE_MU)
    {
      //mode for PLCP header fields sent with HE modulation
      mcsHeaderMode = WifiPhy::GetHePlcpHeaderMode ();
    }
  WifiMode headerMode = WifiPhy::GetPlcpHeaderMode (txVector);
  Time plcpHeaderStart = j->first + WifiPhy::GetPlcpPreambleDuration (txVector); //packet start time + preamble
  Time plcpHsigHeaderStart = plcpHeaderStart + WifiPhy::GetPlcpHeaderDuration (txVector); //packet start time + preamble + L-SIG
  Time plcpTrainingSymbolsStart = plcpHsigHeaderStart + WifiPhy::GetPlcpHtSigHeaderDuration (preamble) + WifiPhy::GetPlcpSigA1Duration (preamble) + WifiPhy::GetPlcpSigA2Duration (preamble); //packet start time + preamble + L-SIG + HT-SIG or SIG-A
  Time plcpPayloadStart = plcpTrainingSymbolsStart + WifiPhy::GetPlcpTrainingSymbolDuration (txVector) + WifiPhy::GetPlcpSigBDuration (preamble); //packet start time + preamble + L-SIG + HT-SIG or SIG-A + Training + SIG-B
  double noiseInterferenceW = m_firstPowerPerBand.begin ()->second;
  double powerW = event->GetRxPowerW ();
  while (++j != ni.end ())
    {
      Time current = j->first;
      NS_LOG_DEBUG ("previous= " << previous << ", current=" << current);
      NS_ASSERT (current >= previous);
      //Case 1: previous and current after playload start: nothing to do
      if (previous >= plcpPayloadStart)
        {
          psr *= 1;
          NS_LOG_DEBUG ("Case 1 - previous and current after playload start: nothing to do");
        }
      //Case 2: previous is in training or in SIG-B: legacy will not enter here since it didn't enter in the last two and they are all the same for legacy
      else if (previous >= plcpTrainingSymbolsStart)
        {
          NS_ASSERT ((preamble != WIFI_PREAMBLE_LONG) && (preamble != WIFI_PREAMBLE_SHORT));
          //Case 2a: current after payload start
          if (current >= plcpPayloadStart)
            {
              psr *= CalculateChunkSuccessRate (CalculateSnr (powerW,
                                                              noiseInterferenceW,
                                                              txVector.GetChannelWidth ()),
                                                plcpPayloadStart - previous,
                                                mcsHeaderMode, txVector);
              NS_LOG_DEBUG ("Case 2a - previous is in training or in SIG-B and current after payload start: mode=" << mcsHeaderMode << ", psr=" << psr);
            }
          //Case 2b: current is in training or in SIG-B
          else
            {
              psr *= CalculateChunkSuccessRate (CalculateSnr (powerW,
                                                              noiseInterferenceW,
                                                              txVector.GetChannelWidth ()),
                                                current - previous,
                                                mcsHeaderMode, txVector);
              NS_LOG_DEBUG ("Case 2b - previous is in training or in SIG-B and current is in training or in SIG-B: mode=" << mcsHeaderMode << ", psr=" << psr);
            }
        }
      //Case 3: previous is in HT-SIG or SIG-A: legacy will not enter here since it didn't enter in the last two and they are all the same for legacy
      else if (previous >= plcpHsigHeaderStart)
        {
          NS_ASSERT ((preamble != WIFI_PREAMBLE_LONG) && (preamble != WIFI_PREAMBLE_SHORT));
          //Case 3a: current after payload start
          if (current >= plcpPayloadStart)
            {
              psr *= CalculateChunkSuccessRate (CalculateSnr (powerW,
                                                              noiseInterferenceW,
                                                              txVector.GetChannelWidth ()),
                                                plcpPayloadStart - plcpTrainingSymbolsStart,
                                                mcsHeaderMode, txVector);
              //Case 3ai: VHT or HE format
              if (preamble == WIFI_PREAMBLE_VHT_SU || preamble == WIFI_PREAMBLE_HE_SU || preamble == WIFI_PREAMBLE_VHT_MU || preamble == WIFI_PREAMBLE_HE_MU)
                {
                  //SIG-A is sent using legacy OFDM modulation
                  psr *= CalculateChunkSuccessRate (CalculateSnr (powerW,
                                                                  noiseInterferenceW,
                                                                  txVector.GetChannelWidth ()),
                                                    plcpTrainingSymbolsStart - previous,
                                                    headerMode, txVector);
                  NS_LOG_DEBUG ("Case 3ai - previous is in SIG-A and current after payload start: mcs mode=" << mcsHeaderMode << ", legacy mode=" << headerMode << ", psr=" << psr);
                }
              //Case 3aii: HT formats
              else
                {
                  psr *= CalculateChunkSuccessRate (CalculateSnr (powerW,
                                                                  noiseInterferenceW,
                                                                  txVector.GetChannelWidth ()),
                                                    plcpTrainingSymbolsStart - previous,
                                                    mcsHeaderMode, txVector);
                  NS_LOG_DEBUG ("Case 3aii - previous is in HT-SIG and current after payload start: mode=" << mcsHeaderMode << ", psr=" << psr);
                }
            }
          //Case 3b: current is in training or in SIG-B
          else if (current >= plcpTrainingSymbolsStart)
            {
              psr *= CalculateChunkSuccessRate (CalculateSnr (powerW,
                                                              noiseInterferenceW,
                                                              txVector.GetChannelWidth ()),
                                                current - plcpTrainingSymbolsStart,
                                                mcsHeaderMode, txVector);
              //Case 3bi: VHT or HE format
              if (preamble == WIFI_PREAMBLE_VHT_SU || preamble == WIFI_PREAMBLE_HE_SU || preamble == WIFI_PREAMBLE_VHT_MU || preamble == WIFI_PREAMBLE_HE_MU)
                {
                  //SIG-A is sent using legacy OFDM modulation
                  psr *= CalculateChunkSuccessRate (CalculateSnr (powerW,
                                                                  noiseInterferenceW,
                                                                  txVector.GetChannelWidth ()),
                                                    plcpTrainingSymbolsStart - previous,
                                                    headerMode, txVector);
                  NS_LOG_DEBUG ("Case 3bi - previous is in SIG-A and current is in training or in SIG-B: mode=" << headerMode << ", psr=" << psr);
                }
              //Case 3bii: HT formats
              else
                {
                  psr *= CalculateChunkSuccessRate (CalculateSnr (powerW,
                                                                  noiseInterferenceW,
                                                                  txVector.GetChannelWidth ()),
                                                    plcpTrainingSymbolsStart - previous,
                                                    mcsHeaderMode, txVector);
                  NS_LOG_DEBUG ("Case 3bii - previous is in HT-SIG and current is in HT training: mode=" << mcsHeaderMode << ", psr=" << psr);
                }
            }
          //Case 3c: current with previous in HT-SIG or SIG-A
          else
            {
              //Case 3ci: VHT or HE format
              if (preamble == WIFI_PREAMBLE_VHT_SU || preamble == WIFI_PREAMBLE_HE_SU || preamble == WIFI_PREAMBLE_VHT_MU || preamble == WIFI_PREAMBLE_HE_MU)
                {
                  //SIG-A is sent using legacy OFDM modulation
                  psr *= CalculateChunkSuccessRate (CalculateSnr (powerW,
                                                                  noiseInterferenceW,
                                                                  txVector.GetChannelWidth ()),
                                                    current - previous,
                                                    headerMode, txVector);
                  NS_LOG_DEBUG ("Case 3ci - previous with current in SIG-A: mode=" << headerMode << ", psr=" << psr);
                }
              //Case 3cii: HT mixed format or HT greenfield
              else
                {
                  psr *= CalculateChunkSuccessRate (CalculateSnr (powerW,
                                                                  noiseInterferenceW,
                                                                  txVector.GetChannelWidth ()),
                                                    current - previous,
                                                    mcsHeaderMode, txVector);
                  NS_LOG_DEBUG ("Case 3cii - previous with current in HT-SIG: mode=" << mcsHeaderMode << ", psr=" << psr);
                }
            }
        }
      //Case 4: previous in L-SIG: HT GF will not reach here because it will execute the previous if and exit
      else if (previous >= plcpHeaderStart)
        {
          NS_ASSERT (preamble != WIFI_PREAMBLE_HT_GF);
          //Case 4a: current after payload start
          if (current >= plcpPayloadStart)
            {
              //Case 4ai: legacy format
              if (preamble == WIFI_PREAMBLE_LONG || preamble == WIFI_PREAMBLE_SHORT)
                {
                  psr *= 1;
                  NS_LOG_DEBUG ("Case 4ai - previous in L-SIG and current after payload start: nothing to do");
                }
              //Case 4aii: VHT or HE format
              else if (preamble == WIFI_PREAMBLE_VHT_SU || preamble == WIFI_PREAMBLE_HE_SU || preamble == WIFI_PREAMBLE_VHT_MU || preamble == WIFI_PREAMBLE_HE_MU)
                {
                  psr *= CalculateChunkSuccessRate (CalculateSnr (powerW,
                                                                  noiseInterferenceW,
                                                                  txVector.GetChannelWidth ()),
                                                    plcpPayloadStart - plcpTrainingSymbolsStart,
                                                    mcsHeaderMode, txVector);
                  psr *= CalculateChunkSuccessRate (CalculateSnr (powerW,
                                                                  noiseInterferenceW,
                                                                  txVector.GetChannelWidth ()),
                                                    plcpTrainingSymbolsStart - plcpHsigHeaderStart,
                                                    headerMode, txVector);
                  NS_LOG_DEBUG ("Case 4aii - previous is in L-SIG and current after payload start: mcs mode=" << mcsHeaderMode << ", legacy mode=" << headerMode << ", psr=" << psr);
                }
              //Case 4aiii: HT mixed format
              else
                {
                  psr *= CalculateChunkSuccessRate (CalculateSnr (powerW,
                                                                  noiseInterferenceW,
                                                                  txVector.GetChannelWidth ()),
                                                    plcpPayloadStart - plcpHsigHeaderStart,
                                                    mcsHeaderMode, txVector);
                  NS_LOG_DEBUG ("Case 4aiii - previous in L-SIG and current after payload start: mcs mode=" << mcsHeaderMode << ", psr=" << psr);
                }
            }
          //Case 4b: current is in training or in SIG-B. legacy will not come here since it went in previous if or if the previous if is not true this will be not true
          else if (current >= plcpTrainingSymbolsStart)
            {
              NS_ASSERT ((preamble != WIFI_PREAMBLE_LONG) && (preamble != WIFI_PREAMBLE_SHORT));
              //Case 4bi: VHT or HE format
              if (preamble == WIFI_PREAMBLE_VHT_SU || preamble == WIFI_PREAMBLE_HE_SU || preamble == WIFI_PREAMBLE_VHT_MU || preamble == WIFI_PREAMBLE_HE_MU)
                {
                  psr *= CalculateChunkSuccessRate (CalculateSnr (powerW,
                                                                  noiseInterferenceW,
                                                                  txVector.GetChannelWidth ()),
                                                    current - plcpTrainingSymbolsStart,
                                                    mcsHeaderMode, txVector);
                  psr *= CalculateChunkSuccessRate (CalculateSnr (powerW,
                                                                  noiseInterferenceW,
                                                                  txVector.GetChannelWidth ()),
                                                    plcpTrainingSymbolsStart - plcpHsigHeaderStart,
                                                    headerMode, txVector);
                  NS_LOG_DEBUG ("Case 4bi - previous is in L-SIG and current in training or in SIG-B: mcs mode=" << mcsHeaderMode << ", legacy mode=" << headerMode << ", psr=" << psr);
                }
              //Case 4bii: HT mixed format
              else
                {
                  psr *= CalculateChunkSuccessRate (CalculateSnr (powerW,
                                                                  noiseInterferenceW,
                                                                  txVector.GetChannelWidth ()),
                                                    current - plcpHsigHeaderStart,
                                                    mcsHeaderMode, txVector);
                  NS_LOG_DEBUG ("Case 4bii - previous in L-SIG and current in HT training: mcs mode=" << mcsHeaderMode << ", psr=" << psr);
                }
            }
          //Case 4c: current in HT-SIG or in SIG-A. Legacy will not come here since it went in previous if or if the previous if is not true this will be not true
          else if (current >= plcpHsigHeaderStart)
            {
              NS_ASSERT ((preamble != WIFI_PREAMBLE_LONG) && (preamble != WIFI_PREAMBLE_SHORT));
              //Case 4ci: VHT format
              if (preamble == WIFI_PREAMBLE_VHT_SU || preamble == WIFI_PREAMBLE_HE_SU || preamble == WIFI_PREAMBLE_VHT_MU || preamble == WIFI_PREAMBLE_HE_MU)
                {
                  psr *= CalculateChunkSuccessRate (CalculateSnr (powerW,
                                                                  noiseInterferenceW,
                                                                  txVector.GetChannelWidth ()),
                                                    current - plcpHsigHeaderStart,
                                                    headerMode, txVector);
                  NS_LOG_DEBUG ("Case 4ci - previous is in L-SIG and current in SIG-A: mode=" << headerMode << ", psr=" << psr);
                }
              //Case 4cii: HT mixed format
              else
                {
                  psr *= CalculateChunkSuccessRate (CalculateSnr (powerW,
                                                                  noiseInterferenceW,
                                                                  txVector.GetChannelWidth ()),
                                                    current - plcpHsigHeaderStart,
                                                    mcsHeaderMode, txVector);
                  NS_LOG_DEBUG ("Case 4cii - previous in L-SIG and current in HT-SIG: mcs mode=" << mcsHeaderMode << ", psr=" << psr);
                }
            }
          //Case 4d: current with previous in L-SIG
          else
            {
              psr *= 1;
              NS_LOG_DEBUG ("Case 4d - current with previous in L-SIG: nothing to do");
            }
        }
      //Case 5: previous is in the preamble works for all cases
      else
        {
          //Case 5a: current after payload start
          if (current >= plcpPayloadStart)
            {
              //Case 5ai: legacy format (No HT-SIG or Training Symbols)
              if (preamble == WIFI_PREAMBLE_LONG || preamble == WIFI_PREAMBLE_SHORT)
                {
                  psr *= 1;
                  NS_LOG_DEBUG ("Case 5ai - previous is in the preamble and current is after payload start: nothing to do");
                }
              //Case 5aii: VHT or HE format
              else if (preamble == WIFI_PREAMBLE_VHT_SU || preamble == WIFI_PREAMBLE_HE_SU || preamble == WIFI_PREAMBLE_VHT_MU || preamble == WIFI_PREAMBLE_HE_MU)
                {
                  psr *= CalculateChunkSuccessRate (CalculateSnr (powerW,
                                                                  noiseInterferenceW,
                                                                  txVector.GetChannelWidth ()),
                                                    plcpPayloadStart - plcpTrainingSymbolsStart,
                                                    mcsHeaderMode, txVector);
                  psr *= CalculateChunkSuccessRate (CalculateSnr (powerW,
                                                                  noiseInterferenceW,
                                                                  txVector.GetChannelWidth ()),
                                                    plcpTrainingSymbolsStart - plcpHsigHeaderStart,
                                                    headerMode, txVector);
                  NS_LOG_DEBUG ("Case 5aii - previous is in the preamble and current is after payload start: mcs mode=" << mcsHeaderMode << ", legacy mode=" << headerMode << ", psr=" << psr);
                }
              //Case 5aiii: HT formats
              else
                {
                  psr *= CalculateChunkSuccessRate (CalculateSnr (powerW,
                                                                  noiseInterferenceW,
                                                                  txVector.GetChannelWidth ()),
                                                    plcpPayloadStart - plcpHsigHeaderStart,
                                                    mcsHeaderMode, txVector);
                  NS_LOG_DEBUG ("Case 5aiii - previous is in the preamble and current is after payload start: mcs mode=" << mcsHeaderMode << ", legacy mode=" << headerMode << ", psr=" << psr);
                }
            }
          //Case 5b: current is in training or in SIG-B. Legacy will not come here since it went in previous if or if the previous if is not true this will be not true
          else if (current >= plcpTrainingSymbolsStart)
            {
              NS_ASSERT ((preamble != WIFI_PREAMBLE_LONG) && (preamble != WIFI_PREAMBLE_SHORT));
              //Case 5bi: VHT or HE format
              if (preamble == WIFI_PREAMBLE_VHT_SU || preamble == WIFI_PREAMBLE_HE_SU || preamble == WIFI_PREAMBLE_VHT_MU || preamble == WIFI_PREAMBLE_HE_MU)
                {
                  psr *= CalculateChunkSuccessRate (CalculateSnr (powerW,
                                                                  noiseInterferenceW,
                                                                  txVector.GetChannelWidth ()),
                                                    current - plcpTrainingSymbolsStart,
                                                    mcsHeaderMode, txVector);
                  psr *= CalculateChunkSuccessRate (CalculateSnr (powerW,
                                                                  noiseInterferenceW,
                                                                  txVector.GetChannelWidth ()),
                                                    plcpTrainingSymbolsStart - plcpHsigHeaderStart,
                                                    headerMode, txVector);
                  NS_LOG_DEBUG ("Case 5bi - previous is in the preamble and current in training or in SIG-B: mcs mode=" << mcsHeaderMode << ", legacy mode=" << headerMode << ", psr=" << psr);
                }
              //Case 5bii: HT mixed format
              else
                {
                  psr *= CalculateChunkSuccessRate (CalculateSnr (powerW,
                                                                  noiseInterferenceW,
                                                                  txVector.GetChannelWidth ()),
                                                    current - plcpHsigHeaderStart,
                                                    mcsHeaderMode, txVector);
                  NS_LOG_DEBUG ("Case 5bii - previous is in the preamble and current in HT training: mcs mode=" << mcsHeaderMode << ", psr=" << psr);
                }
            }
          //Case 5c: current in HT-SIG or in SIG-A. Legacy will not come here since it went in previous if or if the previous if is not true this will be not true
          else if (current >= plcpHsigHeaderStart)
            {
              NS_ASSERT ((preamble != WIFI_PREAMBLE_LONG) && (preamble != WIFI_PREAMBLE_SHORT));
              //Case 5ci: VHT or HE format
              if (preamble == WIFI_PREAMBLE_VHT_SU || preamble == WIFI_PREAMBLE_HE_SU || preamble == WIFI_PREAMBLE_VHT_MU || preamble == WIFI_PREAMBLE_HE_MU)
                {
                  psr *= CalculateChunkSuccessRate (CalculateSnr (powerW,
                                                                  noiseInterferenceW,
                                                                  txVector.GetChannelWidth ()),
                                                    current - plcpHsigHeaderStart,
                                                    headerMode, txVector);
                  NS_LOG_DEBUG ("Case 5ci - previous is in preamble and current in SIG-A: mode=" << headerMode << ", psr=" << psr);
                }
              //Case 5cii: HT formats
              else
                {
                  psr *= CalculateChunkSuccessRate (CalculateSnr (powerW,
                                                                  noiseInterferenceW,
                                                                  txVector.GetChannelWidth ()),
                                                    current - plcpHsigHeaderStart,
                                                    mcsHeaderMode, txVector);
                  NS_LOG_DEBUG ("Case 5cii - previous in preamble and current in HT-SIG: mcs mode=" << mcsHeaderMode << ", psr=" << psr);
                }
            }
          //Case 5d: current is in L-SIG. HT-GF will not come here
          else if (current >= plcpHeaderStart)
            {
              NS_ASSERT (preamble != WIFI_PREAMBLE_HT_GF);
              psr *= 1;
              NS_LOG_DEBUG ("Case 5d - previous is in the preamble and current is in L-SIG: nothing to do");
            }
        }

      noiseInterferenceW = j->second.GetPower () - powerW;
      previous = j->first;
    }

  double per = 1 - psr;
  return per;
}

struct InterferenceHelper::SnrPer
InterferenceHelper::CalculatePayloadSnrPer (Ptr<Event> event, uint16_t primaryChannelFrequency, std::pair<Time, Time> relativeMpduStartStop) const
{
  NiChangesPerBand ni;
  uint16_t channelWidth = event->GetTxVector ().GetChannelWidth ();
  auto band = std::make_pair (primaryChannelFrequency, channelWidth == 22 ? 20 : channelWidth);
  double noiseInterferenceW = CalculateNoiseInterferenceW (event, &ni, band);
  double snr = CalculateSnr (event->GetRxPowerW (band),
                             noiseInterferenceW,
                             channelWidth);

  /* calculate the SNIR at the start of the MPDU (located through windowing) and accumulate
   * all SNIR changes in the snir vector.
   */
  double per = CalculatePayloadPer (event, &ni, band, relativeMpduStartStop);

  struct SnrPer snrPer;
  snrPer.snr = snr;
  snrPer.per = per;
  return snrPer;
}

double
InterferenceHelper::CalculateSnr (Ptr<Event> event, uint16_t primaryChannelFrequency) const
{
  NiChangesPerBand ni;
  uint16_t channelWidth = event->GetTxVector ().GetChannelWidth ();
  auto band = std::make_pair (primaryChannelFrequency, channelWidth == 22 ? 20 : channelWidth);
  double noiseInterferenceW = CalculateNoiseInterferenceW (event, &ni, band);
  double snr = CalculateSnr (event->GetRxPowerW (band),
                             noiseInterferenceW,
                             channelWidth);
  return snr;
}

struct InterferenceHelper::SnrPer
InterferenceHelper::CalculateLegacyPhyHeaderSnrPer (Ptr<Event> event, uint16_t primaryChannelFrequency) const
{
  NS_LOG_FUNCTION (this << primaryChannelFrequency << event->GetTxVector ().GetChannelWidth ());
  NiChangesPerBand ni;
  uint16_t channelWidth;
  if (event->GetTxVector ().GetChannelWidth () >= 40)
    {
      channelWidth = 20; //calculate PER on the 20 MHz primary channel for L-SIG
    }
  else
    {
      channelWidth = event->GetTxVector ().GetChannelWidth ();
    }
  auto band = std::make_pair (primaryChannelFrequency, channelWidth == 22 ? 20 : channelWidth);
  double noiseInterferenceW = CalculateNoiseInterferenceW (event, &ni, band);
  double snr = CalculateSnr (event->GetRxPowerW (band),
                             noiseInterferenceW,
                             channelWidth);

  /* calculate the SNIR at the start of the plcp header and accumulate
   * all SNIR changes in the snir vector.
   */
  double per = CalculateLegacyPhyHeaderPer (event, &ni);

  struct SnrPer snrPer;
  snrPer.snr = snr;
  snrPer.per = per;
  return snrPer;
}

struct InterferenceHelper::SnrPer
InterferenceHelper::CalculateNonLegacyPhyHeaderSnrPer (Ptr<Event> event, uint16_t primaryChannelFrequency) const
{
  NiChangesPerBand ni;
  uint16_t channelWidth;
  if (event->GetTxVector ().GetChannelWidth () >= 40)
    {
      channelWidth = 20; //calculate PER on the 20 MHz primary channel for L-SIG
    }
  else
    {
      channelWidth = event->GetTxVector ().GetChannelWidth ();
    }
  auto band = std::make_pair (primaryChannelFrequency, channelWidth == 22 ? 20 : channelWidth);
  double noiseInterferenceW = CalculateNoiseInterferenceW (event, &ni, band);
  double snr = CalculateSnr (event->GetRxPowerW (band),
                             noiseInterferenceW,
                             channelWidth);
  
  /* calculate the SNIR at the start of the plcp header and accumulate
   * all SNIR changes in the snir vector.
   */
  double per = CalculateNonLegacyPhyHeaderPer (event, &ni);
  
  struct SnrPer snrPer;
  snrPer.snr = snr;
  snrPer.per = per;
  return snrPer;
}

void
InterferenceHelper::EraseEvents (void)
{
  for (auto it : m_niChangesPerBand)
    {
      it.second.clear ();
      // Always have a zero power noise event in the list
      AddNiChangeEvent (Time (0), NiChange (0.0, 0), it.first);
      m_firstPowerPerBand.at(it.first) = 0.0;
    }
  m_rxing = false;
}

InterferenceHelper::NiChanges::const_iterator
InterferenceHelper::GetNextPosition (Time moment, FrequencyWidthPair band) const
{
  auto it = m_niChangesPerBand.find (band);
  NS_ASSERT (it != m_niChangesPerBand.end ());
  return it->second.upper_bound (moment);
}

InterferenceHelper::NiChanges::const_iterator
InterferenceHelper::GetPreviousPosition (Time moment, FrequencyWidthPair band) const
{
  auto it = GetNextPosition (moment, band);
  // This is safe since there is always an NiChange at time 0,
  // before moment.
  --it;
  return it;
}

InterferenceHelper::NiChanges::iterator
InterferenceHelper::AddNiChangeEvent (Time moment, NiChange change, FrequencyWidthPair band)
{
  auto it = m_niChangesPerBand.find (band);
  NS_ASSERT (it != m_niChangesPerBand.end ());
  return it->second.insert (GetNextPosition (moment, band), std::make_pair (moment, change));
}

void
InterferenceHelper::NotifyRxStart ()
{
  NS_LOG_FUNCTION (this);
  m_rxing = true;
}

void
InterferenceHelper::NotifyRxEnd ()
{
  NS_LOG_FUNCTION (this);
  m_rxing = false;
  //Update m_firstPower for frame capture
  for (auto ni : m_niChangesPerBand)
    {
      auto it = ni.second.find (Simulator::Now ());
      it--;
      m_firstPowerPerBand.find (ni.first)->second = it->second.GetPower ();
    }
}

} //namespace ns3
