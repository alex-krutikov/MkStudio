#ifndef __GLOBAL_EXIT_MANAGER__H__
#define __GLOBAL_EXIT_MANAGER__H__

#include <atomic>

class GlobalExitManager
{
public:
    class Exception
    {
    };

public:
    static GlobalExitManager &instance()
    {
        static GlobalExitManager Instance;
        return Instance;
    }

    void check()
    {
        if (m_exitFlag.load(std::memory_order_relaxed)) throw Exception{};
    }

    void activateExit() { m_exitFlag = true; }

private:
    GlobalExitManager() = default;

private:
    std::atomic_bool m_exitFlag{false};
};

#endif
