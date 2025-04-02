#pragma once

#include <system_error>
#include <chrono>

class MutexIpc;
enum MutexIpcEvent;

using FMutexIpc = void(const std::error_code& ec, MutexIpc&);
using FEventSenter = void(MutexIpcEvent);

enum MutexIpcEvent
{
  REQUST_LOCK,
  LOCK_DENIED,
  LOCK_ACCEPTED,
};

class MutexIpc
{
public:

  MutexIpc(FEventSenter& sedner) :m_sedner{ sedner } {}

  void lock(FMutexIpc *cb);
  void unlock();
  void try_lock(FMutexIpc cb, std::chrono::steady_clock wait = {});
  void event(MutexIpcEvent);
private:
  FEventSenter& m_sedner;
  FMutexIpc *m_cb{};
  
  std::atomic<bool> m_isLocked{};
  std::atomic<bool> m_isLockReqested{};
};

