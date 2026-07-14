#pragma once

#include <cstddef>
#include <cstdint>
#include <functional>
#include <type_traits>
#include <unordered_map>
#include <vector>

#include "EgoCore/Assert/Assert.h"
#include "EgoCore/Containers/ObjectPool/ObjectPool.h"
#include "EgoCore/Reference/Pointer.h"

#include "EgoEvent/Event.h"

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

        template <typename TEvent>
        EventCallbackID addEventCallback(const std::function<void(const TEvent&)>& _callback)
        {
            return addEventCallback<TEvent, TEvent>(_callback);
        }

        template <typename TEvent, typename TReceivingEvent>
        EventCallbackID addEventCallback(const std::function<void(const TReceivingEvent&)>& _callback)
        {
            static_assert(std::is_base_of_v<TReceivingEvent, TEvent>);

            return addEventCallback(
                EGO_EVENT_TYPE(TEvent),
                [callback = _callback](const Event& _event)
                {
                    callback(static_cast<const TEvent&>(_event));
                });
        }

        template <typename TEvent>
        bool emitEvent(const TEvent& _event) const
        {
            return emitEvent<TEvent, TEvent>(_event);
        }

        template <typename TEvent, typename TSendingEvent>
        bool emitEvent(const TSendingEvent& _event) const
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

        template <typename TEvent>
        InstancedEventCallbackID addInstanceEventCallback(InstancedEventID _eventID, const std::function<void(const TEvent&)>& _callback)
        {
            return addInstanceEventCallback(
                _eventID,
                [callback = _callback](const Event& _event)
                {
                    EGO_ASSERT((rtti::IsObjectBasedOn(_event, EGO_EVENT_TYPE(TEvent))));
                    callback(static_cast<const TEvent&>(_event));
                });
        }

        void removeInstancedEventDispatcher(InstancedEventCallbackID _dispatcherID);

        template <typename TEvent>
        bool emitInstancedEvent(InstancedEventID _eventID, const TEvent& _event) const
        {
            EGO_ASSERT(rtti::IsObjectBasedOn<TEvent>(_eventID.m_type));
            return emitEvent(_eventID, _event);
        }

    private:
        struct EventCallbackData final
        {
            EventCallback m_callback;
            EventType m_eventType = InvalidEventType;
            uint32_t m_orderIndex = -1;
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

        bool registerEvent(EventType _type);
        bool unregisterEvent(EventType _type);

        EventCallbackID addEventCallback(EventType _type, const EventCallback& _callback);
        bool emitEvent(EventType _type, const Event& _event) const;

        InstancedEventID registerInstancedEvent(EventType _type);

        InstancedEventCallbackID addInstanceEventCallback(InstancedEventID _eventID, const InstancedEventCallback& _callback);

        bool emitEvent(InstancedEventID _eventID, const Event& _event) const;

        CallbackPool m_callbacks;
        CallbackOrdersCollection m_callbackOrders;

        InstancedEventPool m_instancedEvents;
        InstancedCallbackPool m_instancedCallbacks;
        InstancedCallbackOrdersCollection m_instancedCallbackOrders;
    };

    EGO_POINTER(EventController);
} // namespace ego
