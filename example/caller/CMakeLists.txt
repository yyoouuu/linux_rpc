#set(SRC_LIST CallerUserservice.cc ../user.pb.cc)
set(SRC_LIST CallFriendService.cc ../friend.pb.cc)
add_executable(consumer ${SRC_LIST})
target_link_libraries(consumer linux_rpc protobuf)