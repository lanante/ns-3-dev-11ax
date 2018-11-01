/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright 2012 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
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


#include "lte-hex-grid-enb-topology-helper.h"
#include <ns3/double.h>
#include <ns3/log.h>
#include <ns3/abort.h>
#include <ns3/pointer.h>
#include <ns3/epc-helper.h>
#include <iostream>


namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("LteHexGridEnbTopologyHelper");

NS_OBJECT_ENSURE_REGISTERED (LteHexGridEnbTopologyHelper);

LteHexGridEnbTopologyHelper::LteHexGridEnbTopologyHelper ()
{
  NS_LOG_FUNCTION (this);
}

LteHexGridEnbTopologyHelper::~LteHexGridEnbTopologyHelper (void)
{
  NS_LOG_FUNCTION (this);
}

TypeId LteHexGridEnbTopologyHelper::GetTypeId (void)
{
  static TypeId
    tid =
    TypeId ("ns3::LteHexGridEnbTopologyHelper")
    .SetParent<Object> ()
    .AddConstructor<LteHexGridEnbTopologyHelper> ()
    .AddAttribute ("InterSiteDistance",
                   "The distance [m] between nearby sites",
                   DoubleValue (500),
                   MakeDoubleAccessor (&LteHexGridEnbTopologyHelper::m_d),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("SectorOffset",
                   "The offset [m] in the position for the node of each sector with respect "
		   "to the center of the three-sector site",
                   DoubleValue (0.5),
                   MakeDoubleAccessor (&LteHexGridEnbTopologyHelper::m_offset),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("SiteHeight",
                   "The height [m] of each site",
                   DoubleValue (30),
                   MakeDoubleAccessor (&LteHexGridEnbTopologyHelper::m_siteHeight),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("MinX", "The x coordinate where the hex grid starts.",
                   DoubleValue (0.0),
                   MakeDoubleAccessor (&LteHexGridEnbTopologyHelper::m_xMin),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("MinY", "The y coordinate where the hex grid starts.",
                   DoubleValue (0.0),
                   MakeDoubleAccessor (&LteHexGridEnbTopologyHelper::m_yMin),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("GridWidth", "The number of sites in even rows (odd rows will have one additional site).",
                   UintegerValue (1),
                   MakeUintegerAccessor (&LteHexGridEnbTopologyHelper::m_gridWidth),
                   MakeUintegerChecker<uint32_t> ())
  ;
  return tid;
}

void
LteHexGridEnbTopologyHelper::DoDispose ()
{
  NS_LOG_FUNCTION (this);
  Object::DoDispose ();
}


void 
LteHexGridEnbTopologyHelper::SetLteHelper (Ptr<LteHelper> h)
{
  NS_LOG_FUNCTION (this << h);
  m_lteHelper = h;
}

Vector
LteHexGridEnbTopologyHelper::GetSitePosition (uint32_t cellIndex)
{
  NS_LOG_FUNCTION (this << cellIndex);

  double yd = std::sqrt (0.75) * m_d;
  uint32_t currentSite = cellIndex / 3;
  uint32_t biRowIndex = (currentSite / (m_gridWidth + m_gridWidth + 1));
  uint32_t biRowRemainder = currentSite % (m_gridWidth + m_gridWidth + 1);
  uint32_t rowIndex = biRowIndex*2;
  uint32_t colIndex = biRowRemainder;
  if (biRowRemainder >= m_gridWidth)
    {
      ++rowIndex;
      colIndex -= m_gridWidth;
    }
  NS_LOG_LOGIC ("cellIndex " << cellIndex << " site " << currentSite
                << " rowIndex " << rowIndex
                << " colIndex " << colIndex
                << " biRowIndex " << biRowIndex
                << " biRowRemainder " << biRowRemainder);
  double y = m_yMin + yd * rowIndex;
  double x;
  if ((rowIndex % 2) == 0)
    {
      x = m_xMin + m_d * colIndex;
    }
  else // row is odd
    {
      x = m_xMin -(0.5*m_d) + m_d * colIndex;
    }
  return Vector (x, y, 0);
}

Vector
LteHexGridEnbTopologyHelper::GetAntennaPosition (uint32_t cellIndex)
{
  NS_LOG_FUNCTION (this << cellIndex);

  Vector p = GetSitePosition (cellIndex);
  switch (cellIndex % 3)
    {
    case 0:
      p.x += m_offset;
      break;

    case 1:
      p.x -= m_offset/2.0;
      p.y += m_offset*std::sqrt (0.75);
      break;

    case 2:
      p.x -= m_offset/2.0;
      p.y -= m_offset*std::sqrt (0.75);
      break;

    // no default, n%3 = 0, 1, 2
    }
  p.z = m_siteHeight;
  return p;
}

double
LteHexGridEnbTopologyHelper::GetAntennaOrientationDegrees (uint32_t cellIndex)
{
  NS_LOG_FUNCTION (this << cellIndex);

  double antennaOrientation = 0;
  switch (cellIndex % 3)
    {
    case 0:
      antennaOrientation = 0;
      break;

    case 1:
      antennaOrientation = 120;
      break;

    case 2:
      antennaOrientation = -120;
      break;

    // no default, n%3 = 0, 1, 2
    }
  return antennaOrientation;
}

Vector
LteHexGridEnbTopologyHelper::GetCellCenterPosition (uint32_t cellIndex)
{
  NS_LOG_FUNCTION (this << cellIndex);

  Vector p = GetSitePosition (cellIndex);
  double r = GetCellRadius ();
  switch (cellIndex % 3)
    {
    case 0:
      p.x += r;
      break;

    case 1:
      p.x -= r/2.0;
      p.y += r*std::sqrt (0.75);
      break;

    case 2:
      p.x -= r/2.0;
      p.y -= r*std::sqrt (0.75);
      break;

    // no default, n%3 = 0, 1, 2
    }
  return p;
}

double
LteHexGridEnbTopologyHelper::GetCellRadius ()
{
  NS_LOG_FUNCTION (this);

  return m_d / 3.0;
}

NetDeviceContainer 
LteHexGridEnbTopologyHelper::SetPositionAndInstallEnbDevice (NodeContainer c)
{
  NS_LOG_FUNCTION (this);
  NetDeviceContainer enbDevs;
  for (uint32_t n = 0; n < c.GetN (); ++n)
    {
      double antennaOrientation = GetAntennaOrientationDegrees (n);
      Vector pos = GetAntennaPosition (n);
      switch (n%3)
        {
        case 0:
          m_lteHelper->SetFfrAlgorithmAttribute("FrCellTypeId", UintegerValue (1));
          break;

        case 1:
          m_lteHelper->SetFfrAlgorithmAttribute("FrCellTypeId", UintegerValue (2));
          break;

        case 2:
          m_lteHelper->SetFfrAlgorithmAttribute("FrCellTypeId", UintegerValue (3));
          break;

          // no default, n%3 = 0, 1, 2
        }
      Ptr<Node> node = c.Get (n);
      Ptr<MobilityModel> mm = node->GetObject<MobilityModel> ();
      NS_LOG_LOGIC ("node " << n << " at " << pos << " antennaOrientation " << antennaOrientation);
      mm->SetPosition (pos);
      m_lteHelper->SetEnbAntennaModelAttribute ("Orientation", DoubleValue (antennaOrientation));
      enbDevs.Add (m_lteHelper->InstallEnbDevice (node));
    }
  return enbDevs;
}

} // namespace ns3
 
