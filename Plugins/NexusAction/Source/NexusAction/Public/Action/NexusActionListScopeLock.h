#pragma once

class UNexusActionComponent;

struct FNexusActionListScopeLock
{
	FNexusActionListScopeLock(UNexusActionComponent& InActionComponent);
	~FNexusActionListScopeLock();

	UNexusActionComponent& ActionComponent;
};

#define ACTION_LIST_SCOPE_LOCK() FNexusActionListScopeLock ActionListScopeLock(*this)