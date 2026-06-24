#pragma once

namespace ego
{
    class NonCopyable
    {
    public:
        NonCopyable() = default;
        virtual ~NonCopyable() = default;

        NonCopyable(const NonCopyable&) = delete;

        void operator=(const NonCopyable&) = delete;
    };
} // namespace ego