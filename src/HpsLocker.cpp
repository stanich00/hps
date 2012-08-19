
// Copyright (C) Stanislav Cherkasov

#include "HpsLocker.h"

namespace Hps
{
    Locker::Locker(Mutex &mutex)
        :m_mutex(mutex)
    {
        m_mutex.Lock();
    }

    Locker::~Locker()
    {
        m_mutex.UnLock();
    }
}//namespace Hpg
