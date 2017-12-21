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

#ifndef MATLAB_OFDM_ERROR_RATE_MODEL_H
#define MATLAB_OFDM ERROR_RATE_MODEL_H

#include <stdint.h>
#include "error-rate-model.h"
#include "wifi-mode.h"
#include "wifi-tx-vector.h"
#include "ns3/object.h"
#include "dsss-error-rate-model.h"


namespace ns3 {

typedef struct ErrorTableData
  {
    int m_mcs;
    std::map<double, double> m_snrPer;
  }ErrorTableData;

typedef struct ErrorTableInfo
  {
    WifiModulationClass m_wifiModClass;
    uint32_t m_frameSize;
  }ErrorTableInfo;

inline bool operator <(const ErrorTableInfo& l, const ErrorTableInfo& r)
{
  if (l.m_frameSize < r.m_frameSize)
    {
      return 1;
    }
  else if (l.m_frameSize == r.m_frameSize)
    {
      return (l.m_wifiModClass < r.m_wifiModClass);
    }
  else 
    {
      return 0;
    }
}

inline bool operator ==(const ErrorTableInfo& l, const ErrorTableInfo& r)
{
  return ((l.m_frameSize == r.m_frameSize) & (l.m_wifiModClass == r.m_wifiModClass));
}


/**
 * \ingroup wifi
 * \brief the interface for a table-driven OFDM Wifi's error models
 *
 */
class MatlabOfdmErrorRateModel : public ErrorRateModel
{

public:
  static TypeId GetTypeId (void);
 
  MatlabOfdmErrorRateModel ();

  /**
   * A pure virtual method that must be implemented in the subclass.
   * This method returns the probability that the given 'chunk' of the
   * packet will be successfully received by the PHY.
   *
   * A chunk can be viewed as a part of a packet with equal SNR.
   * The probability of successfully receiving the chunk depends on
   * the mode, the SNR, and the size of the chunk.
   *
   * Note that both a WifiMode and a WifiTxVector (which contains a WifiMode)
   * are passed into this method.  The WifiTxVector may be from a signal that
   * contains multiple modes (e.g. PLCP header sent differently from PLCP
   * payload).  Consequently, the mode parameter is what the method uses
   * to calculate the chunk error rate, and the txVector is used for
   * other information as needed.
   *
   * \param mode the Wi-Fi mode applicable to this chunk
   * \param txvector TXVECTOR of the overall transmission
   * \param snr the SNR of the chunk
   * \param nbits the number of bits in this chunk
   *
   * \return probability of successfully receiving the chunk
   */
  virtual double GetChunkSuccessRate (WifiMode mode, WifiTxVector txVector, double snr, uint32_t nbits) const;
  virtual double GetFrameSuccessRate (WifiMode mode, WifiTxVector txVector, double snr, uint32_t frameSizeBytes) const;
  
  /**
   * Temporary debugging method to print out file contents
   */
  void Print (void) const;
  /**
   * Provide path to error model data file
   * \param fileName file name
   */
  void SetFileName (std::string fileName);
  /**
   * Method to load the .dat file with the SNR and PER values, for the corresponding error model 
   */
  void LoadFile ( ErrorTableInfo data, std::string filePath);
protected:
  virtual void DoInitialize (void);

private:
  
  /**
   * Round SNR to the specified precision
   */
  double RoundSnr (double snr) const;
  /**
   * Returns the frame success rate for a given WifiTxVector, snr and frame size.
   */
  double FetchFsr (WifiTxVector txVector, double snr, uint32_t frameSizeBytes) const;
  
  /**
   * \brief Utility function to convert WifiMode to an MCS value
   * \param mode WifiMode
   * \return the MCS value
   */
  uint8_t GetMcsForMode (WifiMode mode) const;
  mutable std::string m_traceFile;
  std::vector<ErrorTableData> m_errorTableValues;
  std::vector<ErrorTableInfo> m_errorTableInfo;
  mutable std::map<double,double> m_errorTableCache[8];

  mutable std::map<ErrorTableInfo, std::vector<ErrorTableData>> m_errorTable;
  
};

} //namespace ns3

#endif /* MATLAB_OFDM_ERROR_RATE_MODEL_H */

