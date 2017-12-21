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

#include "RBIR.h"
#include <ns3/log.h>
#include <ns3/assert.h>
#include <ns3/double.h>
#include <cmath> // for M_PI
#include <iomanip> // for M_PI
#include <random>
#include <string>
#include <fstream>
#include <list>
#include <vector>
#include "ns3/log.h"

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (RBIR);

NS_LOG_COMPONENT_DEFINE ("RBIR");


RBIR::RBIR ()
{
}

RBIR::~RBIR ()
{
}


TypeId
RBIR::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::RBIR")
    .SetParent<Object> ()
    .SetGroupName ("Wifi")
    .AddConstructor<RBIR> ()
  ;
  return tid;
}

double
RBIR::RoundOff (double input,int precision) const
{
  double multiplier = round(std::pow (10.0, precision));
  return std::floor(input * multiplier + 0.5)/multiplier;
}

void 
RBIR::LoadFile ()
{
 RBIRTableData read_Values;
 std::string line;
 std::ifstream infile("src/wifi/model/error-rate-model-data/RBIR_11ax.csv");
 int lineCount = -1;
 char sign1;
 

 double x_prev=0;
 double y_prev=0;
 double count = 1;

 while (std::getline (infile, line))
    {

      if (line[0] == '#')
        {
          std::istringstream iss (line.substr (1));
          if (lineCount > -1)
            {
              m_RBIRTableValues.push_back (read_Values); 
            }
          lineCount += 1;
          std::string x;
          int y;
          while (iss >> x >> y)
            {
              read_Values.modulationtype = x;  
              read_Values.constellationsize = y;
            }
        }
      else
        { 
          std::istringstream iss (line);
          double x,y;

          while (iss >> x >> sign1 >> y)
            {
              read_Values.snrRBIR[RoundOff(x,2)] = y;
            }
          if (fabs(RoundOff(y_prev,2) - RoundOff(y,2)) < 0.00001){
                count = count + 1;
                x_prev = x_prev+x;
             }else{
                read_Values.RBIRsnr[RoundOff(y_prev,2)]=x_prev/count;       
                y_prev=y;
                x_prev=x;
                count = 1;
           } 

        }
    }
 m_RBIRTableValues.push_back (read_Values);

 /*for(std::map<double,double>::iterator it = read_Values.RBIRsnr.begin(); it != read_Values.RBIRsnr.end(); ++it)
   {
    std::cout << it->first << " " << it->second << "\n";
   }
*/
}


Ptr<SpectrumValue>
RBIR::GetSubcarrierMI(Ptr<SpectrumValue> txPsd,std::string modulationtype, int constellationsize) const
{
 Ptr<SpectrumValue> MIvector = Copy<SpectrumValue> (txPsd);

 double BOLTZMANN=1.3803e-23;
 double noisePSD = BOLTZMANN * 290.0;  // 312.5 KHz width
 double bandBandwidth = 78125;

 uint32_t guardBandwidth = 10; //take as input
 uint32_t channelWidth = 20;   //take as input
 uint32_t nGuardBands = static_cast<uint32_t>(((2 * guardBandwidth * 1e6) / bandBandwidth) + 0.5);
 
  uint32_t start1;
  uint32_t stop1;
  uint32_t start2;
  uint32_t stop2;
  uint32_t start3;
  uint32_t stop3;
  uint32_t start4;
  uint32_t stop4;


 Values::iterator mi = MIvector->ValuesBegin ();
 switch (channelWidth)
   {

   case 20:
       start1 = (nGuardBands / 2) + 12;
       stop1 = start1 + 121 - 1;
       start2 = stop1 + 4;
       stop2 = start2 + 121 - 1;
        for (size_t i = 0; i < MIvector->GetSpectrumModel ()->GetNumBands (); i++,mi++)
        {
          if (((i >= start1 && i <= stop1) || (i >= start2 && i <= stop2)) && *mi != 0) 
            {
              *mi = *mi/noisePSD;     // snr (linear)
              *mi = GetMI(10*log10(*mi),modulationtype,constellationsize);  // Get MIvector from snr(dB)
           }
          else{
              *mi = 0;
           }
        }
        break;        
        

   case 40:
        start1 = (nGuardBands / 2) + 12;
        stop1 = start1 + 242 - 1;
        start2 = stop1 + 6;
        stop2 = start2 + 242 - 1;
        for (size_t i = 0; i < MIvector->GetSpectrumModel ()->GetNumBands (); i++,mi++)
        {
          if ((i >= start1 && i <= stop1) || (i >= start2 && i <= stop2))
            {
              *mi = *mi/noisePSD;     // snr (linear)
              *mi = GetMI(10*log10(*mi),modulationtype,constellationsize);  // Get MIvector from snr(dB)
           }
          else{
              *mi = 0;
           }
        }
        break;


   case 80:
        start1 = (nGuardBands / 2) + 12;
        stop1 = start1 + 498 - 1;
        start2 = stop1 + 6;
        stop2 = start2 + 498 - 1;
        for (size_t i = 0; i < MIvector->GetSpectrumModel ()->GetNumBands (); i++,mi++)
        {
          if ((i >= start1 && i <= stop1) || (i >= start2 && i <= stop2))
            {
              *mi = *mi/noisePSD;     // snr (linear)
              *mi = GetMI(10*log10(*mi),modulationtype,constellationsize);  // Get MIvector from snr(dB)
           }
          else{
              *mi = 0;
           }
        }
        break;

   case 160:
        start1 = (nGuardBands / 2) + 12;
        stop1 = start1 + 498 - 1;
        start2 = stop1 + 6;
        stop2 = start2 + 498 - 1;
        start3 = stop2 + (2 * 12);
        stop3 = start3 + 498 - 1;
        start4 = stop3 + 6;
        stop4 = start4 + 498 - 1;
        for (size_t i = 0; i < MIvector->GetSpectrumModel ()->GetNumBands (); i++,mi++)
        {
          if ((i >= start1 && i <= stop1) || (i >= start2 && i <= stop2) ||
              (i >= start3 && i <= stop3) || (i >= start4 && i <= stop4))
            {
              *mi = *mi/noisePSD;     // snr (linear)
              *mi = GetMI(10*log10(*mi),modulationtype,constellationsize);  // Get MIvector from snr(dB)
           }
          else{
              *mi = 0;
           }
        }
        break;
  }
 
  return MIvector;

}

double 
RBIR::GetMI(double snr_dB, std::string modulationtype, int constellationsize) const
{

  snr_dB = RoundOff(snr_dB,2);
  double var = std::round(100*snr_dB);
  int check = (int)var % 2;
  if( modulationtype=="BPSK" && constellationsize==2)
  {
    if (snr_dB > 30){ 
       return 1; }
    else if (snr_dB < -20){
       return 0.0144;}
    else if (check != 0){
       return (m_RBIRTableValues[0].snrRBIR.find(RoundOff(snr_dB-0.01,2))->second + m_RBIRTableValues[0].snrRBIR.find(RoundOff(snr_dB+0.01,2))->second)/2;  
     }                   
    return m_RBIRTableValues[0].snrRBIR.find(snr_dB)->second; 
   }

  else if ( modulationtype=="QPSK" && constellationsize==4)
  {
    if (snr_dB > 30){ 
       return 2; }
    else if (snr_dB < -20){
       return 0.0148;}
    else if (check != 0){
       return (m_RBIRTableValues[1].snrRBIR.find(RoundOff(snr_dB-0.01,2))->second + m_RBIRTableValues[1].snrRBIR.find(RoundOff(snr_dB+0.01,2))->second)/2;  
     }
    return m_RBIRTableValues[1].snrRBIR.find(snr_dB)->second;
   }

  else if ( modulationtype=="QAM" && constellationsize==16)
  {
    if (snr_dB > 30){ 
       return 4; }
    else if (snr_dB < -20){
       return 0.0139;}
    else if (check != 0){
       return (m_RBIRTableValues[2].snrRBIR.find(RoundOff(snr_dB-0.01,2))->second + m_RBIRTableValues[2].snrRBIR.find(RoundOff(snr_dB+0.01,2))->second)/2;  
     }
    return m_RBIRTableValues[2].snrRBIR.find(snr_dB)->second;
   }

  else if ( modulationtype=="QAM" && constellationsize==64)
  {
    if (snr_dB > 30){ 
       return 6; }
    else if (snr_dB < -20){
       return 0.0144;}
    else if (check != 0){
       return (m_RBIRTableValues[3].snrRBIR.find(RoundOff(snr_dB-0.01,2))->second + m_RBIRTableValues[3].snrRBIR.find(RoundOff(snr_dB+0.01,2))->second)/2;  
     }
    return m_RBIRTableValues[3].snrRBIR.find(snr_dB)->second;
   }

  else if ( modulationtype=="QAM" && constellationsize==256)
  {
    if (snr_dB > 30){ 
       return 8; }
    else if (snr_dB < -20){
       return 0.0144;}
    else if (check != 0){
       return (m_RBIRTableValues[4].snrRBIR.find(RoundOff(snr_dB-0.01,2))->second + m_RBIRTableValues[4].snrRBIR.find(RoundOff(snr_dB+0.01,2))->second)/2;  
     }
    return m_RBIRTableValues[4].snrRBIR.find(snr_dB)->second;
   }
  else if ( modulationtype=="QAM" && constellationsize==1024)
  {
    NS_FATAL_ERROR ("1024 QAM to be added, max supported modulation: QAM 256 \n"); 
    return 0;
   }
  else{
    NS_FATAL_ERROR ("Modulation not Supported \n"); 
    return 0;
  }
 
}




double 
RBIR::GetEffectiveSNR(double MI, std::string modulationtype, int constellationsize) const
{
  MI = RoundOff(MI,2);
 
  if( modulationtype=="BPSK" && constellationsize==2)
  {
    if (MI < 0.01){ 
       return -20; }
    else if (MI >= 1){
       return 10;}
        
    return m_RBIRTableValues[0].RBIRsnr.find(MI)->second; 
   }

  else if ( modulationtype=="QPSK" && constellationsize==4)
  {
    if (MI < 0.01){ 
       return -20; }
    else if (MI >= 2){
       return 15;}
    return m_RBIRTableValues[1].RBIRsnr.find(MI)->second;
   }

  else if ( modulationtype=="QAM" && constellationsize==16)
  {
    if (MI < 0.01){ 
       return -20; }
    else if (MI >= 4){
       return 20;}
    return m_RBIRTableValues[2].RBIRsnr.find(MI)->second;
   }

  else if ( modulationtype=="QAM" && constellationsize==64)
  {
    if (MI < 0.01){ 
       return -20; }
    else if (MI >= 6){
       return 25;}
    return m_RBIRTableValues[3].RBIRsnr.find(MI)->second;
   }

  else if ( modulationtype=="QAM" && constellationsize==256)
  {
    if (MI < 0.01){ 
       return -20; }
    else if (MI >= 8){
       return 30;}
    return m_RBIRTableValues[3].RBIRsnr.find(MI)->second;
   }
  else if ( modulationtype=="QAM" && constellationsize==1024)
  {
    NS_FATAL_ERROR ("1024 QAM to be added, max supported modulation: QAM 256 \n"); 
    return 0;
   }
  else{
    NS_FATAL_ERROR ("Modulation not Supported \n"); 
    return 0;
  }
 }

// This function is for SISO only, update for MIMO required 
double
RBIR::GetEffectivePower(Ptr<SpectrumValue> txPsd,double channelWidth, uint8_t mcs, int constellationsize, int N_SS) const
{ 
 std::string modulationtype;
 if(mcs==0)
   {modulationtype="BPSK";}
 else if(mcs < 3)
   {modulationtype="QPSK";}
 else if(mcs <12)
   {modulationtype="QAM";}
 else
   {NS_FATAL_ERROR ("MCS not supported\n"); }
        

 if (N_SS==1){
   Ptr<SpectrumValue> MI = GetSubcarrierMI(txPsd,modulationtype,constellationsize);
   double MI_avg = 0;
   double count = 0;
   //double channelWidth = 20;   
   double BOLTZMANN=1.3803e-23;
   double noisePSD = BOLTZMANN * 290.0;
     
        
   Values::iterator mi = MI->ValuesBegin ();
   for (size_t i = 0; i < MI->GetSpectrumModel ()->GetNumBands (); i++,mi++)
     {
      if (*mi != 0)
        {
          MI_avg = MI_avg + *mi;
          count = count + 1; 
        }
     }
   MI_avg = MI_avg/count;
   double snr_eff_dB = GetEffectiveSNR (MI_avg,modulationtype,constellationsize);
   // convert to power
   double snr_eff = std::pow(10.0,(snr_eff_dB/10));
   snr_eff = (snr_eff*256)/242; // factor for null carriers
    //std::cout <<"snr_eff_db: "<< snr_eff_dB << "  snreff: " << snr_eff << std::endl;
   NS_LOG_DEBUG ("Effective SNR value (linear) w/o cosidering noise figure of 7 dB " << snr_eff);
   double output_power=snr_eff*noisePSD*channelWidth*1e6;
   return output_power;
  }

  NS_FATAL_ERROR ("No MIMO Support, use N_SS = 1 \n");
  return 0;
 }

}
