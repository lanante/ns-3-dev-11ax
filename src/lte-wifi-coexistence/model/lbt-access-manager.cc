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
 * Created on: Aug 11, 2015
 * Authors: Biljana Bojovic <bbojovic@cttc.es> and Tom Henderson <tomh@tomh.org>
 */


#include "lbt-access-manager.h"
#include "ns3/assert.h"
#include "ns3/log.h"
#include "ns3/spectrum-wifi-phy.h"
#include "ns3/wifi-phy-listener.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("LbtAccessManager");

NS_OBJECT_ENSURE_REGISTERED (LbtAccessManager);


/**
 * Listener for PHY events. Forwards to lbtaccessmanager
 */
class LbtPhyListener : public WifiPhyListener
{
public:
  /**
   * Create a PhyListener for the given DcfManager.
   *
   * \param dcf
   */
  LbtPhyListener (ns3::LbtAccessManager *lbt) : m_lbtAccessManager (lbt)
  {
  }

  virtual ~LbtPhyListener ()
  {
  }

  void NotifyRxStart (Time duration)
  {
    NS_LOG_FUNCTION (this << duration);
    m_lbtAccessManager->NotifyRxStartNow (duration);
  }
  void NotifyRxEndOk (void)
  {
    m_lbtAccessManager->NotifyRxEndOkNow ();
  }
  void NotifyRxEndError (void)
  {
    m_lbtAccessManager->NotifyRxEndErrorNow ();
  }
  void NotifyTxStart (Time duration, double txPowerDbm)
  {
    NS_LOG_FUNCTION (this << duration << txPowerDbm);
    m_lbtAccessManager->NotifyTxStartNow (duration);
  }
  void NotifyMaybeCcaBusyStart (Time duration)
  {
    NS_LOG_FUNCTION (this << duration);
    m_lbtAccessManager->NotifyMaybeCcaBusyStartNow (duration);
  }
  void NotifySwitchingStart (Time duration)
  {
    NS_LOG_FUNCTION (this << duration);
    m_lbtAccessManager->NotifySwitchingStartNow (duration);
  }
  void NotifySleep (void)
  {
    m_lbtAccessManager->NotifySleepNow ();
  }
  void NotifyOff (void)
  {
    m_lbtAccessManager->NotifyOffNow ();
  }
  void NotifyWakeup (void)
  {
    m_lbtAccessManager->NotifyWakeupNow ();
  }
  void NotifyOn (void)
  {
    m_lbtAccessManager->NotifyOnNow ();
  }

private:
  LbtAccessManager *m_lbtAccessManager;  //!< DcfManager to forward events to
};


TypeId
LbtAccessManager::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::LbtAccessManager")
    .SetParent<LteChannelAccessManager> ()
    .SetGroupName ("lte-wifi-coexistence")
    .AddAttribute ("Slot", "The duration of a Slot.",
                   TimeValue (MicroSeconds (9)),
                   MakeTimeAccessor (&LbtAccessManager::m_slotTime),
                   MakeTimeChecker ())
    .AddAttribute ("DeferTime", "TimeInterval to defer during CCA",
                   TimeValue (MicroSeconds (43)),
                   MakeTimeAccessor (&LbtAccessManager::m_deferTime),
                   MakeTimeChecker ())
    .AddAttribute ("MinCw", "The minimum value of the contention window.",
                   UintegerValue (15),
                   MakeUintegerAccessor (&LbtAccessManager::m_cwMin),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("MaxCw", "The maximum value of the contention window. For the priority class 3 this value is set to 63, and for priority class 4 it is 1023.",
                   UintegerValue (63),
                   MakeUintegerAccessor (&LbtAccessManager::m_cwMax),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("Txop",
                   "Duration of channel access grant.",
                   TimeValue (MilliSeconds (8)),
                   MakeTimeAccessor (&LbtAccessManager::m_txop),
                   MakeTimeChecker (MilliSeconds (4), MilliSeconds (20)))
    .AddAttribute ("UseReservationSignal",
                   "Whether to use a reservation signal when there is no data ready to be transmitted.",
                   BooleanValue (true),
                   MakeBooleanAccessor (&LbtAccessManager::m_reservationSignal),
                   MakeBooleanChecker ())
    .AddAttribute ("CwUpdateRule",
                   "Rule according which CW will be updated",
                   EnumValue (LbtAccessManager::NACKS_80_PERCENT),
                   MakeEnumAccessor (&LbtAccessManager::m_cwUpdateRule),
                   MakeEnumChecker (LbtAccessManager::ALL_NACKS, "ALL_NACKS",
                                    LbtAccessManager::ANY_NACK, "ANY_NACK",
                                    LbtAccessManager::NACKS_10_PERCENT, "NACKS_10_PERCENT",
                                    LbtAccessManager::NACKS_80_PERCENT, "NACKS_80_PERCENT"))
    .AddTraceSource ("Cw",
                     "CW change trace source; logs changes to CW",
                     MakeTraceSourceAccessor (&LbtAccessManager::m_cw),
                     "ns3::TracedValue::Uint32Callback")
    .AddTraceSource ("Backoff",
                     "Backoff change trace source; logs any new backoff",
                     MakeTraceSourceAccessor (&LbtAccessManager::m_currentBackoffSlots),
                     "ns3::TracedValue::Uint32Callback")
    .AddAttribute ("HarqFeedbackDelay", "Harq feedback delay",
                   TimeValue (MilliSeconds (5)),
                   MakeTimeAccessor (&LbtAccessManager::m_harqFeedbackDelay),
                   MakeTimeChecker ())
    .AddAttribute ("HarqFeedbackExpirationTime", "If harq feedback is older than this time,  do not use it for contention windown update.",
                   TimeValue (MilliSeconds (100)),
                   MakeTimeAccessor (&LbtAccessManager::m_harqFeedbackExpirationTime),
                   MakeTimeChecker ())
    .AddConstructor<LbtAccessManager> ()
  ;
  return tid;
}


LbtAccessManager::LbtAccessManager ()
  : LteChannelAccessManager (),
    m_lbtPhyListener (0),
    m_state (IDLE),
    m_currentBackoffSlots (0),
    m_backoffCount (0),
    m_grantRequested (false),
    m_backoffStartTime (Seconds (0)),
    m_lastBusyTime (Seconds (0)),
    m_lastConsideredTxopStartTime (Seconds (0))
{
  NS_LOG_FUNCTION (this);
  m_rng = CreateObject<UniformRandomVariable> ();
}

LbtAccessManager::~LbtAccessManager ()
{
  NS_LOG_FUNCTION (this);
  // TODO Auto-generated destructor stub
  delete m_lbtPhyListener;
  m_lbtPhyListener = 0;
  if (m_waitForDeferEventId.IsRunning ())
    {
      m_waitForDeferEventId.Cancel ();
    }
  if (m_waitForBackoffEventId.IsRunning ())
    {
      m_waitForBackoffEventId.Cancel ();
    }
  if (m_waitForBusyEventId.IsRunning ())
    {
      m_waitForBusyEventId.Cancel ();
    }
}

int64_t
LbtAccessManager::AssignStreams (int64_t stream)
{
  NS_LOG_FUNCTION (this << stream);
  m_rng->SetStream (stream);
  return 1;
}

void
LbtAccessManager::SetupPhyListener (Ptr<SpectrumWifiPhy> phy)
{
  NS_LOG_FUNCTION (this << phy);
  if (m_lbtPhyListener != 0)
    {
      NS_LOG_DEBUG ("Deleting previous listener " << m_lbtPhyListener);
      phy->UnregisterListener (m_lbtPhyListener);
      delete m_lbtPhyListener;
    }
  m_lbtPhyListener = new LbtPhyListener (this);
  phy->RegisterListener (m_lbtPhyListener);
}

void
LbtAccessManager::SetWifiPhy (Ptr<SpectrumWifiPhy> phy)
{
  NS_LOG_FUNCTION (this << phy);
  SetupPhyListener (phy);
  m_wifiPhy = phy;
  // Configure the WifiPhy to treat each incoming signal as a foreign signal
  // (energy detection only)
  m_wifiPhy->SetAttribute ("DisableWifiReception", BooleanValue (true));
  m_wifiPhy->SetAttribute ("CcaEdThreshold", DoubleValue (m_edThreshold));
  // Initialization of post-attribute-construction variables can be done here
  m_cw = m_cwMin;
}

void
LbtAccessManager::SetLteEnbMac (Ptr<LteEnbMac> lteEnbMac)
{
  NS_LOG_FUNCTION (this << lteEnbMac);
  m_lteEnbMac = lteEnbMac;
  m_lteEnbMac->TraceConnectWithoutContext ("DlHarqFeedback", MakeCallback (&ns3::LbtAccessManager::UpdateCwBasedOnHarq,this));
}

void
LbtAccessManager::SetLteEnbPhy (Ptr<LteEnbPhy> lteEnbPhy)
{
  NS_LOG_FUNCTION (this << lteEnbPhy);
  m_lteEnbPhy = lteEnbPhy;
  m_lteEnbPhy->SetUseReservationSignal (m_reservationSignal);
}


LbtAccessManager::LbtState
LbtAccessManager::GetLbtState () const
{
  NS_LOG_FUNCTION (this);
  return m_state;
}

void
LbtAccessManager::SetLbtState (LbtAccessManager::LbtState state)
{
  NS_LOG_FUNCTION (this << state);
  m_state = state;
}

void
LbtAccessManager::DoRequestAccess ()
{
  NS_LOG_FUNCTION (this);
  NS_ASSERT_MSG (m_wifiPhy, "LbtAccessManager not connected to a WifiPhy");

  if (m_grantRequested == true)
    {
      NS_LOG_LOGIC ("Already waiting to grant access; ignoring request");
      return;
    }

  if (Simulator::Now () - m_lastBusyTime >= m_deferTime)
    {
      // may grant access immediately according to ETSI BRAN flowchart
      NS_LOG_LOGIC ("Grant access immediately, channel is free more than differ time");
      SetGrant();
      return;
    }
  m_grantRequested = true;

  // Draw new backoff value upon entry to the access granting process
  m_currentBackoffSlots = GetBackoffSlots (); // don't decrement traced value
  m_backoffCount = m_currentBackoffSlots; // decrement this counter instead
  NS_LOG_DEBUG ("New backoff count " << m_backoffCount);

  if (GetLbtState () == IDLE)
    {
      // Continue to wait until defer time has expired
      Time deferRemaining = m_deferTime - (Simulator::Now () - m_lastBusyTime);
      NS_LOG_LOGIC ("Must wait " << deferRemaining.GetSeconds () << " defer period");
      m_waitForDeferEventId = Simulator::Schedule (deferRemaining, &LbtAccessManager::RequestAccessAfterDefer, this);
      SetLbtState (WAIT_FOR_DEFER);
    }
  else if (m_lastBusyTime > Simulator::Now ())
    {
      // Access has come in while channel is already busy
      NS_ASSERT_MSG (Simulator::Now () < m_lastBusyTime, "Channel is busy but m_lastBusyTime in the past");
      Time busyRemaining = m_lastBusyTime - Simulator::Now ();
      NS_LOG_LOGIC ("Must wait " << busyRemaining.GetSeconds () << " sec busy period");
      TransitionToBusy (busyRemaining);
    }
}

void
LbtAccessManager::TransitionFromBusy ()
{
  NS_LOG_FUNCTION (this);
  if (m_lastBusyTime > Simulator::Now ())
    {
      Time busyRemaining = m_lastBusyTime - Simulator::Now ();
      NS_LOG_LOGIC ("Must wait additional " << busyRemaining.GetSeconds () << " busy period");
      m_waitForBusyEventId = Simulator::Schedule (busyRemaining, &LbtAccessManager::TransitionFromBusy, this);
      SetLbtState (BUSY);
    }
  else if (m_grantRequested == true)
    {
      NS_LOG_DEBUG ("Scheduling defer time to expire " << m_deferTime.GetMicroSeconds () + Simulator::Now ().GetMicroSeconds ());
      m_waitForDeferEventId = Simulator::Schedule (m_deferTime, &LbtAccessManager::RequestAccessAfterDefer, this);
      SetLbtState (WAIT_FOR_DEFER);
    }
  else 
    {
      SetLbtState (IDLE);
    }
}

void
LbtAccessManager::TransitionToBusy (Time duration)
{
  NS_LOG_FUNCTION (this);
  switch (GetLbtState ())
  {
    case IDLE:
      {
        NS_ASSERT_MSG (m_backoffCount == 0, "was idle but m_backoff nonzero");
        m_lastBusyTime = Simulator::Now () + duration;
        m_waitForBusyEventId = Simulator::Schedule (duration, &LbtAccessManager::TransitionFromBusy, this);
        NS_LOG_DEBUG ("Going busy until " << m_lastBusyTime.GetMicroSeconds ());
        SetLbtState (BUSY);
        break;
      }
    case TXOP_GRANTED:
     {
        m_lastBusyTime = Simulator::Now () + duration;
        m_waitForBusyEventId = Simulator::Schedule (duration, &LbtAccessManager::TransitionFromBusy, this);
        NS_LOG_DEBUG ("Going busy until " << m_lastBusyTime.GetMicroSeconds ());
        SetLbtState (BUSY);
        break;
     }
    case BUSY:
      {
        // Update last busy time
        if (m_lastBusyTime < (Simulator::Now () + duration))
          {
            NS_LOG_DEBUG ("Update last busy time to " << m_lastBusyTime.GetMicroSeconds ());
            m_lastBusyTime = Simulator::Now () + duration;
          }
        // Since we went busy, a request for access may have come in
        if (!m_waitForBusyEventId.IsRunning () && m_grantRequested > 0)
          {
            Time busyRemaining = m_lastBusyTime - Simulator::Now ();
            m_waitForBusyEventId = Simulator::Schedule (busyRemaining, &LbtAccessManager::TransitionFromBusy, this);
            NS_LOG_DEBUG ("Going busy until " << m_lastBusyTime.GetMicroSeconds ());
            SetLbtState (BUSY);
          }
      }
      break;
    case WAIT_FOR_DEFER:
      {
        NS_LOG_DEBUG ("TransitionToBusy from WAIT_FOR_DEFER");
        if (m_waitForDeferEventId.IsRunning ())
          {
            NS_LOG_DEBUG ("Cancelling Defer");
            m_waitForDeferEventId.Cancel ();
          }
        m_waitForBusyEventId = Simulator::Schedule (duration, &LbtAccessManager::TransitionFromBusy, this);
        m_lastBusyTime = Simulator::Now () + duration;
        NS_LOG_DEBUG ("Going busy until " << m_lastBusyTime.GetMicroSeconds ());
        SetLbtState (BUSY);
      }
      break;
    case WAIT_FOR_BACKOFF:
      {
        NS_LOG_DEBUG ("TransitionToBusy from WAIT_FOR_BACKOFF");
        if (m_waitForBackoffEventId.IsRunning ())
          {
            NS_LOG_DEBUG ("Cancelling Backoff");
            m_waitForBackoffEventId.Cancel ();
          }
        Time timeSinceBackoffStart = Simulator::Now () - m_backoffStartTime;
        NS_ASSERT (timeSinceBackoffStart < m_backoffCount * m_slotTime);
        // Decrement backoff count for every full and fractional m_slot time
        while (timeSinceBackoffStart > Seconds (0) && m_backoffCount > 0)
          {
            m_backoffCount--;
            timeSinceBackoffStart -= m_slotTime;
          }
        m_waitForBusyEventId = Simulator::Schedule (duration, &LbtAccessManager::TransitionFromBusy, this);
        m_lastBusyTime = Simulator::Now () + duration;
        NS_LOG_DEBUG ("Suspend backoff, go busy until " << m_lastBusyTime.GetMicroSeconds ());
        SetLbtState (BUSY);
      }
      break;
    default:
      NS_FATAL_ERROR ("Should be unreachable " << GetLbtState ());
  }
}

void
LbtAccessManager::RequestAccessAfterDefer ()
{
  NS_LOG_FUNCTION (this);
  // if channel has remained idle so far, wait for a number of backoff
  // slots to see if it is remains idle
  if (GetLbtState () == WAIT_FOR_DEFER)
    {
      if (m_backoffCount == 0)
        {
          NS_LOG_DEBUG ("Defer succeeded, backoff count already zero");
          SetGrant();
        }
      else
        {
          NS_LOG_DEBUG ("Defer succeeded, scheduling for " << m_backoffCount << " backoffSlots");
          m_backoffStartTime = Simulator::Now ();
          Time timeForBackoff = m_slotTime * m_backoffCount;
          m_waitForBackoffEventId = Simulator::Schedule (timeForBackoff, &LbtAccessManager::RequestAccessAfterBackoff, this);
          SetLbtState (WAIT_FOR_BACKOFF);
        }
    }
  else
    {
      NS_LOG_DEBUG ("Was not deferring, scheduling for " << m_deferTime.GetMicroSeconds () << " microseconds");
      m_waitForDeferEventId = Simulator::Schedule (m_deferTime, &LbtAccessManager::RequestAccessAfterDefer, this);
      SetLbtState (WAIT_FOR_DEFER);
    }
}

void
LbtAccessManager::RequestAccessAfterBackoff ()
{
  NS_LOG_FUNCTION (this);
  // if the channel has remained idle, grant access now for a TXOP
  if (GetLbtState () == WAIT_FOR_BACKOFF)
    {
      SetGrant();
      m_backoffCount = 0;
    }
  else
    { 
      NS_FATAL_ERROR ("Unreachable?");
    }
}

uint32_t
LbtAccessManager::GetBackoffSlots ()
{
  NS_LOG_FUNCTION (this);
  // Integer between 0 and m_cw
  return (m_rng->GetInteger (0, m_cw.Get ()));
}

void
LbtAccessManager::NotifyWakeupNow ()
{
  NS_LOG_FUNCTION (this);
  //ResetCw ();
}

void
LbtAccessManager::NotifyRxStartNow (Time duration)
{
  NS_LOG_FUNCTION (this << duration.GetSeconds ());
  TransitionToBusy (duration);
}

void
LbtAccessManager::NotifyRxEndOkNow ()
{
  NS_LOG_FUNCTION (this);
  // Ignore; may be busy still
}

void
LbtAccessManager::NotifyRxEndErrorNow ()
{
  NS_LOG_FUNCTION (this);
  // Ignore; may be busy still
}

void
LbtAccessManager::NotifyTxStartNow (Time duration)
{
  NS_LOG_FUNCTION (this << duration.GetSeconds ());
  NS_FATAL_ERROR ("Unimplemented");
}

void
LbtAccessManager::NotifyMaybeCcaBusyStartNow (Time duration)
{
  NS_LOG_FUNCTION (this << duration.GetSeconds ());
  TransitionToBusy (duration);
}

void
LbtAccessManager::NotifySwitchingStartNow (Time duration)
{
  NS_LOG_FUNCTION (this << duration.GetSeconds ());
  NS_FATAL_ERROR ("Unimplemented");
}

void
LbtAccessManager::NotifySleepNow ()
{
  NS_LOG_FUNCTION (this);
  NS_FATAL_ERROR ("Unimplemented");
}

void
LbtAccessManager::NotifyOffNow ()
{
  NS_LOG_FUNCTION (this);
  NS_FATAL_ERROR ("Unimplemented");
}

void
LbtAccessManager::NotifyOnNow ()
{
  NS_LOG_FUNCTION (this);
  NS_FATAL_ERROR ("Unimplemented");
}

void
LbtAccessManager::ResetCw (void)
{
  NS_LOG_FUNCTION (this);
  NS_LOG_DEBUG ("CW reset from " << m_cw  << " to " << m_cwMin);
  m_cw = m_cwMin;
}

void
LbtAccessManager::UpdateFailedCw (void)
{
  NS_LOG_FUNCTION (this);
  uint32_t oldValue = m_cw.Get ();
  m_cw = std::min ( 2 * (m_cw.Get () + 1) - 1, m_cwMax);
  NS_LOG_DEBUG ("CW updated from " << oldValue << " to " << m_cw);
  m_lastCWUpdateTime = Simulator::Now ();
}

void LbtAccessManager::SetGrant()
{
   NS_LOG_DEBUG ("Granting access through LteChannelAccessManager at time " << Simulator::Now ().GetMicroSeconds ());
   LteChannelAccessManager::SetGrantDuration (m_txop);
   LteChannelAccessManager::DoRequestAccess ();
   SetLbtState (TXOP_GRANTED);
   m_grantRequested = false;

   // save only lates txops - the ones for which we are expecting harq feedback, txops that started at after Simulator::Now() - m_harqFeedbackDelay
   std::vector<Time>::iterator it = m_txopHistory.begin();
   while (it != m_txopHistory.end())
      {
        if (Simulator::Now() - m_harqFeedbackExpirationTime > *it)
          {
            m_txopHistory.erase(it);
          }
        else
          {
            break; // there are no more elements to delete
          }
      }
   m_txopHistory.push_back(GetLastTxopStartTime());
}

void
LbtAccessManager::UpdateCwBasedOnHarq (std::vector<DlInfoListElement_s> m_dlInfoListReceived)
{
  NS_LOG_FUNCTION (this);
  uint32_t nackCounter = 0;
  bool considerThisHarqFeedback = false;
  NS_LOG_INFO("Update cw at:"<<Simulator::Now().GetMilliSeconds()<<"ms. Txop history size:"<<m_txopHistory.size());

  // Check if this harq feedback should be processed. Rule adopted here is to update CW based on first available harq feedback for the last packet burst. Other feedbacks of the same burst ignore.
  std::vector<Time>::iterator it = m_txopHistory.begin();
  while (it!=m_txopHistory.end())
    {
      Time oldestTxop = *it;
      // check if harq feedback belongs to oldest txop
      NS_LOG_INFO ("The oldestTxop before check: " << oldestTxop);
      if (Simulator::Now() - m_harqFeedbackDelay  >= oldestTxop)
        {
          considerThisHarqFeedback = true;
          m_lastConsideredTxopStartTime = *it;
          m_txopHistory.erase(it); // ignore other feedbacks for this txop
          NS_LOG_INFO("The feedback maybe belongs to the oldest txop. Delete the oldest and check the next oldest. ");
        }
      else
        { // if we didn't find txop in the list to which corresponds this harq feedback then ignore it
          if (!considerThisHarqFeedback)
            {
              considerThisHarqFeedback = false;
              NS_LOG_INFO ("Ignore this feedback, this harq belongs to the last considered txop:" << m_lastConsideredTxopStartTime << ", next txop for harq will be:" << oldestTxop);
            }
         else
           {
             NS_LOG_INFO ("Use this feedback. The feedback belongs to txop that starts at:"<<m_lastConsideredTxopStartTime <<" We erased it in order to ignore other feedbacks for this txop.");
           }
         break;
        }
    }

  if (!considerThisHarqFeedback)
    {
      NS_LOG_INFO ("Feedback ignored at:"<<Simulator::Now());
      return;
    }
  else
    {
      NS_LOG_INFO ("Feedback considered at:" << Simulator::Now() << " corresponds to transmission opportunity that starts at:" << m_lastConsideredTxopStartTime);
    }

  // update harq feedback trace source
  std::vector<uint8_t> harqFeedback;
  for (uint16_t i = 0; i < m_dlInfoListReceived.size(); i++)
    {
      for (uint8_t layer = 0; layer < m_dlInfoListReceived.at(i).m_harqStatus.size (); layer++)
        {
          if (m_dlInfoListReceived.at(i).m_harqStatus.at(layer) == DlInfoListElement_s::ACK)
            {
              NS_LOG_INFO ("Harq feedback contains an ACK");
              harqFeedback.push_back(0);
            }
          else if (m_dlInfoListReceived.at(i).m_harqStatus.at(layer) == DlInfoListElement_s::NACK)
            {
              NS_LOG_INFO ("Harq feedback contains a NACK");
              harqFeedback.push_back(1);
              nackCounter++;
            }
        }
    }

  if (harqFeedback.size () == 0)
    {
      NS_LOG_INFO ("Harq feedback empty at:"<<Simulator::Now());
      return;
    }

  bool updateFailedCw = false;

  switch (m_cwUpdateRule)
        {
         case ALL_NACKS:
          {
            if (double(nackCounter)/(double)harqFeedback.size() == 1)
              {
                updateFailedCw = true;
              }
          }
          break;
        case ANY_NACK:
          {
            if (nackCounter > 0)
              {
                updateFailedCw  = true;
              }
          }
          break;
        case NACKS_80_PERCENT:
          {
            if (double(nackCounter)/(double)harqFeedback.size() >= 0.8)
              {
                updateFailedCw = true;
                NS_LOG_INFO ("CW will be updated, since NACKs are greater than 80%");
              }
           else
             {
                NS_LOG_INFO ("CW will not be updated, since NACKs are less than 80%");
             }
          }
          break;
        case NACKS_10_PERCENT:
          {
            if (double(nackCounter)/(double)harqFeedback.size() >= 0.1)
              {
                updateFailedCw = true;
              }
          }
          break;
        default:
          {
            NS_FATAL_ERROR ("Unreachable?");
            return;
          }
        }

  if (updateFailedCw)
    {
      UpdateFailedCw ();
    }
  else
    {
      ResetCw ();
    }
}

uint32_t
LbtAccessManager::GetCurrentBackoffCount (void) const
{
  NS_LOG_FUNCTION (this);
  return m_backoffCount;
}

} // ns3 namespace

