/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2017 University of Washington
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
 * Author: Rohan Patidar <rpatidar@uw.edu>
 */

#include <ns3/wifi-spectrum-value-helper.h>
#include <ns3/random-variable-stream.h>
#include "EESM.h"
#include <ns3/log.h>
#include <ns3/assert.h>
#include <ns3/double.h>
#include <cmath> // for M_PI
#include <iomanip> // for M_PI
#include <random>
#include <fstream>
#include <list>
#include <vector>
#include "ns3/log.h"
#include "ns3/boolean.h"
#include "ns3/string.h"

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (EESM);

NS_LOG_COMPONENT_DEFINE ("EESM");


EESM::EESM ()
{
}

EESM::~EESM ()
{
}


TypeId
EESM::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::EESM")
    .SetParent<Object> ()
    .SetGroupName ("Wifi")
    .AddConstructor<EESM> ()
  ;
  return tid;
}


void
EESM::LoadBetaEesm(uint32_t channelWidth, std::map<uint8_t,double> &m_mcsBeta, std::string fastfading)
{
 
 std::string line;
 //int lineCount = -1;
 std::ifstream infile("src/wifi/model/error-rate-model-data/"+fastfading+"-Beta.txt");
 uint32_t scanbw;
 int scanflag=0;
 while (std::getline (infile, line)) // sequentially reading each line
    {
      if (line[0] == '#')
        {
          if (scanflag == 1){  // it has already scanned for required bw
                break;           
            }
          std::istringstream iss (line.substr (1)); // scan the channelbw
          iss >> scanbw; 

          if (scanbw == channelWidth) // start scanning
            {
              scanflag=1;  
            }
        }

      else if (scanflag==1)      
        { 
          std::istringstream iss (line);
          double x;
          double y;
          while (iss >> x >> y)
            {
              m_mcsBeta[(uint32_t)x] = y;
            }
        }
    }

/*   std::map<uint8_t,double>::iterator pos = m_mcsBeta.begin();
  while(pos!=m_mcsBeta.end())
{ std::cout << pos->first << " " << pos->second << std::endl;
  pos++;
}*/
}


double EESM::GetEffectivePower (Ptr<const SpectrumValue> input, uint32_t channelWidth,uint32_t GuardBW , uint8_t McsValue,std::string fastfading,WifiPhyStandard Standard)
{
  //std::cout << channeltype <<"life saved"<<std::endl;
  uint32_t num_subcarriers;
  uint32_t fftsize = 64;
  Ptr<SpectrumValue> temp = Copy<SpectrumValue> (input);
  double snr_eff;
  double itr_v=0, snr_min=1e10;
  double BOLTZMANN=1.3803e-23;
  double bandBandwidth=312500;
  double nv = BOLTZMANN * 290.0*bandBandwidth;  // 312.5 KHz width
  double output_power;  
  std::map<uint8_t,double> m_mcsBeta;
  double beta=-1;
  uint32_t nGuardBands = static_cast<uint32_t>(((2 * GuardBW * 1e6) / bandBandwidth) + 0.5);

  uint32_t start1 = (nGuardBands / 2) + 4;
  uint32_t stop1 = start1 + 28 - 1;
  uint32_t start2 = stop1 + 2;
  uint32_t stop2 = start2 + 28 - 1;
  //40MHz
  uint32_t start3 = (nGuardBands / 2) + 6;
  uint32_t stop3 = start3 + 57 - 1;
  uint32_t start4 = stop3 + 4;
  uint32_t stop4 = start4 + 57 - 1;
  
  //80MHz
  uint32_t start5 = (nGuardBands / 2) + 6;
  uint32_t stop5 = start5 + 121 - 1;
  uint32_t start6 = stop5 + 4;
  uint32_t stop6 = start6 + 121 - 1;
  
  //160MHz 80+80
  uint32_t start7 = stop6 + 2;
  uint32_t stop7 = start7 + 121 - 1;
  uint32_t start8 = stop7 + 4;
  uint32_t stop8 = start8 + 121 - 1;


  LoadBetaEesm(channelWidth ,m_mcsBeta,fastfading); // load specific Beta for a channel type (m_channeltype)
  std::map<uint8_t,double>::iterator pos = m_mcsBeta.find(McsValue);
  if (pos == m_mcsBeta.end()) {
      NS_ASSERT ("Bad Value");
    }else {
        beta = (double)pos->second;
    }


  Values::iterator psdit = temp->ValuesBegin ();

  switch (Standard)
    {
      case WIFI_PHY_STANDARD_80211a:
      case WIFI_PHY_STANDARD_80211g:
        num_subcarriers=52;     // 48 + 4 pilots 
        for (size_t i = 0; i < temp->GetSpectrumModel ()->GetNumBands (); i++, psdit++)
        {
          if ((i >=38 && i <=63) || (i >=65 && i <=90))
          {
            snr_min = std::min(snr_min,*psdit*bandBandwidth/nv);  
            itr_v=itr_v + exp(-(*psdit*bandBandwidth)/beta/nv);
          }
        }
        break;
  
      case WIFI_PHY_STANDARD_80211_10MHZ:
        std::cout << "bandwidth" << 10 << std::endl;
        num_subcarriers=28;    // 24 + 4 pilots
        fftsize = 32;          //
        for (size_t i = 0; i < temp->GetSpectrumModel ()->GetNumBands (); i++, psdit++)
        {
        if ((i >=34 && i <=47) || (i >=49 && i <=62))
         {
           snr_min = std::min(snr_min,*psdit*bandBandwidth/nv);
           itr_v=itr_v + exp(-(*psdit*bandBandwidth)/beta/nv);
         }
        }
        break;
    
      case WIFI_PHY_STANDARD_80211_5MHZ:
        std::cout << "bandwidth" << 5 << std::endl;
        num_subcarriers=16;    // 12  + 4 
        fftsize = 16;        // null carriers ??
        for (size_t i = 0; i < temp->GetSpectrumModel ()->GetNumBands (); i++, psdit++)
        {
        if ((i >=32 && i <=39) || (i >=41 && i <=48))
          {
            snr_min = std::min(snr_min,*psdit*bandBandwidth/nv); 
            itr_v=itr_v + exp(-(*psdit*bandBandwidth)/beta/nv);
          }
        }
        break;      
    //case WIFI_PHY_STANDARD_80211b:
      //break;
    //case WIFI_PHY_STANDARD_holland:
      //num_subcarriers=?;
      //break;
      
      case WIFI_PHY_STANDARD_80211n_2_4GHZ:
      case WIFI_PHY_STANDARD_80211n_5GHZ:  
      case WIFI_PHY_STANDARD_80211ac:
        switch (channelWidth)
          {
            case 20:
             // std::cout << "bandwidth =>" << 20 << std::endl;
              num_subcarriers=56;   // 52  + 4  pilots
              for (size_t i = 0; i < temp->GetSpectrumModel ()->GetNumBands (); i++, psdit++)
              {
                //if ((i >=36 && i <=63) || (i >=65 && i <=92))
                if ((i >= start1 && i <= stop1) || (i >= start2 && i <= stop2))
                {
                  snr_min = std::min(snr_min,*psdit*bandBandwidth/nv);
                  itr_v=itr_v + exp(-(*psdit*bandBandwidth)/beta/nv);   
                }
              } 
              break;
          
            case 40:
              std::cout << "bandwidth=>" << 40 << std::endl;
              num_subcarriers=114;  // 108 + 6  pilots
              fftsize=128;            
              for (size_t i = 0; i < temp->GetSpectrumModel ()->GetNumBands (); i++, psdit++)
              {
                if ((i >= start3 && i <= stop3) || (i >= start4 && i <= stop4))
                //if ((i >=38 && i <=94) || (i >=98 && i <=154) )
                {
                  snr_min = std::min(snr_min,*psdit*bandBandwidth/nv);
                  itr_v=itr_v + exp(-(*psdit*bandBandwidth)/beta/nv);
                }
              }  
              break;
          
            case 80:
              std::cout << "bandwidth" << 80 << std::endl;
              num_subcarriers=242;  // 234 + 8  pilots
              fftsize=256;            
              for (size_t i = 0; i < temp->GetSpectrumModel ()->GetNumBands (); i++, psdit++)
              {
                if ((i >= start5 && i <= stop5) || (i >= start6 && i <= stop6)) 
                //if ((i >=38 && i <=158) || (i >=162 && i <= 282) )
                {
                  snr_min = std::min(snr_min,*psdit*bandBandwidth/nv);
                  itr_v=itr_v + exp(-(*psdit*bandBandwidth)/beta/nv);
                }
              }
              break;
          
            case 160:
              std::cout << "bandwidth" << 160 << std::endl;
              // this is 80 + 80
              num_subcarriers=484;  // 468 + 16 pilots
              fftsize=512;            
              for (size_t i = 0; i < temp->GetSpectrumModel ()->GetNumBands (); i++, psdit++)
              {
                if ((i >= start5 && i <= stop5) || (i >= start6 && i <= stop6) ||
              (i >= start7 && i <= stop7) || (i >= start8 && i <= stop8))
                //if ((i >=38 && i <=158)|| (i >=162 && i <=282) || (i >=294 && i <= 414) || (i >=418 && i <= 538) )
                {
                  snr_min = std::min(snr_min,*psdit*bandBandwidth/nv);
                  itr_v=itr_v + exp(-(*psdit*bandBandwidth)/beta/nv);
                }
              }
              break;
          }  
      break;
      
  

     /* case WIFI_PHY_STANDARD_80211ax_5GHZ:
        // SU all RUs allocated to a User
        switch (channelWidth)
          {
            case 20:
              std::cout << "bandwidth => " << 20 << std::endl;
              num_subcarriers=234;   // Only Pilots  pilots
              fftsize=256;
              for (size_t i = 0; i < temp->GetSpectrumModel ()->GetNumBands (); i++, psdit++)
              {
                if ((i >=262 && i <=382) || (i >=386 && i <=506))
                {
                  itr_v=itr_v + exp(-(*psdit*bandBandwidth)/beta/nv);
                }
              } 
              break;
          
            case 40:
              std::cout << "bandwidth => " << 40 << std::endl;
              num_subcarriers=484;  // 468 + 16  pilots
              fftsize=512;            
              for (size_t i = 0; i < temp->GetSpectrumModel ()->GetNumBands (); i++, psdit++)
              {
                if ((i >=268 && i <=509) || (i >=515 && i <=756) )
                {
                  itr_v=itr_v + exp(-(*psdit*bandBandwidth)/beta/nv);
                }
              }  
              break;
          
            case 80:
              std::cout << "bandwidth => " << 80 << std::endl;
              num_subcarriers=996;  // 980 + 16  pilots
              fftsize=1024;            
              for (size_t i = 0; i < temp->GetSpectrumModel ()->GetNumBands (); i++, psdit++)
              {
                if ((i >=268 && i <=765) || (i >=771 && i <= 1268) )
                {
                  itr_v=itr_v + exp(-(*psdit*bandBandwidth)/beta/nv);
                }
              }
              break;
          
            case 160:
              std::cout << "bandwidth" << 160 << std::endl;
              // this is 80 + 80
              num_subcarriers=1992;  // 1960 + 32 pilots
              fftsize=2048;            
              for (size_t i = 0; i < temp->GetSpectrumModel ()->GetNumBands (); i++, psdit++)
              {
                if ((i >=268 && i <=765)|| (i >=771 && i <=1268) || (i >=1292 && i <= 1789) || (i >=1795 && i <= 2292) )
                {
                  itr_v=itr_v + exp(-(*psdit*bandBandwidth)/beta/nv);
                }
              }
              break;
          }  
        break;  // check this break


       
    // HE added in RBIR (once we have pilot/null carrier info from CISCO)    

    */
    default:
      NS_FATAL_ERROR ("Standard or Channelwidth not supported/unkown");
      break;
    }

    if (itr_v == 0)
     {
      snr_eff = snr_min;
     } else {
      snr_eff= -beta*((log(itr_v/num_subcarriers) )/log(exp(1))); 
     } 
  
  snr_eff=snr_eff*fftsize/num_subcarriers; // factor for null carriers
  NS_LOG_DEBUG ("Effective SNR value (linear) w/o cosidering noise figure of 7 dB " << snr_eff);
  output_power=snr_eff*nv*channelWidth*1e6/bandBandwidth;
  return output_power;
}


}
