#include "MutexIpc.h"

#include <iostream>
#include <random>
#include <asio.hpp>

asio::io_context io;

MutexIpc* m1{}, * m2{};

void spi_write()
{
  std::random_device rd;
  std::mt19937 gen(rd());

  std::uniform_int_distribution<> size_dist(5, 200);

  int size = size_dist(gen);

  std::uniform_int_distribution<> num_dist('0', '9');
  std::uniform_int_distribution<> time_dist(300, 600);

  auto time = time_dist(gen);

  std::this_thread::sleep_for(asio::chrono::microseconds(time));

  std::vector<int> randomVector(size);
  std::generate(randomVector.begin(), randomVector.end(), [&]() { return num_dist(gen); });


  std::cout << "Vector size: " << randomVector.size() << std::endl;
  std::cout << "Vector elements: ";
  for (int num : randomVector) {
    std::cout << num << " ";
  }
  std::cout << std::endl;

}

void senderE1(MutexIpcEvent_ event)
{
  io.post([event]() {m2->event(event); });
}

void senderE2(MutexIpcEvent_ event)
{
  io.post([event]() {m1->event(event); });
}

void do_loop(MutexIpc& mutex)
{
  mutex.lock([](const std::error_code& ec, MutexIpc& mutex)
    {
      if (ec)
      {
        std::cout << "Error: " << ec.message() << '\n';
        return;
      }
      std::cout << "Lock\n";
      spi_write();
      mutex.unlock();
      do_loop(mutex);
    });
}

void thread_loop_1()
{
  MutexIpc mutex(io, senderE1);
  m1 = &mutex;
  std::this_thread::sleep_for(std::chrono::seconds(1));
  std::cout << "Thread 1 loop\n";
  do_loop(mutex);
  while (true) { std::this_thread::sleep_for(std::chrono::seconds(1)); }
}

void thread_loop_2()
{
  MutexIpc mutex(io, senderE2);
  m2 = &mutex;
  std::this_thread::sleep_for(std::chrono::seconds(1));
  std::cout << "Thread 2 loop\n";
  do_loop(mutex);
  while (true) { std::this_thread::sleep_for(std::chrono::seconds(1)); }
}

int main()
{
  std::thread tio([]()
    {
      asio::steady_timer t{ io };

      t.expires_after(std::chrono::minutes(1000));
      t.async_wait([](const std::error_code& ec)
        {
          if (ec)
          {
            std::cout << "Error: " << ec.message() << '\n';
            return;
          }
        });
      io.run();
    }
  );

  std::thread t1(thread_loop_1);
  std::thread t2(thread_loop_2);

  t1.join();
  t2.join();
  tio.join();
  return 0;
}
