#ifndef ASIGNAL_HPP
#define ASIGNAL_HPP

#include <athreadbase.hpp>
/**
 * 名称：信号类
 * 功能：
 *      1.提供信号的发出，连接槽，取消连接槽，管理槽映射等
 *      2.如果连接的槽属于同一个线程则在发出消息之后直接调用(不会向本线程推送消息)
 * 限制：
 *      1.不能被继承，不能被拷贝和赋值
 *      2.只能连接返回值为void的槽
 *      3.在connect之前必须为槽所属的对象指定线程(通过moveToThread方法)
 *      4.每个类的槽在第一次connect之前，该类必须指定线程，
 *        connect之后无法再指定！(每个类只能在连接前，指定唯一所属进程)
 */
template<typename... _ArgTypes>
class ASignal final
{
public:
    using TObjectId        = uint32_t;
    using TSlotID          = uint32_t;
    using TObjectSlotID    = std::pair<TObjectId, TSlotID>;
    using TCheckPointer    = std::weak_ptr<check_shared_ptr>;
    using TCheckAndSlot    = std::pair<TCheckPointer ,ADefineSet<TSlotID> >;
    using TObjectToSlot    = ADefineMap<TObjectId,TCheckAndSlot>;
    using TThreadCheck     = std::weak_ptr<AThreadBase>;
    using TThreadCheckHold = std::shared_ptr<AThreadBase>;
    using TThreadMap       = ADefineMap<AThreadBase*,TThreadCheck >;
    using TThreadToObject  = ADefineMap<AThreadBase*,TObjectToSlot>;
public:
    ASignal(){}
    ~ASignal(){}

    void operator()(const _ArgTypes... _args)
    {
        any args = std::make_tuple(_args...);
        TThreadMap thrdMap = getThreadMap();
        AThreadBase* pCurrent = AThreadBase::current().lock().get();
        for(auto iter = thrdMap.begin();iter!=thrdMap.end();++iter)
        {
            if(iter->first==pCurrent) continue;
            if(iter->second.expired())
            {
                eraseThreadMap(iter->first);
                eraseThreadToObject(iter->first);
                continue;
            }
            TObjectToSlot obj2slt = getObjectToSlot(iter->first);
            pushMessage(obj2slt,args,iter->second);
        }
        if(thrdMap.count(pCurrent)<=0) return;
        TObjectToSlot obj2slt = getObjectToSlot(pCurrent);
        callSlot(obj2slt,args,pCurrent);
    }

    template<typename TType>
    void connect(void (*func)(_ArgTypes...),const std::shared_ptr<TType>& type)
    {
        TObjectSlotID objSltId((TObjectId)type.get(),(TSlotID)func);
        regObjectSlot(objSltId,type);
        type->regSlot(objSltId.second,TFuntorWrapper2<_ArgTypes...>(func));
    }

    template<typename TType>
    void connect(void (TType::*func)(_ArgTypes...), const std::shared_ptr<TType>& type)
    {
        TObjectSlotID objSltId((TObjectId)type.get(),memfunc_convert<TSlotID>(func));
        regObjectSlot(objSltId,type);
        TFunctionBind<sizeof...(_ArgTypes),TType,_ArgTypes...> funbind;
        type->regSlot(objSltId.second,TFuntorWrapper2<_ArgTypes...>(funbind(func,type.get())));
    }

    template<typename TFunctor,typename TType>
    void connect(TFunctor *functor,const std::shared_ptr<TType>& type)
    {
        if((TSlotID)functor==(TSlotID)this) return ; //不允许信号连接自己
        TObjectSlotID objSltId((TObjectId)type.get(),memfunc_convert<TSlotID>(&(functor->operator())));
        regObjectSlot(objSltId,type);
        type->regSlot(objSltId.second,TFuntorWrapper2<_ArgTypes...>(std::reference_wrapper<TFunctor>(*functor)));
    }

    template<typename TType>
    void disconnect(void (*func)(_ArgTypes...),const std::shared_ptr<TType>& type)
    {
        TObjectSlotID objSltId((TObjectId)type.get(),(TSlotID)func);
        unregObjectSlot(objSltId,type->thread().lock().get());
    }

    template<typename TType>
    void disconnect(void (TType::*func)(_ArgTypes...), const std::shared_ptr<TType>& type)
    {
        TObjectSlotID objSltId((TObjectId)type.get(),memfunc_convert<TSlotID>(func));
        unregObjectSlot(objSltId,type->thread().lock().get());
    }

    template<typename TFunctor,typename TType>
    void disconnect(TFunctor *fuctor,const std::shared_ptr<TType>& type)
    {
        TObjectSlotID objSltId((TObjectId)type.get(),(TSlotID)fuctor);
        unregObjectSlot(objSltId,type->thread().lock().get());
    }

    bool empty()
    {
        return m_thrdObject.empty();
    }

    void clear()
    {
        m_thrdMap.clear();
        m_thrdObject.clear();
    }

protected:
    void unregObjectSlot(TObjectSlotID& objSltId,AThreadBase* pThread)
    {
        if(isThreadExist(pThread))
        {
            if(isObjectMapExist(pThread,objSltId.first))
            {
                std::lock_guard<std::recursive_mutex> lk(m_mtxsigslt);
                if(m_thrdObject[pThread][objSltId.first].second.count(objSltId.second)>0)
                {
                    m_thrdObject[pThread][objSltId.first].second.erase(objSltId.second);
                }
                if(m_thrdObject[pThread][objSltId.first].second.empty())
                {
                    m_thrdObject[pThread].erase(objSltId.first);
                }
                if(m_thrdObject[pThread].empty())
                {
                    m_thrdObject.erase(pThread);
                    m_thrdMap.erase(pThread);
                }
            }
        }
    }

    void callSlot(TObjectToSlot& obj2slt, any& args, AThreadBase* pThread)
    {
        for(auto iter = obj2slt.begin();iter!=obj2slt.end();++iter)
        {
            if(true==iter->second.first.expired())
            {
                eraseObjectToSlot(pThread,iter->first);
            }
            else
            {
                auto mapSlot = iter->second.second;
                for(auto item : mapSlot)
                {
                    if(false==callSlot_Inner(iter->second.first.lock(),item,args))
                    {
                        eraseObjectToSlot(pThread,iter->first);
                        break;
                    }
                }
            }
        }
    }

    template<typename TCaller>
    bool callSlot_Inner(TCaller caller, TSlotID sltid, any& args)
    {
        try
        {
            if(caller)caller->callSlot(sltid,args);
            return true;
        }
        catch(std::exception& e)
        {
            (void)e;
        }
        catch(...)
        {}
        if(caller->valid())
            return true;
        return false;
    }

    void pushMessage(TObjectToSlot& obj2slt, any& args, TThreadCheck thrd)
    {
        if(thrd.expired()) return;
        thrd.lock()->push(std::make_pair(obj2slt,args));
    }

    template<typename TType>
    void regObjectSlot(TObjectSlotID& objSltId, const std::shared_ptr<TType>& type)
    {
        TThreadCheckHold thrd = type->thread().lock();
        if(isThreadExist(thrd.get()))
        {
            if(isThreadInValid(thrd.get()))
            {
                dealThreadInValid(thrd,objSltId,type);
            }
            else
            {
                dealThreadValid(thrd,objSltId,type);
            }
        }
        else
        {
            dealThreadNoExist(thrd,objSltId,type);
        }
    }

    bool isThreadExist(AThreadBase* pThread)
    {
        std::lock_guard<std::recursive_mutex> lk(m_mtxsigslt);
        return m_thrdMap.count(pThread)>0;
    }

    bool isThreadInValid(AThreadBase* pThread)
    {
        std::lock_guard<std::recursive_mutex> lk(m_mtxsigslt);
        return m_thrdMap[pThread].expired();
    }

    bool isObjectMapExist(AThreadBase* pThread,TObjectId& obj)
    {
        std::lock_guard<std::recursive_mutex> lk(m_mtxsigslt);
        return m_thrdObject[pThread].count(obj)>0;
    }

    bool isObjectInValid(AThreadBase* pThread,TObjectId& obj)
    {
        std::lock_guard<std::recursive_mutex> lk(m_mtxsigslt);
        return m_thrdObject[pThread][obj].first.expired();
    }

    template<typename TType>
    void dealThreadInValid(TThreadCheckHold& thrd, TObjectSlotID& objSltId, const std::shared_ptr<TType>& type)
    {
        {
            std::lock_guard<std::recursive_mutex> lk(m_mtxsigslt);
            m_thrdMap[thrd.get()] = thrd;
        }
        TObjectToSlot objtoslot;
        objtoslot.insert(std::make_pair(objSltId.first ,make_checkslot<TType>(objSltId,type->checker())));
        {
            std::lock_guard<std::recursive_mutex> lk(m_mtxsigslt);
            m_thrdObject[thrd.get()] = objtoslot;
        }
    }

    template<typename TType>
    void dealThreadNoExist(TThreadCheckHold& thrd, TObjectSlotID& objSltId, const std::shared_ptr<TType>& type)
    {
        {
            std::lock_guard<std::recursive_mutex> lk(m_mtxsigslt);
            m_thrdMap.insert(std::make_pair(thrd.get(),thrd));
        }

        TObjectToSlot objtoslot;
        objtoslot.insert(std::make_pair(objSltId.first,make_checkslot<TType>(objSltId,type->checker())));
        {
            std::lock_guard<std::recursive_mutex> lk(m_mtxsigslt);
            m_thrdObject.insert(std::make_pair(thrd.get(), objtoslot));
        }
    }

    template<typename TType>
    void dealThreadValid(TThreadCheckHold& thrd, TObjectSlotID& objSltId, const std::shared_ptr<TType>& type)
    {
        if(isObjectMapExist(thrd.get(),objSltId.first))
        {
            if(isObjectInValid(thrd.get(),objSltId.first))
            {
                TCheckAndSlot checkslot = make_checkslot<TType>(objSltId,type->checker());
                {
                    std::lock_guard<std::recursive_mutex> lk(m_mtxsigslt);
                    m_thrdObject[thrd.get()][objSltId.first] = checkslot;
                }
            }
            else
            {
                std::lock_guard<std::recursive_mutex> lk(m_mtxsigslt);
                if(m_thrdObject[thrd.get()][objSltId.first].second.count(objSltId.second)<=0)
                    m_thrdObject[thrd.get()][objSltId.first].second.insert(objSltId.second);
            }
        }
        else
        {
            TCheckAndSlot checkslot = make_checkslot<TType>(objSltId,type->checker());
            {
                std::lock_guard<std::recursive_mutex> lk(m_mtxsigslt);
                m_thrdObject[thrd.get()].insert(std::make_pair(objSltId.first, checkslot));
            }
        }
    }

    template<typename TType>
    TCheckAndSlot make_checkslot(TObjectSlotID& objSltId, const std::weak_ptr<check_shared_ptr> checker )
    {
        ADefineSet<TSlotID> setSlot;
        setSlot.insert(objSltId.second);
        return std::make_pair(checker,setSlot);
    }

    TThreadMap getThreadMap(void)
    {
        std::lock_guard<std::recursive_mutex> lk(m_mtxsigslt);
        return m_thrdMap;
    }

    TObjectToSlot getObjectToSlot(AThreadBase* pThrd)
    {
        std::lock_guard<std::recursive_mutex> lk(m_mtxsigslt);
        return m_thrdObject[pThrd];
    }

    void eraseThreadMap(AThreadBase* pThrd)
    {
        std::lock_guard<std::recursive_mutex> lk(m_mtxsigslt);
        if(m_thrdMap.count(pThrd)>0)
            m_thrdMap.erase(pThrd);
    }

    void eraseThreadToObject(AThreadBase* pThrd)
    {
        std::lock_guard<std::recursive_mutex> lk(m_mtxsigslt);
        if(m_thrdObject.count(pThrd)>0)
            m_thrdObject.erase(pThrd);
    }

    void eraseObjectToSlot(AThreadBase* pThrd,TObjectId objId)
    {
        std::lock_guard<std::recursive_mutex> lk(m_mtxsigslt);
        m_thrdObject[pThrd].erase(objId);
    }
protected:
    TThreadMap m_thrdMap;
    TThreadToObject m_thrdObject;
    std::recursive_mutex m_mtxsigslt;
RTC_DISALLOW_COPY_AND_ASSIGN(ASignal)
};
#endif // ASIGNAL_HPP
