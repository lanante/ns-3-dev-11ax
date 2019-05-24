/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2018 University of Washington
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
 * Author: Leonardo Lanante Jr <leonardolanante@gmail.com>
 */

#include "ns3/log.h"
#include "ns3/node.h"
#include "ns3/config.h"
#include "ns3/double.h"
#include "ns3/uinteger.h"
#include "dynamic-obss-pd-algorithm.h"
#include "sta-wifi-mac.h"
#include "wifi-utils.h"
#include "wifi-phy.h"
#include "wifi-net-device.h"
#include "he-configuration.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("DynamicObssPdAlgorithm");
NS_OBJECT_ENSURE_REGISTERED (DynamicObssPdAlgorithm);

DynamicObssPdAlgorithm::DynamicObssPdAlgorithm ()
  : ObssPdAlgorithm ()
{
  NS_LOG_FUNCTION (this);
}

TypeId
DynamicObssPdAlgorithm::GetTypeId (void)
{
  static ns3::TypeId tid = ns3::TypeId ("ns3::DynamicObssPdAlgorithm")
    .SetParent<ObssPdAlgorithm> ()
    .SetGroupName ("Wifi")
    .AddConstructor<DynamicObssPdAlgorithm> ()
  ;
  return tid;
}

void
DynamicObssPdAlgorithm::ConnectWifiNetDevice (const Ptr<WifiNetDevice> device)
{
  Ptr<WifiPhy> phy = device->GetPhy ();
  phy->TraceConnectWithoutContext ("EndOfHePreamble", MakeCallback (&DynamicObssPdAlgorithm::ReceiveHeSig, this));
  ObssPdAlgorithm::ConnectWifiNetDevice (device);
}

void
DynamicObssPdAlgorithm::ReceiveHeSig (HePreambleParameters params)
{
  NS_LOG_FUNCTION (this << +params.bssColor << WToDbm (params.rssiW));

  Ptr<StaWifiMac> mac = m_device->GetMac ()->GetObject<StaWifiMac>();
  if (mac && !mac->IsAssociated ())
    {
      NS_LOG_DEBUG ("This is not an associated STA: skip OBSS PD algorithm");
      return;
    }

  //NS_LOG_DEBUG ("RSSI=" << WToDbm (params.rssiW) << " dBm , BSS color=" << +params.bssColor);

  Ptr<HeConfiguration> heConfiguration = m_device->GetHeConfiguration ();
  Ptr<WifiPhy> phy = m_device->GetPhy ();
  NS_ASSERT (heConfiguration);
  UintegerValue bssColorAttribute;
  heConfiguration->GetAttribute ("BssColor", bssColorAttribute);
  uint8_t bssColor = bssColorAttribute.Get ();
uint32_t currentNodeId=   m_device->GetNode ()->GetId ();
uint8_t n=10;
bool isAP=(currentNodeId%(1+n))==0;
if (isAP)
      {NS_LOG_DEBUG ("I am AP ");}
  if (bssColor == 0)
    {
      NS_LOG_DEBUG ("BSS color is 0");
      return;
    }
  if (params.bssColor == 0)
    {
      NS_LOG_DEBUG ("Received BSS color is 0");
      return;
    }
      NS_LOG_DEBUG ("Received BSS color is "<<+params.bssColor);
  //TODO: SRP_AND_NON-SRG_OBSS-PD_PROHIBITED=1 => OBSS_PD SR is not allowed
	double r=m_r;
	double d=m_d;
	uint8_t mcs=m_Mcs;
  bool isObss = (bssColor != params.bssColor);
  if (isObss)
    {
	//------------------------------------------------------
	
	        double n=3.5;
	        double Pref_high=-30;
	    //    double obssSNRA[12]={0.7,3.7,6.2,9.3,12.6,16.8,18.2,19.4,23.5,25.1,28,31};
	        double obssSNRA[12]={0.7,3.7,6.2,9.3,12.6,16.8,18.2,19.4,23.5,25.1,28,31};
	        double Pn=-94;

  	        double  obssSNR=obssSNRA[mcs];
	        double APx[7]={0, 1, 0.5, -0.5, -1, -0.5, 0.5};
	        double APy[7]={0,0, 0.866, 0.866, 0, -0.866, -0.866};
	        double myx=0;
	        double myy=0;
	        double ox=0;
	        double oy=0;


	        if (bssColor>0||bssColor<8)
		        {myx=APx[bssColor-1];
	                myy=APy[bssColor-1];}
	        else{
		        std::cout <<"Unknown BSS color";
		        exit(1); // terminate with error
		        }

	        if (params.bssColor>0||params.bssColor<8)
	                {ox=APx[params.bssColor-1];
	                oy=APy[params.bssColor-1];}
	        else{
	                std::cout <<"Unknown BSS color";
	                exit(1); // terminate with error
	                }

	
	        double R=d*sqrt(pow(ox-myx,2)+pow(oy-myy,2));
	        double DI=sqrt(pow(R,2)+pow(r,2)/2);
		double expectedRSSI=Pref_high-35*log10(DI);
	        double Interference=(params.rssiW-DbmToW(expectedRSSI));
                if (Interference<DbmToW(-72))
{Interference=0;}
	        double Pn2=WToDbm(DbmToW(Pn)+Interference*1);

	NS_LOG_LOGIC ("expected RSSI is "<<expectedRSSI <<"Actual RSSI is "<<WToDbm(params.rssiW));
NS_LOG_DEBUG("Interference is "<<WToDbm(Interference));
	        double num= pow(10,(Pref_high-obssSNR)/10)*pow(DI,n);
	        double den= pow(10,Pref_high/10)+pow(10,Pn2/10)*pow(DI,n);
	        double S=pow(num/den,2/n)/pow(r,2)*2;


	        //NS_LOG_DEBUG ("my BSS color "<<+bssColor);
	        //NS_LOG_DEBUG ("OBSS color "<<+params.bssColor);
	        //NS_LOG_DEBUG ("MY AP POs "<<myx<<" "<<myy);
	        //NS_LOG_DEBUG ("OBSS AP POs "<<ox<<" "<<oy);
	        //NS_LOG_DEBUG ("nBss is "<<+nBss);
	        //NS_LOG_DEBUG ("d is "<<d);
	        NS_LOG_DEBUG ("num is "<<num);
	        NS_LOG_DEBUG ("den is "<<den);
	        NS_LOG_DEBUG ("DI is "<<DI);
                //NS_LOG_DEBUG ("R is "<<R);

		NS_LOG_DEBUG ("Noise is "<<Pn <<" Noise Interference  is "<<Pn2);

		m_obssPdLevel=WToDbm(params.rssiW)+10*log10(S);
	//------------------------------------------------------
 
      if (S> 1&&WToDbm(Interference)<-72)
        {
          NS_LOG_LOGIC ("Frame is OBSS and RSSI " << WToDbm(params.rssiW) << " is below OBSS-PD level of " << m_obssPdLevel << "; reset PHY to IDLE");
          m_obssPdLevelMin=m_obssPdLevel;
   phy->SetCcaEdThreshold (m_obssPdLevel);
          ResetPhy (params);
        }
      else
        {
    NS_LOG_LOGIC ("Frame is OBSS and RSSI " << WToDbm(params.rssiW) << " is above OBSS-PD level of " << m_obssPdLevel << "; reset PHY to IDLE");
		NS_LOG_LOGIC ("S is "<<S);
 //         NS_LOG_LOGIC("Frame is OBSS and RSSI is above OBSS-PD level");
    phy->SetCcaEdThreshold (-82);
        }
    }
}

void
DynamicObssPdAlgorithm::ReceiveBeacon (HeBeaconReceptionParameters params)
{
  NS_LOG_FUNCTION (this);
}

} //namespace ns3
