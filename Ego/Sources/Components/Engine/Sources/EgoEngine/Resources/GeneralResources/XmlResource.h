#pragma once

#include "EgoEngine/Resources/Resource/Resource.h"

#include "EgoCore/Parsers/XmlParser/XmlDocument.h"

namespace ego
{
    class XmlResource final : public Resource
    {
    public:
        XmlResource() = default;
        explicit XmlResource(const SharedPointer<XmlDocument>& _document);

        bool init(const SharedPointer<XmlDocument>& _document);
        const XmlDocument* getDocument() const;
        XmlNode getRootNode() const;

        EGO_RESOURCE(XmlResource, Resource);

    protected:
        bool onLoad(FileContent&& _content, ResourceLoadingContext& _loadingContext) override;
        void onUnload() override;

    private:
        SharedPointer<XmlDocument> m_document;
    };

    EGO_POINTER(XmlResource);
} // namespace ego
