/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2007 INRIA
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
 */
#include "position-allocator.h"
#include "ns3/double.h"
#include "ns3/string.h"
#include "ns3/pointer.h"
#include "ns3/uinteger.h"
#include "ns3/enum.h"
#include "ns3/log.h"
#include "ns3/mobility-model.h"
#include <cmath>
#include <fstream>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("PositionAllocator");

NS_OBJECT_ENSURE_REGISTERED (PositionAllocator);

TypeId 
PositionAllocator::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::PositionAllocator")
    .SetParent<Object> ()
    .SetGroupName ("Mobility");
  return tid;
}

PositionAllocator::PositionAllocator ()
{
}

PositionAllocator::~PositionAllocator ()
{
}

NS_OBJECT_ENSURE_REGISTERED (ListPositionAllocator);

TypeId
ListPositionAllocator::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::ListPositionAllocator")
    .SetParent<PositionAllocator> ()
    .SetGroupName ("Mobility")
    .AddConstructor<ListPositionAllocator> ()
  ;
  return tid;
}
ListPositionAllocator::ListPositionAllocator ()
{
}
void
ListPositionAllocator::Add (Vector v)
{
  m_positions.push_back (v);
  m_current = m_positions.begin ();
}
Vector
ListPositionAllocator::GetNext (void) const
{
  Vector v = *m_current;
  m_current++;
  if (m_current == m_positions.end ())
    {
      m_current = m_positions.begin ();
    }
  return v;
}
int64_t
ListPositionAllocator::AssignStreams (int64_t stream)
{
  return 0;
}

uint32_t
ListPositionAllocator::GetSize (void) const
{
  return m_positions.size ();
}

NS_OBJECT_ENSURE_REGISTERED (GridPositionAllocator);

TypeId 
GridPositionAllocator::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::GridPositionAllocator")
    .SetParent<PositionAllocator> ()
    .SetGroupName ("Mobility")
    .AddConstructor<GridPositionAllocator> ()
    .AddAttribute ("GridWidth", "The number of objects laid out on a line.",
                   UintegerValue (10),
                   MakeUintegerAccessor (&GridPositionAllocator::m_n),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("MinX", "The x coordinate where the grid starts.",
                   DoubleValue (1.0),
                   MakeDoubleAccessor (&GridPositionAllocator::m_xMin),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("MinY", "The y coordinate where the grid starts.",
                   DoubleValue (0.0),
                   MakeDoubleAccessor (&GridPositionAllocator::m_yMin),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("Z",
                   "The z coordinate of all the positions allocated.",
                   DoubleValue (0.0),
                   MakeDoubleAccessor (&GridPositionAllocator::m_z),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("DeltaX", "The x space between objects.",
                   DoubleValue (1.0),
                   MakeDoubleAccessor (&GridPositionAllocator::m_deltaX),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("DeltaY", "The y space between objects.",
                   DoubleValue (1.0),
                   MakeDoubleAccessor (&GridPositionAllocator::m_deltaY),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("LayoutType", "The type of layout.",
                   EnumValue (ROW_FIRST),
                   MakeEnumAccessor (&GridPositionAllocator::m_layoutType),
                   MakeEnumChecker (ROW_FIRST, "RowFirst",
                                    COLUMN_FIRST, "ColumnFirst"))
  ;
  return tid;
}
GridPositionAllocator::GridPositionAllocator ()
  : m_current (0)
{
}

void
GridPositionAllocator::SetMinX (double xMin)
{
  m_xMin = xMin;
}
void
GridPositionAllocator::SetMinY (double yMin)
{
  m_yMin = yMin;
}
void
GridPositionAllocator::SetZ (double z)
{
  m_z = z;
}
void
GridPositionAllocator::SetDeltaX (double deltaX)
{
  m_deltaX = deltaX;
}
void
GridPositionAllocator::SetDeltaY (double deltaY)
{
  m_deltaY = deltaY;
}
void
GridPositionAllocator::SetN (uint32_t n)
{
  m_n = n;
}
void
GridPositionAllocator::SetLayoutType (enum LayoutType layoutType)
{
  m_layoutType = layoutType;
}

double
GridPositionAllocator::GetMinX (void) const
{
  return m_xMin;
}
double
GridPositionAllocator::GetMinY (void) const
{
  return m_yMin;
}
double
GridPositionAllocator::GetDeltaX (void) const
{
  return m_deltaX;
}
double
GridPositionAllocator::GetDeltaY (void) const
{
  return m_deltaY;
}
uint32_t
GridPositionAllocator::GetN (void) const
{
  return m_n;
}
enum GridPositionAllocator::LayoutType
GridPositionAllocator::GetLayoutType (void) const
{
  return m_layoutType;
}

Vector
GridPositionAllocator::GetNext (void) const
{
  double x = 0.0, y = 0.0;
  switch (m_layoutType) {
    case ROW_FIRST:
      x = m_xMin + m_deltaX * (m_current % m_n);
      y = m_yMin + m_deltaY * (m_current / m_n);
      break;
    case COLUMN_FIRST:
      x = m_xMin + m_deltaX * (m_current / m_n);
      y = m_yMin + m_deltaY * (m_current % m_n);
      break;
    }
  m_current++;
  return Vector (x, y, m_z);
}

int64_t
GridPositionAllocator::AssignStreams (int64_t stream)
{
  return 0;
}

NS_OBJECT_ENSURE_REGISTERED (RandomRectanglePositionAllocator);

TypeId
RandomRectanglePositionAllocator::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::RandomRectanglePositionAllocator")
    .SetParent<PositionAllocator> ()
    .SetGroupName ("Mobility")
    .AddConstructor<RandomRectanglePositionAllocator> ()
    .AddAttribute ("X",
                   "A random variable which represents the x coordinate of a position in a random rectangle.",
                   StringValue ("ns3::UniformRandomVariable[Min=0.0|Max=1.0]"),
                   MakePointerAccessor (&RandomRectanglePositionAllocator::m_x),
                   MakePointerChecker<RandomVariableStream> ())
    .AddAttribute ("Y",
                   "A random variable which represents the y coordinate of a position in a random rectangle.",
                   StringValue ("ns3::UniformRandomVariable[Min=0.0|Max=1.0]"),
                   MakePointerAccessor (&RandomRectanglePositionAllocator::m_y),
                   MakePointerChecker<RandomVariableStream> ())
    .AddAttribute ("Z",
                   "The z coordinate of all the positions allocated.",
                   DoubleValue (0.0),
                   MakeDoubleAccessor (&RandomRectanglePositionAllocator::m_z),
                   MakeDoubleChecker<double> ())
    ;
  return tid;
}

RandomRectanglePositionAllocator::RandomRectanglePositionAllocator ()
{
}
RandomRectanglePositionAllocator::~RandomRectanglePositionAllocator ()
{
}

void
RandomRectanglePositionAllocator::SetX (Ptr<RandomVariableStream> x)
{
  m_x = x;
}
void
RandomRectanglePositionAllocator::SetY (Ptr<RandomVariableStream> y)
{
  m_y = y;
}
void
RandomRectanglePositionAllocator::SetZ (double z)
{
  m_z = z;
}

Vector
RandomRectanglePositionAllocator::GetNext (void) const
{
  double x = m_x->GetValue ();
  double y = m_y->GetValue ();
  return Vector (x, y, m_z);
}

int64_t
RandomRectanglePositionAllocator::AssignStreams (int64_t stream)
{
  m_x->SetStream (stream);
  m_y->SetStream (stream + 1);
  return 2;
}

NS_OBJECT_ENSURE_REGISTERED (RandomBoxPositionAllocator);

TypeId
RandomBoxPositionAllocator::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::RandomBoxPositionAllocator")
    .SetParent<PositionAllocator> ()
    .SetGroupName ("Mobility")
    .AddConstructor<RandomBoxPositionAllocator> ()
    .AddAttribute ("X",
                   "A random variable which represents the x coordinate of a position in a random box.",
                   StringValue ("ns3::UniformRandomVariable[Min=0.0|Max=1.0]"),
                   MakePointerAccessor (&RandomBoxPositionAllocator::m_x),
                   MakePointerChecker<RandomVariableStream> ())
    .AddAttribute ("Y",
                   "A random variable which represents the y coordinate of a position in a random box.",
                   StringValue ("ns3::UniformRandomVariable[Min=0.0|Max=1.0]"),
                   MakePointerAccessor (&RandomBoxPositionAllocator::m_y),
                   MakePointerChecker<RandomVariableStream> ())
    .AddAttribute ("Z",
                   "A random variable which represents the z coordinate of a position in a random box.",
                   StringValue ("ns3::UniformRandomVariable[Min=0.0|Max=1.0]"),
                   MakePointerAccessor (&RandomBoxPositionAllocator::m_z),
                   MakePointerChecker<RandomVariableStream> ());
  return tid;
}

RandomBoxPositionAllocator::RandomBoxPositionAllocator ()
{
}
RandomBoxPositionAllocator::~RandomBoxPositionAllocator ()
{
}

void
RandomBoxPositionAllocator::SetX (Ptr<RandomVariableStream> x)
{
  m_x = x;
}
void
RandomBoxPositionAllocator::SetY (Ptr<RandomVariableStream> y)
{
  m_y = y;
}
void
RandomBoxPositionAllocator::SetZ (Ptr<RandomVariableStream> z)
{
  m_z = z;
}

Vector
RandomBoxPositionAllocator::GetNext (void) const
{
  double x = m_x->GetValue ();
  double y = m_y->GetValue ();
  double z = m_z->GetValue ();
  return Vector (x, y, z);
}

int64_t
RandomBoxPositionAllocator::AssignStreams (int64_t stream)
{
  m_x->SetStream (stream);
  m_y->SetStream (stream + 1);
  m_z->SetStream (stream + 2);
  return 3;
}

NS_OBJECT_ENSURE_REGISTERED (RandomDiscPositionAllocator);

TypeId
RandomDiscPositionAllocator::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::RandomDiscPositionAllocator")
    .SetParent<PositionAllocator> ()
    .SetGroupName ("Mobility")
    .AddConstructor<RandomDiscPositionAllocator> ()
    .AddAttribute ("Theta",
                   "A random variable which represents the angle (gradients) of a position in a random disc.",
                   StringValue ("ns3::UniformRandomVariable[Min=0.0|Max=6.2830]"),
                   MakePointerAccessor (&RandomDiscPositionAllocator::m_theta),
                   MakePointerChecker<RandomVariableStream> ())
    .AddAttribute ("Rho",
                   "A random variable which represents the radius of a position in a random disc.",
                   StringValue ("ns3::UniformRandomVariable[Min=0.0|Max=200.0]"),
                   MakePointerAccessor (&RandomDiscPositionAllocator::m_rho),
                   MakePointerChecker<RandomVariableStream> ())
    .AddAttribute ("X",
                   "The x coordinate of the center of the random position disc.",
                   DoubleValue (0.0),
                   MakeDoubleAccessor (&RandomDiscPositionAllocator::m_x),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("Y",
                   "The y coordinate of the center of the random position disc.",
                   DoubleValue (0.0),
                   MakeDoubleAccessor (&RandomDiscPositionAllocator::m_y),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("Z",
                   "The z coordinate of all the positions in the disc.",
                   DoubleValue (0.0),
                   MakeDoubleAccessor (&RandomDiscPositionAllocator::m_z),
                   MakeDoubleChecker<double> ())
  ;
  return tid;
}

RandomDiscPositionAllocator::RandomDiscPositionAllocator ()
{
}
RandomDiscPositionAllocator::~RandomDiscPositionAllocator ()
{
}

void
RandomDiscPositionAllocator::SetTheta (Ptr<RandomVariableStream> theta)
{
  m_theta = theta;
}
void
RandomDiscPositionAllocator::SetRho (Ptr<RandomVariableStream> rho)
{
  m_rho = rho;
}
void
RandomDiscPositionAllocator::SetX (double x)
{
  m_x = x;
}
void
RandomDiscPositionAllocator::SetY (double y)
{
  m_y = y;
}
void
RandomDiscPositionAllocator::SetZ (double z)
{
  m_z = z;
}
Vector
RandomDiscPositionAllocator::GetNext (void) const
{
  double theta = m_theta->GetValue ();
  double rho = m_rho->GetValue ();
  double x = m_x + std::cos (theta) * rho;
  double y = m_y + std::sin (theta) * rho;
  NS_LOG_DEBUG ("Disc position x=" << x << ", y=" << y);
  return Vector (x, y, m_z);
}

int64_t
RandomDiscPositionAllocator::AssignStreams (int64_t stream)
{
  m_theta->SetStream (stream);
  m_rho->SetStream (stream + 1);
  return 2;
}



NS_OBJECT_ENSURE_REGISTERED (UniformDiscPositionAllocator);

TypeId
UniformDiscPositionAllocator::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::UniformDiscPositionAllocator")
    .SetParent<PositionAllocator> ()
    .SetGroupName ("Mobility")
    .AddConstructor<UniformDiscPositionAllocator> ()
    .AddAttribute ("rho",
                   "The radius of the disc",
                   DoubleValue (0.0),
                   MakeDoubleAccessor (&UniformDiscPositionAllocator::m_rho),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("X",
                   "The x coordinate of the center of the  disc.",
                   DoubleValue (0.0),
                   MakeDoubleAccessor (&UniformDiscPositionAllocator::m_x),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("Y",
                   "The y coordinate of the center of the  disc.",
                   DoubleValue (0.0),
                   MakeDoubleAccessor (&UniformDiscPositionAllocator::m_y),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("Z",
                   "The z coordinate of all the positions in the disc.",
                   DoubleValue (0.0),
                   MakeDoubleAccessor (&UniformDiscPositionAllocator::m_z),
                   MakeDoubleChecker<double> ())
  ;
  return tid;
}

UniformDiscPositionAllocator::UniformDiscPositionAllocator ()
{
  m_rv = CreateObject<UniformRandomVariable> ();
}
UniformDiscPositionAllocator::~UniformDiscPositionAllocator ()
{
}

void
UniformDiscPositionAllocator::SetRho (double rho)
{
  m_rho = rho;
}
void
UniformDiscPositionAllocator::SetX (double x)
{
  m_x = x;
}
void
UniformDiscPositionAllocator::SetY (double y)
{
  m_y = y;
}
void
UniformDiscPositionAllocator::SetZ (double z)
{
  m_z = z;
}
Vector
UniformDiscPositionAllocator::GetNext (void) const
{
  double x,y;
  do
    {
      x = m_rv->GetValue (-m_rho, m_rho);
      y = m_rv->GetValue (-m_rho, m_rho);
    }
  while (std::sqrt (x*x + y*y) > m_rho);

  x += m_x;
  y += m_y;
  NS_LOG_DEBUG ("Disc position x=" << x << ", y=" << y);
  return Vector (x, y, m_z);
}

int64_t
UniformDiscPositionAllocator::AssignStreams (int64_t stream)
{
  m_rv->SetStream (stream);
  return 1;
}


NS_OBJECT_ENSURE_REGISTERED (UniformHexagonPositionAllocator);

TypeId
UniformHexagonPositionAllocator::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::UniformHexagonPositionAllocator")
    .SetParent<PositionAllocator> ()
    .SetGroupName ("Mobility")
    .AddConstructor<UniformHexagonPositionAllocator> ()
    .AddAttribute ("rho",
                   "The radius of the hexagon",
                   DoubleValue (1.0),
                   MakeDoubleAccessor (&UniformHexagonPositionAllocator::m_rho),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("theta",
                   "The orientation angle of the hexagon, in radians, counterclockwise from the positive y-axis",
                   DoubleValue (0.0),
                   MakeDoubleAccessor (&UniformHexagonPositionAllocator::m_theta),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("X",
                   "The x coordinate of the center of the  hexagon.",
                   DoubleValue (0.0),
                   MakeDoubleAccessor (&UniformHexagonPositionAllocator::m_x),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("Y",
                   "The y coordinate of the center of the  hexagon.",
                   DoubleValue (0.0),
                   MakeDoubleAccessor (&UniformHexagonPositionAllocator::m_y),
                   MakeDoubleChecker<double> ())
  ;
  return tid;
}

UniformHexagonPositionAllocator::UniformHexagonPositionAllocator ()
{
  m_rv = CreateObject<UniformRandomVariable> ();
}

UniformHexagonPositionAllocator::~UniformHexagonPositionAllocator ()
{
}


void
UniformHexagonPositionAllocator::PrintToGnuplotFile (std::string filename)
{
  std::ofstream outFile;
  outFile.open (filename.c_str (), std::ios_base::out | std::ios_base::trunc);
  if (!outFile.is_open ())
    {
      NS_LOG_ERROR ("Can't open file " << filename);
      return;
    }

  outFile << "set object 1 polygon from \\\n";

  for (uint32_t vertexId = 0; vertexId < 6; ++vertexId)
    {
      // angle of the vertex w.r.t. y-axis
      double a = vertexId * (M_PI/3.0) + m_theta;
      double x =  - m_rho * sin (a) + m_x;
      double y =  m_rho * cos (a) + m_y;
      outFile << x << ", " << y << " to \\\n";
    }
  // repeat vertex 0 to close polygon
  uint32_t vertexId = 0;
  double a = vertexId * (M_PI/3.0) + m_theta;
  double x =  - m_rho * sin (a) + m_x;
  double y =  m_rho * cos (a) + m_y;
  outFile << x << ", " << y << std::endl;
}


Vector
UniformHexagonPositionAllocator::GetNext (void) const
{
  NS_LOG_FUNCTION (this);

  Vector p;
  do
    {
      p.x = m_rv->GetValue (-m_rho, m_rho);
      p.y = m_rv->GetValue (-m_rho, m_rho);
      NS_LOG_LOGIC ("new random point: " << p << ", m_rho=" << m_rho);
    }
  while (!IsInsideCenteredNoRotation (p));

  // rotate and offset
  Vector p2;

  p2.x = p.x * cos (m_theta) - p.y * sin (m_theta);
  p2.y = p.x * sin (m_theta) + p.y * cos (m_theta);

  p2.x += m_x;
  p2.y += m_y;

  NS_LOG_DEBUG ("Hexagon position x=" << p2.x << ", y=" << p2.y);
  return p2;
}

int64_t
UniformHexagonPositionAllocator::AssignStreams (int64_t stream)
{
  m_rv->SetStream (stream);
  return 1;
}

bool
UniformHexagonPositionAllocator::IsInsideCenteredNoRotation (Vector q) const
{
  NS_LOG_FUNCTION (this << q);
  // method from http://www.playchilla.com/how-to-check-if-a-point-is-inside-a-hexagon

  // rotate to positive quadrant
  Vector q2 (std::abs (q.x), std::abs (q.y), 0);

  double v = m_rho / 2;
  double h = m_rho * cos (M_PI/6.0);

  // check bounding box
  if ((q2.x > h) || (q2.y > (2*v)))
    {
      return false;
    }

  // check dot product
  double dotProduct = (2*v*h - v*q2.x - h*q2.y);
  NS_LOG_LOGIC ("dot product = " << dotProduct);
  return (dotProduct >= 0);
}




NS_OBJECT_ENSURE_REGISTERED (Min2dDistancePositionAllocator);

TypeId
Min2dDistancePositionAllocator::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::Min2dDistancePositionAllocator")
    .SetParent<PositionAllocator> ()
    .SetGroupName ("Mobility")
    .AddConstructor<Min2dDistancePositionAllocator> ()
    .AddAttribute ("MaxAttempts",
                   "Maximum number of attempts to get a position satisfying the min 2D distance before giving up",
                   UintegerValue (1000),
                   MakeUintegerAccessor (&Min2dDistancePositionAllocator::m_maxAttempts),
                   MakeUintegerChecker<uint32_t> ())
  ;
  return tid;
}

Min2dDistancePositionAllocator::Min2dDistancePositionAllocator ()
{
}

Min2dDistancePositionAllocator::~Min2dDistancePositionAllocator ()
{
}

Vector
Min2dDistancePositionAllocator::GetNext (void) const
{
  bool satisfiesMin2dDistance;
  Vector p1;
  uint32_t attempts = 0;
  do
    {
      ++attempts;
      if (attempts > m_maxAttempts)
        {
          NS_FATAL_ERROR ("too many failed attempts, please revise your distance constraints");
        }

      satisfiesMin2dDistance = true;
      p1 = m_positionAllocator->GetNext ();
      Vector2D p12d (p1.x, p1.y);

      for (std::list<Min2dDistancePositionAllocator::NodesDistance>::const_iterator it
             = m_nodesDistanceList.begin ();
           satisfiesMin2dDistance && it != m_nodesDistanceList.end ();
           ++it)
        {
          for (NodeContainer::Iterator ncit = it->nodes.Begin ();
               satisfiesMin2dDistance && ncit != it->nodes.End ();
               ++ncit)
            {
              Vector p2 = (*ncit)->GetObject<MobilityModel> ()->GetPosition ();
              Vector2D p22d (p2.x, p2.y);
              double dist = CalculateDistance (p12d, p22d);
              satisfiesMin2dDistance &= (dist >= it->distance);
            }
        }

      for (std::list<Min2dDistancePositionAllocator::PositionDistance>::const_iterator it
             = m_positionDistanceList.begin ();
           satisfiesMin2dDistance && it != m_positionDistanceList.end ();
           ++it)
        {
          Vector2D p22d (it->position.x, it->position.y);
          double dist = CalculateDistance (p12d, p22d);
          satisfiesMin2dDistance &= (dist >= it->distance);
        }
    }
  while (!satisfiesMin2dDistance);
  return p1;
}

int64_t
Min2dDistancePositionAllocator::AssignStreams (int64_t stream)
{
  return m_positionAllocator->AssignStreams (stream);
}


void
Min2dDistancePositionAllocator::SetPositionAllocator (Ptr<PositionAllocator> p)
{
  m_positionAllocator = p;
}

void
Min2dDistancePositionAllocator::AddNodesDistance (NodeContainer nodes, double distance)
{
  Min2dDistancePositionAllocator::NodesDistance nd;
  nd.nodes = nodes;
  nd.distance = distance;
  m_nodesDistanceList.push_back (nd);
}

void
Min2dDistancePositionAllocator::AddPositionDistance (Vector position, double distance)
{
  Min2dDistancePositionAllocator::PositionDistance nd;
  nd.position = position;
  nd.distance = distance;
  m_positionDistanceList.push_back (nd);
}


} // namespace ns3 
