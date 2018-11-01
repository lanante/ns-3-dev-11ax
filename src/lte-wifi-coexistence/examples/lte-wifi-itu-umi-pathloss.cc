
#include <ns3/core-module.h>
#include <ns3/propagation-module.h>
#include <ns3/mobility-module.h>
#include <ns3/gnuplot.h>


using namespace ns3;

int main (int argc, char** argv)
{

  Ptr<ConstantPositionMobilityModel> a = CreateObject<ConstantPositionMobilityModel> ();
  Ptr<ConstantPositionMobilityModel> b = CreateObject<ConstantPositionMobilityModel> ();

  Ptr<ItuUmiPropagationLossModel> ituUmi = CreateObject<ItuUmiPropagationLossModel> ();


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

  for (double distance = 0.0; distance < 2000.0; distance += 10.0)
    {
      b->SetPosition (Vector (distance, 0.0, 0.0));

      // RX power = TX power + antenna gain - path loss
      losDataset.Add (distance,  18 + 5 - ituUmi->GetLosPathLossDb (a, b));
      nlosDataset.Add (distance, 18 + 5 - ituUmi->GetNlosPathLossDb (a, b));
      plosDataset.Add (distance, ituUmi->GetLosProbability (a, b));
    }
  rxPowerPlot.AddDataset (losDataset);
  rxPowerPlot.AddDataset (nlosDataset);
  // noise power for kT = -174 dBm/Hz , 20 MHz bandwidth, 9dB noise figure
  rxPowerPlot.AddDataset ( Gnuplot2dFunction ("noise power", "-102") );
  pLosPlot.AddDataset (plosDataset);

  std::ofstream outFile;
  outFile.open ("lte-wifi-itu-umi-pathloss.gnuplot", std::ofstream::out | std::ofstream::trunc);

  GnuplotCollection gcRxPowerPdf ("lte-wifi-itu-umi-rx-power.pdf");
  gcRxPowerPdf.AddPlot (rxPowerPlot);
  gcRxPowerPdf.GenerateOutput (outFile);

  GnuplotCollection gcRxPowerPng ("lte-wifi-itu-umi-rx-power.png");
  gcRxPowerPng.AddPlot (rxPowerPlot);
  gcRxPowerPng.GenerateOutput (outFile);

  GnuplotCollection gcPLosPdf ("lte-wifi-itu-umi-plos.pdf");
  gcPLosPdf.AddPlot (pLosPlot);
  gcPLosPdf.GenerateOutput (outFile);

  GnuplotCollection gcPLosPng ("lte-wifi-itu-umi-plos.png");
  gcPLosPng.AddPlot (pLosPlot);
  gcPLosPng.GenerateOutput (outFile);

}
