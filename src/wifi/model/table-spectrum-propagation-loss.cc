/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2009 CTTC
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
 * Author: Nicola Baldo <nbaldo@cttc.es>
 */

//#include <ns3/mobility-model.h>
#include <ns3/wifi-spectrum-value-helper.h>
#include <ns3/random-variable-stream.h>
#include "table-spectrum-propagation-loss.h"
#include <ns3/log.h>
#include <ns3/assert.h>
#include <ns3/double.h>
#include <cmath> // for M_PI
#include <iomanip> // for M_PI
#include <complex> // for M_PI
#include <random>
#include <string>
#include <fstream>
#include <list>
#include <vector>


namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (TableSpectrumPropagationLossModel);

NS_LOG_COMPONENT_DEFINE ("TableSpectrumPropagationLossModel");


TableSpectrumPropagationLossModel::TableSpectrumPropagationLossModel ()
{
}

TableSpectrumPropagationLossModel::~TableSpectrumPropagationLossModel ()
{
}


TypeId
TableSpectrumPropagationLossModel::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::TableSpectrumPropagationLossModel")
    .SetParent<Object> ()
    .SetGroupName ("Wifi")
    .AddConstructor<TableSpectrumPropagationLossModel> ()
  ;
  return tid;
}



Ptr<SpectrumValue> 
TableSpectrumPropagationLossModel::CalRxSpectralDensity (Ptr<const SpectrumValue> txPsd,WifiModulationClass ModulationClass,std::string channeltype, double BandBW, uint16_t CenterFrequency, uint8_t ChannelWidth, uint32_t GuardBW) const

{
  NS_LOG_FUNCTION (this);
  Ptr<SpectrumValue> rxPsd = Copy<SpectrumValue> (txPsd);

  uint32_t nGuardBands = static_cast<uint32_t>(((2*GuardBW * 1e6) / BandBW) + 0.5);
  Ptr<SpectrumValue> channelFreqResponse = GetChannelResponse (ModulationClass,CenterFrequency,ChannelWidth,1,GuardBW,channeltype,nGuardBands);
  Values::iterator chanit = channelFreqResponse->ValuesBegin ();
  Values::iterator psdit = rxPsd->ValuesBegin ();
 
  if (ModulationClass == WIFI_MOD_CLASS_HE) {

       uint32_t start1;
       uint32_t stop1;
       uint32_t start2;
       uint32_t stop2;
       uint32_t start3;
       uint32_t stop3;
       uint32_t start4;
       uint32_t stop4;
       uint32_t nRUs = 242;
       uint32_t nPilots_2;
  
       //note: moving all pilots in the center for convinience
       switch (ChannelWidth)
          {
           case 20:
                //18 or 8 pilots 
                if (nRUs == 26|| nRUs == 52)
                 {
                  nPilots_2 = 9;
                 }
                else if (nRUs == 106|| nRUs == 242)
                 {
                  nPilots_2 = 4;
                 }else{
                  NS_FATAL_ERROR ("20 MHz only supports: 26, 52, 106 or 242 RUs\n");       
                 }
           
                start1 = (nGuardBands / 2) + 12;
                stop1 = start1 + 121 - 1- nPilots_2;
                start2 = stop1 + nPilots_2 + 4 + nPilots_2; 
                stop2 = start2 + 121 - 1;
                //std::cout << "start :" << start1 <<std::endl;
                for (size_t i = 0; i < channelFreqResponse->GetSpectrumModel ()->GetNumBands (); i++, chanit++, psdit++)
                 {    
                  if ((i >=start1 && i <=stop1) || (i >=start2 && i <=stop2))
                        {   
                          *psdit *= *chanit;
                        }
                  else
                        {
                          *psdit = 0;
                        }
                 }
                break;

           case 40:
                //36 or 16 pilots
                if (nRUs == 26|| nRUs == 52)
                 {
                  nPilots_2 = 18;
                 }
                else if (nRUs == 106|| nRUs == 242|| nRUs == 484)
                 {
                  nPilots_2 = 8;
                 }else{
                  NS_FATAL_ERROR ("40 MHz only supports: 26, 52, 106, 242 or 484 RUs\n");       
                 }
                start1 = (nGuardBands / 2) + 12;
                stop1 = start1 + 242 - 1-nPilots_2;
                start2 = stop1 + nPilots_2 + 6 + nPilots_2;
                stop2 = start2 + 242 - 1;
                //std::cout << "start :" << start1 <<std::endl;
                for (size_t i = 0; i < channelFreqResponse->GetSpectrumModel ()->GetNumBands (); i++, chanit++, psdit++)
                 {
                  if ((i >=start1 && i <=stop1) || (i >=start2 && i <=stop2))
                        {   
                          *psdit *= *chanit;
                        }
                  else
                        {
                          *psdit = 0;
                        }
                 }
                break;


           case 80:
                //74 or 32 or 16 pilots
                if (nRUs == 26|| nRUs == 52)
                 {
                  nPilots_2 = 37;
                 }
                else if (nRUs == 106|| nRUs == 242|| nRUs == 484)
                 {
                  nPilots_2 = 16;
                 }
                else if (nRUs == 996)
                {
                 nPilots_2 = 8;
                }                
                else{
                  NS_FATAL_ERROR ("80 MHz only supports: 26, 52, 106, 242, 484 or 996 RUs\n");       
                 }
                start1 = (nGuardBands / 2) + 12;
                stop1 = start1 + 498 - 1 -nPilots_2;
                start2 = stop1 + nPilots_2 + 6 + nPilots_2;
                stop2 = start2 + 498 - 1;
                //std::cout << "start :" << start1 <<std::endl;
                for (size_t i = 0; i < channelFreqResponse->GetSpectrumModel ()->GetNumBands (); i++, chanit++, psdit++)
                 {
                  if ((i >=start1 && i <=stop1) || (i >=start2 && i <=stop2))
                        {   
                          *psdit *= *chanit;
                        }
                  else
                        {
                          *psdit = 0;
                        }
                 }
                break;

           case 160:
                // 74*2 or 16*2
                 if (nRUs == 26|| nRUs == 52 || nRUs == 106|| nRUs == 242|| nRUs == 484)
                 {
                  nPilots_2 = 74;
                 }
                else if (nRUs == 996)
                {
                 nPilots_2 = 16;
                }                
                else{
                  NS_FATAL_ERROR ("80 MHz only supports: 26, 52, 106, 242, 484 or 996 RUs\n");       
                 }

                start1 = (nGuardBands / 2) + 12;
                stop1 = start1 + 498 - 1-nPilots_2/2;
                start2 = stop1 + nPilots_2/2 + 6 + nPilots_2/2;
                stop2 = start2 + 498 - 1;
                start3 = stop2 + (2 * 12);
                stop3 = start3 + 498 - 1-nPilots_2/2;
                start4 = stop3 + nPilots_2/2 + 6 + nPilots_2/2;
                stop4 = start4 + 498 - 1;
                //std::cout << "start :" << start1 <<std::endl;
                for (size_t i = 0; i < channelFreqResponse->GetSpectrumModel ()->GetNumBands (); i++, chanit++, psdit++)
                 {
                  if ((i >= start1 && i <= stop1) || (i >= start2 && i <= stop2) ||
                      (i >= start3 && i <= stop3) || (i >= start4 && i <= stop4))
                        {   
                          *psdit *= *chanit;
                        }
                  else
                        {
                          *psdit = 0;
                        }
                 }
                break;
          } 


    }
  else {

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

          switch (ChannelWidth)
            {
            case 20:
              // 56 subcarriers (52 data + 4 pilot)
              //std::cout << "start :" << start1 <<std::endl;
              for (size_t i = 0; i < channelFreqResponse->GetSpectrumModel ()->GetNumBands (); i++, chanit++, psdit++)
              {
                // Assume 20 MHz 802.11n
                if ((i >=start1 && i <=stop1) || (i >=start2 && i <=stop2))
                        {   
                          *psdit *= *chanit;
                        }
                else
                        {
                          *psdit = 0;
                        }
               }

              NS_LOG_DEBUG ("Added signal power to subbands " << start1 << "-" << stop1 <<
                            " and " << start2 << "-" << stop2);
              break;
            case 40:
              // 112 subcarriers (104 data + 8 pilot) 
              //std::cout << "start :" << start3 <<std::endl;
              for (size_t i = 0; i < channelFreqResponse->GetSpectrumModel ()->GetNumBands (); i++, chanit++, psdit++)
              {
                // Assume 20 MHz 802.11n
                if ((i >=start3 && i <=stop3) || (i >=start4 && i <=stop4))
                        {   
                          *psdit *= *chanit;
                        }
                else
                        {
                          *psdit = 0;
                        }
               }
              break;
            case 80:
              // 242 subcarriers (234 data + 8 pilot)
              //std::cout << "start :" << start1 <<std::endl;
              for (size_t i = 0; i < channelFreqResponse->GetSpectrumModel ()->GetNumBands (); i++, chanit++, psdit++)
              {
                // Assume 20 MHz 802.11n
                if ((i >=start5 && i <=stop5) || (i >=start6 && i <=stop6))
                        {   
                          *psdit *= *chanit;
                        }
                else
                        {
                          *psdit = 0;
                        }
               }
              break;
            case 160:
              // 484 subcarriers (468 data + 16 pilot)
              //std::cout << "start :" << start1 <<std::endl;
              for (size_t i = 0; i < channelFreqResponse->GetSpectrumModel ()->GetNumBands (); i++, chanit++, psdit++)
              {
                // Assume 20 MHz 802.11n
                if ((i >= start5 && i <= stop5) || (i >= start6 && i <= stop6) ||
                      (i >= start7 && i <= stop7) || (i >= start8 && i <= stop8))
                        {   
                          *psdit *= *chanit;
                        }
                else
                        {
                          *psdit = 0;
                        }
               }
              break;
            }

  }     
  NS_LOG_DEBUG ("Post 1 " << *rxPsd);
  return rxPsd;
}

Ptr<SpectrumValue>
TableSpectrumPropagationLossModel::GetChannelResponse (WifiModulationClass ModulationClass, uint16_t CenterFrequency, uint8_t ChannelWidth, double Power, uint32_t GuardBW, std::string channeltype, uint32_t nGuardBands) const
{
  // same energy distribution of HT and VHT for now
  Ptr<SpectrumValue> channelFreqResponse;
  std::stringstream ss;
  if (ModulationClass == WIFI_MOD_CLASS_HE) // Check for HE, test the loop
    {
      channelFreqResponse = WifiSpectrumValueHelper::CreateHeOfdmTxPowerSpectralDensity (CenterFrequency, ChannelWidth, Power, GuardBW);  
      ss << "src/wifi/model/error-rate-model-data/HE_"+channeltype+".csv";     
    }
  else{
      channelFreqResponse = WifiSpectrumValueHelper::CreateHtOfdmTxPowerSpectralDensity (CenterFrequency, ChannelWidth, Power, GuardBW);  
      ss << "src/wifi/model/error-rate-model-data/"+channeltype+".csv"; 
    }
  std::ifstream infile(ss.str ());      
  // Placeholder; return a single channel
  std::list<std::complex<double> > v;  
  std::string line;
  std::complex<double> c;
  double re, im;
  char sign1, sign2, sign3;
  int random = 0;
  int numOfLines = 0;
  double min = 0.0;
  double max = 999.0;
  Ptr<UniformRandomVariable> x = CreateObject<UniformRandomVariable> ();
  x->SetAttribute ("Min", DoubleValue (min));
  x->SetAttribute ("Max", DoubleValue (max));
  random = x->GetInteger ();

  while(getline(infile, line))
  {
  if(numOfLines == random)
  {
    std::stringstream stream(line); 
    while (stream >> re >> sign1 >> im >> sign2 >> sign3)
    {
        std::complex<double> c(re, (sign1 == '-') ? -im : im);
        v.push_back (c);
    }
    stream >> re >> sign1 >> im >> sign2;
    std::complex<double> c(re, (sign1 == '-') ? -im : im);
    v.push_back (c);     	   
  }
  ++numOfLines;
  }  
  NS_LOG_DEBUG ("Size " << v.size ());
  Values::iterator chanit = channelFreqResponse->ValuesBegin ();
  
  if (ModulationClass == WIFI_MOD_CLASS_HE) // Check for HE, test the loop
    {
       uint32_t start1;
       uint32_t stop1;
       uint32_t start2;
       uint32_t stop2;
       uint32_t start3;
       uint32_t stop3;
       uint32_t start4;
       uint32_t stop4;
       uint32_t nRUs = 242;
       uint32_t nPilots_2;
  

        
       //note: moving all pilots in the center for convinience
       switch (ChannelWidth)
          {
           case 20:      
                //18 or 8 pilots 
                if (nRUs == 26|| nRUs == 52)
                 {
                  nPilots_2 = 9;
                 }
                else if (nRUs == 106|| nRUs == 242)
                 {      
                  nPilots_2 = 4;
                 }else{
                  NS_FATAL_ERROR ("20 MHz only supports: 26, 52, 106 or 242 RUs\n");       
                 }
           
                start1 = (nGuardBands / 2) + 12;
                stop1 = start1 + 121 - 1- nPilots_2;
                start2 = stop1 + nPilots_2 + 4 + nPilots_2; 
                stop2 = start2 + 121 - 1 - nPilots_2;
               for (size_t i = 0; i < channelFreqResponse->GetSpectrumModel ()->GetNumBands (); i++, chanit++)
                {
                  if ((i >= start1 && i <= stop1) || (i >= start2 && i <= stop2)) 
                  //if ((i >=38 && i <=63) || (i >=65 && i <=90)) // testing RBIR with 20 - 64subcarriers
                    {
                      NS_ASSERT_MSG (v.size () > 0, "Size zero at i " << i);
                      std::complex<double> value = v.front ();
                      v.pop_front ();
                      NS_LOG_DEBUG ("Abs of " << value << " is " << std::abs (value));
                      *chanit = std::abs (value);
                    }
                  else
                    {
                       *chanit = 0;
                    }
                }
                break;        
                

           case 40:
                //36 or 16 pilots
                if (nRUs == 26|| nRUs == 52)
                 {
                  nPilots_2 = 18;
                 }
                else if (nRUs == 106|| nRUs == 242|| nRUs == 484)
                 {
                  nPilots_2 = 8;
                 }else{
                  NS_FATAL_ERROR ("40 MHz only supports: 26, 52, 106, 242 or 484 RUs\n");       
                 }
                start1 = (nGuardBands / 2) + 12;
                stop1 = start1 + 242 - 1-nPilots_2;
                start2 = stop1 + nPilots_2 + 6 + nPilots_2;
                stop2 = start2 + 242 - 1 - nPilots_2;
                for (size_t i = 0; i < channelFreqResponse->GetSpectrumModel ()->GetNumBands (); i++, chanit++)
                {
                  if ((i >= start1 && i <= stop1) || (i >= start2 && i <= stop2))
                    {
                      NS_ASSERT_MSG (v.size () > 0, "Size zero at i " << i);
                      std::complex<double> value = v.front ();
                      v.pop_front ();
                      NS_LOG_DEBUG ("Abs of " << value << " is " << std::abs (value));
                      *chanit = std::abs (value);
                    }
                  else
                   {
                      *chanit = 0;
                   }
                }
                break;


           case 80:
                //74 or 32 or 16 pilots
                if (nRUs == 26|| nRUs == 52)
                 {
                  nPilots_2 = 37;
                 }
                else if (nRUs == 106|| nRUs == 242|| nRUs == 484)
                 {
                  nPilots_2 = 16;
                 }
                else if (nRUs == 996)
                {
                 nPilots_2 = 8;
                }                
                else{
                  NS_FATAL_ERROR ("80 MHz only supports: 26, 52, 106, 242, 484 or 996 RUs\n");       
                 }
                start1 = (nGuardBands / 2) + 12;
                stop1 = start1 + 498 - 1 -nPilots_2;
                start2 = stop1 + nPilots_2 + 6 + nPilots_2;
                stop2 = start2 + 498 - 1- nPilots_2;
                for (size_t i = 0; i < channelFreqResponse->GetSpectrumModel ()->GetNumBands (); i++, chanit++)
                {
                  if ((i >= start1 && i <= stop1) || (i >= start2 && i <= stop2))
                    {
                      NS_ASSERT_MSG (v.size () > 0, "Size zero at i " << i);
                      std::complex<double> value = v.front ();
                      v.pop_front ();
                      NS_LOG_DEBUG ("Abs of " << value << " is " << std::abs (value));
                      *chanit = std::abs (value);
                    }
                  else
                    {
                      *chanit = 0;
                   }
                }
                break;

           case 160:
                // 74*2 or 16*2
                 if (nRUs == 26|| nRUs == 52 || nRUs == 106|| nRUs == 242|| nRUs == 484)
                 {
                  nPilots_2 = 74;
                 }
                else if (nRUs == 996)
                {
                 nPilots_2 = 16;
                }                
                else{
                  NS_FATAL_ERROR ("80 MHz only supports: 26, 52, 106, 242, 484 or 996 RUs\n");       
                 }

                start1 = (nGuardBands / 2) + 12;
                stop1 = start1 + 498 - 1-nPilots_2/2;
                start2 = stop1 + nPilots_2/2 + 6 + nPilots_2/2;
                stop2 = start2 + 498 - 1- nPilots_2/2;
                start3 = stop2 + (2 * 12);
                stop3 = start3 + 498 - 1-nPilots_2/2;
                start4 = stop3 + nPilots_2/2 + 6 + nPilots_2/2;
                stop4 = start4 + 498 - 1- nPilots_2/2;
                for (size_t i = 0; i < channelFreqResponse->GetSpectrumModel ()->GetNumBands (); i++, chanit++)
                {
                  if ((i >= start1 && i <= stop1) || (i >= start2 && i <= stop2) ||
                      (i >= start3 && i <= stop3) || (i >= start4 && i <= stop4))
                    {
                      NS_ASSERT_MSG (v.size () > 0, "Size zero at i " << i);
                      std::complex<double> value = v.front ();
                      v.pop_front ();
                      NS_LOG_DEBUG ("Abs of " << value << " is " << std::abs (value));
                      *chanit = std::abs (value);
                    }
                  else
                   {
                     *chanit = 0;
                   }
                }
                break;
          }

      
    }else{ 

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

          switch(ChannelWidth)
           {
             case 20:
              // 56 subcarriers for now
              for (size_t i = 0; i < channelFreqResponse->GetSpectrumModel ()->GetNumBands (); i++, chanit++)
                {
                  
                        if ((i >=start1 && i <=stop1) || (i >=start2 && i <=stop2))
                          {
                            NS_ASSERT_MSG (v.size () > 0, "Size zero at i " << i);
                            std::complex<double> value = v.front ();
                            v.pop_front ();
                            NS_LOG_DEBUG ("Abs of " << value << " is " << std::abs (value));
                            *chanit = std::abs (value);
                           }
                        else
                         {
                            *chanit = 0;
                          }
               }
              break;

            case 40:
              // 114 subcarriers for now
              for (size_t i = 0; i < channelFreqResponse->GetSpectrumModel ()->GetNumBands (); i++, chanit++)
                {
                        if ((i >=start3 && i <=stop3) || (i >=start4 && i <=stop4))
                          {
                            NS_ASSERT_MSG (v.size () > 0, "Size zero at i " << i);
                            std::complex<double> value = v.front ();
                            v.pop_front ();
                            NS_LOG_DEBUG ("Abs of " << value << " is " << std::abs (value));
                            *chanit = std::abs (value);
                           }
                        else
                         {
                            *chanit = 0;
                          }
               }
              break; 

            case 80:
              // 242 subcarriers for now
              for (size_t i = 0; i < channelFreqResponse->GetSpectrumModel ()->GetNumBands (); i++, chanit++)
                {
                        if ((i >=start5 && i <=stop5) || (i >=start6 && i <=stop6))
                          {
                            NS_ASSERT_MSG (v.size () > 0, "Size zero at i " << i);
                            std::complex<double> value = v.front ();
                            v.pop_front ();
                            NS_LOG_DEBUG ("Abs of " << value << " is " << std::abs (value));
                            *chanit = std::abs (value);
                           }
                        else
                         {
                            *chanit = 0;
                          }
               }
              break;

            case 160:
              // 484 subcarriers (468 data + 16 pilot)
              for (size_t i = 0; i < channelFreqResponse->GetSpectrumModel ()->GetNumBands (); i++, chanit++)
              {
                       if ((i >= start5 && i <= stop5) || (i >= start6 && i <= stop6) || (i >= start7 && i <= stop7) || (i >= start8 && i <= stop8))   
                          { 
                            NS_ASSERT_MSG (v.size () > 0, "Size zero at i " << i);
                            std::complex<double> value = v.front ();
                            v.pop_front ();
                            NS_LOG_DEBUG ("Abs of " << value << " is " << std::abs (value));
                            *chanit = std::abs (value);
                           }
                      else
                        {
                          *chanit = 0;
                         }
               }
              break;

           } // end of switch
        } // end of else

  return channelFreqResponse;

}

}  // namespace ns3
