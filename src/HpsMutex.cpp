
// Copyright (C) Stanislav Cherkasov

#include "HpsMutex.h"
#include <stdexcept>

namespace Hps
{
    Mutex::Mutex()
    {
        int r = pthread_mutex_init(&m_mutex, 0);
        if(r != 0)
            throw std::runtime_error("ERROR: Could not create mutex");
    }

    Mutex::~Mutex()
    {
        pthread_mutex_destroy(&m_mutex);
    }

    void Mutex::Lock()
    {
        pthread_mutex_lock(&m_mutex);
    }

    void Mutex::UnLock()
    {
        pthread_mutex_unlock(&m_mutex);
    }

} // namespace Hpg
