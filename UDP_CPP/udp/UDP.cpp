//
//  UDP.cpp
//  UDP_CPP
//
//  Created by Shun Sato on 8/7/18.
//  Copyright © 2018 ShunSato. All rights reserved.
//


#include "UDP.hpp"

#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <iostream>
#include <thread>

template<class T>
static inline void safeDelete( T*& p ){
    delete (p);
    (p) = nullptr;
}

UDP::UDPSocket::UDPSocket()
:addressInfo(new AddressInfo())
{}

UDP::UDPSocket::~UDPSocket() {
    safeDelete(addressInfo);
    close(socketHandle);
}

UDP::UDPSocket* UDP::UDPSocket::create(const std::string& address, int port) {
    auto newSocket = new UDPSocket();
    newSocket->address = address;
    newSocket->port = port;
    
    newSocket->addressInfo->sin_family=AF_INET;
    inet_pton(AF_INET, address.c_str(), &newSocket->addressInfo->sin_addr.s_addr);
    newSocket->addressInfo->sin_port=htons(port);
    
    int socketHandle = -1;
    if((socketHandle=socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        std::cout << "UDP::connect failed to create socket" << std::endl;
        
        safeDelete(newSocket);
        return nullptr;
    }
    newSocket->socketHandle = socketHandle;
    return newSocket;
}

UDP::UDP()
:hasReceiveThreadStarted(false)
,shouldStopReceivingThread(false)
{
}

UDP::~UDP() {
    for (auto it=sendingSockets.begin(); it!=sendingSockets.end(); ++it) {
        safeDelete(*it);
    }
    safeDelete(receivingSocket);
}

UDP& UDP::get() {
    static UDP sharedInstance;
    return sharedInstance;
}

const UDP::UDPSocket* UDP::addSendingSocket(const std::string& remoteAddress, int remotePort) {
    for (auto it = sendingSockets.begin(); it!=sendingSockets.end(); ++it) {
        if ((*it)->address == remoteAddress) {
            return *it;
        }
    }
    
    auto newSocket = UDPSocket::create(remoteAddress, remotePort);
    if (newSocket == nullptr) {
        std::cout << "UDP::addSendingSocket failed to create a socket." << std::endl;
    }
    sendingSockets.push_back(newSocket);
    return newSocket;
}

void UDP::removeSendingSocket(const UDPSocket* socket) {
    for (auto it=sendingSockets.begin(); it!=sendingSockets.end();) {
        if ((*it) == socket) {
            it = sendingSockets.erase(it);
        } else {
            ++it;
        }
    }
}

void UDP::send(const UDPSocket* socket, const std::string& message) {
    std::cout << "UDP::send message:" << message << " address:" << socket->address << ":" << socket->port << std::endl;
    if(sendto(socket->socketHandle, message.c_str(), message.size(), 0, (struct sockaddr *)socket->addressInfo, sizeof(*(socket->addressInfo)))!=message.size()) {
        std::cout << "UDP::send failed to send message." << " address:" << socket->address << ":" << socket->port << std::endl;
    }
}

void UDP::startReceiving(const std::string& selfAddress, int selfPort) {
    if (hasReceiveThreadStarted) {
        return;
    }
    
    receivingSocket = UDPSocket::create(selfAddress, selfPort);
    
    if(bind(receivingSocket->socketHandle, (struct sockaddr *)receivingSocket->addressInfo, sizeof(*(receivingSocket->addressInfo))) < 0) {
        std::cout << "UDP::connect failed to bind" << std::endl;
        return;
    }
    
    // TODO: multithread
    std::thread t([=]()
                  {
                      std::cout << "UDP::startReceiving thread started" << std::endl;
                      
                      while(true) {
                          //Receive the datagram back from server
                          int addrLength = sizeof(*(receivingSocket->addressInfo));
                          ssize_t received = 0;
                          char buffer[256] = {0};
                          
                          // FIXME: add timeout.
                          if((received=recvfrom(receivingSocket->socketHandle, buffer, 256, 0, (sockaddr *)receivingSocket->addressInfo, (socklen_t*)&addrLength)) < 0) {
                              std::cout << "UDP::startReceiving failed to receive" << std::endl;
                              return;
                          }
                          buffer[received]='\0';
                          
                          if (shouldStopReceivingThread) {
                              shouldStopReceivingThread = false;
                              break;
                          }
                          
                          for (auto it=receiveListeners.begin(); it!=receiveListeners.end(); ++it) {
                              (*it)->notify(buffer);
                          }
                          
                          std::cout << "UDP::startReceiving server (" << inet_ntoa(receivingSocket->addressInfo->sin_addr) << ") echoed: " << buffer << std::endl;
                      }
                      hasReceiveThreadStarted = false;
                  });
    
    t.detach();
    
    hasReceiveThreadStarted = true;
}

void UDP::stopReceiving() {
    shouldStopReceivingThread = true;
}

void UDP::registerReceiveListener(UDPReceiveListener* listener) {
    receiveListeners.push_back(listener);
}

void UDP::unregisterReceiveListener(UDPReceiveListener* listener) {
    receiveListeners.remove(listener);
}
