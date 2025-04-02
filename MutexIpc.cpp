#include "MutexIpc.h"

#include <system_error>
#include <chrono>
#include <iostream>

int MutexIpc::id_;

void MutexIpc::lock(FMutexIpc* cb)
{
  if (m_cb)
  {
    if (cb) cb(std::make_error_code(std::errc::resource_unavailable_try_again), *this);
    return;
  }

  m_cb = cb;
  m_sedner(MutexIpcEvent_::REQUST_LOCK);
}

void MutexIpc::unlock()
{
  m_isLocked = false;

  if (m_tryLockTinmer)
  {
    m_tryLockTinmer->cancel();
    m_tryLockTinmer = {};
  }

  if (m_isLockReqested)
  {
    m_sedner(MutexIpcEvent_::LOCK_ACCEPTED);
    m_isLockReqested = {};
  }
}

void MutexIpc::try_lock(std::chrono::steady_clock::duration wait, FMutexIpc cb)
{
  if (m_tryLockTinmer || m_isLocked)
  {
    cb(std::make_error_code(std::errc::resource_unavailable_try_again), *this);
    return;
  }

  m_cb = cb;
  m_sedner(MutexIpcEvent_::REQUST_LOCK);
  m_tryLockTinmer = std::make_shared<asio::steady_timer>(m_io);
  
  m_tryLockTinmer->expires_after(wait);
  
  m_tryLockTinmer->async_wait([this](const std::error_code& ec)
    {
      if (ec)
      {
        return;
      }

      m_cb(std::make_error_code(std::errc::resource_unavailable_try_again), *this);
      m_cb = {};
      m_tryLockTinmer = {};
      this->unlock();
    });
}

void MutexIpc::event(MutexIpcEvent_ e)
{
  switch (e)
  {
  case MutexIpcEvent_::REQUST_LOCK:
    std::cout << "REQUST_LOCK" << m_id << std::endl;
    m_isLockReqested = true;
    m_sedner(m_isLocked ? MutexIpcEvent_::LOCK_DENIED : MutexIpcEvent_::LOCK_ACCEPTED);
    break;

  case MutexIpcEvent_::LOCK_DENIED:
    std::cout << "LOCK_DENIED" << m_id << std::endl;
    break;

  case MutexIpcEvent_::LOCK_ACCEPTED:
    std::cout << "LOCK_ACCEPTED" << m_id << std::endl;
    
    if (m_tryLockTinmer)
    {
      m_tryLockTinmer->cancel();
      m_tryLockTinmer = {};
    }

    m_isLocked = true;
    if (m_cb)
    {
      auto cb = m_cb;
      m_cb = {};
      cb({}, *this);
    }
    break;
  }
}
