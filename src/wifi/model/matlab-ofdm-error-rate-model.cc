/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2005,2006 INRIA
 * Copyright 2016 University of Washington
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
 * Author: Mathieu Lacage <mathieu.lacage@sophia.inria.fr>
 *
 * Modified for link simulation error model generated from MATLAB WLAN
 * System Toolbox.
 */

#include <fstream>
#include <sstream>
#include <string>
#include <map>
#include <vector>
#include <cmath>
#include <algorithm>
#include "ns3/assert.h"
#include "ns3/object.h"
#include "ns3/log.h"
#include "ns3/string.h"
#include "matlab-ofdm-error-rate-model.h"
#include "wifi-mode.h"
#include "wifi-tx-vector.h"
#include "wifi-phy.h"


static const double SNR_PRECISION = 2;

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (MatlabOfdmErrorRateModel);

NS_LOG_COMPONENT_DEFINE ("MatlabOfdmErrorRateModel");

TypeId MatlabOfdmErrorRateModel::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::MatlabOfdmErrorRateModel")
    .SetParent<ErrorRateModel> ()
    .SetGroupName ("Wifi")
    .AddConstructor<MatlabOfdmErrorRateModel> ()
    .AddAttribute ("FileName",
                   "Name of file to load",
                   StringValue ("src/wifi/model/error-rate-model-data/Error_11a_32.dat"),
                   MakeStringAccessor (&MatlabOfdmErrorRateModel::SetFileName),
                   MakeStringChecker ())
  ;
  return tid;
}

MatlabOfdmErrorRateModel::MatlabOfdmErrorRateModel (void)
{
  NS_LOG_FUNCTION (this);
}

double
MatlabOfdmErrorRateModel::RoundSnr (double snr) const
{
  NS_LOG_FUNCTION (this << snr);
  double multiplier = round(std::pow (10.0, SNR_PRECISION));
  return std::floor(snr * multiplier + 0.5)/multiplier;
}

uint8_t
MatlabOfdmErrorRateModel::GetMcsForMode (WifiMode mode) const
{
  NS_LOG_FUNCTION (this << mode);
  uint8_t mcs = 0xff; // Initialize to invalid mcs
  if (mode.GetModulationClass () == WIFI_MOD_CLASS_OFDM)
    {
      if (mode.GetConstellationSize () == 2)
        {
          if (mode.GetCodeRate () == WIFI_CODE_RATE_1_2)
            {
              mcs = 0;
            }
          if (mode.GetCodeRate () == WIFI_CODE_RATE_3_4)
            {
              mcs = 1;
            }
        }
      else if (mode.GetConstellationSize () == 4)
        {
          if (mode.GetCodeRate () == WIFI_CODE_RATE_1_2)
            {
              mcs = 2;
            }
          else if (mode.GetCodeRate () == WIFI_CODE_RATE_3_4)
            {
              mcs = 3;
            }
        }
      else if (mode.GetConstellationSize () == 16)
        {
          if (mode.GetCodeRate () == WIFI_CODE_RATE_1_2)
            {
              mcs = 4;
            }
          else if (mode.GetCodeRate () == WIFI_CODE_RATE_3_4)
            {
              mcs = 5;
            }
        }
      else if (mode.GetConstellationSize () == 64)
        {
          if (mode.GetCodeRate () == WIFI_CODE_RATE_2_3)
            {
              mcs = 6;
            }
          else if (mode.GetCodeRate () == WIFI_CODE_RATE_3_4)
            {
              mcs = 7;
            }
        }
    }
  else if (mode.GetModulationClass () == WIFI_MOD_CLASS_HT||mode.GetModulationClass () == WIFI_MOD_CLASS_VHT||mode.GetModulationClass () == WIFI_MOD_CLASS_HE)
    {
       mcs = mode.GetMcsValue();
    }
  NS_ABORT_MSG_IF (mcs == 0xff, "Error, MCS value for mode not found");
  return mcs;
}

double
MatlabOfdmErrorRateModel::FetchFsr (WifiTxVector txVector, double snr, uint32_t frameSizeBytes) const
{
  NS_LOG_FUNCTION (this << txVector.GetMode () << snr << frameSizeBytes);
  double per;
  std::map<double, double>::iterator it;
  WifiMode mode = txVector.GetMode();
  uint8_t mcs = GetMcsForMode (txVector.GetMode ()); 
  ErrorTableInfo index = {mode.GetModulationClass (), frameSizeBytes}; 
  if ( std::find(m_errorTableInfo.begin(), m_errorTableInfo.end(), index) == m_errorTableInfo.end() )
    {
      if (frameSizeBytes < 400)
        {
          if (mode.GetModulationClass () == WIFI_MOD_CLASS_OFDM)
            {
              index = {WIFI_MOD_CLASS_OFDM, 32};
            }
          else if (mode.GetModulationClass () == WIFI_MOD_CLASS_HT||mode.GetModulationClass () == WIFI_MOD_CLASS_VHT||mode.GetModulationClass () == WIFI_MOD_CLASS_HE)
            {
              index = {WIFI_MOD_CLASS_HT, 32};
            }
        }
      else
        {
          if (mode.GetModulationClass () == WIFI_MOD_CLASS_OFDM)
            {
              index = {WIFI_MOD_CLASS_OFDM, 1458};
            }
          else if (mode.GetModulationClass () == WIFI_MOD_CLASS_HT||mode.GetModulationClass () == WIFI_MOD_CLASS_VHT||mode.GetModulationClass () == WIFI_MOD_CLASS_HE)
            {
              index = {WIFI_MOD_CLASS_HT, 1458};
            }
        }
    }     
 
  it = m_errorTable[index][mcs].m_snrPer.find (snr);
  
  double minSnr =  m_errorTable[index][mcs].m_snrPer.begin()->first;
  double maxSnr = (--m_errorTable[index][mcs].m_snrPer.end())->first;
 
  if (it == m_errorTable[index][mcs].m_snrPer.end ())
    {
      if (snr < minSnr )
        {
          return 0;
        }
      else if ( snr > maxSnr)
        { 
          return 1;
        }
      else
        {
          double a, b, prev_snr, next_snr;
          
          for (std::map<double, double>::iterator i = m_errorTable[index][mcs].m_snrPer.begin(); i != m_errorTable[index][mcs].m_snrPer.end(); i++)
            {
              if(i->first < snr)
                {
                  prev_snr = i->first;
                  a = i->second;
                }
              else
                {
                  next_snr = i->first;
                  b = i->second;
                  break;
                }
            }
          per = a + (snr - prev_snr)*(b - a)/(next_snr-prev_snr);
        }
    }
  else
    {
      per = it->second;
    }
  if (frameSizeBytes != index.m_frameSize)
    {  
       //From IEEE document 11-14/0803r1 (Packet Length for Box 0 Calibration)
       double newPer;
       newPer = (1-std::pow((1-per),(frameSizeBytes/double(index.m_frameSize))));
       return 1 - newPer;
    }

  return 1 - per;
}

double
MatlabOfdmErrorRateModel::GetFrameSuccessRate (WifiMode mode, WifiTxVector txVector, double snr, uint32_t frameSizeBytes) const
{
  NS_LOG_FUNCTION (this << snr << frameSizeBytes);
  NS_ASSERT (frameSizeBytes > 0);
  
  uint32_t nbits = 8 * frameSizeBytes;
  double snrValue = RoundSnr(10.0 * std::log10 (snr));
   if (mode.GetModulationClass () == WIFI_MOD_CLASS_OFDM
      || mode.GetModulationClass () == WIFI_MOD_CLASS_HT ||mode.GetModulationClass () == WIFI_MOD_CLASS_VHT||mode.GetModulationClass () == WIFI_MOD_CLASS_HE)
    {
      std::cout << "(if_He->11): " <<mode.GetModulationClass () << " Frame: "<< frameSizeBytes <<", AWGN/Effective SNR(dB):  "<< RoundSnr(10.0 * std::log10 (snr)) <<", PER:  "<< 1-FetchFsr (txVector, snrValue, frameSizeBytes) << std::endl; 
      return FetchFsr (txVector, snrValue, frameSizeBytes);
    }
  else if (mode.GetModulationClass () == WIFI_MOD_CLASS_DSSS || mode.GetModulationClass () == WIFI_MOD_CLASS_HR_DSSS)
    {
      switch (mode.GetDataRate (20, 0, 1))
        {
        case 1000000:
          return DsssErrorRateModel::GetDsssDbpskSuccessRate (snr, nbits);
        case 2000000:
          return DsssErrorRateModel::GetDsssDqpskSuccessRate (snr, nbits);
        case 5500000:
          return DsssErrorRateModel::GetDsssDqpskCck5_5SuccessRate (snr, nbits);
        case 11000000:
          return DsssErrorRateModel::GetDsssDqpskCck11SuccessRate (snr, nbits);
        default:
          NS_ASSERT ("undefined DSSS/HR-DSSS datarate");
        }
    }
  return 0;
}

double
MatlabOfdmErrorRateModel::GetChunkSuccessRate (WifiMode mode, WifiTxVector txVector, double snr, uint32_t phy_nbits) const
{
  NS_LOG_FUNCTION (this << mode << snr << phy_nbits);
  NS_ASSERT (phy_nbits > 0);
  double code_rate = (double)mode.GetDataRate(txVector)/(double)mode.GetPhyRate (txVector);
  uint32_t data_bits = phy_nbits*code_rate;
  double a= GetFrameSuccessRate (mode, txVector, snr, (data_bits/8));
  return a;
}

void
MatlabOfdmErrorRateModel::SetFileName (std::string fileName)
{
  NS_LOG_FUNCTION (this << fileName);
  m_traceFile = fileName;
}

void 
MatlabOfdmErrorRateModel::LoadFile ( ErrorTableInfo data, std::string filePath)
{
  NS_LOG_FUNCTION (this << data.m_wifiModClass << data.m_frameSize << filePath);
  m_traceFile = filePath; 
  m_errorTableInfo.push_back(data);
  
  std::ifstream infile (m_traceFile);
  if (!infile.is_open ())
    {
      NS_ABORT_MSG ("file " << m_traceFile << " could not be opened");
    }
  ErrorTableData readValues;
  m_errorTableValues.clear();
  std::string line;
  int lineCount = -1;
  //Reading the file, parsing the data and storing in the defined data structure
  while (std::getline (infile, line))
    {
      if (line[0] == '#')
        {
          std::istringstream iss (line.substr (1));
          if (lineCount > -1)
            {
              m_errorTableValues.push_back (readValues);
            }
          lineCount += 1;
          std::string x;
          int y;
          while (iss >> x >> y)
            {
              readValues.m_mcs = y;
            }
        }
      else
        {
          std::istringstream iss (line);
          double x,y;
          while (iss >> x >> y)
            {
              readValues.m_snrPer[RoundSnr(x)] = y;
            }
        }
    }
  m_errorTableValues.push_back (readValues);
  //m_errorTable[data]= m_errorTableValues;
  m_errorTable.insert(std::pair<ErrorTableInfo, std::vector<ErrorTableData>>(data, m_errorTableValues));

  //Performing Sanity Checks
  unsigned int j = 0;
  double monotonically_increasing_counter;
  while (j < m_errorTableValues.size ())
    {
      monotonically_increasing_counter = 1;
      std::map<double, double>::iterator it = m_errorTableValues[j].m_snrPer.begin ();
      while (it != m_errorTableValues[j].m_snrPer.end ())
        {
          if (monotonically_increasing_counter < it->second)
            {
              std::cout << "ERROR: Values in MCS " << m_errorTableValues[j].m_mcs << " are not monotonically decreasing." << it->first << "::" << it->second << std::endl;
            }
          if (it->second < 0)
            {
              std::cout << "ERROR: Value in " << m_errorTableValues[j].m_mcs << " corresponding to " << it->first << " db SNR is less than 0: " << it->second << std::endl;
            }
          if (it->second > 1)
            {
              std::cout << "ERROR: Value in " << m_errorTableValues[j].m_mcs << " corresponding to " << it->first << " dB SNR is greater than 1: " << it->second << std::endl;
            }
          monotonically_increasing_counter = it->second;
          it++;
        }
      j += 1;
    }
  infile.close ();
}
  
void
MatlabOfdmErrorRateModel::DoInitialize (void)
{
  NS_LOG_FUNCTION (this);
  ErrorTableInfo index;
  
  index = {WIFI_MOD_CLASS_OFDM, 32};
  LoadFile(index, "src/wifi/model/error-rate-model-data/Error_11a_32.dat");
  index = {WIFI_MOD_CLASS_OFDM, 1458};
  LoadFile(index, "src/wifi/model/error-rate-model-data/Error_11a_1458.dat");
  index = {WIFI_MOD_CLASS_HT, 32};
  LoadFile(index, "src/wifi/model/error-rate-model-data/Error_11n_32.dat");
  index = {WIFI_MOD_CLASS_HT, 1458};
  LoadFile(index, "src/wifi/model/error-rate-model-data/Error_11n_1458.dat");
  index = {WIFI_MOD_CLASS_VHT, 32};
  LoadFile(index, "src/wifi/model/error-rate-model-data/Error_11ax_32.dat");    // currently using 11ax for mcs 0-9
  index = {WIFI_MOD_CLASS_VHT, 1458};
  LoadFile(index, "src/wifi/model/error-rate-model-data/Error_11ax_1458.dat");  // currently using 11ax for mcs 0-9
  index = {WIFI_MOD_CLASS_HE, 32};
  LoadFile(index, "src/wifi/model/error-rate-model-data/Error_11ax_32.dat");
  index = {WIFI_MOD_CLASS_HE, 1458};
  LoadFile(index, "src/wifi/model/error-rate-model-data/Error_11ax_1458.dat");
  
}

void
MatlabOfdmErrorRateModel::Print (void) const
{
  NS_LOG_FUNCTION (this);
  std::ifstream infile (m_traceFile);
  if (!infile.is_open ())
    {
      NS_ABORT_MSG ("file " << m_traceFile << " could not be opened");
    }
  std::string line;
  while (std::getline (infile, line))
    {
      std::cout << line << std::endl;
    }
  infile.close ();
}

} //namespace ns3
