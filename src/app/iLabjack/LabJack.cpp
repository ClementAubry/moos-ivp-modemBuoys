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
#include <string>

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

      if(key == "SET_FIOX_STATE") //This data contain "FIO=X;VALUE=Y;"
      {
        string strMsg = msg.GetString();
        int fioToSet = -1;
        int valueToSet = -1;
        // 1) parse string message to knwow which FIO to set
        std::size_t fioStart = strMsg.find("FIO=");
        std::size_t fioEnd = strMsg.find(";");
        if (fioStart!=std::string::npos && fioEnd!=std::string::npos)
        {
          char buffer[16];
          std::size_t length = strMsg.copy(buffer,fioEnd-fioStart+4,fioStart+4);
          buffer[length]='\0';
          fioToSet = atoi(buffer);
        }
        std::size_t valueStart = strMsg.find("VALUE=");
        if (valueStart!=std::string::npos)
        {
          char buffer[16];
          std::size_t length = strMsg.copy(buffer,1,valueStart+6);
          buffer[length]='\0';
          valueToSet = atoi(buffer);
        }
MOOSTrace("iLabjack: reading SET_FIOX_STATE variable. value : [%s], extracted FIO number = [%d], extracted FIO value = [%d]\n",msg.GetString().c_str(),fioToSet, valueToSet);
        if (valueToSet >= 0 && valueToSet <=1 && fioToSet >= 0 && fioToSet <= 7)
        {
          reportEvent("iLabjack: Calling eDO to set FIO state");
          //long eDO( DeviceHandle,ForceConfigIO?,ChannelNumber,State)
          if( (error = eDO(hDevice, 1, fioToSet, valueToSet)) != 0 )
          {
            reportRunWarning("iLabjack : received an error code calling eDO on FIOX " + error);
            closeUSBConnection(hDevice);
          }
          else
          {
            MOOSTrace("iLabjack: FIO%d correctly setted to %d",fioToSet, valueToSet);
            Notify("FIOX_STATE", msg.GetString());
          }
        }
        else
        {
          MOOSTrace("iLabjack: Error reading SET_FIOX_STATE variable. value : [%s], extracted FIO number = [%d], extracted FIO value = [%d]",msg.GetString().c_str(),fioToSet, valueToSet);
        }
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
    }
    else
    {
      reportEvent("iLabjack: Labjack USB re-connexion success");
    }
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

void LabJack::registerVariables()
{
  AppCastingMOOSApp::RegisterVariables();
  Register("SET_FIOX_STATE", 0);
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
