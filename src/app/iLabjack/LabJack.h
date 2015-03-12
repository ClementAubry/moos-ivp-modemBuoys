/************************************************************/
/*    FILE: LabJack.h
/*    ORGN: ENSTA Bretagne
/*    AUTH: Clément Aubry
/*    DATE: 2015
/************************************************************/

#ifndef LabJack_HEADER
#define LabJack_HEADER

#include "MOOS/libMOOS/Thirdparty/AppCasting/AppCastingMOOSApp.h"
#include "u3.h"

class LabJack : public AppCastingMOOSApp
{
  public:
    LabJack();
    ~LabJack() {};

  protected: // Standard MOOSApp functions to overload
    bool OnNewMail(MOOSMSG_LIST &NewMail);
    bool Iterate();
    bool OnConnectToServer();
    bool OnStartUp();

  protected: // Standard AppCastingMOOSApp functions to overload
    bool buildReport();
    void registerVariables();

  protected: // LabJack functions


  private: // Configuration variables


  private: // State variables

    HANDLE hDevice;
    u3CalibrationInfo caliInfo;
    int localID;
    long error;

};

#endif
