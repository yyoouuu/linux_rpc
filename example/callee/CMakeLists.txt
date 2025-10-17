#set(SRC_LIST Userservice.cc ../user.pb.cc)
set(SRC_LIST FriendService.cc ../friend.pb.cc)
add_executable(provider ${SRC_LIST})
target_link_libraries(provider linux_rpc protobuf)