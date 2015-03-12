#--------------------------------------------------------
# The CMakeLists.txt for: iLabJack
# ENSTA Bretagne
# Author(s): Clément Aubry
#--------------------------------------------------------

SET(SRC
  LabJack.cpp
  u3.cpp
  main.cpp
)

ADD_EXECUTABLE(iLabJack ${SRC})

TARGET_LINK_LIBRARIES(iLabJack
   ${MOOS_LIBRARIES}
   ensta-documentation
   apputil
   mbutil
   m
   pthread
   labjackusb)
