#include "MutexIpc.h"

#include <iostream>
#include <random>
#include <asio.hpp>

asio::io_context io;

std::reference_wrapper<MutexIpc> m1, m2;

void spi_write()
{
  
  
  std::cout << "spi_write\n";
}

void senderE1(MutexIpcEvent_ event)
{
  m2.get().event(event);
}

void senderE2(MutexIpcEvent_ event)
{
  m1.get().event(event);
}

void thread_loop_1()
{
  MutexIpc mutex(io, senderE1);
  m1 = mutex;

  std::cout << "Thread 1 loop\n";
}

void thread_loop_2()
{
  MutexIpc mutex(io, senderE2);
  m2 = mutex;

  std::cout << "Thread 1 loop\n";
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
  mutex.lock([](const std::error_code& ec, MutexIpc& mutex)
    {
      if (ec)
      {
        std::cout << "Error: " << ec.message() << '\n';
        return;
      }

      std::cout << "Lock\n";
      mutex.unlock();
    });

  mutex.try_lock(std::chrono::seconds(1), [](const std::error_code& ec, MutexIpc& mutex)
    {
      if (ec)
      {
        std::cout << "Error: " << ec.message() << '\n';
        return;
      }
      std::cout << "Try lock\n";
      mutex.unlock();
    });
lock();

  io.run();
  return 0;
}
