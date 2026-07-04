#include "BindingLayout.h"

ego::gpu::BindingLayout::BindingLayout(const BindingLayoutDesc& _desc)
    : m_desc(_desc)
{
}

const ego::gpu::BindingLayoutDesc& ego::gpu::BindingLayout::getDesc() const
{
    return m_desc;
}
