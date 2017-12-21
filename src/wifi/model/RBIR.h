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

#ifndef RBIR_H
#define RBIR_H


#include <ns3/wifi-mode.h>
#include <ns3/object.h>
#include <string>
#include <map>
#include <vector>
#include <ns3/spectrum-value.h>
namespace ns3 {

//class MobilityModel;


/**
 * \ingroup spectrum
 * \brief Table spectrum propagation loss model
 *
 * The propagation loss is calculated according to a simplified version of Table'
 * formula in which antenna gains are unitary:
 *
 * \f$ L = \frac{4 \pi * d * f}{C^2}\f$
 *
 * where C = 3e8 m/s is the light speed in the vacuum. The intended
 * use is to calculate Prx = Ptx * G
 */

typedef struct RBIRTableData
  {
    std::string modulationtype;
    int constellationsize;
    std::map<double, double> snrRBIR;
    std::map<double, double> RBIRsnr;
  }RBIRTableData;

/*typedef struct RBIRTableInfo
  {
    WifiModulationClass m_wifiModClass;
    uint32_t m_frameSize;
  }RBIRTableInfo;
*/


class RBIR : public Object
{
public:
  RBIR ();
  ~RBIR ();
  
  /**
   * \brief Get the type ID.
   * \return the object TypeId
   */
  static TypeId GetTypeId ();

 void LoadFile ();
 Ptr<SpectrumValue> GetSubcarrierMI(Ptr<SpectrumValue> txPsd,std::string modulationtype, int constellationsize) const;
 std::vector<RBIRTableData> m_RBIRTableValues;
 double GetMI(double snr, std::string modulationtype, int constellationsize) const;
 double RoundOff (double input,int precision) const;
 double GetEffectiveSNR(double MI, std::string modulationtype, int constellationsize) const;
 
 // Outputs single effective power after RBIR mapping 
 double GetEffectivePower(Ptr<SpectrumValue> txPsd,double channelWidth, uint8_t mcs, int constellationsize, int N_SS) const;

       
};

/*inline bool RBIR::operator ==(const double& l, const double& r)
        {
            return fabs(l-r) < 0.0000001;
        }*/
} // namespace ns3
 
#endif /* RBIR_H */
