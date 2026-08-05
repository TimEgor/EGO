#include "Subsystem.h"

void ego::subsystem::Subsystem::SubsystemAccessor::NotifyUnregistered(Subsystem& _subsystem)
{
    _subsystem.onUnregistered();
}
