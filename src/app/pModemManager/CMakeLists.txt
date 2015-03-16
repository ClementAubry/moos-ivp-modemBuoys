#--------------------------------------------------------
# The CMakeLists.txt for: pModemManager
# ENSTA Bretagne
# Author(s): Clément Aubry
#--------------------------------------------------------

SET(SRC
  ModemManager.cpp
  main.cpp
)

ADD_EXECUTABLE(pModemManager ${SRC})

TARGET_LINK_LIBRARIES(pModemManager
   ${MOOS_LIBRARIES}
   ensta-documentation
   apputil
   mbutil
   m
   pthread)
