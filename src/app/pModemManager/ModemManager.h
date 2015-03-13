/************************************************************/
/*    FILE: ModemManager.h
/*    ORGN: ENSTA Bretagne
/*    AUTH: Clément Aubry
/*    DATE: 2015
/************************************************************/

#ifndef ModemManager_HEADER
#define ModemManager_HEADER

#include "MOOS/libMOOS/Thirdparty/AppCasting/AppCastingMOOSApp.h"

class ModemManager : public AppCastingMOOSApp
{
  public:
    ModemManager();
    ~ModemManager();

  protected: // Standard MOOSApp functions to overload
    bool OnNewMail(MOOSMSG_LIST &NewMail);
    bool Iterate();
    bool OnConnectToServer();
    bool OnStartUp();

  protected: // Standard AppCastingMOOSApp functions to overload
    bool buildReport();
    void registerVariables();

  protected: // ModemManager functions
    void ModemManagerTempoFunction();

  private: // Configuration variables


  private: // State variables
    std::string m_sModemPowerOnLabjack;
    int m_iModemPowerOnLabjack;
    std::string m_sMagnetPowerOnLabjack;
    int m_iMagnetPowerOnLabjack;
    unsigned int m_uiNbRobots;

    bool         m_bCommunicationAndRangingStarted;;

    unsigned int m_uiTimeoutUS;

    int m_iInConfigTime;

  private: // ModemManager functions
    //Thread created for timeouts, probably not the right way to do but it works like a charm
    CMOOSThread   m_thread_tempo;
    static bool modem_manager_tempo_thread_func(void *pModemManagerObject)
    {
      ModemManager* pModemManager = static_cast<ModemManager*> (pModemManagerObject);
      if (pModemManager)
      {
        std::cout<<"Timeout configuration thread launched"<<std::endl;
        pModemManager->ModemManagerTempoFunction();
        return true;
      }
      else return false;
    }

};

#endif
