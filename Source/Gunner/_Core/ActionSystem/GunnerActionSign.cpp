// Fill out your copyright notice in the Description page of Project Settings.


#include "GunnerActionSign.h"

struct FGunnerSignalTargetDataDeleter
{
	void operator()(FGunnerTargetDataBase* Object)
	{
		UScriptStruct* ScriptStruct = Object->GetStructType();
		ScriptStruct->DestroyStruct(Object); // Polymorphic 타입의 Destructor 호출
		FMemory::Free(Object);
	}
};


void UGunnerActionSign::OnSignaled_Implementation()
{
}


bool FGunnerTargetDataHandle::NetSerialize(FArchive& Ar, UPackageMap* Map, bool& bOutSuccess)
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
			FGunnerTargetDataBase* NewData = static_cast<FGunnerTargetDataBase*>(FMemory::Malloc(StructType->GetStructureSize()));
			Data = TSharedPtr<FGunnerTargetDataBase>(NewData, FGunnerSignalTargetDataDeleter());
			StructType->InitializeStruct(Data.Get()); // Polymorphic 타입의 Constructor 호출
			StructType->GetCppStructOps()->NetSerialize(Ar, Map, bOutSuccess, Data.Get());
		}
	}

	return bOutSuccess;
}

UWorld* UGunnerActionSign::GetWorld() const
{
	// https://forums.unrealengine.com/t/can-you-use-a-blueprint-function-library-in-an-object-class/350918/37
	if (HasAllFlags(RF_ClassDefaultObject))
	{
		return nullptr;
	}
	return GetOuter()->GetWorld();
}
