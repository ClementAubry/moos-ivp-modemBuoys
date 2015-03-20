/************************************************************/
/*    FILE: ModemScheduler.h
/*    ORGN: ENSTA Bretagne
/*    AUTH: Clément Aubry
/*    DATE: 2015
/************************************************************/

#ifndef ModemScheduler_HEADER
#define ModemScheduler_HEADER

#include "MOOS/libMOOS/Thirdparty/AppCasting/AppCastingMOOSApp.h"

class ModemScheduler : public AppCastingMOOSApp
{
  public:
    ModemScheduler();
    ~ModemScheduler() {};

  protected: // Standard MOOSApp functions to overload  
    bool OnNewMail(MOOSMSG_LIST &NewMail);
    bool Iterate();
    bool OnConnectToServer();
    bool OnStartUp();

  protected: // Standard AppCastingMOOSApp functions to overload 
    bool buildReport();
    void registerVariables();

  protected: // ModemScheduler functions


  private: // Configuration variables


  private: // State variables
    

};

#endif 
