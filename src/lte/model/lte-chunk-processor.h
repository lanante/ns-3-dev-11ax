/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2009, 2010 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
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


#ifndef LTE_CHUNK_PROCESSOR_H
#define LTE_CHUNK_PROCESSOR_H

#include <ns3/ptr.h>
#include <ns3/nstime.h>
#include <ns3/object.h>

namespace ns3 {

class SpectrumValue;

/** 
 * This abstract class is used to process the time-vs-frequency
 * SINR/interference/power chunk of a received LTE signal
 * which was calculated by the LteInterference object.
 */
class LteChunkProcessor : public SimpleRefCount<LteChunkProcessor>
{
public:
  LteChunkProcessor ();
  virtual ~LteChunkProcessor ();

  /**
    * \brief Clear internal variables
    *
    * This function clears internal variables in the beginning of
    * calculation
    */
  virtual void Start () = 0;

  /**
    * \brief Collect SpectrumValue and duration of signal
    *
    * Passed values are collected in m_sumValues and m_totDuration variables.
    *
    * \param sinr the SINR
    * \param duration the duration
    */
  virtual void EvaluateChunk (const SpectrumValue& sinr, Time duration) = 0;

  /**
    * \brief Finish calculation and inform interested objects about calculated value
    *
    * During this function all callbacks from list are executed
    * to inform interested object about calculated value. This
    * function is called at the end of calculation.
    */
  virtual void End () = 0;

};

/**
 * Callback type used by LteAverageChunkProcessor for reporting
 *
 */
typedef Callback< void, const SpectrumValue& > LteAverageChunkProcessorCallback;

/**
 * An implementation of LteChunkProcessor that aggregates SpectrumValues
 * in the time domain by performing a linear average weighted by the
 * duration associated with each SpectrumValue
 *
 */
class LteAverageChunkProcessor : public LteChunkProcessor
{
public:

  /**
    * \brief Add callback to list
    *
    * This function adds callback c to list. Each callback pass
    * calculated value to its object and is called in
    * LteChunkProcessor::End ().
    */
  virtual void AddCallback (LteAverageChunkProcessorCallback c);

  // inherited from LteChunkProcessor
  virtual void Start ();
  virtual void EvaluateChunk (const SpectrumValue& sinr, Time duration);
  virtual void End ();

private:
  Ptr<SpectrumValue> m_sumValues; ///< sum values
  Time m_totDuration; ///< total duration

  std::list<LteAverageChunkProcessorCallback> m_lteAverageChunkProcessorCallbacks; ///< chunk processor callback
};



/**
 * An implementation of LteChunkProcessor that aggregates SpectrumValues
 * in the time domain by storing them in a list together with their associated
 * duration
 *
 */
class LteListChunkProcessor : public LteChunkProcessor
{
public:

  /**
   * Data structure holding a SpectrumValue and its duration in time
   */
  struct Chunk
  {
    /**
     * constructor
     *
     * \param spectrumValue
     * \param duration
     */
    Chunk (Ptr<SpectrumValue> spectrumValue, Time duration);

    /**
     * (unused) constructor; requires definition to allow Python API scanning
     */
    Chunk ();
    Ptr<SpectrumValue> m_spectrumValue;
    Time m_duration;
  };


  /**
   * Callback type used by LteListChunkProcessor for reporting
   *
   */
  typedef Callback< void, std::list<Chunk> > LteListChunkProcessorCallback;

  /**
    * \brief Add callback to list
    *
    * This function adds callback c to list. Each callback pass
    * calculated value to its object and is called in
    * LteChunkProcessor::End ().
    */
  virtual void AddCallback (LteListChunkProcessorCallback c);

  // inherited from LteChunkProcessor
  virtual void Start ();
  virtual void EvaluateChunk (const SpectrumValue& sinr, Time duration);
  virtual void End ();

private:

  std::list<Chunk> m_chunkList;
  std::list<LteListChunkProcessorCallback> m_lteListChunkProcessorCallbacks; ///< chunk processor callback
};


/**
 * A sink to be plugged to the callback of LteChunkProcessor allowing
 * to save and later retrieve the latest reported value 
 * 
 */
class LteSpectrumValueCatcher
{
public:

  /** 
   * function to be plugged to LteChunkProcessor::AddCallback ()
   * 
   * \param value 
   */
  void ReportValue (const SpectrumValue& value);

  /** 
   * 
   * 
   * \return the latest value reported by the LteChunkProcessor
   */
  Ptr<SpectrumValue> GetValue ();
  
private:
  Ptr<SpectrumValue> m_value; ///< spectrum value
};

} // namespace ns3



#endif /* LTE_CHUNK_PROCESSOR_H */
