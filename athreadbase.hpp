#ifndef ATHREADBASE_HPP
#define ATHREADBASE_HPP

#include <thread>
#include <condition_variable>
#include <mutex>
#include <apingpongqueue.hpp>
#include <function_assistant.hpp>

/**
 * @brief The AThreadBase class
 * 名称：线程基类
 * 功能：定义了用于处理消息循环的基本接口和基本功能的抽象类
 */
class AThreadBase
{
public:
    using TSlotID = uint32_t;
    using TObjectId = uint32_t;
    using TCheckPointer = std::weak_ptr<check_shared_ptr>;
    using TCheckAndSlot = std::pair<TCheckPointer ,ADefineSet<TSlotID> >;
    using TObjectToSlot = ADefineMap<TObjectId,TCheckAndSlot>;
    using TMessage = std::pair<TObjectToSlot, any >;
    using TThreadCheck = std::weak_ptr<AThreadBase>;
public:
    static TThreadCheck current()
    {
        return get(std::this_thread::get_id());
    }

    static TThreadCheck get(std::thread::id id)
    {
        std::lock_guard<std::recursive_mutex> lk(m_mtxstatic);
        if(m_idtothread.count(id)>0)
        {
            return m_idtothread[id];
        }
        return TThreadCheck();
    }
protected:
    static void add(std::thread::id id, TThreadCheck thrd)
    {
        std::lock_guard<std::recursive_mutex> lk(m_mtxstatic);
        if(m_idtothread.count(id)<=0)
        {
            m_idtothread.insert(std::make_pair(id,thrd));
        }
    }

    static void erase(std::thread::id id)
    {
        std::lock_guard<std::recursive_mutex> lk(m_mtxstatic);
        if(m_idtothread.count(id)>0)
        {
            m_idtothread.erase(id);
        }
    }
public:
    AThreadBase()
    {
    }
    virtual ~AThreadBase()
    {}
    void push(TMessage& msg)
    {
        std::lock_guard<std::recursive_mutex> lk(m_mtxque);
        m_queue.push(msg);
        start();
    }
    void push(TMessage&& msg)
    {
        std::lock_guard<std::recursive_mutex> lk(m_mtxque);
        m_queue.push(msg);
        start();
    }

    virtual void dealMessage(void)
    {
        if(m_queue.empty())
        {
            if(m_queue.empty_re())
            {
                wait(std::chrono::milliseconds(100));
                return ;
            }
            else
                m_queue.turn();
        }

        TMessage msg = m_queue.front();
        m_queue.pop();
        TObjectToSlot &obj2slt = msg.first;
        for(auto iter = obj2slt.begin();iter!=obj2slt.end();++iter)
        {
            if(false==iter->second.first.expired())
            {
                auto mapSlot = iter->second.second;
                for(auto item : mapSlot)
                {
                    if(false == callSlot(iter->second.first.lock(),item,msg.second))
                        break;
                }
            }
        }
    }
    virtual std::thread::id id() = 0;
    virtual void start(void)=0;
    virtual void stop(void)=0;

protected:
    virtual void handup()=0;
    virtual void wait(std::chrono::milliseconds ms)=0;

    template<typename TCaller>
    bool callSlot(TCaller caller, TSlotID sltid, any& args)
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
protected:
    static ADefineMap<std::thread::id,TThreadCheck> m_idtothread;
    static std::recursive_mutex m_mtxstatic;
protected:
    APingPongQueue<TMessage> m_queue;
    std::recursive_mutex m_mtxque;
DEFINE_SIGNALSLOTMAP_THREAD(AThreadBase)
};

#endif // ATHREADBASE_H
