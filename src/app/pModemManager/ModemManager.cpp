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


  m_uiTimeoutUS = 0;
  m_sModemPowerOnLabjack = "FIO0";
  m_sMagnetPowerOnLabjack = "FIO1";

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

    if(key == "FIO0_STATE")
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
        MOOSTrace("ModemManager: Error receiving FIO0_STATE acknowledgement, unhandled case\n");
    }
    if(key == "FIO1_STATE")
    {
      if (m_iInConfigTime == 4)
        m_iInConfigTime = 5;
      else if (m_iInConfigTime == 9)
        m_iInConfigTime = 10;
      else
        MOOSTrace("ModemManager: Error receiving FIO1_STATE acknowledgement, unhandled case\n");
    }
    if(key == "MODEM_IS_ALIVE")
    {
      if (m_iInConfigTime == 6 || m_iInConfigTime == 7)
        m_iInConfigTime = 8;
      else
        MOOSTrace("ModemManager: Error receiving MODEM_IS_ALIVE, unhandled case\n");
    }
    if(key == "MODEM_CONFIGURATION_COMPLETE")
    {
      if (m_iInConfigTime == 10)
        m_iInConfigTime = 11;
      else
        MOOSTrace("ModemManager: Error receiving MODEM_CONFIGURATION_COMPLETE, unhandled case\n");
    }


    else if(key != "APPCAST_REQ") // handle by AppCastingMOOSApp
      reportRunWarning("pModemManager: Unhandled Mail: " + key);
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
    switch (m_iInConfigTime) {
      case 0:
        MOOSTrace("ModemManager: Not in Config Mode, nothing to do\n");
      break;
      case 1:
        Notify("MODEM_MGR_START_CONFIG_TIME", MOOSTime());
        // 1) notify iModem to configure modem
        MOOSTrace("ModemManager: Asking iModem Configuration\n");
        Notify("MODEM_CONFIGURATION_REQUIRED","slave");
        // 2) notify iLabjack to power down modem
        MOOSTrace("ModemManager: Asking iLabjack modem powering down\n");
        Notify("SET_FIO0_STATE","false");
        m_iInConfigTime=2;
      break;
      case 2:
        MOOSTrace("ModemManager: Modem powering down asked to iLabjack, waiting aknowledgement to power up the magnet\n");
      break;
      case 3:
        MOOSTrace("ModemManager: Modem powered down aknowledged by iLabjack\n");
        // 3) notify iLabjack to power up magnet
        MOOSTrace("ModemManager: Asking iLabjack magnet powering up\n");
        Notify("SET_FIO1_STATE","true");
        m_iInConfigTime=4;
      break;
      case 4:
        MOOSTrace("ModemManager: Magnet powering up asked to iLabjack, waiting aknowledgement to power up the modem\n");
      break;
      case 5:
        MOOSTrace("ModemManager: Modem powered off and magnet powered up acknowledged by iLabjack, can power up the modem \n");
        // 4) notify iLabjack to power up modem
        Notify("SET_FIO0_STATE","true");
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
        Notify("SET_FIO1_STATE","false");
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
        Notify("SET_FIO0_STATE","false");
        m_iInConfigTime=12;
      break;
      case 12:
        MOOSTrace("ModemManager: iLabjack asked for modem powering down\n");
      break;
      case 13:
        MOOSTrace("ModemManager: Modem powered off acknowledged by iLabjack, can power on the modem\n");
        MOOSPause(500);
        Notify("SET_FIO0_STATE","true");
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

  AppCastingMOOSApp::PostReport();
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
      handled = true;
    }
    else if(param == "MAGNET_LJ_POWER")
    {
      m_sMagnetPowerOnLabjack = value;
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
  Register("FIO0_STATE", 0);
  Register("FIO1_STATE", 0);
  Register("MODEM_IS_ALIVE", 0);
  Register("MODEM_CONFIGURATION_COMPLETE", 0);
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
