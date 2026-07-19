#pragma once

#include <functional>

#include "EgoCore/Reference/Reference.h"

namespace ego
{
    template <typename... TArguments>
    class StableCallback final
    {
    public:
        using Function = std::function<void(TArguments...)>;

        void set(Function _function);
        void invoke(TArguments... _arguments) const;

    private:
        class State final : public STDDestroyMTCountable
        {
        public:
            explicit State(Function _function);

            Function m_function;
        };

        using StateReference = Reference<State>;

        StateReference m_state;
    };
} // namespace ego

#include "StableCallback.hpp"
