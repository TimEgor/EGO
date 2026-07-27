#pragma once

#include <concepts>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <vector>

#include "EgoCore/Assert/Assert.h"
#include "EgoCore/Containers/ObjectPool/ObjectPool.h"
#include "EgoCore/Reference/Pointer.h"

#include "EgoCore/Event/Event.h"

namespace ego
{
    using EventCallback = std::function<void(const Event&)>;
    using EventCallbackID = uint32_t;

    union InstancedEventID final
    {
        uint64_t m_id;

        struct
        {
            EventType m_type;
            uint32_t m_key;
        };

        constexpr InstancedEventID(uint64_t _id)
            : m_id(_id)
        {
        }
        constexpr InstancedEventID(EventType _type, uint32_t _key)
            : m_type(_type),
              m_key(_key)
        {
        }

        bool operator==(const InstancedEventID& _id) const
        {
            return m_id == _id.m_id;
        }
        bool operator!=(const InstancedEventID& _id) const
        {
            return m_id != _id.m_id;
        }

        bool operator<(const InstancedEventID& _id) const
        {
            return m_id < _id.m_id;
        }
        bool operator>(const InstancedEventID& _id) const
        {
            return m_id > _id.m_id;
        }
    };
} // namespace ego

namespace std
{
    template <>
    struct hash<ego::InstancedEventID>
    {
        std::size_t operator()(const ego::InstancedEventID& _instance) const noexcept
        {
            return std::hash<uint64_t>()(_instance.m_id);
        }
    };
} // namespace std

namespace ego
{
    using InstancedEventCallback = std::function<void(const Event&)>;
    using InstancedEventCallbackID = uint32_t;

    constexpr EventCallbackID InvalidEventCallbackID = 0;
    constexpr InstancedEventID InvalidInstancedEventID = 0;
    constexpr InstancedEventCallbackID InvalidInstancedEventCallbackID = 0;

    class EventController final
    {
    public:
        EventController() = default;
        ~EventController()
        {
            release();
        }

        bool init();
        void release();

        template <typename TEvent>
        bool registerEvent()
        {
            static_assert(std::is_base_of_v<Event, TEvent>);
            return registerEvent(TEvent::GetEventType());
        }

        template <typename TEvent>
        bool unregisterEvent()
        {
            static_assert(std::is_base_of_v<Event, TEvent>);
            return unregisterEvent(TEvent::GetEventType());
        }

        void removeEventCallback(EventCallbackID _dispatcherID);

        template <typename TEvent, typename TCallback>
            requires std::derived_from<TEvent, Event> && std::copy_constructible<std::decay_t<TCallback>> &&
                     std::invocable<std::decay_t<TCallback>&, const TEvent&>
        EventCallbackID addEventCallback(TCallback&& _callback)
        {
            using Callback = std::decay_t<TCallback>;

            return addEventCallback(
                EGO_EVENT_TYPE(TEvent),
                [callback = Callback(std::forward<TCallback>(_callback))](const Event& _event) mutable
                {
                    std::invoke(callback, static_cast<const TEvent&>(_event));
                });
        }

        template <typename TEvent, typename TReceiver, typename TMethod>
            requires std::derived_from<TEvent, Event> && std::is_member_function_pointer_v<TMethod> && std::invocable<TMethod, TReceiver&, const TEvent&>
        EventCallbackID addEventCallback(TReceiver& _receiver, TMethod _method)
        {
            return addEventCallback(
                EGO_EVENT_TYPE(TEvent),
                [receiver = std::ref(_receiver), method = _method](const Event& _event)
                {
                    std::invoke(method, receiver.get(), static_cast<const TEvent&>(_event));
                });
        }

        template <typename TEvent>
        bool emitEvent(const TEvent& _event)
        {
            return emitEvent<TEvent, TEvent>(_event);
        }

        template <typename TEvent, typename TSendingEvent>
        bool emitEvent(const TSendingEvent& _event)
        {
            static_assert(std::is_base_of_v<TEvent, TSendingEvent>);
            return emitEvent(EGO_EVENT_TYPE(TEvent), _event);
        }

        template <typename TEvent>
        InstancedEventID registerInstancedEvent()
        {
            static_assert(std::is_base_of_v<Event, TEvent>);
            return registerInstancedEvent(TEvent::GetEventType());
        }

        bool unregisterInstancedEvent(InstancedEventID _eventID);

        template <typename TEvent, typename TCallback>
            requires std::derived_from<TEvent, Event> && std::copy_constructible<std::decay_t<TCallback>> &&
                     std::invocable<std::decay_t<TCallback>&, const TEvent&>
        InstancedEventCallbackID addInstanceEventCallback(InstancedEventID _eventID, TCallback&& _callback)
        {
            using Callback = std::decay_t<TCallback>;

            return addInstanceEventCallback(
                _eventID,
                [callback = Callback(std::forward<TCallback>(_callback))](const Event& _event) mutable
                {
                    EGO_ASSERT((rtti::IsObjectBasedOn(_event, EGO_EVENT_TYPE(TEvent))));
                    std::invoke(callback, static_cast<const TEvent&>(_event));
                });
        }

        template <typename TEvent, typename TReceiver, typename TMethod>
            requires std::derived_from<TEvent, Event> && std::is_member_function_pointer_v<TMethod> && std::invocable<TMethod, TReceiver&, const TEvent&>
        InstancedEventCallbackID addInstanceEventCallback(InstancedEventID _eventID, TReceiver& _receiver, TMethod _method)
        {
            return addInstanceEventCallback(
                _eventID,
                [receiver = std::ref(_receiver), method = _method](const Event& _event)
                {
                    EGO_ASSERT((rtti::IsObjectBasedOn(_event, EGO_EVENT_TYPE(TEvent))));
                    std::invoke(method, receiver.get(), static_cast<const TEvent&>(_event));
                });
        }

        void removeInstancedEventDispatcher(InstancedEventCallbackID _dispatcherID);

        template <typename TEvent>
        bool emitInstancedEvent(InstancedEventID _eventID, const TEvent& _event)
        {
            EGO_ASSERT(rtti::IsObjectBasedOn<TEvent>(_eventID.m_type));
            return emitEvent(_eventID, _event);
        }

    private:
        struct EventCallbackData final
        {
            EventCallback m_callback;
            EventType m_eventType = InvalidEventType;
            EventCallbackID m_callbackID = InvalidEventCallbackID;
            uint32_t m_orderIndex = static_cast<uint32_t>(-1);
        };

        using CallbackPool = ObjectPool<EventCallbackData, ObjectPoolHandle32>;
        using CallbackPoolElementInfo = CallbackPool::NewElementInfo;
        using CallbackOrderCollection = std::vector<EventCallbackData*>;
        using CallbackOrdersCollection = std::unordered_map<EventType, CallbackOrderCollection>;

        struct InstancedEventCallbackData final
        {
            InstancedEventCallback m_callback;
            InstancedEventID m_eventID = InvalidInstancedEventID;
            InstancedEventCallbackID m_prevSiblingID = InvalidInstancedEventCallbackID;
            InstancedEventCallbackID m_nextSiblingID = InvalidInstancedEventCallbackID;
        };

        using InstancedEventPool = ObjectPool<EventType, ObjectPoolHandle32>;
        using InstancedCallbackPool = ObjectPool<InstancedEventCallbackData, ObjectPoolHandle32>;
        using InstancedCallbackPoolElementInfo = InstancedCallbackPool::NewElementInfo;
        using InstancedCallbackOrdersCollection = std::unordered_map<InstancedEventID, InstancedEventCallbackID>;

        enum class PendingOperationType
        {
            RegisterEvent,
            UnregisterEvent,
            AddEventCallback,
            RemoveEventCallback,
            RegisterInstancedEvent,
            UnregisterInstancedEvent,
            AddInstancedEventCallback,
            RemoveInstancedEventCallback
        };

        struct PendingOperation final
        {
            PendingOperationType m_type = PendingOperationType::RegisterEvent;
            EventType m_eventType = InvalidEventType;
            EventCallbackID m_eventCallbackID = InvalidEventCallbackID;
            InstancedEventID m_instancedEventID = InvalidInstancedEventID;
            InstancedEventCallbackID m_instancedEventCallbackID = InvalidInstancedEventCallbackID;
        };

        using PendingOperationCollection = std::vector<PendingOperation>;

        bool registerEvent(EventType _type);
        bool unregisterEvent(EventType _type);
        bool registerEventImmediate(EventType _type);
        bool unregisterEventImmediate(EventType _type);
        bool isEventRegisteredIncludingPending(EventType _type) const;

        EventCallbackID addEventCallback(EventType _type, EventCallback _callback);
        bool addEventCallbackImmediate(EventCallbackID _callbackID);
        void removeEventCallbackImmediate(EventCallbackID _callbackID);
        bool emitEvent(EventType _type, const Event& _event);

        InstancedEventID registerInstancedEvent(EventType _type);
        bool registerInstancedEventImmediate(InstancedEventID _eventID);
        bool unregisterInstancedEventImmediate(InstancedEventID _eventID);
        bool isInstancedEventRegisteredIncludingPending(InstancedEventID _eventID) const;

        InstancedEventCallbackID addInstanceEventCallback(InstancedEventID _eventID, InstancedEventCallback _callback);
        bool addInstanceEventCallbackImmediate(InstancedEventCallbackID _callbackID);
        void removeInstancedEventCallbackImmediate(InstancedEventCallbackID _callbackID);

        bool emitEvent(InstancedEventID _eventID, const Event& _event);

        bool isDispatching() const;
        void beginDispatch();
        void endDispatch();
        void applyPendingOperations();

        CallbackPool m_callbacks;
        CallbackOrdersCollection m_callbackOrders;

        InstancedEventPool m_instancedEvents;
        InstancedCallbackPool m_instancedCallbacks;
        InstancedCallbackOrdersCollection m_instancedCallbackOrders;

        PendingOperationCollection m_pendingOperations;
        uint32_t m_dispatchDepth = 0;
    };

    EGO_POINTER(EventController);
} // namespace ego
