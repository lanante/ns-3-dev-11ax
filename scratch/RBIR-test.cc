/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
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
 * Authors: Rohan Patidar <rpatidar@uw.edu> and Tom Henderson <tomh@tomh.org>
 */

// This example is used to validate NIST and YANS error rate models for HT rates.
//
// It outputs plots of the Frame Success Rate versus the Signal-to-noise ratio for
// both NIST and YANS error rate models and for every HT MCS value.

#include "ns3/core-module.h"
#include "ns3/RBIR.h"
#include "ns3/table-spectrum-propagation-loss.h"
#include "ns3/spectrum-value.h"
#include "ns3/wifi-spectrum-value-helper.h"
#include "ns3/wifi-mode.h"
#include "ns3/gnuplot.h"
#include <map>
using namespace ns3;

int main (int argc, char *argv[])
{

  // Create RBIR object
  Ptr <RBIR> rbir = CreateObject<RBIR> ();
  
  // Load all Modulation in table
  rbir->LoadFile();

  std::string modulationtype[] = {"BPSK","QPSK","QAM","QAM","QAM"};
  double constellationsize[] = {2,4,16,64,256};
  std::stringstream ss;


  // PLOT of RBIR vs SNR(dB) for all modulation types (Test for retrival function)
  for(int i = 0;i<5;i++)
  {
    double a=-20;    
    ss << modulationtype[i] << "-" << constellationsize[i] << ".plt";    
    std::ofstream makeplot (ss.str ().c_str ());
    ss.str ("");
    ss << modulationtype[i] << "-" << constellationsize[i] << ".eps";
    Gnuplot rbirplot = Gnuplot (ss.str ());
    ss.str ("");
    Gnuplot2dDataset dataset;
    dataset.SetStyle (Gnuplot2dDataset::LINES_POINTS);
    dataset.SetTitle("\u03A6");
     while (a <= 30){ 
        double  out = rbir->GetMI(a,modulationtype[i],constellationsize[i]);
        dataset.Add (a, out);
        a = a + 0.01;
     }
    rbirplot.AddDataset (dataset);
    rbirplot.SetTerminal ("postscript eps color enh \"Times-BoldItalic\"");
    rbirplot.SetLegend ("SNR (dB)", "MI");
    ss << "MI (" << modulationtype[i] << "-" << constellationsize[i]<<") vs SNR (dB)";
    rbirplot.SetTitle  (ss.str ());
    ss.str ("");
    rbirplot.GenerateOutput (makeplot);
    makeplot.close ();
  }



  // Testing SNR vs RBIR (Reverse mapping)
  double MImax[] ={1,2,4,6,8}; 
  for(int i = 0;i<5;i++)
  {
    double a=0;
    ss <<modulationtype[i] << "-" << constellationsize[i] << "-inv"<< ".plt";    
    std::ofstream makeplot (ss.str ().c_str ());
    ss.str ("");
    ss <<modulationtype[i] << "-" << constellationsize[i] << "-inv" << ".eps";
    Gnuplot rbirplot = Gnuplot (ss.str ());
    ss.str ("");
    Gnuplot2dDataset dataset;
    dataset.SetStyle (Gnuplot2dDataset::LINES_POINTS);
    dataset.SetTitle("\u03A6");
     while (a <= MImax[i]){ 
        double  out = rbir->GetEffectiveSNR(a,modulationtype[i],constellationsize[i]);
        dataset.Add (a, out);
        a = a + .01;
     }
    rbirplot.AddDataset (dataset);
    rbirplot.SetTerminal ("postscript eps color enh \"Times-BoldItalic\"");
    rbirplot.SetLegend ("MI ", "SNR(dB)");
    ss << "Inverse SNR_dB (" << modulationtype[i] << "-" << constellationsize[i]<<") vs MI";
    rbirplot.SetTitle  (ss.str ());
    ss.str ("");
    rbirplot.GenerateOutput (makeplot);
    makeplot.close ();
  }


  // create filtered signal
  Ptr<const SpectrumValue> SignalPsd = WifiSpectrumValueHelper::CreateHeOfdmTxPowerSpectralDensity (5.18, 20, 0.00000000001, 10); 
  WifiModulationClass MC = WIFI_MOD_CLASS_HE; 
  
  // Need to apply pathloss ?? 

  // Apply random channel to the signal 
  TableSpectrumPropagationLossModel tb;
  Ptr<SpectrumValue>received_spectrum = tb.CalRxSpectralDensity (SignalPsd, MC ,"TGN-D", 78125 ,5180,20, 10);
  

  // print channel
/*Values::iterator chanit = received_spectrum->ValuesBegin ();
  for (size_t i = 0; i < received_spectrum->GetSpectrumModel ()->GetNumBands (); i++,chanit++)
      {std::cout << *chanit << std::endl;}
 */
  


 int MCS=0;
 int N_CP = 2; 
 int Nss=1;
 // Test for the given channel
  double effPower = rbir->GetEffectivePower(received_spectrum, 20.0 ,MCS, N_CP,Nss);
  std::cout <<"Total Input Received Power: " << Integral(*SignalPsd) << "  Output Effective Power : "<< effPower << std::endl;

  // To get Subcarrier MI
/*Ptr<SpectrumValue> SNRvector = rbir->GetSubcarrierMI(received_spectrum,Mod, N_CP); 
 Values::iterator newSNR = SNRvector->ValuesBegin ();
 for (size_t i = 0; i < SNRvector->GetSpectrumModel ()->GetNumBands (); i++,newSNR++)
      {
       if ((i >=38 && i <=63) || (i >=65 && i <=90))
          {
             std::cout<< *newSNR << std::endl;   
           }
      }
*/
  
 // can use below to plot RBIR values
  //std::cout <<a <<" Success-A "<<rbir->GetMI(-19.5, "BPSK", 2) << std::endl;
 //std::cout <<aa <<" Effective MI "<< std::endl;

  /*for(std::map<double,double>::iterator it = rbir->m_SNRRBIRTableValues[1].snrRBIR.begin(); it != rbir->m_SNRRBIRTableValues[1].snrRBIR.end(); ++it)
    {
     //std::cout <<"Test: "<<it->first << " " << it->second << "\n";
    }
  */

}

