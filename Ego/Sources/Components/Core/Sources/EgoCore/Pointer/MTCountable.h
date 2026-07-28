#pragma once

#include <atomic>

namespace ego
{
    template <typename T>
    class IntrusivePointer;

    template <typename T, typename... TArguments>
    IntrusivePointer<T> MakeIntrusive(TArguments&&... _arguments);

    template <typename T, typename TDeleter, typename... TArguments>
    IntrusivePointer<T> MakeIntrusiveWithDeleter(TArguments&&... _arguments);

    class MTCountable
    {
        template <typename T, typename TDeleter, typename... TArguments>
        friend IntrusivePointer<T> MakeIntrusiveWithDeleter(TArguments&&... _arguments);

    public:
        MTCountable();
        virtual ~MTCountable();

        void addReference();
        void releaseReference();

        uint32_t getReferenceCount() const;

    protected:
        void incrementCount();
        void decrementCount();

    private:
        using DestroyFunction = void (*)(MTCountable*) noexcept;

        void setDestroyFunction(DestroyFunction _destroyFunction);

    private:
        std::atomic<uint32_t> m_count;
        DestroyFunction m_destroyFunction;
    };
} // namespace ego
