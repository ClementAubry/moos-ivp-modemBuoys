#--------------------------------------------------------
# The CMakeLists.txt for: uModemScheduler
# ENSTA Bretagne
# Author(s): Clément Aubry
#--------------------------------------------------------

SET(SRC
  ModemScheduler.cpp
  main.cpp
)

ADD_EXECUTABLE(uModemScheduler ${SRC})

TARGET_LINK_LIBRARIES(uModemScheduler
   ${MOOS_LIBRARIES}
   ensta-documentation
   apputil
   mbutil
   m
   pthread)
