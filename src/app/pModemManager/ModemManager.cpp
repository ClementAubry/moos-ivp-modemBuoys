/************************************************************/
/*    FILE: ModemManager.cpp
/*    ORGN: ENSTA Bretagne
/*    AUTH: Clément Aubry
/*    DATE: 2015
/************************************************************/

#include <iterator>
#include "MBUtils.h"
#include "ACTable.h"
#include "ModemManager.h"

using namespace std;

//---------------------------------------------------------
// Constructor

ModemManager::ModemManager()
{
  m_uiNbRobots = 0;
  m_iInConfigTime = 0;

  m_uiTimeoutUS = 0;
  m_sModemPowerOnLabjack = "FIO0";
  m_sMagnetPowerOnLabjack = "FIO1";

  m_sModemRoleRequired = "slave";

  m_bCommunicationAndRangingStarted = true;

  string strMsg = m_sModemPowerOnLabjack;
  //parse string message to knwow which FIO to set
  std::size_t fioStart = strMsg.find("FIO");
  if (fioStart!=std::string::npos)
  {
    char buffer[16];
    std::size_t length = strMsg.copy(buffer,1,fioStart+3);
    buffer[length]='\0';
    m_iModemPowerOnLabjack = atoi(buffer);
  }
  else
  {
    m_iModemPowerOnLabjack = -1;
    MOOSFail("pModemManager: Error initializating FIO number for modem power management\n");
  }
  strMsg = m_sMagnetPowerOnLabjack;
  //parse string message to knwow which FIO to set
  fioStart = strMsg.find("FIO");
  if (fioStart!=std::string::npos)
  {
    char buffer[16];
    std::size_t length = strMsg.copy(buffer,1,fioStart+3);
    buffer[length]='\0';
    m_iMagnetPowerOnLabjack = atoi(buffer);
  }
  else
  {
    m_iMagnetPowerOnLabjack = -1;
    MOOSFail("pModemManager: Error initializating FIO number for magnet power management\n");
  }
  //Init threads
  if (!m_thread_tempo.Initialise(modem_manager_tempo_thread_func, (void*)this))
      MOOSFail("pModemManager: Error initializating tempo thread function...\n");
}

ModemManager::~ModemManager()
{
    MOOSTrace("pModemManager: stopping threads.\n");
    m_thread_tempo.Stop();
    MOOSTrace("pModemManager: finished.\n");
}

//---------------------------------------------------------
// Procedure: OnNewMail

bool ModemManager::OnNewMail(MOOSMSG_LIST &NewMail)
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

    if(key == "FIOX_STATE")
    {
      cout << "pModemManager: received FIOX_STATE: " << msg.GetString() << endl;
      reportRunWarning("pModemManager: received FIOX_STATE: " + msg.GetString());
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
      if (valueToSet >= 0 && valueToSet <=1 && fioToSet >= 0 && fioToSet <= 7)
      {
        if (fioToSet == m_iModemPowerOnLabjack)
        {
          if (m_iInConfigTime == 2)
            m_iInConfigTime = 3;
          else if (m_iInConfigTime == 6)
            m_iInConfigTime = 7;
          else if (m_iInConfigTime == 12)
            m_iInConfigTime = 13;
          else if (m_iInConfigTime == 14)
            m_iInConfigTime = 15;
          else
            MOOSTrace("ModemManager: Error receiving FIOX_STATE acknowledgement, unhandled case\n");
        }
        else if  (fioToSet == m_iMagnetPowerOnLabjack)
        {
          if (m_iInConfigTime == 4)
            m_iInConfigTime = 5;
          else if (m_iInConfigTime == 9)
            m_iInConfigTime = 10;
          else
            MOOSTrace("ModemManager: Error receiving FIOX_STATE acknowledgement, unhandled case\n");
        }
      }
    }
    else if(key == "MODEM_IS_ALIVE")
    {
      if (m_iInConfigTime == 7)
      {
        m_iInConfigTime = 8;
        MOOSTrace("ModemManager: receiving MODEM_IS_ALIVE\n");
      }
    }
    else if(key == "MODEM_CONFIGURATION_COMPLETE")
    {
      if (m_iInConfigTime == 10)
        m_iInConfigTime = 11;
      else
        MOOSTrace("ModemManager: Error receiving MODEM_CONFIGURATION_COMPLETE, unhandled case\n");
    }
    //Debug purpose, asking for configuration process to start, in the future, the app will hold a schedule to do so
    else if(key == "MODEM_MGR_CONFIG_ASK")
    {
      m_iInConfigTime = 1;
      m_sModemRoleRequired = msg.GetString();
      MOOSTrace("ModemManager: Modem Config process asked (debug purposeonly)\n");
    }
    else if(key != "APPCAST_REQ") // handle by AppCastingMOOSApp
      reportRunWarning("pModemManager: Unhandled Mail: " + key + msg.GetString());
  }

  return true;
}

//---------------------------------------------------------
// Procedure: OnConnectToServer

bool ModemManager::OnConnectToServer()
{
  registerVariables();
  return true;
}

//---------------------------------------------------------
// Procedure: Iterate()
//            happens AppTick times per second

bool ModemManager::Iterate()
{
  AppCastingMOOSApp::Iterate();

  if (m_bCommunicationAndRangingStarted)
  {
        char buffer[16]; //To publish "FIO=x;VALUE=y;" string
    switch (m_iInConfigTime) {
      case 0:
        MOOSTrace("ModemManager: Not in Config Mode, nothing to do\n");
      break;
      case 1:
        Notify("MODEM_MGR_START_CONFIG_TIME", MOOSTime());
        // 1) notify iModem to configure modem
        MOOSTrace("ModemManager: Asking iModem Configuration\n");
        Notify("MODEM_CONFIGURATION_REQUIRED",m_sModemRoleRequired);
        // 2) notify iLabjack to power down modem
        MOOSTrace("ModemManager: Asking iLabjack modem powering down\n");
        sprintf (buffer, "FIO=%d;VALUE=%d;",m_iModemPowerOnLabjack, 0);
        Notify("SET_FIOX_STATE",buffer);
        m_iInConfigTime=2;
      break;
      case 2:
        MOOSTrace("ModemManager: Modem powering down asked to iLabjack, waiting aknowledgement to power up the magnet\n");
      break;
      case 3:
        MOOSTrace("ModemManager: Modem powered down aknowledged by iLabjack\n");
        // 3) notify iLabjack to power up magnet
        MOOSTrace("ModemManager: Asking iLabjack magnet powering up\n");
        sprintf (buffer, "FIO=%d;VALUE=%d;",m_iMagnetPowerOnLabjack, 1);
        Notify("SET_FIOX_STATE",buffer);
        m_iInConfigTime=4;
      break;
      case 4:
        MOOSTrace("ModemManager: Magnet powering up asked to iLabjack, waiting aknowledgement to power up the modem\n");
      break;
      case 5:
        MOOSTrace("ModemManager: Modem powered off and magnet powered up acknowledged by iLabjack, can power up the modem \n");
        // 4) notify iLabjack to power up modem
        sprintf (buffer, "FIO=%d;VALUE=%d;",m_iModemPowerOnLabjack, 1);
        Notify("SET_FIOX_STATE",buffer);
        //We are now waiting iModem to say that modem is alive in order to power down the magnet
        m_iInConfigTime=6;
      break;
      case 6:
        MOOSTrace("ModemManager: Modem powering up asked to iLabjack, waiting aknowledgement\n");
      break;
      case 7:
        // ModemManager asked for modem powering down, magnet powering up
        MOOSTrace("ModemManager: Modem not (yet) alive\n");
      break;
      case 8:
        MOOSTrace("ModemManager: Modem is alive, asking iLabjack to power down the magnet\n");
        // 5) iModem has published a MODEM_IS_ALIVE message, notify iLabjack to power down magnet
        sprintf (buffer, "FIO=%d;VALUE=%d;",m_iMagnetPowerOnLabjack, 0);
        Notify("SET_FIOX_STATE",buffer);
        m_iInConfigTime=9;
      break;
      case 9:
        MOOSTrace("ModemManager: Magnet powered off asked to iLabjack\n");
      break;
      case 10:
        MOOSTrace("ModemManager: Magnet powered off acknowledged by iLabjack, waiting modem configuration to be complete\n");
      break;
      case 11:
        // 6) when iModem publish an MODEM_CONFIGURATION_COMPLETE message,
        //     notify iLabjack to power down modem,
        MOOSTrace("ModemManager: Modem Configuration complete, asking iLabjack modem powering down\n");
        sprintf (buffer, "FIO=%d;VALUE=%d;",m_iModemPowerOnLabjack, 0);
        Notify("SET_FIOX_STATE",buffer);
        m_iInConfigTime=12;
      break;
      case 12:
        MOOSTrace("ModemManager: iLabjack asked for modem powering down\n");
      break;
      case 13:
        MOOSTrace("ModemManager: Modem powered off acknowledged by iLabjack, can power on the modem\n");
        MOOSPause(3000);
        sprintf (buffer, "FIO=%d;VALUE=%d;",m_iModemPowerOnLabjack, 1);
        Notify("SET_FIOX_STATE",buffer);
        m_iInConfigTime=14;
      break;
      case 14:
        MOOSTrace("ModemManager: iLabjack asked for modem powering up\n");
      break;
      case 15:
        MOOSTrace("ModemManager: Modem powered on after configuration acknowledged by iLabjack, configuration process complete\n");
        m_iInConfigTime=0;
        Notify("MODEM_MGR_END_CONFIG_TIME", MOOSTime());
      break;
      default:
        MOOSTrace("ModemManager: Lost in config mode\n");
    }
  }
  //Comment/Uncomment thisline if report not needed
  // AppCastingMOOSApp::PostReport();
  return true;
}

//---------------------------------------------------------
// Procedure: OnStartUp()
//            happens before connection is open

bool ModemManager::OnStartUp()
{
  AppCastingMOOSApp::OnStartUp();

  STRING_LIST sParams;
  m_MissionReader.EnableVerbatimQuoting(false);
  if(!m_MissionReader.GetConfiguration(GetAppName(), sParams))
    reportConfigWarning("pModemManager: No config block found for " + GetAppName());

  STRING_LIST::iterator p;
  sParams.reverse();
  for(p = sParams.begin() ; p != sParams.end() ; p++)
  {
    string orig  = *p;
    string line  = *p;
    string param = toupper(biteStringX(line, '='));
    string value = line;
    bool handled = false;

    if(param == "MODEM_LJ_POWER")
    {
      m_sModemPowerOnLabjack = value;
      string strMsg = m_sModemPowerOnLabjack;
      //parse string message to knwow which FIO to set
      std::size_t fioStart = strMsg.find("FIO");
      if (fioStart!=std::string::npos)
      {
        char buffer[16];
        std::size_t length = strMsg.copy(buffer,1,fioStart+3);
        buffer[length]='\0';
        m_iModemPowerOnLabjack = atoi(buffer);
      }
      else
      {
        m_iModemPowerOnLabjack = -1;
        MOOSFail("pModemManager: OnStartUp: Error initializating FIO number for modem power management\n");
      }

      handled = true;
    }
    else if(param == "MAGNET_LJ_POWER")
    {
      m_sMagnetPowerOnLabjack = value;
      string strMsg = m_sMagnetPowerOnLabjack;
      //parse string message to knwow which FIO to set
      std::size_t fioStart = strMsg.find("FIO");
      if (fioStart!=std::string::npos)
      {
        char buffer[16];
        std::size_t length = strMsg.copy(buffer,1,fioStart+3);
        buffer[length]='\0';
        m_iMagnetPowerOnLabjack = atoi(buffer);
      }
      else
      {
        m_iMagnetPowerOnLabjack = -1;
        MOOSFail("pModemManager: OnStartUp: Error initializating FIO number for magnet power management\n");
      }
      handled = true;
    }
    else if(param == "MODEM_MGR_NB_ROBOTS")
    {
      m_uiNbRobots = atoi(value.c_str());
      handled = true;
    }


    if(!handled)
      reportUnhandledConfigWarning(orig);
  }

  registerVariables();
  return true;
}

//---------------------------------------------------------
// Procedure: registerVariables

void ModemManager::registerVariables()
{
  AppCastingMOOSApp::RegisterVariables();
  // Register("FOOBAR", 0);
  Register("FIOX_STATE", 0);
  Register("MODEM_IS_ALIVE", 0);
  Register("MODEM_CONFIGURATION_COMPLETE", 0);
  Register("MODEM_MGR_CONFIG_ASK", 0);
}

//------------------------------------------------------------
// Procedure: buildReport()

bool ModemManager::buildReport()
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

void ModemManager::ModemManagerTempoFunction()
{
    if (!m_thread_tempo.IsQuitRequested() && m_uiTimeoutUS != 0)
    {
        MOOSPause(m_uiTimeoutUS);
        m_uiTimeoutUS = 0;
    }
}
