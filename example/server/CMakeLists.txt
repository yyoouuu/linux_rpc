set(SRC_LIST user_server.cpp ../../proto/user.pb.cc)

add_executable(user_server ${SRC_LIST}) # 服务提供者

target_link_libraries(user_server rpc protobuf)