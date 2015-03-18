#--------------------------------------------------------
# The CMakeLists.txt for:                       iModemInterface
# Author(s):                              Clément Aubry (from V.Drevelle iSonar App (url?))
#--------------------------------------------------------

SET(SRC
  seanetmsg.cpp
  ModemInterface.cpp
  main.cpp
)

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_DEBUG")

ADD_EXECUTABLE(iModemInterface ${SRC})

TARGET_LINK_LIBRARIES(iModemInterface
   ${MOOS_LIBRARIES}
   ensta-documentation
   apputil
   mbutil
   m
   pthread
   rt
   )
