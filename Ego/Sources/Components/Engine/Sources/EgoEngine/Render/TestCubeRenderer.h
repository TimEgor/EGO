#pragma once

#include <cstdint>

namespace ego
{
    class Window;
}

namespace ego::render
{
    class TestCubeRenderer final
    {
    public:
        TestCubeRenderer() = default;
        ~TestCubeRenderer();

        bool init(Window& _window);
        void release();
        void render(float _deltaTime);

    private:
        struct Impl;

        Impl* m_impl = nullptr;
    };
}
