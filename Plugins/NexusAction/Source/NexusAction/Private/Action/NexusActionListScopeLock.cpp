#include "Action/NexusActionListScopeLock.h"
#include "Action/NexusActionComponent.h"

FNexusActionListScopeLock::FNexusActionListScopeLock(UNexusActionComponent& InActionComponent): ActionComponent(InActionComponent)
{
	ActionComponent.IncrementActionListLock();
}

FNexusActionListScopeLock::~FNexusActionListScopeLock()
{
	ActionComponent.DecrementActionListLock();
}

FNexusLocalActionInstanceMapScopeLock::FNexusLocalActionInstanceMapScopeLock(UNexusActionComponent& InActionComponent) : ActionComponent(InActionComponent)
{
	ActionComponent.IncrementLocalActionInstanceMapLock();
}

FNexusLocalActionInstanceMapScopeLock::~FNexusLocalActionInstanceMapScopeLock()
{
	ActionComponent.DecrementLocalActionInstanceMapLock();
}
