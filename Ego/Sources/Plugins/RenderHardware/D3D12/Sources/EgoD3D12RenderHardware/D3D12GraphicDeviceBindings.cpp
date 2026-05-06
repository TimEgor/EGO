#include "D3D12GraphicDevice.h"

#include <algorithm>

#include "Common/D3D12Utils.h"
#include "Objects/D3D12Pipeline.h"
#include "Objects/D3D12Resources.h"

#include "EgoCore/Assert/AssertCore.h"

namespace
{
    bool FindBindingItem(
        const ego::gpu::BindingSetDesc& _desc,
        uint32_t _binding,
        const ego::gpu::BindingSetItemDesc*& _item
    )
    {
        for (const ego::gpu::BindingSetItemDesc& item : _desc.m_items)
        {
            if (item.m_binding == _binding)
            {
                _item = &item;
                return true;
            }
        }

        _item = nullptr;
        return false;
    }
}


