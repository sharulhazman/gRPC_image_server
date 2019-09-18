cmake_minimum_required( VERSION 2.8 )
project( gRPC_server )

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOMRCC ON)

find_package( OpenCV REQUIRED )

# SETUP PROTOBUF
set(protobuf_MODULE_COMPATIBLE TRUE)
find_package( Protobuf REQUIRED )
set(_PROTOBUF_LIBPROTOBUF protobuf::libprotobuf)

# SETUP GRPC
set(gRPC "")
find_package( gRPC REQUIRED )
set(_GRPC_GRPCPP_UNSECURE gRPC::grpc++_unsecure)
# set(_GRPC_CPP_PLUGIN_EXECUTABLE $<TARGET_FILE:gRPC::grpc_cpp_plugin>)

INCLUDE_DIRECTORIES(${PROTOBUF_INCLUDE_DIR})
INCLUDE_DIRECTORIES(${GRPC_INCLUDE_DIR})

SET(CMAKE_CXX_FLAGS "-std=c++0x -Wall -g -Werror")

add_executable( server server.cpp compiled/image.grpc.pb.cc compiled/image.pb.cc )
target_link_libraries( server ${OpenCV_LIBS} ${GRPC_GRPCPP_LIBS} ${PROTOBUF_LIBS} ) # ${OpenCV_LIBS} ${_GRPC_GRPCPP_UNSECURE} ${_PROTOBUF_LIBPROTOBUF} )

add_executable( client client.cpp compiled/image.grpc.pb.cc compiled/image.pb.cc )
target_link_libraries( client ${OpenCV_LIBS} ${GRPC_GRPCPP_LIBS} ${PROTOBUF_LIBS} ) # ${OpenCV_LIBS} ${_PROTOBUF_LIBPROTOBUF} ${_GRPC_GRPCPP_UNSECURE} )



## OLD STUFF REFERENCE
# add_executable( server Server.cpp PracticalSocket.cpp )
# target_link_libraries( server ${OpenCV_LIBS} )
# add_executable( client Client.cpp PracticalSocket.cpp )
# target_link_libraries( client ${OpenCV_LIBS} )

# add_executable( master master.cpp PracticalSocket.cpp I2C_interface.cpp )
# target_link_libraries( master ${OpenCV_LIBS} )
# target_link_libraries( master Threads::Threads -lpthread)

# add_executable( slave slave.cpp PracticalSocket.cpp I2C_interface.cpp )
# target_link_libraries( slave ${OpenCV_LIBS} )
# target_link_libraries( slave Threads::Threads -lpthread )
