//
//  main.cpp
//  UDP_CPP
//
//  Created by Shun Sato on 8/7/18.
//  Copyright © 2018 ShunSato. All rights reserved.
//

#include <iostream>
#include <thread>

#include "UDP.hpp"

class UDPReceiveTest: UDP::UDPReceiveListener {
public:
    void notify(const std::string& message) override {
        std::cout << "received message:" << message << std::endl;
    }
};

int main(int argc, const char * argv[]) {
    auto socket = UDP::get().addSendingSocket("192.168.1.3", 8889);
    auto socket2 = UDP::get().addSendingSocket("192.168.1.26", 8889);
    
    UDP::get().startReceiving("192.168.1.25", 8889);
    auto receiveTest = UDPReceiveTest();
    
    // Sends "Hello" to the address of "192.168.1.3:8889" and "192.168.1.26:8889" every 2 seconds.
    while(true) {
        std::this_thread::sleep_for(std::chrono::seconds(2));
        UDP::get().send(socket, "Hello");
        UDP::get().send(socket2, "Hello");
    }
    return 0;
}
