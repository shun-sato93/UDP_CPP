//
//  UDP.hpp
//  UDP_CPP
//
//  Created by Shun Sato on 8/7/18.
//  Copyright © 2018 ShunSato. All rights reserved.
//

#ifndef UDP_hpp
#define UDP_hpp

#include <atomic>
#include <string>
#include <list>

class UDPReceiveListener;

typedef struct sockaddr_in AddressInfo;

class UDP {
public:
    class UDPSocket {
    public:
        static UDPSocket* create(const std::string& address, int port);
        ~UDPSocket();
        
        std::string address;
        int port;
        int socketHandle;
        AddressInfo* addressInfo;
    private:
        UDPSocket();
    };
    
    class UDPReceiveListener {
    public:
        UDPReceiveListener() {
            UDP::get().registerReceiveListener(this);
        }
        
        virtual ~UDPReceiveListener() {
            UDP::get().unregisterReceiveListener(this);
        }
        
        virtual void notify(const std::string& message) = 0;
    };
    
    static UDP& get();
    ~UDP();
    
    const UDPSocket* addSendingSocket(const std::string& remoteAddress, int remotePort);
    void removeSendingSocket(const UDPSocket* socket);
    void send(const UDPSocket* socket, const std::string& message);
    
    void startReceiving(const std::string& selfAddress, int selfPort);
    void stopReceiving();
    void registerReceiveListener(UDPReceiveListener* listener);
    void unregisterReceiveListener(UDPReceiveListener* listener);
    
private:
    UDP();
    void setupInfo(const std::string& address, int port, AddressInfo* info);
    
    std::atomic<bool> hasReceiveThreadStarted;
    std::atomic<bool> shouldStopReceivingThread;
    
    UDPSocket* receivingSocket;
    std::list<UDPSocket*> sendingSockets;
    
    std::list<UDPReceiveListener*> receiveListeners;
};

#endif /* UDP_hpp */
