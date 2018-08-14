
# UDP_CPP

Simple implementation of UDP in C++.

## Installation
Put "UDP.h" and "UDP.cpp" into your project.

## Usage

#### Send

```
int main() {
  // create a socket with remote address and port.
  auto socket = UDP::get().addSendingSocket("192.168.1.3", 8889);
  UDP::get().send(socket, "Hello");
}
```

#### Receive
```
class SomeClass: UDPReceiveListener {
  void notify(const std::string& message) override {
    std::cout << "received message:" << message << std::endl;
  }
};

int main() {
  auto someClass = SomeClass();
  // self address and port.
  UDP::get().startReceiving("192.168.1.1", 8889);
}
```
