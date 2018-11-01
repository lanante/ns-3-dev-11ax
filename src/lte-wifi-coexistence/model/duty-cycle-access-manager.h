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
#include "ns3/lte-channel-access-manager.h"

#ifndef DUTYCYCLEACCESSMANAGER_H_
#define DUTYCYCLEACCESSMANAGER_H_

namespace ns3 {

/*
 *  DutyCycleAccessManager relies on simulation time to determine ON and OFF periods.
 *  There are 3 parameters to configure: OnDuration, OnStartTime and DutyCyclePeriod.
 *  Simulation time is seen as series duty cycle periods. OnDuration defines duration
 *  of ON period duration during single duty cycle period. DutyCyclePeriod is total
 *  duration of ON and OFF period. OnStartTime defines at which time of DutyCyclePeriod
 *  is starting the ON period. E.g. by using these 3 parameters it could be configured
 *  that different duty cycle channel access managers have the same duty cycle period,
 *  but that ON periods do not overlap. The precision of this duty cycle is at nano seconds
 *  level.
 */
class DutyCycleAccessManager : public LteChannelAccessManager
{

public:
  static TypeId GetTypeId (void);
  DutyCycleAccessManager ();
  virtual ~DutyCycleAccessManager ();
  /*
   * Set duration of ON states.
   * \param onTime - duration
   */
  void SetOnDuration (Time onDuration);
  /*
   * Set when ON state starts inside of duty cycle period.
   * \param onStartTime - ON start time
   */
  void SetOnStartTime (Time onStartTime);

  /*
   * Set duration of duty cycle period
   * \param onTime - duration of duty cycle period
   */
  void SetDutyCyclePeriod (Time offTime);

  /*
   * Get duration of ON states.
   * \returns duration
   */
  Time GetOnDuration () const;
  /*
   * Get when ON state starts
   * \returns ON state start time
   */
  Time GetOnStartTime () const;

  /*
   * Get duration of duty cycle period
   * \returns duration
   */
  Time GetDutyCyclePeriod () const;

protected:
  virtual void DoInitialize ();
  virtual void DoRequestAccess ();
  void DelayedAccessGrantedCallback();
  void Wait(Time time);

  Time m_onDuration;
  Time m_onStartTime;
  Time m_dutyCyclePeriod;
  bool m_waitingForChannel;
};

}

#endif /* DUTYCYCLEACCESSMANAGER_H_ */
