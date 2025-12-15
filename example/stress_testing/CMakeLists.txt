set(SRC_LIST stress_test.cpp ../../proto/user.pb.cc)

add_executable(stress_testing ${SRC_LIST}) # 服务提供者

target_link_libraries(stress_testing rpc protobuf)