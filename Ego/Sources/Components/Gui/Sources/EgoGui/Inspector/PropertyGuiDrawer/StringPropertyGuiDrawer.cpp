#include "EgoGui/Inspector/PropertyGuiDrawer/StringPropertyGuiDrawer.h"

#include <string>

#include "EgoCore/Assert/Assert.h"
#include "EgoCore/RTTI/Property/PropertyMetaInfoCast.h"
#include "EgoCore/RTTI/Property/Types/String/StringPropertyMetaInfo.h"

#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>

void ego::gui::StringPropertyGuiDrawer::draw(const char* _name, void* _value, const rtti::PropertyMetaInfo& _propertyMetaInfo, const PropertyGuiDrawFunction&)
    const
{
    const rtti::StringPropertyMetaInfo* stringPropertyMetaInfo = rtti::CastPropertyMetaInfo<rtti::StringPropertyMetaInfo>(&_propertyMetaInfo);
    EGO_ASSERT(stringPropertyMetaInfo);
    if (!stringPropertyMetaInfo)
    {
        return;
    }

    std::string stringValue = stringPropertyMetaInfo->getValue(_value);
    if (ImGui::InputText(_name, &stringValue))
    {
        stringPropertyMetaInfo->setValue(_value, stringValue.c_str());
    }
}
