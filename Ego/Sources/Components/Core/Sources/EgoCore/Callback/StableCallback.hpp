#include <utility>

template <typename... TArguments>
ego::StableCallback<TArguments...>::State::State(Function _function)
    : m_function(std::move(_function))
{
}

template <typename... TArguments>
void ego::StableCallback<TArguments...>::set(Function _function)
{
    if (!_function)
    {
        m_state.reset();
        return;
    }

    m_state = new State(std::move(_function));
}

template <typename... TArguments>
void ego::StableCallback<TArguments...>::invoke(TArguments... _arguments) const
{
    const StateReference state = m_state;
    if (state)
    {
        state->m_function(std::forward<TArguments>(_arguments)...);
    }
}
