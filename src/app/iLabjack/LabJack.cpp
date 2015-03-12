/************************************************************/
/*    FILE: LabJack.cpp
/*    ORGN: ENSTA Bretagne
/*    AUTH: Clément Aubry
/*    DATE: 2015
/************************************************************/

#include <iterator>
#include "MBUtils.h"
#include "ACTable.h"
#include "LabJack.h"

using namespace std;

//---------------------------------------------------------
// Constructor

LabJack::LabJack()
{
}

//---------------------------------------------------------
// Procedure: OnNewMail

bool LabJack::OnNewMail(MOOSMSG_LIST &NewMail)
{
  AppCastingMOOSApp::OnNewMail(NewMail);

  MOOSMSG_LIST::iterator p;
  for(p = NewMail.begin() ; p != NewMail.end() ; p++)
  {
    CMOOSMsg &msg = *p;
    string key    = msg.GetKey();

    #if 0 // Keep these around just for template
      string comm  = msg.GetCommunity();
      double dval  = msg.GetDouble();
      string sval  = msg.GetString();
      string msrc  = msg.GetSource();
      double mtime = msg.GetTime();
      bool   mdbl  = msg.IsDouble();
      bool   mstr  = msg.IsString();
    #endif
      reportEvent("iLabjack: OnNewMail: " + key);

      if(key == "SET_FIO0_STATE")
      {
        reportEvent("iLabjack: Calling eDO to set FIO0 state");
      //long eDO( DeviceHandle,ForceConfigIO?,ChannelNumber,State)
        if( (error = eDO(hDevice, 1, 0, msg.GetDouble())) != 0 )
        {
          reportRunWarning("iLabjack : received an error code calling eDO onFIO0 " + error);
          closeUSBConnection(hDevice);
        }
        else
          MOOSTrace("iLabjack: FIO0 correctly setted to %d",msg.GetDouble())

      }
      if(key == "SET_FIO1_STATE")
      {
        reportEvent("iLabjack: Calling eDO to set FIO1 state");
      //long eDO( DeviceHandle,ForceConfigIO?,ChannelNumber,State)
        if( (error = eDO(hDevice, 1, 1, msg.GetDouble())) != 0 )
        {
          reportRunWarning("iLabjack : received an error code calling eDO onFIO1 " + error);
          closeUSBConnection(hDevice);
        }
        else
          MOOSTrace("iLabjack: FIO1 correctly setted to %d",msg.GetDouble())
      }
      if(key == "SET_FIO2_STATE")
      {
        reportEvent("iLabjack: Calling eDO to set FIO2 state");
      //long eDO( DeviceHandle,ForceConfigIO?,ChannelNumber,State)
        if( (error = eDO(hDevice, 1, 2, msg.GetDouble())) != 0 )
        {
          reportRunWarning("iLabjack : received an error code calling eDO onFIO2 " + error);
          closeUSBConnection(hDevice);
        }
        else
          MOOSTrace("iLabjack: FIO2 correctly setted to %d",msg.GetDouble())
      }
      if(key == "SET_FIO3_STATE")
      {
        reportEvent("iLabjack: Calling eDO to set FIO3 state");
      //long eDO( DeviceHandle,ForceConfigIO?,ChannelNumber,State)
        if( (error = eDO(hDevice, 1, 3, msg.GetDouble())) != 0 )
        {
          reportRunWarning("iLabjack : received an error code calling eDO onFIO3 " + error);
          closeUSBConnection(hDevice);
        }
        else
          MOOSTrace("iLabjack: FIO3 correctly setted to %d",msg.GetDouble())
      }
      if(key == "SET_FIO4_STATE")
      {
        reportEvent("iLabjack: Calling eDO to set FIO4 state");
      //long eDO( DeviceHandle,ForceConfigIO?,ChannelNumber,State)
        if( (error = eDO(hDevice, 1, 4, msg.GetDouble())) != 0 )
        {
          reportRunWarning("iLabjack : received an error code calling eDO onFIO4 " + error);
          closeUSBConnection(hDevice);
        }
        else
          MOOSTrace("iLabjack: FIO4 correctly setted to %d",msg.GetDouble())
      }
      if(key == "SET_FIO5_STATE")
      {
        reportEvent("iLabjack: Calling eDO to set FIO5 state");
      //long eDO( DeviceHandle,ForceConfigIO?,ChannelNumber,State)
        if( (error = eDO(hDevice, 1, 5, msg.GetDouble())) != 0 )
        {
          reportRunWarning("iLabjack : received an error code calling eDO onFIO5 " + error);
          closeUSBConnection(hDevice);
        }
        else
          MOOSTrace("iLabjack: FIO5 correctly setted to %d",msg.GetDouble())
      }
      if(key == "SET_FIO6_STATE")
      {
        reportEvent("iLabjack: Calling eDO to set FIO6 state");
      //long eDO( DeviceHandle,ForceConfigIO?,ChannelNumber,State)
        if( (error = eDO(hDevice, 1, 6, msg.GetDouble())) != 0 )
        {
          reportRunWarning("iLabjack : received an error code calling eDO onFIO6 " + error);
          closeUSBConnection(hDevice);
        }
        else
          MOOSTrace("iLabjack: FIO6 correctly setted to %d",msg.GetDouble())
      }
      if(key == "SET_FIO7_STATE")
      {
        reportEvent("iLabjack: Calling eDO to set FIO7 state");
      //long eDO( DeviceHandle,ForceConfigIO?,ChannelNumber,State)
        if( (error = eDO(hDevice, 1, 7, msg.GetDouble())) != 0 )
        {
          reportRunWarning("iLabjack : received an error code calling eDO onFIO7 " + error);
          closeUSBConnection(hDevice);
        }
        else
          MOOSTrace("iLabjack: FIO7 correctly setted to %d",msg.GetDouble())
      }

    else if(key != "APPCAST_REQ") // handle by AppCastingMOOSApp
      reportRunWarning("iLabjack: Unhandled Mail: " + key);
  }

  return true;
}

//---------------------------------------------------------
// Procedure: OnConnectToServer

bool LabJack::OnConnectToServer()
{
  registerVariables();
  return true;
}

//---------------------------------------------------------
// Procedure: Iterate()
//            happens AppTick times per second

bool LabJack::Iterate()
{
  AppCastingMOOSApp::Iterate();


  //Test Labjack connection by getting calibrations infos
  if( getCalibrationInfo(hDevice, &caliInfo) < 0 )
  {
    reportRunWarning("iLabjack: Received an error code of " + error);
    closeUSBConnection(hDevice);
    reportEvent("iLabjack: Labjack USB disconnected");
    //Open first found U3 over USB
    if( (hDevice = openUSBConnection(localID)) == NULL )
    {
      reportRunWarning("iLabjack: Error openning Labjack USB connexion");
      return 0;
    }
    reportEvent("iLabjack: Labjack USB re-connexion success");
  }

  AppCastingMOOSApp::PostReport();
  return true;
}

//---------------------------------------------------------
// Procedure: OnStartUp()
//            happens before connection is open

bool LabJack::OnStartUp()
{
  AppCastingMOOSApp::OnStartUp();

  STRING_LIST sParams;
  m_MissionReader.EnableVerbatimQuoting(false);
  if(!m_MissionReader.GetConfiguration(GetAppName(), sParams))
    reportConfigWarning("iLabjack: No config block found for " + GetAppName());

  STRING_LIST::iterator p;
  sParams.reverse();
  for(p = sParams.begin() ; p != sParams.end() ; p++)
  {
    string orig  = *p;
    string line  = *p;
    string param = toupper(biteStringX(line, '='));
    string value = line;
    bool handled = false;

    if(param == "FOO")
    {
      handled = true;
    }

    else if(param == "BAR")
    {
      handled = true;
    }

    if(!handled)
      reportUnhandledConfigWarning(orig);
  }

  //Open first found U3 over USB
  localID = -1;
  if( (hDevice = openUSBConnection(localID)) == NULL )
  {
    reportConfigWarning("iLabjack: Error openning Labjack USB connexion");
    return 0;
  }

  //Get calibration information from U3
  if( getCalibrationInfo(hDevice, &caliInfo) < 0 )
  {
    if( error > 0 )
      reportConfigWarning("iLabjack: Received an error code of " + error);
    closeUSBConnection(hDevice);
    return 0;
  }

  registerVariables();
  return true;
}

//---------------------------------------------------------
// Procedure: registerVariables

void LabJack::registerVariables()
{
  AppCastingMOOSApp::RegisterVariables();
  Register("SET_FIO0_STATE", 0);
  Register("SET_FIO1_STATE", 0);
  Register("SET_FIO2_STATE", 0);
  Register("SET_FIO3_STATE", 0);
  Register("SET_FIO4_STATE", 0);
  Register("SET_FIO5_STATE", 0);
  Register("SET_FIO6_STATE", 0);
  Register("SET_FIO7_STATE", 0);
  Register("FOO", 0);
  // Register("FOOBAR", 0);
}

//------------------------------------------------------------
// Procedure: buildReport()

bool LabJack::buildReport()
{
  #if 0 // Keep these around just for template
    m_msgs << "============================================ \n";
    m_msgs << "File:                                        \n";
    m_msgs << "============================================ \n";

    ACTable actab(4);
    actab << "Alpha | Bravo | Charlie | Delta";
    actab.addHeaderLines();
    actab << "one" << "two" << "three" << "four";
    m_msgs << actab.getFormattedString();
  #endif

    return true;
  }

/** Posting messages :
reportEvent("Good msg received: " + message);
reportRunWarning("Bad msg received: " + message);
reportConfigWarning("Problem configuring FOOBAR. Expected a number but got: " + str);
reportUnhandledConfigWarning(original_full_config_line); //voir dans OnStartUp()
*/
