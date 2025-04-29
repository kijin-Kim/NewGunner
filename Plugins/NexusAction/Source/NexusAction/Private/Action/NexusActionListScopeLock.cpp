#include "Action/NexusActionListScopeLock.h"
#include "Action/NexusActionComponent.h"

FNexusActionListScopeLock::FNexusActionListScopeLock(UNexusActionComponent& InActionComponent): ActionComponent(InActionComponent)
{
	ActionComponent.IncreaseActionListLock();
}

FNexusActionListScopeLock::~FNexusActionListScopeLock()
{
	ActionComponent.DecreaseActionListLock();
}

FNexusLocalActionInstanceMapScopeLock::FNexusLocalActionInstanceMapScopeLock(UNexusActionComponent& InActionComponent) : ActionComponent(InActionComponent)
{
	ActionComponent.IncreaseLocalActionInstanceMapLock();
}

FNexusLocalActionInstanceMapScopeLock::~FNexusLocalActionInstanceMapScopeLock()
{
	ActionComponent.DecreaseLocalActionInstanceMapLock();
}
