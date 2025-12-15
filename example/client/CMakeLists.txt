set(SRC_LIST user_client.cpp ../../proto/user.pb.cc)

add_executable(user_client ${SRC_LIST}) # 服务提供者

target_link_libraries(user_client rpc protobuf)