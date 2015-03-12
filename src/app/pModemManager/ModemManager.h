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
    ~ModemManager() {};

  protected: // Standard MOOSApp functions to overload  
    bool OnNewMail(MOOSMSG_LIST &NewMail);
    bool Iterate();
    bool OnConnectToServer();
    bool OnStartUp();

  protected: // Standard AppCastingMOOSApp functions to overload 
    bool buildReport();
    void registerVariables();

  protected: // ModemManager functions


  private: // Configuration variables


  private: // State variables
    

};

#endif 
