/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2010 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
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
 * Modified by : Marco Miozzo <mmiozzo@cttc.es>
 *        (move from CQI to Ctrl and Data SINR Chunk processors)
 * Modified by : Piotr Gawlowicz <gawlowicz.p@gmail.com>
 *        (removed all Lte***ChunkProcessor implementations
 *        and created generic LteChunkProcessor)
 * Modified by : Nicola Baldo <nbaldo@cttc.es>
 *        (specialized into average and MI chunk processors)
 */


#include <ns3/log.h>
#include <ns3/spectrum-value.h>
#include "lte-chunk-processor.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("LteChunkProcessor");

LteChunkProcessor::LteChunkProcessor ()
{
  NS_LOG_FUNCTION (this);
}

LteChunkProcessor::~LteChunkProcessor ()
{
  NS_LOG_FUNCTION (this);
}

void
LteAverageChunkProcessor::AddCallback (LteAverageChunkProcessorCallback c)
{
  NS_LOG_FUNCTION (this);
  m_lteAverageChunkProcessorCallbacks.push_back (c);
}

void
LteAverageChunkProcessor::Start ()
{
  NS_LOG_FUNCTION (this);
  m_sumValues = 0;
  m_totDuration = MicroSeconds (0);
}


void
LteAverageChunkProcessor::EvaluateChunk (const SpectrumValue& sinr, Time duration)
{
  NS_LOG_FUNCTION (this << sinr << duration);
  if (m_sumValues == 0)
    {
      m_sumValues = Create<SpectrumValue> (sinr.GetSpectrumModel ());
    }
  (*m_sumValues) += sinr * duration.GetSeconds ();
  m_totDuration += duration;
}

void
LteAverageChunkProcessor::End ()
{
  NS_LOG_FUNCTION (this);
  if (m_totDuration.GetSeconds () > 0)
    {
      std::list<LteAverageChunkProcessorCallback>::iterator it;
      for (it = m_lteAverageChunkProcessorCallbacks.begin (); it != m_lteAverageChunkProcessorCallbacks.end (); it++)
        {
          (*it)((*m_sumValues) / m_totDuration.GetSeconds ());
        }
    }
  else
    {
      NS_LOG_WARN ("no values to report");
    }
}



LteListChunkProcessor::Chunk::Chunk (Ptr<SpectrumValue> spectrumValue, Time duration)
  : m_spectrumValue (spectrumValue),
    m_duration (duration)
{
}

LteListChunkProcessor::Chunk::Chunk ()
{
}

void
LteListChunkProcessor::AddCallback (LteListChunkProcessorCallback c)
{
  NS_LOG_FUNCTION (this);
  m_lteListChunkProcessorCallbacks.push_back (c);
}

void
LteListChunkProcessor::Start ()
{
  NS_LOG_FUNCTION (this);
}

void
LteListChunkProcessor::EvaluateChunk (const SpectrumValue& sinr, Time duration)
{
  NS_LOG_FUNCTION (this << sinr << duration);
  m_chunkList.push_back (Chunk (sinr.Copy (), duration));
}

void
LteListChunkProcessor::End ()
{
  NS_LOG_FUNCTION (this);
  if (!m_chunkList.empty ())
    {
      NS_LOG_FUNCTION (this << "reporting " << m_chunkList.size () << " chunks");
      std::list<LteListChunkProcessorCallback>::iterator it;
      for (it = m_lteListChunkProcessorCallbacks.begin (); it != m_lteListChunkProcessorCallbacks.end (); it++)
        {
          (*it)(m_chunkList);
        }
      m_chunkList.clear ();
    }
  else
    {
      NS_LOG_WARN ("no values to report");
    }
}


  
void
LteSpectrumValueCatcher::ReportValue (const SpectrumValue& value)
{
  m_value = value.Copy ();
}

Ptr<SpectrumValue> 
LteSpectrumValueCatcher::GetValue ()
{
  return m_value;
}


} // namespace ns3
