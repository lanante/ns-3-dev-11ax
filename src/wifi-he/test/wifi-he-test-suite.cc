/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

// This is a placeholder for wifi-he tests.
#include "ns3/test.h"

using namespace ns3;

// This is an example TestCase.
class WifiHeTestCase1 : public TestCase
{
public:
  WifiHeTestCase1 ();
  virtual ~WifiHeTestCase1 ();

private:
  virtual void DoRun (void);
};

// Add some help text to this case to describe what it is intended to test
WifiHeTestCase1::WifiHeTestCase1 ()
  : TestCase ("WifiHe test case (does nothing)")
{
}

// This destructor does nothing but we include it as a reminder that
// the test case should clean up after itself
WifiHeTestCase1::~WifiHeTestCase1 ()
{
}

//
// This method is the pure virtual method from class TestCase that every
// TestCase must implement
//
void
WifiHeTestCase1::DoRun (void)
{
  // A wide variety of test macros are available in src/core/test.h
  NS_TEST_ASSERT_MSG_EQ (true, true, "true doesn't equal true for some reason");
  // Use this one for floating point comparisons
  NS_TEST_ASSERT_MSG_EQ_TOL (0.01, 0.01, 0.001, "Numbers are not equal within tolerance");
}

// The TestSuite class names the TestSuite, identifies what type of TestSuite,
// and enables the TestCases to be run.  Typically, only the constructor for
// this class must be defined
//
class WifiHeTestSuite : public TestSuite
{
public:
  WifiHeTestSuite ();
};

WifiHeTestSuite::WifiHeTestSuite ()
  : TestSuite ("wifi-he", UNIT)
{
  // TestDuration for TestCase can be QUICK, EXTENSIVE or TAKES_FOREVER
  AddTestCase (new WifiHeTestCase1, TestCase::QUICK);
}

// Do not forget to allocate an instance of this TestSuite
static WifiHeTestSuite wifiHeTestSuite;

