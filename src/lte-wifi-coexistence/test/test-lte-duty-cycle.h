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
 *  Author: Biljana Bojovic <bbojovic@cttc.es>
 */

#ifndef TEST_LTE_DUTY_CYCLE_H_
#define TEST_LTE_DUTY_CYCLE_H_

#include "ns3/test.h"
using namespace ns3;

/**
 * Test that the LTE transmission works according to ON/OFF duty cycle pattern.
 * We configure duty cycle access manager parameters of each cell so that either
 * they never transmit at the same time or they transmit at the same time. For each
 * test case is checked level of interference. E.g. in the first case it is expected
 * that even if cells are placed near, they should not experience any interference,
 * the modulation and coding scheme (MCS) is expected to be 28. Additionally, if any
 * PHY or MAC layer activity is detected during its OFF period the test will
 * fail. Test starts to check test conditions after 330 ms, this is to allow for RRC
 * connection establishment, SRS, and CQI update.
 */
class LteDutyCycleTestSuite : public TestSuite
{
public:
  LteDutyCycleTestSuite ();
};

class LteDutyCycleTestCase : public TestCase
{
public:
  /*
   * \param onTime - cell1 onTime parameter, this is offTime for cell2
   * \param offTime - cell1 offTime parameter, this is onTime for cell2
   */
  LteDutyCycleTestCase (std::string name, double d1, double d2, int64_t dutyCycleTime, int64_t cell1OnTime, uint64_t cell1OnStartTime, int64_t cell2OnTime, uint64_t cell2OnStartTime,
                        double dlSinr, double ulSinr, uint16_t dlMcs, uint16_t ulMcs);

  virtual ~LteDutyCycleTestCase ();

  void DelayedDutyCycleStart (NetDeviceContainer enbDevs);

  std::string BuildNameString (std::string name, int64_t dutyCycleTime, int64_t cell1OnStartTime, int64_t cell1OnTime, int64_t cell2OnStartTime, int64_t cell2OnTime);

  void DlScheduling1 (uint32_t frameNo, uint32_t subframeNo, uint16_t rnti,
                      uint8_t mcsTb1, uint16_t sizeTb1, uint8_t mcsTb2, uint16_t sizeTb2);

  void DlScheduling2 (uint32_t frameNo, uint32_t subframeNo, uint16_t rnti,
                      uint8_t mcsTb1, uint16_t sizeTb1, uint8_t mcsTb2, uint16_t sizeTb2);

  void UlScheduling1 (uint32_t frameNo, uint32_t subframeNo, uint16_t rnti,
                      uint8_t mcs, uint16_t sizeTb);

  void UlScheduling2 (uint32_t frameNo, uint32_t subframeNo, uint16_t rnti,
                      uint8_t mcs, uint16_t sizeTb);

private:
  virtual void DoRun (void);

  double m_d1;
  double m_d2;

  int64_t m_dutyCycleTime;
  int64_t m_cell1onTime;
  uint64_t m_cell1onStartTime;
  int64_t m_cell2onTime;
  uint64_t m_cell2onStartTime;
  double m_expectedDlSinrDb;
  double m_expectedUlSinrDb;
  uint16_t m_dlMcs;
  uint16_t m_ulMcs;

  Time m_testCheckStartTime;
};

#endif /* TEST_LTE_DUTY_CYCLE_H_ */
