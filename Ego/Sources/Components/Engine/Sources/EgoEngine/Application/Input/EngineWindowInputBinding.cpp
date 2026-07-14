#include "EngineWindowInputBinding.h"

#include "EgoCore/UtilsMacros.h"

#include "EgoInput/InputController.h"

#include "EgoGui/GuiController.h"
#include "EgoGui/GuiViewport.h"

#include "EgoApplication/Window/ApplicationWindow.h"

#include "EgoEngine/EngineSession.h"
#include "../Gui/ApplicationWindowGuiViewportInputAdapter.h"
#include "ApplicationWindowInputKeyProvider.h"

ego::engine::EngineWindowInputBinding::~EngineWindowInputBinding()
{
    release();
}

bool ego::engine::EngineWindowInputBinding::init(
    const EngineSessionPointer& _engineSession,
    const EventControllerPointer& _eventController,
    const application::ApplicationWindowPointer& _window)
{
    EGO_CHECK_INITIALIZATION(!m_inputController);
    EGO_CHECK_INITIALIZATION(!m_guiViewport);
    EGO_CHECK_INITIALIZATION(!m_guiViewportInputAdapter);
    EGO_CHECK_INITIALIZATION(!m_inputKeyProvider);
    EGO_CHECK_INITIALIZATION(_engineSession);
    EGO_CHECK_INITIALIZATION(_eventController);
    EGO_CHECK_INITIALIZATION(_window && _window->isValid());

    m_inputController = _engineSession->getInputControllerPointer();
    EGO_CHECK_INITIALIZATION(m_inputController);

    const EventControllerPointer inputEventController = m_inputController->getEventControllerPointer();
    EGO_CHECK_INITIALIZATION(inputEventController);

    m_guiViewportInputAdapter = new ApplicationWindowGuiViewportInputAdapter();
    EGO_CHECK_INITIALIZATION(m_guiViewportInputAdapter);
    EGO_CHECK_INITIALIZATION(m_guiViewportInputAdapter->init(_window, _eventController, inputEventController));
    EGO_CHECK_INITIALIZATION(m_guiViewportInputAdapter->setGuiController(_engineSession->getGuiControllerPointer()));

    const gui::GuiControllerPointer guiController = _engineSession->getGuiControllerPointer();
    m_guiViewport = guiController ? guiController->getViewport() : nullptr;
    EGO_CHECK_INITIALIZATION(m_guiViewport);
    m_guiViewport->setInputAdapter(m_guiViewportInputAdapter);

    m_inputKeyProvider = new ApplicationWindowInputKeyProvider();
    EGO_CHECK_INITIALIZATION(m_inputKeyProvider);
    EGO_CHECK_INITIALIZATION(m_inputKeyProvider->init(_window, _eventController));
    EGO_CHECK_INITIALIZATION(m_inputController->registerKeyProvider(m_inputKeyProvider));

    return true;
}

void ego::engine::EngineWindowInputBinding::release()
{
    if (m_inputKeyProvider)
    {
        if (m_inputController)
        {
            m_inputController->unregisterKeyProvider(m_inputKeyProvider);
        }

        m_inputKeyProvider->release();
        m_inputKeyProvider = nullptr;
    }

    if (m_guiViewport)
    {
        m_guiViewport->setInputAdapter(nullptr);
    }

    EGO_SAFE_RESET_POINTER_WITH_RELEASING(m_guiViewportInputAdapter);
    m_guiViewport = nullptr;
    m_inputController = nullptr;
}

void ego::engine::EngineWindowInputBinding::updateInputDevices()
{
    if (m_inputKeyProvider)
    {
        m_inputKeyProvider->updateDevices();
    }
}
