#ifndef BTY_ENGINE_SINGLETON_HPP
#define BTY_ENGINE_SINGLETON_HPP

namespace bty {

template <typename T>
struct SingletonProvider {
    static T& instance()
    {
        static T instance_ {};
        return instance_;
    }
};

}    // namespace bty

#endif    // BTY_ENGINE_SINGLETON_HPP
