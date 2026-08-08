#include "Assert.h"

#include "AssertController.h"

#include "EgoCore/Diagnostic/DiagnosticSubsystem.h"

bool ego::TryGenerateAssertError(const char* _message, const char* _file, uint32_t _line)
{
    const AssertControllerPointer controller = GetAssertController();

    return controller && controller->generateError(_message, _file, _line);
}
