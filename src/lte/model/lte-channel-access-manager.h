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
#include <ns3/vector.h>

#ifndef LTE_CHANNEL_ACCESS_MANAGER_H_
#define LTE_CHANNEL_ACCESS_MANAGER_H_

namespace ns3 {

typedef Callback<void, ns3::Time> AccessGrantedCallback;

/**
 * \ingroup lte
 *
 * \brief Parent class of objects that control LTE access to the channel.
 *
 * This class can be used to control access to the channel for situations
 * in which LTE must share use with other nodes or technologies contending
 * for use of the channel.  The main interaction defined between the
 * client and this class is the RequestAccess () request and the callback
 * invoked (SetAccessGrantedCallback) when access is granted.
 */
class LteChannelAccessManager : public Object
{

public:
  static TypeId GetTypeId (void);
  LteChannelAccessManager ();
  virtual ~LteChannelAccessManager ();
  /**
   * \brief Request access (access will be later granted via the callback)
   */
  void RequestAccess ();
  /**
   * \brief Set callback to grant access.
   */
  void SetAccessGrantedCallback (AccessGrantedCallback accessGrantedCallback);

  /*
   * Set duration of grant for transmission.
   * \param grantDuration duration of grant
   */
  void SetGrantDuration (Time grantDuration);

  /*
   * Get grant duration time.
   * \returns default grant duration
   */
  Time GetGrantDuration () const;

  Time GetLastTxopStartTime () const;

protected:
  virtual void DoInitialize ();
  virtual void DoRequestAccess ();
  AccessGrantedCallback m_accessGrantedCallback;
  Time m_grantDuration;
  double m_edThreshold;

private:
  Time m_lastTxopStartTime;

};

}

#endif /* LTE_CHANNEL_ACCESS_MANAGER_H_ */
