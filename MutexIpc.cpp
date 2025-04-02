#include "MutexIpc.h"

#include <system_error>
#include <chrono>

void MutexIpc::lock(FMutexIpc *cb) 
{
	if (!m_cb)
	{
		if (cb) cb(std::make_error_code(std::errc::bad_message), *this);
		return;
	}

  m_cb = cb;
  m_sedner(MutexIpcEvent::REQUST_LOCK);
}

void MutexIpc::unlock() 
{

}

void MutexIpc::try_lock(FMutexIpc cb, std::chrono::steady_clock wait) 
{

}

void MutexIpc::event(MutexIpcEvent e) 
{
	switch (e)
	{
	case REQUST_LOCK:
		m_isLockReqested = true;
		m_sedner(m_isLocked ? MutexIpcEvent::LOCK_DENIED : MutexIpcEvent::LOCK_ACCEPTED);
		break;

	case LOCK_DENIED:
		break;
	case LOCK_ACCEPTED:
		if (!m_cb)
		{
			break;
		}
		
		m_cb({}, *this);
		m_cb = {};
		break;
	}
}
