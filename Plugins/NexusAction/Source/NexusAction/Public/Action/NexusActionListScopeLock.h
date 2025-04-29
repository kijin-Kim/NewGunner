#pragma once

class UNexusActionComponent;

struct FNexusActionListScopeLock
{
	FNexusActionListScopeLock(UNexusActionComponent& InActionComponent);
	~FNexusActionListScopeLock();

	UNexusActionComponent& ActionComponent;
};

#define ACTION_LIST_SCOPE_LOCK() FNexusActionListScopeLock ActionListScopeLock(*this)


struct FNexusLocalActionInstanceMapScopeLock
{
	FNexusLocalActionInstanceMapScopeLock(UNexusActionComponent& InActionComponent);
	~FNexusLocalActionInstanceMapScopeLock();

	UNexusActionComponent& ActionComponent;
};

#define ACTION_INSTANCE_MAP_SCOPE_LOCK() FNexusLocalActionInstanceMapScopeLock LocalActionInstanceMapScopeLock(*this)