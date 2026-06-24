#pragma once

namespace ego
{
    class NonInstanceable
    {
    public:
        NonInstanceable() = delete;
        virtual ~NonInstanceable() = default;
    };
} // namespace ego