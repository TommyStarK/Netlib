#include "include/hermes.hpp"

#include <unistd.h>
#include <chrono>
#include <iostream>
#include <memory>
#include <thread>

int main() {
  std::mutex m;
  hermes::internal::thread_pool tp(100);
  hermes::network::tcp::socket s;

  try {
    s.bind("127.0.0.1", 27017);
    s.listen(10);
  } catch (const std::exception& e) {
    std::cerr << e.what() << '\n';
    return 1;
  }

   
   for(;;) {
    try {
      
      {
        std::lock_guard<std::mutex> lock(m);
        std::cout << "waiting for connections !!\n";
      }

      auto client = std::make_shared<hermes::network::tcp::socket>(s.accept());
      
      {
        std::lock_guard<std::mutex> lock(m);
        std::cout << "client accepted: " << client->fd() << std::endl;
      }
      
      tp.register_task([client, &m] {
        while (1) {
          auto data = client->receive();

          {
            std::lock_guard<std::mutex> lock(m);
            std::cout << "data received: " << data.data();
          }

          client->send(data.data());
        }
      });
    } catch (const std::exception& e) {
      std::cerr << e.what() << '\n';
    }
   }

  return 0;
}