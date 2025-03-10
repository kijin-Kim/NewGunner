// Fill out your copyright notice in the Description page of Project Settings.


#include "TargetData/NexusTargetData.h"


struct FNexusTargetDataDeleter
{
	void operator()(FNexusTargetDataBase* Object)
	{
		UScriptStruct* ScriptStruct = Object->GetStructType();
		ScriptStruct->DestroyStruct(Object); // Polymorphic 타입의 Destructor 호출
		FMemory::Free(Object);
	}
};

bool FNexusTargetDataHandle::NetSerialize(FArchive& Ar, UPackageMap* Map, bool& bOutSuccess)
{
	if (Ar.IsSaving() && Data)
	{
		if (UScriptStruct* StructType = Data->GetStructType())
		{
			Ar << StructType;
			StructType->GetCppStructOps()->NetSerialize(Ar, Map, bOutSuccess, Data.Get());
		}
	}
	else
	{
		UScriptStruct* StructType = nullptr;
		Ar << StructType;
		if (StructType)
		{
			FNexusTargetDataBase* NewData = static_cast<FNexusTargetDataBase*>(FMemory::Malloc(StructType->GetStructureSize()));
			Data = TSharedPtr<FNexusTargetDataBase>(NewData, FNexusTargetDataDeleter());
			StructType->InitializeStruct(Data.Get()); // Polymorphic 타입의 Constructor 호출
			StructType->GetCppStructOps()->NetSerialize(Ar, Map, bOutSuccess, Data.Get());
		}
	}

	return bOutSuccess;
}
