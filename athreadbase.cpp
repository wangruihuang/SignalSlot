#include "athreadbase.hpp"

ADefineMap<std::thread::id, AThreadBase::TThreadCheck > AThreadBase::m_idtothread;
std::recursive_mutex AThreadBase::m_mtxstatic;
