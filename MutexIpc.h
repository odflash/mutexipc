#pragma once

#include <system_error>
#include <chrono>
#include <memory>
#include <asio.hpp>

enum class MutexIpcEvent_ : int
{
  REQUST_LOCK,
  LOCK_DENIED,
  LOCK_ACCEPTED,
};

class MutexIpc;
using FEventSenter = void(MutexIpcEvent_ e);
using FMutexIpc = void(const std::error_code& ec, MutexIpc&);

class MutexIpc
{
public:

  MutexIpc(asio::io_context&io, FEventSenter& sedner) :m_io{io}, m_sedner{ sedner } {}

  void lock(FMutexIpc *cb);
  void unlock();
  void try_lock(std::chrono::steady_clock::duration wait, FMutexIpc cb);
  void event(MutexIpcEvent_);

private:
  asio::io_context& m_io;
  FEventSenter& m_sedner;
  FMutexIpc *m_cb{};
  
  std::atomic<bool> m_isLocked{};
  std::atomic<bool> m_isLockReqested{};
  std::shared_ptr<asio::steady_timer> m_tryLockTinmer{};
};
