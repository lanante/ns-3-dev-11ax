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

#include <fstream>
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/applications-module.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("VoiceExample");

int
main (int argc, char *argv[])
{

  bool verbose = false;

  CommandLine cmd;
  cmd.AddValue ("verbose", "Enable logging in debug build if true", verbose);
  cmd.Parse (argc, argv);

  if (verbose)
    {
      LogComponentEnable ("VoiceApplication", LOG_LEVEL_ALL);
    }

  ns3::PacketMetadata::Enable();

  NodeContainer n;
  n.Create (2);

  PacketSocketHelper packetSocket;
  packetSocket.Install (n);

  Ptr<SimpleNetDevice> device0;
  device0 = CreateObject<SimpleNetDevice> ();
  n.Get (0)->AddDevice (device0);

  Ptr<SimpleNetDevice> device1;
  device1 = CreateObject<SimpleNetDevice> ();
  n.Get (1)->AddDevice (device1);

  Ptr<SimpleChannel> channel = CreateObject<SimpleChannel> ();
  channel->SetAttribute ("Delay", TimeValue (MilliSeconds (10)));
  device0->SetChannel (channel);
  device1->SetChannel (channel);
  device0->SetNode (n.Get (0));
  device1->SetNode (n.Get (1));

  // Devices should be able to pass data bidirectionally
  PacketSocketAddress socketAddr0;  // goes on application 0
  socketAddr0.SetSingleDevice (device0->GetIfIndex ());
  socketAddr0.SetPhysicalAddress (device1->GetAddress ());

  PacketSocketAddress socketAddr1;  // goes on application 1
  socketAddr1.SetSingleDevice (device1->GetIfIndex ());
  socketAddr1.SetPhysicalAddress (device0->GetAddress ());

  Ptr<VoiceApplication> voice0 = CreateObject<VoiceApplication> ();
  voice0->SetStartTime (Seconds (1));
  voice0->SetStopTime (Seconds (11.01));
  n.Get (0)->AddApplication (voice0);
  voice0->SetAttribute ("Remote", AddressValue (socketAddr0));

  Ptr<VoiceApplication> voice1 = CreateObject<VoiceApplication> ();
  voice1->SetStartTime (Seconds (1));
  voice1->SetStopTime (Seconds (12));
  n.Get (1)->AddApplication (voice1);
  voice1->SetAttribute ("Remote", AddressValue (socketAddr1));
  voice1->SetAttribute ("LatencyThreshold", TimeValue (MilliSeconds (10)));
  // Disable sending packets from voice1 to voice0
  voice1->SetAttribute ("SendEnabled", BooleanValue (false));

  std::cout << "Sent " << voice0->GetNumSent () << std::endl;
  std::cout << "Received " << voice1->GetNumReceived () << std::endl;
  std::cout << "Lost " << voice1->GetNumLost () << std::endl;
  std::cout << "Delayed " << voice1->GetNumDelayed () << std::endl;

  Simulator::Run ();

  std::cout << "Sent " << voice0->GetNumSent () << std::endl;
  std::cout << "Received " << voice1->GetNumReceived () << std::endl;
  std::cout << "Lost " << voice1->GetNumLost () << std::endl;
  std::cout << "Delayed " << voice1->GetNumDelayed () << std::endl;
  Simulator::Destroy ();
}
