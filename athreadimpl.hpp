#ifndef ATHREADIMPL_HPP
#define ATHREADIMPL_HPP

#include <asignal.hpp>

/**
 * @brief The AThreadImpl class
 * 名称：线程实现类
 * 功能：该类使用了std::thread实现了消息循环的功能，用于实现消息的处理。
 *      可以通过组合或继承该类来实现信号槽机制。
 * 限制：不可被拷贝和赋值
 */
class AThreadImpl : public AThreadBase
{
public:
    AThreadImpl(bool isSuspendCreated = true)
        :m_thread(entrance,this),m_isTerminate(false),m_isRun(false),m_isSuspendCreated(isSuspendCreated)
    {}

    virtual ~AThreadImpl()
    {
        erase(m_thread.get_id());
        stop();
    }

    void exec(void)
    {
        sigStart();
        while(!m_isTerminate)
        {
            dealMessage();
        }
        sigStop();
    }

    virtual void run(void)
    {
        exec();
    }
    void entrance(void)
    {
        handup();
        run();
    }

    std::thread::id id()
    {
        return m_thread.get_id();
    }

    void start(void)
    {
        if(false==m_isRun.load())
        {
            if(true==m_isSuspendCreated.load())
                m_isSuspendCreated.store(false);
            m_isRun.store(true);
            m_cv.notify_one();
        }
    }

    void stop(void)
    {
        m_isTerminate.store(true);
        start();
        if(m_thread.joinable())
            m_thread.join();
    }
protected:
    void handup(void)
    {
        if(true==m_isSuspendCreated.load())
        {
            m_isSuspendCreated.store(false);
            m_isRun.store(false);
            std::unique_lock<std::mutex> lk(m_mtx);
            m_cv.wait(lk);
        }
    }

    void wait(std::chrono::milliseconds ms)
    {
        if(true==m_isRun.load())
        {
            m_isRun.store(false);
            std::unique_lock<std::mutex> lk(m_mtx);
            if(std::cv_status::timeout==m_cv.wait_for(lk,ms))
                m_isRun.store(true);
        }
    }
public:
    ASignal<> sigStart,sigStop;
protected:
    std::thread m_thread;
    std::condition_variable m_cv;
    std::mutex m_mtx;
    std::atomic_bool m_isTerminate;
    std::atomic_bool m_isRun;
    std::atomic_bool m_isSuspendCreated;
};

/**
 * @brief The AEventLoopForMain class
 * 名称：主线程消息循环类
 * 功能：
 *      1.提供给主线程的消息循环功能
 *      2.可以提供给全局或静态的函数注册到该单例对象中，并在该消息循环处理
 *      3.开始消息循环时，发出信号sigStart，结束消息循环时，发出信号sigStop
 * 限制：
 *      1.该类为单例，并且不可被赋值和拷贝，不可被继承
 */
class AEventLoopForMain final : public AThreadBase
{
public:
    static std::shared_ptr<AEventLoopForMain>& instance()
    {
        static std::shared_ptr<AEventLoopForMain> main = AThreadBase::create<AEventLoopForMain>();
        return main;
    }
protected:
    AEventLoopForMain()
        :m_isTerminate(false)
    {}

    AEventLoopForMain(AEventLoopForMain&) = delete;
public:
    void exec(void)
    {
        sigStart();
        while(!m_isTerminate)
        {
            dealMessage();
        }
        sigStop();
    }

    std::thread::id id()
    {
        return std::this_thread::get_id();
    }

    void start(void)
    {
        if(false==m_isRun.load())
        {
            m_isRun.store(true);
            m_cv.notify_one();
        }
    }

    void stop(void)
    {
        m_isTerminate.store(true);
        start();
    }
protected:
    void handup(void)
    {
        if(true==m_isRun.load())
        {
            m_isRun.store(false);
            std::unique_lock<std::mutex> lk(m_mtx);
            m_cv.wait(lk);
        }
    }

    void wait(std::chrono::milliseconds ms)
    {
        if(true==m_isRun.load())
        {
            m_isRun.store(false);
            std::unique_lock<std::mutex> lk(m_mtx);
            if(std::cv_status::timeout==m_cv.wait_for(lk,ms))
                m_isRun.store(true);
        }
    }
public:
    ASignal<> sigStart,sigStop;
protected:
    std::condition_variable m_cv;
    std::mutex m_mtx;
    std::atomic_bool m_isTerminate;
    std::atomic_bool m_isRun;

    friend class AThreadBase;
};

/**
 * @brief The AEventLoop class
 * 名称：消息循环类
 * 功能：
 *      用于在该类所属的线程中嵌套启用一个消息循环，
 *      并且在该消息循环退出时，可以返回到开始执行的地方。
 *      开始消息循环时，发出信号sigStart，结束消息循环时，发出信号sigStop
 *      当接收到信号sigQuit时，退出循环
 * 限制：
 *      不可被拷贝和赋值，不可被继承
 */
class AEventLoop final
{
public:
    AEventLoop()
        :m_isQuit(false)
    {
        moveToThread(AThreadBase::current());
    }

    ASignal<> sigQuit;
    void sltQuit(void)
    {
        m_isQuit.store(true);
        if(thread().expired()) return;
        thread().lock()->start();
    }
    void exec(void)
    {
        sigStart();
        sigQuit.connect(&sltQuit,getThis<AEventLoop>());
        while(!m_isQuit)
        {
            if(thread().expired())
            {
                m_isQuit.store(true);
                return ;
            }
            try
            {
                thread().lock()->dealMessage();
                continue;
            }
            catch(std::exception &e)
            {
                (void)e;
            }
            catch(...)
            {}
        }
        sigStop();
    }
public:
    ASignal<> sigStart,sigStop;
protected:
    std::atomic_bool m_isQuit;

DEFINE_SIGNALSLOTMAP_DISINHERITED(AEventLoop)
};

class ATimerSingle : protected AThreadImpl
{
public:
    ATimerSingle()
        :AThreadImpl()
        ,m_nCount(0)
    {}
public:
    void start(uint32_t nCount)
    {
        m_nCount = nCount;
        AThreadImpl::start();
    }

public:
    void run(void)
    {
        while(!m_isTerminate)
        {
            wait();
            std::this_thread::sleep_for(std::chrono::milliseconds(m_nCount));
            sigTimeOut();
        }
    }
public:
    ASignal<> sigTimeOut;
protected:
    void wait(void)
    {
        if(false==m_isRun) return ;
        m_isRun.store(false);
        std::unique_lock<std::mutex> lk(m_mtx);
        m_cv.wait(lk);
    }
    void handup(){}
protected:
    uint32_t m_nCount;
};

template<uint32_t nId=0>
class ATimerThread : public AThreadImpl
{
public:
    static std::shared_ptr<ATimerThread>& instance()
    {
        static std::shared_ptr<ATimerThread> tt(AThreadBase::create<ATimerThread>());
        return tt;
    }
public:
    ATimerThread()
        :AThreadImpl()
        ,m_tpStart(std::chrono::steady_clock::now())
    {}
    ~ATimerThread()
    {}
    void dealMessage()
    {
        if(sigCount.empty())
        {
            wait(std::chrono::seconds(1800));
            return ;
        }
        else
        {
            int32_t ms = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now()-m_tpStart).count();
            if(10>ms)
                wait(std::chrono::milliseconds(ms));
        }
        m_tpStart = std::chrono::steady_clock::now();
        sigCount();
    }
protected:
    void handup(){}
public:
    ASignal<> sigCount;
protected:
    std::chrono::steady_clock::time_point m_tpStart;
};

template<uint32_t nId>
class ATimer final
{
public:
    ATimer()
    {
        this->moveToThread(ATimerThread<nId>::instance());
    }

    ~ATimer()
    {}
    void start(uint32_t nCount)
    {
        m_nts = std::chrono::milliseconds(nCount);
        m_tpStart = std::chrono::steady_clock::now();
        ATimerThread<nId>::instance()->sigCount.connect(&sltCount,getThis<ATimer>());
        ATimerThread<nId>::instance()->start();
    }
protected:
    void sltCount()
    {
        if(std::chrono::steady_clock::now()-m_tpStart >= m_nts)
        {
            ATimerThread<nId>::instance()->sigCount.disconnect(&sltCount,getThis<ATimer>());
            sigTimeOut();
        }
    }
public:
    ASignal<> sigTimeOut;
protected:
    std::chrono::milliseconds m_nts;
    std::chrono::steady_clock::time_point m_tpStart;
DEFINE_SIGNALSLOTMAP_DISINHERITED(ATimer)
};
template<uint32_t nId>
class AWaitProcess
{
public:
    enum : uint32_t{
        WTPROC_UNKNOWN = (uint32_t)-1,     //未知状态，等待处理执行等待时设置为未知
        WTPROC_FINISH  = 0,      //通知等待处理执行完成
        WTPROC_TIMEOUT = 1,      //等待处理超时
    };
public:
    AWaitProcess()
    {
        m_eventloop = AEventLoop::create();
        moveToThread(AThreadBase::current());
    }
    ~AWaitProcess(){}
public:
    uint32_t wait(uint32_t ms)
    {
        m_nStatus.store(WTPROC_UNKNOWN);
        m_args.reset();
        sigNotify.connect(&sltNotify,getThis<AWaitProcess>());
        std::shared_ptr<ATimer<nId>> timer = ATimer<nId>::create();
        timer->sigTimeOut.connect(&(m_eventloop->sigQuit),m_eventloop);
        timer->start(ms);
        m_eventloop->exec();
        if(WTPROC_UNKNOWN==m_nStatus.load())
            m_nStatus.store(WTPROC_TIMEOUT);
        return m_nStatus;
    }

    any& get(void){ return m_args; }
public:
    ASignal<any> sigNotify;
    void sltNotify(any args)
    {
        if(WTPROC_UNKNOWN!=m_nStatus.load()) return ;
        m_args = args;
        m_nStatus.store(WTPROC_FINISH);
        m_eventloop->sigQuit();
    }
protected:
    std::shared_ptr<AEventLoop> m_eventloop;
    std::atomic_uint m_nStatus;
    any m_args;

DEFINE_SIGNALSLOTMAP_DISINHERITED(AWaitProcess)
};
#endif // ATHREADIMPL_HPP
