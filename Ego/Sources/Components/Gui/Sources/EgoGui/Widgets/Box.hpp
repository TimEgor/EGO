#pragma once

template <typename TWidget>
ego::gui::BoxChild::BoxChild(const ego::SharedPointer<TWidget>& _widget)
    : m_widget(_widget)
{
}

template <typename TWidget>
ego::gui::BoxChild::BoxChild(const ego::SharedPointer<TWidget>& _widget, const BoxSlot& _slot)
    : m_widget(_widget),
      m_slot(_slot)
{
}
