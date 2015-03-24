/************************************************************/
/*    FILE: Labjack.h
/*    ORGN: ENSTA Bretagne
/*    AUTH: Clément Aubry
/*    DATE: 2015
/************************************************************/

#ifndef Labjack_HEADER
#define Labjack_HEADER

#include "MOOS/libMOOS/Thirdparty/AppCasting/AppCastingMOOSApp.h"
#include "u3.h"

class Labjack : public AppCastingMOOSApp
{
  public:
    Labjack();
    ~Labjack() {};

  protected: // Standard MOOSApp functions to overload
    bool OnNewMail(MOOSMSG_LIST &NewMail);
    bool Iterate();
    bool OnConnectToServer();
    bool OnStartUp();

  protected: // Standard AppCastingMOOSApp functions to overload
    bool buildReport();
    void registerVariables();

  protected: // Labjack functions


  private: // Configuration variables


  private: // State variables

    HANDLE hDevice;
    u3CalibrationInfo caliInfo;
    int localID;
    long error;
    int m_iFioState[8];

};

#endif
