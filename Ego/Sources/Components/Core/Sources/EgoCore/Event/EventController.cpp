#include "EgoCore/Event/EventController.h"

#include <utility>

#include "EgoCore/UtilsMacros.h"

bool ego::EventController::init()
{
    EGO_CHECK_INITIALIZATION(!isDispatching());
    EGO_CHECK_INITIALIZATION(m_pendingOperations.empty());

    EGO_CHECK_INITIALIZATION(m_callbacks.init());

    EGO_CHECK_INITIALIZATION(m_instancedEvents.init());
    EGO_CHECK_INITIALIZATION(m_instancedCallbacks.init());

    return true;
}

void ego::EventController::release()
{
    EGO_ASSERT(!isDispatching());
    EGO_CHECK_RETURN(!isDispatching());

    m_pendingOperations.clear();

    m_callbacks.release();
    m_callbackOrders = CallbackOrdersCollection();

    m_instancedEvents.release();
    m_instancedCallbacks.release();
    m_instancedCallbackOrders = InstancedCallbackOrdersCollection();

    m_dispatchDepth = 0;
}

bool ego::EventController::registerEvent(EventType _type)
{
    if (isEventRegisteredIncludingPending(_type))
    {
        return false;
    }

    if (!isDispatching())
    {
        return registerEventImmediate(_type);
    }

    PendingOperation operation;
    operation.m_type = PendingOperationType::RegisterEvent;
    operation.m_eventType = _type;
    m_pendingOperations.push_back(operation);
    return true;
}

bool ego::EventController::unregisterEvent(EventType _type)
{
    if (!isEventRegisteredIncludingPending(_type))
    {
        return false;
    }

    if (!isDispatching())
    {
        return unregisterEventImmediate(_type);
    }

    PendingOperation operation;
    operation.m_type = PendingOperationType::UnregisterEvent;
    operation.m_eventType = _type;
    m_pendingOperations.push_back(operation);
    return true;
}

bool ego::EventController::registerEventImmediate(EventType _type)
{
    return m_callbackOrders.emplace(_type, CallbackOrderCollection()).second;
}

bool ego::EventController::unregisterEventImmediate(EventType _type)
{
    CallbackOrdersCollection::iterator eventIt = m_callbackOrders.find(_type);
    if (eventIt == m_callbackOrders.end())
    {
        return false;
    }

    std::vector<EventCallbackID> callbackIDs;
    callbackIDs.reserve(eventIt->second.size());
    for (const EventCallbackData* callbackData : eventIt->second)
    {
        EGO_ASSERT(callbackData);
        callbackIDs.push_back(callbackData->m_callbackID);
    }

    m_callbackOrders.erase(eventIt);

    for (EventCallbackID callbackID : callbackIDs)
    {
        m_callbacks.removeElement(callbackID);
    }

    return true;
}

bool ego::EventController::isEventRegisteredIncludingPending(EventType _type) const
{
    bool isRegistered = m_callbackOrders.contains(_type);
    for (const PendingOperation& operation : m_pendingOperations)
    {
        if (operation.m_eventType != _type)
        {
            continue;
        }

        if (operation.m_type == PendingOperationType::RegisterEvent)
        {
            isRegistered = true;
        }
        else if (operation.m_type == PendingOperationType::UnregisterEvent)
        {
            isRegistered = false;
        }
    }

    return isRegistered;
}

ego::EventCallbackID ego::EventController::addEventCallback(EventType _type, EventCallback _callback)
{
    if (!isEventRegisteredIncludingPending(_type))
    {
        return InvalidEventCallbackID;
    }

    const CallbackPoolElementInfo callbackInfo =
        m_callbacks.addElement(EventCallbackData{std::move(_callback), _type, InvalidEventCallbackID, static_cast<uint32_t>(-1)});
    const EventCallbackID callbackID = callbackInfo.m_elementHandle.getKey();
    callbackInfo.m_elementPtr->m_callbackID = callbackID;

    if (!isDispatching())
    {
        if (!addEventCallbackImmediate(callbackID))
        {
            return InvalidEventCallbackID;
        }
        return callbackID;
    }

    PendingOperation operation;
    operation.m_type = PendingOperationType::AddEventCallback;
    operation.m_eventCallbackID = callbackID;
    m_pendingOperations.push_back(operation);
    return callbackID;
}

bool ego::EventController::addEventCallbackImmediate(EventCallbackID _callbackID)
{
    EventCallbackData* callbackData = m_callbacks.getElement(_callbackID);
    if (!callbackData)
    {
        return false;
    }

    CallbackOrdersCollection::iterator eventIt = m_callbackOrders.find(callbackData->m_eventType);
    if (eventIt == m_callbackOrders.end())
    {
        m_callbacks.removeElement(_callbackID);
        return false;
    }

    CallbackOrderCollection& order = eventIt->second;
    callbackData->m_orderIndex = static_cast<uint32_t>(order.size());
    order.push_back(callbackData);
    return true;
}

void ego::EventController::removeEventCallback(EventCallbackID _callbackID)
{
    if (!m_callbacks.getElement(_callbackID))
    {
        return;
    }

    if (!isDispatching())
    {
        removeEventCallbackImmediate(_callbackID);
        return;
    }

    PendingOperation operation;
    operation.m_type = PendingOperationType::RemoveEventCallback;
    operation.m_eventCallbackID = _callbackID;
    m_pendingOperations.push_back(operation);
}

void ego::EventController::removeEventCallbackImmediate(EventCallbackID _callbackID)
{
    const EventCallbackData* callbackData = m_callbacks.getElement(_callbackID);
    if (!callbackData)
    {
        return;
    }

    CallbackOrdersCollection::iterator eventIt = m_callbackOrders.find(callbackData->m_eventType);
    if (eventIt == m_callbackOrders.end())
    {
        m_callbacks.removeElement(_callbackID);
        return;
    }

    CallbackOrderCollection& order = eventIt->second;
    if (callbackData->m_orderIndex >= order.size())
    {
        m_callbacks.removeElement(_callbackID);
        return;
    }

    const uint32_t orderIndex = callbackData->m_orderIndex;
    const uint32_t lastOrderIndex = static_cast<uint32_t>(order.size() - 1);
    if (orderIndex != lastOrderIndex)
    {
        EventCallbackData* movedCallbackData = order.back();
        EGO_ASSERT(movedCallbackData);

        order[orderIndex] = movedCallbackData;
        movedCallbackData->m_orderIndex = orderIndex;
    }

    order.pop_back();
    m_callbacks.removeElement(_callbackID);
}

bool ego::EventController::emitEvent(EventType _type, const Event& _event)
{
    CallbackOrdersCollection::iterator eventIt = m_callbackOrders.find(_type);
    if (eventIt == m_callbackOrders.end())
    {
        return false;
    }

    beginDispatch();
    for (EventCallbackData* callbackData : eventIt->second)
    {
        EGO_ASSERT(callbackData);
        callbackData->m_callback(_event);
    }
    endDispatch();

    return true;
}

ego::InstancedEventID ego::EventController::registerInstancedEvent(EventType _type)
{
    const InstancedEventPool::NewElementInfo eventInfo = m_instancedEvents.addElement(_type);
    const InstancedEventID eventID(_type, eventInfo.m_elementHandle.getKey());

    if (!isDispatching())
    {
        if (!registerInstancedEventImmediate(eventID))
        {
            m_instancedEvents.removeElement(eventID.m_key);
            return InvalidInstancedEventID;
        }
        return eventID;
    }

    PendingOperation operation;
    operation.m_type = PendingOperationType::RegisterInstancedEvent;
    operation.m_instancedEventID = eventID;
    m_pendingOperations.push_back(operation);
    return eventID;
}

bool ego::EventController::unregisterInstancedEvent(InstancedEventID _eventID)
{
    if (!isInstancedEventRegisteredIncludingPending(_eventID))
    {
        return false;
    }

    if (!isDispatching())
    {
        return unregisterInstancedEventImmediate(_eventID);
    }

    PendingOperation operation;
    operation.m_type = PendingOperationType::UnregisterInstancedEvent;
    operation.m_instancedEventID = _eventID;
    m_pendingOperations.push_back(operation);
    return true;
}

bool ego::EventController::registerInstancedEventImmediate(InstancedEventID _eventID)
{
    return m_instancedCallbackOrders.emplace(_eventID, InvalidInstancedEventCallbackID).second;
}

bool ego::EventController::unregisterInstancedEventImmediate(InstancedEventID _eventID)
{
    InstancedCallbackOrdersCollection::iterator eventIt = m_instancedCallbackOrders.find(_eventID);
    if (eventIt == m_instancedCallbackOrders.end())
    {
        return false;
    }

    std::vector<InstancedEventCallbackID> callbackIDs;
    InstancedEventCallbackID callbackID = eventIt->second;
    while (callbackID != InvalidInstancedEventCallbackID)
    {
        callbackIDs.push_back(callbackID);

        const InstancedEventCallbackData* callbackData = m_instancedCallbacks.getElement(callbackID);
        EGO_ASSERT(callbackData);
        callbackID = callbackData->m_prevSiblingID;
    }

    m_instancedCallbackOrders.erase(eventIt);
    for (InstancedEventCallbackID registeredCallbackID : callbackIDs)
    {
        m_instancedCallbacks.removeElement(registeredCallbackID);
    }
    m_instancedEvents.removeElement(_eventID.m_key);

    return true;
}

bool ego::EventController::isInstancedEventRegisteredIncludingPending(InstancedEventID _eventID) const
{
    bool isRegistered = m_instancedCallbackOrders.contains(_eventID);
    for (const PendingOperation& operation : m_pendingOperations)
    {
        if (operation.m_instancedEventID != _eventID)
        {
            continue;
        }

        if (operation.m_type == PendingOperationType::RegisterInstancedEvent)
        {
            isRegistered = true;
        }
        else if (operation.m_type == PendingOperationType::UnregisterInstancedEvent)
        {
            isRegistered = false;
        }
    }

    return isRegistered;
}

ego::InstancedEventCallbackID ego::EventController::addInstanceEventCallback(InstancedEventID _eventID, InstancedEventCallback _callback)
{
    if (!isInstancedEventRegisteredIncludingPending(_eventID))
    {
        return InvalidInstancedEventCallbackID;
    }

    const InstancedCallbackPoolElementInfo callbackInfo = m_instancedCallbacks.addElement(
        InstancedEventCallbackData{std::move(_callback), _eventID, InvalidInstancedEventCallbackID, InvalidInstancedEventCallbackID});
    const InstancedEventCallbackID callbackID = callbackInfo.m_elementHandle.getKey();

    if (!isDispatching())
    {
        if (!addInstanceEventCallbackImmediate(callbackID))
        {
            return InvalidInstancedEventCallbackID;
        }
        return callbackID;
    }

    PendingOperation operation;
    operation.m_type = PendingOperationType::AddInstancedEventCallback;
    operation.m_instancedEventCallbackID = callbackID;
    m_pendingOperations.push_back(operation);
    return callbackID;
}

bool ego::EventController::addInstanceEventCallbackImmediate(InstancedEventCallbackID _callbackID)
{
    InstancedEventCallbackData* callbackData = m_instancedCallbacks.getElement(_callbackID);
    if (!callbackData)
    {
        return false;
    }

    InstancedCallbackOrdersCollection::iterator eventIt = m_instancedCallbackOrders.find(callbackData->m_eventID);
    if (eventIt == m_instancedCallbackOrders.end())
    {
        m_instancedCallbacks.removeElement(_callbackID);
        return false;
    }

    const InstancedEventCallbackID previousCallbackID = eventIt->second;
    callbackData->m_prevSiblingID = previousCallbackID;
    callbackData->m_nextSiblingID = InvalidInstancedEventCallbackID;
    if (previousCallbackID != InvalidInstancedEventCallbackID)
    {
        InstancedEventCallbackData* previousCallbackData = m_instancedCallbacks.getElement(previousCallbackID);
        EGO_ASSERT(previousCallbackData);
        previousCallbackData->m_nextSiblingID = _callbackID;
    }

    eventIt->second = _callbackID;
    return true;
}

void ego::EventController::removeInstancedEventDispatcher(InstancedEventCallbackID _callbackID)
{
    if (!m_instancedCallbacks.getElement(_callbackID))
    {
        return;
    }

    if (!isDispatching())
    {
        removeInstancedEventCallbackImmediate(_callbackID);
        return;
    }

    PendingOperation operation;
    operation.m_type = PendingOperationType::RemoveInstancedEventCallback;
    operation.m_instancedEventCallbackID = _callbackID;
    m_pendingOperations.push_back(operation);
}

void ego::EventController::removeInstancedEventCallbackImmediate(InstancedEventCallbackID _callbackID)
{
    const InstancedEventCallbackData* callbackData = m_instancedCallbacks.getElement(_callbackID);
    if (!callbackData)
    {
        return;
    }

    if (callbackData->m_nextSiblingID != InvalidInstancedEventCallbackID)
    {
        InstancedEventCallbackData* nextCallbackData = m_instancedCallbacks.getElement(callbackData->m_nextSiblingID);

        EGO_ASSERT(nextCallbackData);
        EGO_ASSERT(nextCallbackData->m_prevSiblingID == _callbackID);

        nextCallbackData->m_prevSiblingID = callbackData->m_prevSiblingID;
    }
    else
    {
        InstancedCallbackOrdersCollection::iterator eventIt = m_instancedCallbackOrders.find(callbackData->m_eventID);
        if (eventIt != m_instancedCallbackOrders.end())
        {
            EGO_ASSERT(eventIt->second == _callbackID);
            eventIt->second = callbackData->m_prevSiblingID;
        }
    }

    if (callbackData->m_prevSiblingID != InvalidInstancedEventCallbackID)
    {
        InstancedEventCallbackData* previousCallbackData = m_instancedCallbacks.getElement(callbackData->m_prevSiblingID);

        EGO_ASSERT(previousCallbackData);
        EGO_ASSERT(previousCallbackData->m_nextSiblingID == _callbackID);

        previousCallbackData->m_nextSiblingID = callbackData->m_nextSiblingID;
    }

    m_instancedCallbacks.removeElement(_callbackID);
}

bool ego::EventController::emitEvent(InstancedEventID _eventID, const Event& _event)
{
    InstancedCallbackOrdersCollection::iterator eventIt = m_instancedCallbackOrders.find(_eventID);
    if (eventIt == m_instancedCallbackOrders.end())
    {
        return false;
    }

    beginDispatch();

    InstancedEventCallbackID callbackID = eventIt->second;
    while (callbackID != InvalidInstancedEventCallbackID)
    {
        const InstancedEventCallbackData* callbackData = m_instancedCallbacks.getElement(callbackID);
        EGO_ASSERT(callbackData);

        const InstancedEventCallbackID previousCallbackID = callbackData->m_prevSiblingID;
        callbackData->m_callback(_event);
        callbackID = previousCallbackID;
    }

    endDispatch();
    return true;
}

bool ego::EventController::isDispatching() const
{
    return m_dispatchDepth != 0;
}

void ego::EventController::beginDispatch()
{
    ++m_dispatchDepth;
}

void ego::EventController::endDispatch()
{
    EGO_ASSERT(m_dispatchDepth > 0);
    --m_dispatchDepth;

    if (!isDispatching())
    {
        applyPendingOperations();
    }
}

void ego::EventController::applyPendingOperations()
{
    EGO_ASSERT(!isDispatching());

    PendingOperationCollection operations = std::move(m_pendingOperations);
    m_pendingOperations.clear();

    for (const PendingOperation& operation : operations)
    {
        switch (operation.m_type)
        {
        case PendingOperationType::RegisterEvent:
            registerEventImmediate(operation.m_eventType);
            break;
        case PendingOperationType::UnregisterEvent:
            unregisterEventImmediate(operation.m_eventType);
            break;
        case PendingOperationType::AddEventCallback:
            addEventCallbackImmediate(operation.m_eventCallbackID);
            break;
        case PendingOperationType::RemoveEventCallback:
            removeEventCallbackImmediate(operation.m_eventCallbackID);
            break;
        case PendingOperationType::RegisterInstancedEvent:
            if (!registerInstancedEventImmediate(operation.m_instancedEventID))
            {
                m_instancedEvents.removeElement(operation.m_instancedEventID.m_key);
            }
            break;
        case PendingOperationType::UnregisterInstancedEvent:
            unregisterInstancedEventImmediate(operation.m_instancedEventID);
            break;
        case PendingOperationType::AddInstancedEventCallback:
            addInstanceEventCallbackImmediate(operation.m_instancedEventCallbackID);
            break;
        case PendingOperationType::RemoveInstancedEventCallback:
            removeInstancedEventCallbackImmediate(operation.m_instancedEventCallbackID);
            break;
        }
    }
}
