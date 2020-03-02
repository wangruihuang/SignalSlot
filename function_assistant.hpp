#ifndef FUNCTION_ASSISTANT_HPP
#define FUNCTION_ASSISTANT_HPP
#include <tuple>
#include <functional>
#include <memory>

#if __cplusplus <= 201103L
#include <any.hpp>
using any = MyZone::any;
#define any_cast MyZone::any_cast
#else
#include <any>
using any = std::any;
#define any_cast std::any_cast
#endif

#ifdef __USE_UNORDERED
#include <unordered_set>
#include <unordered_map>
#define ADefineSet std::unordered_set
#define ADefineMap std::unordered_map
#else
#include <set>
#include <map>
#define ADefineSet std::set
#define ADefineMap std::map
#endif

template<typename TFunc,typename TTuple, size_t size>
struct TFunctionUnpack;

template<typename TFunc,typename TTuple>
struct TFunctionUnpack<TFunc,TTuple,0>
{
    void operator()(TFunc func, TTuple tp)
    {
        (void)tp;
        func();
    }
};

template<typename TFunc,typename TTuple>
struct TFunctionUnpack<TFunc,TTuple,1>
{
    void operator()(TFunc func, TTuple tp)
    {
        func(std::get<0>(tp));
    }
};

template<typename TFunc,typename TTuple>
struct TFunctionUnpack<TFunc,TTuple,2>
{
    void operator()(TFunc func, TTuple tp)
    {
        func(std::get<0>(tp),std::get<1>(tp));
    }
};

template<typename TFunc,typename TTuple>
struct TFunctionUnpack<TFunc,TTuple,3>
{
    void operator()(TFunc func, TTuple tp)
    {
        func(std::get<0>(tp),std::get<1>(tp),std::get<2>(tp));
    }
};

template<typename TFunc,typename TTuple>
struct TFunctionUnpack<TFunc,TTuple,4>
{
    void operator()(TFunc func, TTuple tp)
    {
        func( std::get<0>(tp),std::get<1>(tp),std::get<2>(tp)
             ,std::get<3>(tp));
    }
};

template<typename TFunc,typename TTuple>
struct TFunctionUnpack<TFunc,TTuple,5>
{
    void operator()(TFunc func, TTuple tp)
    {
        func( std::get<0>(tp),std::get<1>(tp),std::get<2>(tp)
             ,std::get<3>(tp),std::get<4>(tp));
    }
};

template<typename TFunc,typename TTuple>
struct TFunctionUnpack<TFunc,TTuple,6>
{
    void operator()(TFunc func, TTuple tp)
    {
        func( std::get<0>(tp),std::get<1>(tp),std::get<2>(tp)
             ,std::get<3>(tp),std::get<4>(tp),std::get<5>(tp));
    }
};

template<typename TFunc,typename TTuple>
struct TFunctionUnpack<TFunc,TTuple,7>
{
    void operator()(TFunc func, TTuple tp)
    {
        func( std::get<0>(tp),std::get<1>(tp),std::get<2>(tp)
             ,std::get<3>(tp),std::get<4>(tp),std::get<5>(tp)
             ,std::get<6>(tp));
    }
};

template<typename TFunc,typename TTuple>
struct TFunctionUnpack<TFunc,TTuple,8>
{
    void operator()(TFunc func, TTuple tp)
    {
        func( std::get<0>(tp),std::get<1>(tp),std::get<2>(tp)
             ,std::get<3>(tp),std::get<4>(tp),std::get<5>(tp)
             ,std::get<6>(tp),std::get<7>(tp));
    }
};

template<typename TFunc,typename TTuple>
struct TFunctionUnpack<TFunc,TTuple,9>
{
    void operator()(TFunc func, TTuple tp)
    {
        func( std::get<0>(tp),std::get<1>(tp),std::get<2>(tp)
             ,std::get<3>(tp),std::get<4>(tp),std::get<5>(tp)
             ,std::get<6>(tp),std::get<7>(tp),std::get<8>(tp));
    }
};

template<size_t size,typename TType, typename... _ArgTypes>
struct TFunctionBind;

template<typename TType, typename... _ArgTypes>
struct TFunctionBind<0,TType,_ArgTypes...>
{
    typedef void(TType::*TFunc)(_ArgTypes...);
    std::function<void(_ArgTypes...)> operator()(TFunc func,TType *type)
    {
        return std::function<void(_ArgTypes...)>(std::bind(func,type));
    }
};

template<typename TType, typename... _ArgTypes>
struct TFunctionBind<1,TType,_ArgTypes...>
{
    typedef void(TType::*TFunc)(_ArgTypes...);
    std::function<void(_ArgTypes...)> operator()(TFunc func,TType *type)
    {
        return std::function<void(_ArgTypes...)>(std::bind(func,type
                ,std::placeholders::_1));
    }
};

template<typename TType, typename... _ArgTypes>
struct TFunctionBind<2,TType,_ArgTypes...>
{
    typedef void(TType::*TFunc)(_ArgTypes...);
    std::function<void(_ArgTypes...)> operator()(TFunc func,TType *type)
    {
        return std::function<void(_ArgTypes...)>(std::bind(func,type
                ,std::placeholders::_1,std::placeholders::_2));
    }
};

template<typename TType, typename... _ArgTypes>
struct TFunctionBind<3,TType,_ArgTypes...>
{
    typedef void(TType::*TFunc)(_ArgTypes...);
    std::function<void(_ArgTypes...)> operator()(TFunc func,TType *type)
    {
        return std::function<void(_ArgTypes...)>(std::bind(func,type
                ,std::placeholders::_1,std::placeholders::_2,std::placeholders::_3));
    }
};

template<typename TType, typename... _ArgTypes>
struct TFunctionBind<4,TType,_ArgTypes...>
{
    typedef void(TType::*TFunc)(_ArgTypes...);
    std::function<void(_ArgTypes...)> operator()(TFunc func,TType *type)
    {
        return std::function<void(_ArgTypes...)>(std::bind(func,type
                ,std::placeholders::_1,std::placeholders::_2,std::placeholders::_3
                ,std::placeholders::_4));
    }
};

template<typename TType, typename... _ArgTypes>
struct TFunctionBind<5,TType,_ArgTypes...>
{
    typedef void(TType::*TFunc)(_ArgTypes...);
    std::function<void(_ArgTypes...)> operator()(TFunc func,TType *type)
    {
        return std::function<void(_ArgTypes...)>(std::bind(func,type
                ,std::placeholders::_1,std::placeholders::_2,std::placeholders::_3
                ,std::placeholders::_4,std::placeholders::_5));
    }
};

template<typename TType, typename... _ArgTypes>
struct TFunctionBind<6,TType,_ArgTypes...>
{
    typedef void(TType::*TFunc)(_ArgTypes...);
    std::function<void(_ArgTypes...)> operator()(TFunc func,TType *type)
    {
        return std::function<void(_ArgTypes...)>(std::bind(func,type
                ,std::placeholders::_1,std::placeholders::_2,std::placeholders::_3
                ,std::placeholders::_4,std::placeholders::_5,std::placeholders::_6));
    }
};

template<typename TType, typename... _ArgTypes>
struct TFunctionBind<7,TType,_ArgTypes...>
{
    typedef void(TType::*TFunc)(_ArgTypes...);
    std::function<void(_ArgTypes...)> operator()(TFunc func,TType *type)
    {
        return std::function<void(_ArgTypes...)>(std::bind(func,type
                ,std::placeholders::_1,std::placeholders::_2,std::placeholders::_3
                ,std::placeholders::_4,std::placeholders::_5,std::placeholders::_6
                ,std::placeholders::_7));
    }
};

template<typename TType, typename... _ArgTypes>
struct TFunctionBind<8,TType,_ArgTypes...>
{
    typedef void(TType::*TFunc)(_ArgTypes...);
    std::function<void(_ArgTypes...)> operator()(TFunc func,TType *type)
    {
        return std::function<void(_ArgTypes...)>(std::bind(func,type
                ,std::placeholders::_1,std::placeholders::_2,std::placeholders::_3
                ,std::placeholders::_4,std::placeholders::_5,std::placeholders::_6
                ,std::placeholders::_7,std::placeholders::_8));
    }
};

template<typename TType, typename... _ArgTypes>
struct TFunctionBind<9,TType,_ArgTypes...>
{
    typedef void(TType::*TFunc)(_ArgTypes...);
    std::function<void(_ArgTypes...)> operator()(TFunc func,TType *type)
    {
        return std::function<void(_ArgTypes...)>(std::bind(func,type
               ,std::placeholders::_1,std::placeholders::_2,std::placeholders::_3
               ,std::placeholders::_4,std::placeholders::_5,std::placeholders::_6
               ,std::placeholders::_7,std::placeholders::_8,std::placeholders::_9));
    }
};

template<typename TParams>
struct TFuntorWrapper
{
    TFuntorWrapper(std::function<void(TParams)> func)
        :m_func(func){}
    void operator()(any& a)
    {
        m_func(any_cast<TParams>(a));
    }
protected:
    std::function<void(TParams)> m_func;
};

template<typename... _ArgTypes>
struct TFuntorWrapper2
{
    TFuntorWrapper2(std::function<void(_ArgTypes...)> func)
        :m_func(func){}

    void operator()(MyZone::any& a)
    {
        TFunctionUnpack<std::function<void(_ArgTypes...)>,std::tuple<_ArgTypes...>,sizeof...(_ArgTypes)> func;
        func(m_func,any_cast<std::tuple<_ArgTypes...>>(a));
    }
protected:
    std::function<void(_ArgTypes...)> m_func;
};

template<>
struct TFuntorWrapper2<void>
{
    TFuntorWrapper2(std::function<void(void)> func)
        :m_func(func){}

    void operator()(any& a)
    {
        (void)a;
        m_func();
    }
protected:
    std::function<void(void)> m_func;
};

template <typename TRet,typename TFunc>
TRet memfunc_convert(TFunc f)
{
    void * p = malloc(sizeof(TFunc));
    TFunc* pp = new(p) TFunc(f);
    TRet t;
    memcpy(&t,p,sizeof(TRet));
    delete pp;
    free(p);
    return t;
}

class check_shared_ptr
{
public:
    virtual bool valid(void)=0;
    virtual void callSlot(uint32_t sltid, any& args)=0;
    virtual ~check_shared_ptr(){}
};

template<typename T>
class check_shared_ptr_impl : public check_shared_ptr
{
public:
    check_shared_ptr_impl(std::weak_ptr<T> wkptr)
        :m_wkptr(wkptr)
    {}
    bool valid(void)
    {
        return !m_wkptr.expired();
    }

    void callSlot(uint32_t sltid, any& args)
    {
        if(valid())
            get()->callSlot(sltid, args);
    }

    std::shared_ptr<T> get(void)
    {
        return m_wkptr.lock();
    }

protected:
    std::weak_ptr<T> m_wkptr;
};

#define RTC_DISALLOW_ASSIGN(TypeName) \
  void operator=(const TypeName&) = delete;

#define RTC_DISALLOW_COPY_AND_ASSIGN(TypeName) \
  TypeName(const TypeName&) = delete;          \
  RTC_DISALLOW_ASSIGN(TypeName)

#define DECLARE_CREATE_THREAD \
    public: \
        template<typename T, typename ...TArgs>\
        static std::shared_ptr<T> create(TArgs ...args)\
        {\
            std::shared_ptr<T> shptr(new T(args...)); \
            check_shared_ptr *p = new check_shared_ptr_impl<T>(shptr); \
            shptr->m_checker = std::shared_ptr<check_shared_ptr>(p);\
            AThreadBase::add(shptr->id(),shptr);\
            shptr->moveToThread(shptr);\
            return shptr;\
        }
#define DECLARE_CREATE_NORMAL \
    public: \
        template<typename T, typename ...TArgs>\
        static std::shared_ptr<T> create(TArgs ...args)\
        {\
            std::shared_ptr<T> shptr(new T(args...)); \
            check_shared_ptr *p = new check_shared_ptr_impl<T>(shptr); \
            shptr->m_checker = std::shared_ptr<check_shared_ptr>(p);\
            return shptr;\
        }

#define DECLARE_CREATE_DISINHERITED(TypeName) \
    public: \
        template<typename ...TArgs>\
        static std::shared_ptr<TypeName> create(TArgs ...args)\
        {\
            std::shared_ptr<TypeName> shptr(new TypeName(args...)); \
            check_shared_ptr *p = new check_shared_ptr_impl<TypeName>(shptr); \
            shptr->m_checker = std::shared_ptr<check_shared_ptr>(p);\
            return shptr;\
        }
#define DECLARE_SIGNALSLOTMAP(TypeName) \
public: \
    template<typename T> \
    std::shared_ptr<T> getThis(void)\
    {\
        check_shared_ptr_impl<T> *pChecker = \
            dynamic_cast<check_shared_ptr_impl<T>* >(m_checker.get());\
        if(nullptr==pChecker) return std::shared_ptr<T>();\
        return pChecker->get();\
    }\
    void moveToThread(std::weak_ptr<AThreadBase> thrd) \
    {\
        if(empty())\
            m_thrdOwner = thrd;\
    }\
    std::weak_ptr<check_shared_ptr> checker(void)\
    {\
        return m_checker;\
    }\
    void callSlot(uint32_t sltid, any& args) \
    { \
        if(m_mapSlot.count(sltid)<=0) return ; \
        m_mapSlot[sltid](args); \
    } \
    void regSlot(uint32_t sltid, std::function<void(any&)> func) \
    { \
        if(m_mapSlot.count(sltid)>0) return ; \
        m_mapSlot.insert(std::make_pair(sltid,func)); \
    } \
    std::weak_ptr<AThreadBase> thread() \
    { \
        return m_thrdOwner; \
    } \
    bool empty(void) \
    { \
        return m_mapSlot.empty(); \
    } \
    void clear(void) \
    { \
        m_mapSlot.clear(); \
        m_thrdOwner.reset(); \
    } \
protected: \
    std::shared_ptr<check_shared_ptr> m_checker;\
    std::weak_ptr<AThreadBase> m_thrdOwner; \
    ADefineMap<uint32_t,std::function<void(any&)> > m_mapSlot; \
private:
/**
 * 名称：定义信号槽功能宏
 * 功能：
 *      增加改宏，以支持把所属类的方法定义为槽
 *      槽可以为普通方法，虚拟方法，仿函数，静态函数等
 * 限制：
 *      1.成为槽的类不能被拷贝和赋值，如果需要拷贝，请自己实现clone函数
 *      2.如果在槽调用之前对象被析构，可以保证不会响应消息调用该对象的槽，
 *        但是如果是在执行中被析构则会导致异常(如内存非法访问，野指针操作等)
 *        因此，应该确保槽所属类的生命周期大于等于类所属线程的生命周期
 *      3.如果要连接信号，必须为信号指定槽处理的对象
 *      4.只支持返回值为void的信号和槽相连接
 *      5.虽然提供了检测对象有效性的机制，但是无法控制对象的析构操作
 *        如果想对对象的析构进行控制，则必须控制对象的构造，
 *        从而过多干预使用者的行为习惯，所以目前不考虑支持
 */
#define DEFINE_SIGNALSLOTMAP(TypeName)  \
    DECLARE_CREATE_NORMAL \
    DECLARE_SIGNALSLOTMAP(TypeName)

#define DEFINE_SIGNALSLOTMAP_DISINHERITED(TypeName) \
    DECLARE_CREATE_DISINHERITED(TypeName) \
    DECLARE_SIGNALSLOTMAP(TypeName)

#define DEFINE_SIGNALSLOTMAP_THREAD(TypeName) \
    DECLARE_CREATE_THREAD \
    DECLARE_SIGNALSLOTMAP(TypeName)

#endif // FUNCTION_ASSISTANT_HPP
