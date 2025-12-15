
set(SRC_LIST 
    CircuitBreakerProvider.cpp
    CircuitBreaker.cpp 
    ConfigManager.cpp 
    LoadBalancer.cpp 
    RpcApplication.cpp
    RpcClient.cpp
    RpcContoller.cpp
    RpcServer.cpp
    ZKClient.cpp
    ../common/logger/Logger.cpp
    ../proto/user.pb.cc
    ../proto/rpcheader.pb.cc
    )
add_library(rpc  ${SRC_LIST})
target_link_libraries(rpc muduo_net muduo_base pthread zookeeper_mt)