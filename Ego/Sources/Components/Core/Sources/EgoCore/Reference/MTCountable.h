#pragma once

#include <atomic>

namespace ego
{
    class MTCountable
    {
    public:
        MTCountable(uint32_t _initialCounterVal = 0);
        virtual ~MTCountable();

        void addReference();
        void releaseReference();

        uint32_t getReferenceCount() const;

    protected:
        virtual void onRelease() = 0;

        void incrementCount();
        void decrementCount();

    private:
        std::atomic<uint32_t> m_count;
    };

    class STDDestroyMTCountable : public MTCountable
    {
    public:
        STDDestroyMTCountable(uint32_t _initialCounterVal = 0)
            : MTCountable(_initialCounterVal)
        {
        }

    private:
        void onRelease() override;
    };
} // namespace ego
