/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
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
 */

// Network topology
//
//       n0    n1
//       |     |
//       =======
//        Simple
//        channel
//
// - Voice flows from n0 to n1

#include "ns3/voice-application.h"
#include "ns3/test.h"
#include "ns3/simulator.h"

using namespace ns3;

class VoiceApplicationStartStop : public TestCase
{
public:
  VoiceApplicationStartStop ();
  virtual ~VoiceApplicationStartStop ();

private:
  virtual void DoRun (void);
};

VoiceApplicationStartStop::VoiceApplicationStartStop ()
  : TestCase ("Start and stop behavior")
{
}

VoiceApplicationStartStop::~VoiceApplicationStartStop ()
{
}

void
VoiceApplicationStartStop::DoRun (void)
{
  Ptr<VoiceApplication> voice = CreateObject<VoiceApplication> ();
  // Values before starting
  voice->SetStartTime (Seconds (1));
  Simulator::Run ();
  // Values after starting

#if 0
  voice->StopApplication ();
  // Values after stopping

  // Start it again
  voice->StartApplication ();
  voice->StopApplication ();
#endif


  NS_TEST_ASSERT_MSG_EQ (true, true, "true doesn't equal true for some reason");
  NS_TEST_ASSERT_MSG_EQ_TOL (0.01, 0.01, 0.001, "Numbers are not equal within tolerance");
}

class VoiceApplicationTestSuite : public TestSuite
{
public:
  VoiceApplicationTestSuite ();
};

VoiceApplicationTestSuite::VoiceApplicationTestSuite ()
  : TestSuite ("voice-application", UNIT)
{
  AddTestCase (new VoiceApplicationStartStop, TestCase::QUICK);
}

static VoiceApplicationTestSuite voiceApplicationTestSuite;

