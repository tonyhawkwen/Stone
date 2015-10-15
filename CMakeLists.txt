cmake_minimum_required (VERSION 2.6)

PROJECT (Stone)

INCLUDE(FindPkgConfig)

PKG_CHECK_MODULES(PKGS REQUIRED
				  libevent
				  libgflags
				  )


INCLUDE_DIRECTORIES(${PKGS_INCLUDE_DIRS})


INCLUDE_DIRECTORIES(Data
                    Event
                    Public
                    Channel
                    Thread
					Service)


SET(SRC_LIST Event/EventLoop_LibEvent.cpp
             Public/Sockets.cpp
             Public/Times.cpp
             Thread/LoopThread.cpp
			 Channel/TcpChannel.cpp
             Thread/ThreadPool.cpp
			 Service/TcpServer.cpp
			 Server.cpp
	)

MESSAGE("${CMAKE_CXX_FLAGS}")
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
MESSAGE("${CMAKE_CXX_FLAGS}")

MESSAGE("${PKGS_LIBRARIES}")

ADD_EXECUTABLE(stone ${SRC_LIST})
TARGET_LINK_LIBRARIES(stone ${PKGS_LIBRARIES})
