//------------------------------------------------------------------------
// File    : thread.cpp
// Author  : David Poon
// Written : 6 May 2001
// 
// Implementation file for the threading classes defined in thread.hpp
//
// Copyright (C) 2001 David Poon
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of 
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU 
// General Public License for more details.
//
// You should have received a copy of the GNU General Public License 
// along with this program; if not, write to the Free Software Foundation
// Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
//------------------------------------------------------------------------

#ifndef UNITY_BUILD
#include "thread.h"
#endif

Thread::Thread()
{
	m_hThread = 0;
	m_threadId = 0;
	m_canRun = true;
	m_suspended = true;
}

Thread::~Thread()
{
	if (m_hThread)
		CloseHandle(m_hThread);
}

bool Thread::canRun()
{
	Lock guard(m_mutex);
	return m_canRun;
}

bool Thread::create(unsigned int stackSize)
{
	m_hThread = reinterpret_cast<HANDLE>(_beginthreadex(0, stackSize, 
		threadFunc, this, CREATE_SUSPENDED, &m_threadId));

	BOOL priorityResult = SetThreadPriority(m_hThread,THREAD_PRIORITY_NORMAL);
	Assert(priorityResult);
	
	if (m_hThread)
		return true;
	
	return false;
}

void Thread::join()
{
	if(canRun())
	WaitForSingleObject(m_hThread, INFINITE);
}

void Thread::resume()
{
	if (m_suspended)
	{
		Lock guard(m_mutex);
		
		if (m_suspended)
		{
			ResumeThread(m_hThread);
			m_suspended = false;
		}
	}
}

void Thread::shutdown()
{
	if (m_canRun)
	{
		Lock guard(m_mutex);

		if (m_canRun)
			m_canRun = false;

		resume();
	}
}

void Thread::start()
{
	resume();
}

void Thread::suspend()
{
	if (!m_suspended)
	{
		Lock guard(m_mutex);

		if (!m_suspended)
		{
			SuspendThread(m_hThread);
			m_suspended = true;
		}
	}
}

unsigned int Thread::threadId() const
{
	return m_threadId;
}

unsigned int __stdcall Thread::threadFunc(void *args)
{
	Thread *pThread = reinterpret_cast<Thread*>(args);
	
	if (pThread)
		pThread->run();

	//_endthreadex(0);
	pThread->m_canRun = false;
	return 0;
}