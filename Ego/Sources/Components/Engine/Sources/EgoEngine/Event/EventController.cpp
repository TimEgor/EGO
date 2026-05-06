#include "EventController.h"

bool ego::EventController::init()
{
    EGO_CHECK_INITIALIZATION(m_callbacks.init());

    EGO_CHECK_INITIALIZATION(m_instancedEvents.init());
    EGO_CHECK_INITIALIZATION(m_instancedCallbacks.init());

    return true;
}

void ego::EventController::release()
{
    m_callbacks.release();
    m_callbackOrders = CallbackOrdersCollection();

    m_instancedEvents.release();
    m_instancedCallbacks.release();
    m_instancedCallbackOrders = InstancedCallbackOrdersCollection();
}

bool ego::EventController::registerEvent(EventType _type)
{
    auto findEventIter = m_callbackOrders.find(_type);
    if (findEventIter != m_callbackOrders.end())
    {
        return false;
    }

    m_callbackOrders.insert(std::make_pair(_type, CallbackOrderCollection()));

    return true;
}

bool ego::EventController::unregisterEvent(EventType _type)
{
    auto findEventIter = m_callbackOrders.find(_type);
    if (findEventIter == m_callbackOrders.end())
    {
        return false;
    }

    m_callbackOrders.erase(findEventIter);

    return true;
}

ego::EventCallbackID ego::EventController::addEventCallback(EventType _type, const EventCallback& _callback)
{
    auto findEventIter = m_callbackOrders.find(_type);
    if (findEventIter == m_callbackOrders.end())
    {
        return InvalidEventCallbackID;
    }

    const CallbackPoolElementInfo dispatcherInfo = m_callbacks.addElement(
        EventCallbackData{_callback, _type, static_cast<uint32_t>(-1)}
    );
    const EventCallbackID dispatcherID = dispatcherInfo.m_elementHandle.getKey();

    findEventIter->second.push_back(dispatcherInfo.m_elementPtr);
    dispatcherInfo.m_elementPtr->m_orderIndex = findEventIter->second.size() - 1;

    return dispatcherID;
}

void ego::EventController::removeEventCallback(EventCallbackID _dispatcherID)
{
    const EventCallbackData* dispatcherData = m_callbacks.getElement(_dispatcherID);
    if (!dispatcherData)
    {
        return;
    }

    auto findEventIter = m_callbackOrders.find(dispatcherData->m_eventType);
    if (findEventIter == m_callbackOrders.end())
    {
        return;
    }

    CallbackOrderCollection& order = findEventIter->second;
    if (dispatcherData->m_orderIndex != order.size())
    {
        EventCallbackData* swapDispatcherData = order.back();
        EGO_ASSERT(swapDispatcherData);

        order[dispatcherData->m_orderIndex] = swapDispatcherData;
        swapDispatcherData->m_orderIndex = dispatcherData->m_orderIndex;
    }

    m_callbacks.removeElement(_dispatcherID);
    order.pop_back();
}

bool ego::EventController::emitEvent(EventType _type, const Event& _event) const
{
    auto findEventIter = m_callbackOrders.find(_type);
    if (findEventIter == m_callbackOrders.end())
    {
        return false;
    }

    for (const auto& dispatcherData : findEventIter->second)
    {
        dispatcherData->m_callback(_event);
    }

    return true;
}

ego::InstancedEventID ego::EventController::registerInstancedEvent(EventType _type)
{
    const InstancedEventID eventID(_type, m_instancedEvents.addElement(_type).m_elementHandle.getKey());

    m_instancedCallbackOrders.insert(std::make_pair(eventID, InvalidInstancedEventCallbackID));

    return eventID;
}

bool ego::EventController::unregisterInstancedEvent(InstancedEventID _eventID)
{
    auto findEventIter = m_instancedCallbackOrders.find(_eventID);
    if (findEventIter == m_instancedCallbackOrders.end())
    {
        return false;
    }

    InstancedEventCallbackID dispatcherID = findEventIter->second;
    while (dispatcherID != InvalidEventCallbackID)
    {
        const InstancedEventCallbackID nextDispatcherID = m_instancedCallbacks.getElement(dispatcherID)->
            m_nextSiblingID;
        m_instancedCallbacks.removeElement(dispatcherID);

        dispatcherID = nextDispatcherID;
    }

    m_instancedEvents.removeElement(_eventID.m_key);

    return true;
}

ego::InstancedEventCallbackID ego::EventController::addInstanceEventCallback(
    InstancedEventID _eventID,
    const InstancedEventCallback& _callback
)
{
    auto findEventIter = m_instancedCallbackOrders.find(_eventID);
    if (findEventIter == m_instancedCallbackOrders.end())
    {
        return InvalidEventCallbackID;
    }

    const InstancedEventCallbackID prevOrderDispatcherId = findEventIter->second;
    const InstancedEventCallbackID dispatcherID = m_instancedCallbacks.addElement(
        InstancedEventCallbackData{_callback, _eventID, prevOrderDispatcherId, InvalidInstancedEventCallbackID}
    ).m_elementHandle.getKey();

    if (prevOrderDispatcherId != InvalidInstancedEventCallbackID)
    {
        m_instancedCallbacks.getElement(prevOrderDispatcherId)->m_nextSiblingID = dispatcherID;
    }

    findEventIter->second = dispatcherID;

    return dispatcherID;
}

void ego::EventController::removeInstancedEventDispatcher(InstancedEventCallbackID _dispatcherID)
{
    const InstancedEventCallbackData* dispatcherData = m_instancedCallbacks.getElement(_dispatcherID);
    if (!dispatcherData)
    {
        return;
    }

    if (dispatcherData->m_nextSiblingID != InvalidEventCallbackID)
    {
        InstancedEventCallbackData* nextSiblinngDispatcherData = m_instancedCallbacks.getElement(
            dispatcherData->m_nextSiblingID
        );

        EGO_ASSERT(nextSiblinngDispatcherData);
        EGO_ASSERT(nextSiblinngDispatcherData->m_prevSiblingID == _dispatcherID);

        nextSiblinngDispatcherData->m_prevSiblingID = dispatcherData->m_prevSiblingID;
    }
    else
    {
        InstancedEventCallbackID& firstOrderDispatcherID = m_instancedCallbackOrders.at(dispatcherData->m_eventID);
        EGO_ASSERT(firstOrderDispatcherID == _dispatcherID);

        firstOrderDispatcherID = dispatcherData->m_prevSiblingID;
    }

    if (dispatcherData->m_prevSiblingID != InvalidEventCallbackID)
    {
        InstancedEventCallbackData* prevSiblinngDispatcherData = m_instancedCallbacks.getElement(
            dispatcherData->m_prevSiblingID
        );

        EGO_ASSERT(prevSiblinngDispatcherData);
        EGO_ASSERT(prevSiblinngDispatcherData->m_nextSiblingID == _dispatcherID);

        prevSiblinngDispatcherData->m_nextSiblingID = dispatcherData->m_nextSiblingID;
    }

    m_instancedCallbacks.removeElement(_dispatcherID);
}

bool ego::EventController::emitEvent(InstancedEventID _eventID, const Event& _event) const
{
    auto findEventIter = m_instancedCallbackOrders.find(_eventID);
    if (findEventIter == m_instancedCallbackOrders.end())
    {
        return false;
    }

    InstancedEventCallbackID dispatcherID = findEventIter->second;
    while (dispatcherID != InvalidEventCallbackID)
    {
        const InstancedEventCallbackData* dispatcherData = m_instancedCallbacks.getElement(dispatcherID);
        EGO_ASSERT(dispatcherData);

        dispatcherData->m_callback(_event);

        dispatcherID = dispatcherData->m_nextSiblingID;
    }

    return true;
}
