/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2015 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
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
 * Authors: Nicola Baldo <nbaldo@cttc.es> and Tom Henderson <tomh@tomh.org>
 */
#include <ns3/core-module.h>
#include <ns3/propagation-module.h>
#include <ns3/mobility-module.h>
#include <ns3/gnuplot.h>

using namespace ns3;

int main (int argc, char** argv)
{

  Ptr<ConstantPositionMobilityModel> a = CreateObject<ConstantPositionMobilityModel> ();
  Ptr<ConstantPositionMobilityModel> b = CreateObject<ConstantPositionMobilityModel> ();

  Ptr<ItuInhPropagationLossModel> ituInh = CreateObject<ItuInhPropagationLossModel> ();


  Gnuplot rxPowerPlot;
  Gnuplot pLosPlot;

  rxPowerPlot.AppendExtra ("set xlabel 'distance (m)'");
  rxPowerPlot.AppendExtra ("set ylabel 'power (dBm)'");
  rxPowerPlot.AppendExtra ("set key top right");
  rxPowerPlot.AppendExtra ("set grid");


  pLosPlot.AppendExtra ("set xlabel 'distance (m)'");
  pLosPlot.AppendExtra ("set ylabel 'probability'");
  pLosPlot.AppendExtra ("set logscale y");
  pLosPlot.AppendExtra ("set key top right");
  pLosPlot.AppendExtra ("set grid");


  Gnuplot2dDataset losDataset;
  Gnuplot2dDataset nlosDataset;
  Gnuplot2dDataset plosDataset;

  losDataset.SetStyle (Gnuplot2dDataset::LINES);
  nlosDataset.SetStyle (Gnuplot2dDataset::LINES);
  losDataset.SetTitle ("LOS RX power");
  nlosDataset.SetTitle ("NLOS RX power");
  plosDataset.SetTitle ("LOS probability");

  a->SetPosition (Vector (0.0, 0.0, 0.0));

  for (double distance = 0.0; distance < 200.0; distance += 5.0)
    {
      b->SetPosition (Vector (distance, 0.0, 0.0));

      // RX power = TX power + antenna gain - path loss
      losDataset.Add (distance,  18 + 5 - ituInh->GetLosPathLossDb (a, b));
      nlosDataset.Add (distance, 18 + 5 - ituInh->GetNlosPathLossDb (a, b));
      plosDataset.Add (distance, ituInh->GetLosProbability (a, b));
    }
  rxPowerPlot.AddDataset (losDataset);
  rxPowerPlot.AddDataset (nlosDataset);
  // noise power for kT = -174 dBm/Hz , 20 MHz bandwidth, 9dB noise figure
  rxPowerPlot.AddDataset ( Gnuplot2dFunction ("noise power", "-102") );
  pLosPlot.AddDataset (plosDataset);

  std::ofstream outFile;
  outFile.open ("lte-wifi-itu-inh-pathloss.gnuplot", std::ofstream::out | std::ofstream::trunc);

  GnuplotCollection gcRxPowerPdf ("lte-wifi-itu-inh-rx-power.pdf");
  gcRxPowerPdf.AddPlot (rxPowerPlot);
  gcRxPowerPdf.GenerateOutput (outFile);

  GnuplotCollection gcRxPowerPng ("lte-wifi-itu-inh-rx-power.png");
  gcRxPowerPng.AddPlot (rxPowerPlot);
  gcRxPowerPng.GenerateOutput (outFile);

  GnuplotCollection gcPLosPdf ("lte-wifi-itu-inh-plos.pdf");
  gcPLosPdf.AddPlot (pLosPlot);
  gcPLosPdf.GenerateOutput (outFile);

  GnuplotCollection gcPLosPng ("lte-wifi-itu-inh-plos.png");
  gcPLosPng.AddPlot (pLosPlot);
  gcPLosPng.GenerateOutput (outFile);

}
