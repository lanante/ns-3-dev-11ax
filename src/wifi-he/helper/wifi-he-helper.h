/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#ifndef WIFI_HE_HELPER_H
#define WIFI_HE_HELPER_H

#include "ns3/trace-helper.h"
#include "ns3/wifi-phy.h"
#include "ns3/he-configuration.h"

namespace ns3 {

/**
 * \brief helps to create ObssPdAlgorithm objects that are associated with wifi nodes
 *
 * This class can help to create a large set of similar
 * WifiHe objects and to configure a large set of
 * their attributes during creation.
 */
class WifiHeHelper
{
public:
  virtual ~WifiHeHelper ();

  /**
   * Create a Wifi-he helper in an empty state: all its parameters
   * must be set before calling ns3::WifiHeHelper::Install
   *
   */
  WifiHeHelper ();

  /**
   * \param type the type of ns3::ObssPdAlgorithm to create.
   * \param n0 the name of the attribute to set
   * \param v0 the value of the attribute to set
   * \param n1 the name of the attribute to set
   * \param v1 the value of the attribute to set
   * \param n2 the name of the attribute to set
   * \param v2 the value of the attribute to set
   * \param n3 the name of the attribute to set
   * \param v3 the value of the attribute to set
   * \param n4 the name of the attribute to set
   * \param v4 the value of the attribute to set
   * \param n5 the name of the attribute to set
   * \param v5 the value of the attribute to set
   * \param n6 the name of the attribute to set
   * \param v6 the value of the attribute to set
   * \param n7 the name of the attribute to set
   * \param v7 the value of the attribute to set
   *
   * All the attributes specified in this method should exist
   * in the requested ObssPdAlgorithm.
   */
  void SetObssPdAlgorithm (std::string type,
                           std::string n0 = "", const AttributeValue &v0 = EmptyAttributeValue (),
                           std::string n1 = "", const AttributeValue &v1 = EmptyAttributeValue (),
                           std::string n2 = "", const AttributeValue &v2 = EmptyAttributeValue (),
                           std::string n3 = "", const AttributeValue &v3 = EmptyAttributeValue (),
                           std::string n4 = "", const AttributeValue &v4 = EmptyAttributeValue (),
                           std::string n5 = "", const AttributeValue &v5 = EmptyAttributeValue (),
                           std::string n6 = "", const AttributeValue &v6 = EmptyAttributeValue (),
                           std::string n7 = "", const AttributeValue &v7 = EmptyAttributeValue ());

  /**
   * \param n0 the name of the attribute to set
   * \param v0 the value of the attribute to set
   *
   * Set a Wifi-He configuration attribute
   */
  void SetHeConfigurationAttribute (std::string n0 = "", const AttributeValue &v0 = EmptyAttributeValue ());

  /**
   * \param c the set of nodes on which a wifi device must be created
   */
  void
  virtual Install (NodeContainer c) const;

protected:
  ObjectFactory m_heConfiguration; ///< HE configuration
  ObjectFactory m_algorithm; ///< OBSS PD algorithm

};

} //namespace ns3


#endif /* WIFI_HE_HELPER_H */

