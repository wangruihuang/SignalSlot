#ifndef APINGPONGQUEUE_HPP
#define APINGPONGQUEUE_HPP
#include <deque>
#include <atomic>
/**
 * 名称：乒乓队列
 * 功能：使用乒乓操作来管理队列，将写和读的队列分开，对于单一操作(只有一个操作者)，可以不用加锁；
 *      例如：
 *      (front,pop)读提供给唯一线程(消费者)，不用加锁
 *      (push)写提供给多个发送线程(生产者），需要加锁
 */
template<typename T>
class APingPongQueue
{
public:
    typedef typename std::deque<T>::reference reference;
    typedef T value_type;
public:
    void push(const value_type& t)
    {
        if(true==m_switch.load())
        {
            m_deque[0].push_back(t);
        }
        else
        {
            m_deque[1].push_back(t);
        }
    }
#if __cplusplus >= 201103L
    void push(value_type&& t)
    {
        if(true==m_switch.load())
        {
            m_deque[0].push_back(t);
        }
        else
        {
            m_deque[1].push_back(t);
        }
    }
#endif
    void pop(void)
    {
        if(true==m_switch.load())
        {
            m_deque[1].pop_front();
            if(m_deque[1].empty())
            {
                m_switch.store(false);
                return ;
            }
        }
        else
        {
            m_deque[0].pop_front();
            if(m_deque[0].empty())
            {
                m_switch.store(true);
            }
        }
    }

    reference front(void)
    {
        if(true==m_switch.load())
            return m_deque[1].front();
        else
            return m_deque[0].front();
    }

    size_t size(void)
    {
        return m_deque[0].size() + m_deque[1].size();
    }

    bool empty(void)
    {
        if(true==m_switch.load())
            return m_deque[1].empty();
        else
            return m_deque[0].empty();
    }
    bool empty_re(void)
    {
        if(true==m_switch.load())
            return m_deque[0].empty();
        else
            return m_deque[1].empty();
    }

    void turn(void)
    {
        m_switch.store(!m_switch.load());
    }
protected:
    std::atomic_bool m_switch;
    std::deque<T> m_deque[2];
};

#endif
