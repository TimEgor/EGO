#include "EgoGui/Inspector/PropertyGuiDrawer/EnumPropertyGuiDrawer.h"

#include <string>

#include "EgoCore/Assert/Assert.h"
#include "EgoCore/RTTI/Property/PropertyMetaInfoCast.h"
#include "EgoCore/RTTI/Property/Types/Enum/EnumPropertyMetaInfo.h"

#include <imgui.h>

void ego::gui::EnumPropertyGuiDrawer::draw(const char* _name, void* _value, const rtti::PropertyMetaInfo& _propertyMetaInfo, const PropertyGuiDrawFunction&)
    const
{
    const rtti::EnumPropertyMetaInfoBase* enumPropertyMetaInfo = rtti::CastPropertyMetaInfo<rtti::EnumPropertyMetaInfoBase>(&_propertyMetaInfo);
    EGO_ASSERT(enumPropertyMetaInfo);
    if (!enumPropertyMetaInfo)
    {
        return;
    }

    const rtti::EnumPropertyMetaInfoBase::NameCollection names = enumPropertyMetaInfo->getNames();
    const size_t valueIndex = enumPropertyMetaInfo->getValueIndex(_value);
    const std::string preview = valueIndex < names.size() ? std::string(names[valueIndex]) : std::string("<Invalid>");

    if (ImGui::BeginCombo(_name, preview.c_str()))
    {
        for (size_t index = 0; index < names.size(); ++index)
        {
            const bool isSelected = index == valueIndex;
            const std::string valueName(names[index]);
            if (ImGui::Selectable(valueName.c_str(), isSelected))
            {
                enumPropertyMetaInfo->setValueIndex(_value, index);
            }

            if (isSelected)
            {
                ImGui::SetItemDefaultFocus();
            }
        }

        ImGui::EndCombo();
    }
}
