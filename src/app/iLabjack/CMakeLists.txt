#--------------------------------------------------------
# The CMakeLists.txt for: iLabJack
# ENSTA Bretagne
# Author(s): Clément Aubry
#--------------------------------------------------------

SET(SRC
  Labjack.cpp
  u3.cpp
  main.cpp
)

ADD_EXECUTABLE(iLabjack ${SRC})

TARGET_LINK_LIBRARIES(iLabjack
   ${MOOS_LIBRARIES}
   ensta-documentation
   apputil
   mbutil
   m
   pthread
   labjackusb)
