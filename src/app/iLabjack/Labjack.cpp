/************************************************************/
/*    FILE: Labjack.cpp
/*    ORGN: ENSTA Bretagne
/*    AUTH: Clément Aubry
/*    DATE: 2015
/************************************************************/

#include <iterator>
#include "MBUtils.h"
#include "ACTable.h"
#include "Labjack.h"
#include <string>

using namespace std;

//---------------------------------------------------------
// Constructor

Labjack::Labjack()
{
  m_iFioState[0] = 0;
  m_iFioState[1] = 0;
  m_iFioState[2] = 0;
  m_iFioState[3] = 0;
  m_iFioState[4] = 0;
  m_iFioState[5] = 0;
  m_iFioState[6] = 0;
  m_iFioState[7] = 0;
}

//---------------------------------------------------------
// Procedure: OnNewMail

bool Labjack::OnNewMail(MOOSMSG_LIST &NewMail)
{
  AppCastingMOOSApp::OnNewMail(NewMail);
  char buffer[200];

  MOOSMSG_LIST::iterator p;
  for(p = NewMail.begin() ; p != NewMail.end() ; p++)
  {
    CMOOSMsg &msg = *p;
    string key    = msg.GetKey();

    // reportEvent("iLabjack: OnNewMail: " + key);

    if(key == "SET_FIOX_LOW")
    {
      for (unsigned int k=0;k <=7;k++){
        m_iFioState[k] = 0;
        if( (error = eDO(hDevice, 1, k, 0)) != 0 )
        {
          reportRunWarning("iLabjack::NewMail::SET_FIOX_LOW : received an error code calling eDO on FIOX " + error);
          closeUSBConnection(hDevice);
        }
      }
    }
    else if(key == "SET_FIOX_HIGH")
    {
      for (unsigned int k=0;k <=7;k++){
        m_iFioState[k] = 1;
        if( (error = eDO(hDevice, 1, k, 1)) != 0 )
        {
          reportRunWarning("iLabjack::NewMail::SET_FIOX_HIGH : received an error code calling eDO on FIOX " + error);
          closeUSBConnection(hDevice);
        }
      }
    }
    else if(key == "SET_FIOX_STATE")
    {
      int fioToSet = -1;
      int valueToSet = -1;
      if(!MOOSValFromString(fioToSet, msg.GetString(), "FIO"))
        reportRunWarning(msg.GetKey() + ": Unable to find 'channel' parameter");
      else if(!MOOSValFromString(valueToSet, msg.GetString(), "VALUE"))
        reportRunWarning(msg.GetKey() + ": Unable to find 'value' parameter");
      // sprintf (buffer, "iLabjack: FIO%d correctly setted to %d",fioToSet, m_iFioState[fioToSet]);
      // retractRunWarning(buffer);
      // sprintf (buffer, "iLabjack: Calling eDO to setFIO%d state to %d",fioToSet, m_iFioState[fioToSet]);
      // retractEvent(buffer);
      if (valueToSet >= 0 && valueToSet <=1 && fioToSet >= 0 && fioToSet <= 7)
      {
        sprintf (buffer, "iLabjack: Calling eDO to setFIO%d state to %d",fioToSet, valueToSet);
        reportEvent(buffer);
          //long eDO( DeviceHandle,ForceConfigIO?,ChannelNumber,State)
        if( (error = eDO(hDevice, 1, fioToSet, valueToSet)) != 0 )
        {
          reportRunWarning("iLabjack : received an error code calling eDO on FIOX " + error);
          closeUSBConnection(hDevice);
        }
        else
        {
          sprintf (buffer, "iLabjack: FIO%d correctly setted to %d",fioToSet, valueToSet);
          reportEvent(buffer);
          m_iFioState[fioToSet] = valueToSet;
          Notify("FIOX_STATE", msg.GetString());
        }
      }
    }
    else if(key == "SET_FIOX_INPUT")
    {
      int fioToSet = -1;
      if(!MOOSValFromString(fioToSet, msg.GetString(), "FIO"))
        reportRunWarning(msg.GetKey() + ": Unable to find 'channel' parameter");
      if (fioToSet >= 0 && fioToSet <= 7)
      {
        sprintf (buffer, "iLabjack: FIO%d correctly setted to %d",fioToSet, m_iFioState[fioToSet]);
        retractRunWarning(buffer);
        sprintf (buffer, "iLabjack: Calling eDI to set FIO%d as input and read state",fioToSet);
        reportRunWarning(buffer);

        long lngState;
        if( (error = eDI(hDevice, 1, fioToSet, &lngState)) != 0 )
        {
          sprintf (buffer, "iLabjack: received an error code calling eDI on FIO%d, error code : %ld",fioToSet, error);
          reportRunWarning(buffer);
          closeUSBConnection(hDevice);
        }
        else
        {
          sprintf (buffer, "iLabjack: FIO%d setted as input, read state = %ld\n",fioToSet, lngState);
          reportRunWarning(buffer);
          sprintf (buffer, "FIO=%d,VALUE=0\n",fioToSet);
          Notify("FIOX_STATE", buffer);
        }
      }
    }
    else if(key != "APPCAST_REQ") // handle by AppCastingMOOSApp
      reportRunWarning("iLabjack: Unhandled Mail: " + key);
  }

  return true;
}

//---------------------------------------------------------
// Procedure: OnConnectToServer

bool Labjack::OnConnectToServer()
{
  registerVariables();
  return true;
}

//---------------------------------------------------------
// Procedure: Iterate()
//            happens AppTick times per second

bool Labjack::Iterate()
{
  AppCastingMOOSApp::Iterate();


  //Test Labjack connection by getting calibrations infos
  if( getCalibrationInfo(hDevice, &caliInfo) < 0 )
  {
    reportRunWarning("iLabjack: Received an error code of " + error);
    closeUSBConnection(hDevice);
    reportRunWarning("iLabjack: Labjack USB disconnected");
    //Open first found U3 over USB
    if( (hDevice = openUSBConnection(localID)) == NULL )
    {
      reportRunWarning("iLabjack: Error openning Labjack USB connexion");
    }
    else
    {
      reportEvent("iLabjack: Labjack USB re-connexion success");
      retractRunWarning("iLabjack: Error openning Labjack USB connexion");
      retractRunWarning("iLabjack: Labjack USB disconnected");
    }
  }

  AppCastingMOOSApp::PostReport();
  return true;
}

//---------------------------------------------------------
// Procedure: OnStartUp()
//            happens before connection is open

bool Labjack::OnStartUp()
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
  }

  //Get calibration information from U3
  if( getCalibrationInfo(hDevice, &caliInfo) < 0 )
  {
    if( error > 0 )
      reportConfigWarning("iLabjack: Received an error code of " + error);
    closeUSBConnection(hDevice);
  }

  registerVariables();
  return true;
}

//---------------------------------------------------------
// Procedure: registerVariables

void Labjack::registerVariables()
{
  AppCastingMOOSApp::RegisterVariables();
  Register("SET_FIOX_STATE", 0);
  Register("SET_FIOX_INPUT", 0);
  Register("SET_FIOX_LOW", 0);
  Register("SET_FIOX_HIGH", 0);
  // Register("FOOBAR", 0);
}

//------------------------------------------------------------
// Procedure: buildReport()

bool Labjack::buildReport()
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
    m_msgs << "==============================================================\n";
    m_msgs << "iLabjack FIO values :                                         \n";
    m_msgs << "==============================================================\n";

    ACTable actab(8);
    actab << "FIO0 | FIO1 | FIO2 | FIO3 | FIO4 | FIO5 | FIO6 | FIO7";
    actab.addHeaderLines();
    actab << m_iFioState[0] << m_iFioState[1] << m_iFioState[2] << m_iFioState[3] << m_iFioState[4] << m_iFioState[5] << m_iFioState[6] << m_iFioState[7];
    m_msgs << actab.getFormattedString();

    return true;
  }

/** Posting messages :
reportEvent("Good msg received: " + message);
reportRunWarning("Bad msg received: " + message);
reportConfigWarning("Problem configuring FOOBAR. Expected a number but got: " + str);
reportUnhandledConfigWarning(original_full_config_line); //voir dans OnStartUp()
*/
