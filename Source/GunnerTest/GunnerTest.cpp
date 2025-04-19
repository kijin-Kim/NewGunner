#include "EngineUtils.h"
#include "GunnerTestGameMode.h"
#include "GunnerTestGun.h"
#include "LevelEditor.h"
#include "UnrealEdGlobals.h"
#include "Action/NexusAction.h"
#include "Action/NexusActionComponent.h"

#include "GunnerTestPawn.h"
#include "GunnerTestSlotItemPickup.h"
#include "Editor/UnrealEdEngine.h"
#include "Gunner/Slot/GunnerSlotManagerComponent.h"
#include "Gunner/_Core/GunnerNativeGameplayTags.h"
#include "Misc/AutomationTest.h"
#include "Tests/AutomationCommon.h"
#include "Tests/AutomationEditorCommon.h"


IMPLEMENT_MODULE(FDefaultModuleImpl, GunnerTests)


#if WITH_EDITOR


DEFINE_LATENT_AUTOMATION_COMMAND_ONE_PARAMETER(FGunnerCallableTest, TFunction<void()>, Func);


bool FGunnerCallableTest::Update()
{
	Func();
	return true;
}

class FGunnerTestCollection
{
public:
	FGunnerTestCollection(const FWorldContext& InWorldContext, FAutomationTestBase* InTest)
		: WorldContext(InWorldContext), Test(InTest)
	{
	}

	~FGunnerTestCollection()
	{
	}

	void Test_SlotManager_AddItem()
	{
		for (FConstPlayerControllerIterator Iterator = WorldContext.World()->GetPlayerControllerIterator(); Iterator; ++Iterator)
		{
			APlayerController* PC = Iterator->Get();
			if (!PC || !PC->GetPawn() || !PC->GetPawn()->IsA(AGunnerTestPawn::StaticClass()))
			{
				continue;
			}


			APawn* Pawn = PC->GetPawn();


			UGunnerSlotManagerComponent* SlotManagerComponent = GetSlotManagerComponent(Pawn);
			check(SlotManagerComponent);
			if (IsServer(WorldContext))
			{
				SlotManagerComponent->AuthAddItemToSlot(WorldContext.World()->SpawnActor<AGunnerTestGun>());
			}

			CheckAquire(Pawn);
		}
		Test->AddInfo(Prefix(TEXT("Test_SlotManager_AddItem() 완료")));
	}


	void Test_SlotManager_ActivateItem()
	{
		for (FConstPlayerControllerIterator Iterator = WorldContext.World()->GetPlayerControllerIterator(); Iterator; ++Iterator)
		{
			APlayerController* PC = Iterator->Get();
			if (!PC || !PC->GetPawn() || !PC->GetPawn()->IsA(AGunnerTestPawn::StaticClass()))
			{
				continue;
			}


			APawn* Pawn = PC->GetPawn();
			
			UNexusActionComponent* ActionComponent = UNexusActionComponent::GetActionComponentFromActor(Pawn);
			check(ActionComponent);

			if (IsServer(WorldContext))
			{
				ActionComponent->SendEventToSelf<FNexusEventMessage>(GunnerNativeGameplayTags::TAG_Input_ActivateSlot_Primary, {});
				UNexusProperty* SlotIndexProperty = ActionComponent->GetProperty(GunnerNativeGameplayTags::TAG_Property_SlotIndex);
				SlotIndexProperty->Tick();
			}

			UGunnerSlotManagerComponent* SlotManagerComponent = GetSlotManagerComponent(Pawn);
			
			Test->TestEqual(Prefix(TEXT("활성화된 슬롯 아이템의 타입이 같습니다")), SlotManagerComponent->GetCurrentSlotType(), GetDefault<AGunnerTestGun>()->GetSlotType());
			Test->TestNotNull(Prefix(TEXT("슬롯 아이템이 활성화되었습니다")), Cast<AGunnerTestGun>(SlotManagerComponent->GetCurrentSlotItem()));


			const FNexusActionDefContainer& ActionDefs = ActionComponent->GetActionDefs();

			Test->TestTrue(Prefix(TEXT("Transient액션이 추가되었습니다")), ActionDefs.Items.ContainsByPredicate([](const FNexusActionDef& Element)
			{
				return Element.ActionClass == UGunnerTestActionTransient::StaticClass();
			}));
		}
		Test->AddInfo(Prefix(TEXT("Test_SlotManager_ActivateItem() 완료")));
	}

	void Test_SlotManager_ChangeBullet()
	{
		for (FConstPlayerControllerIterator Iterator = WorldContext.World()->GetPlayerControllerIterator(); Iterator; ++Iterator)
		{
			APlayerController* PC = Iterator->Get();
			if (!PC || !PC->GetPawn() || !PC->GetPawn()->IsA(AGunnerTestPawn::StaticClass()))
			{
				continue;
			}
			APawn* Pawn = PC->GetPawn();

			UNexusActionComponent* ActionComponent = UNexusActionComponent::GetActionComponentFromActor(Pawn);
			check(ActionComponent);

			UNexusProperty* BulletProperty = ActionComponent->GetProperty(GunnerNativeGameplayTags::TAG_Property_Weapon_Bullet);
			UNexusProperty* MagazineBulletProperty = ActionComponent->GetProperty(GunnerNativeGameplayTags::TAG_Property_Weapon_MagazineBullet);
			if (IsServer(WorldContext))
			{
				BulletProperty->SetStaticValue(PreDropBulletCount);
				BulletProperty->Tick();

				MagazineBulletProperty->SetStaticValue(PreDropMagazineBulletCount);
				MagazineBulletProperty->Tick();
			}

			Test->TestEqual(Prefix(TEXT("슬롯 아이템의 총알 수가 같습니다")), BulletProperty->GetDynamicValue(), 5.0f);
			Test->TestEqual(Prefix(TEXT("슬롯 아이템의 탄알집의 총알 수가 같습니다")), MagazineBulletProperty->GetDynamicValue(), 10.0f);
		}
		Test->AddInfo(Prefix(TEXT("Test_SlotManager_ChangeBullet() 완료")));
	}

	void Test_SlotManager_Drop()
	{
		for (FConstPlayerControllerIterator Iterator = WorldContext.World()->GetPlayerControllerIterator(); Iterator; ++Iterator)
		{
			APlayerController* PC = Iterator->Get();
			if (!PC || !PC->GetPawn() || !PC->GetPawn()->IsA(AGunnerTestPawn::StaticClass()))
			{
				continue;
			}


			APawn* Pawn = PC->GetPawn();
			UNexusActionComponent* ActionComponent = UNexusActionComponent::GetActionComponentFromActor(Pawn);
			check(ActionComponent);

			UGunnerSlotManagerComponent* SlotManagerComponent = GetSlotManagerComponent(Pawn);
			check(SlotManagerComponent);


			UNexusProperty* SlotIndexProperty = ActionComponent->GetProperty(GunnerNativeGameplayTags::TAG_Property_SlotIndex);
			if (IsServer(WorldContext))
			{
				ActionComponent->SendEventToSelf<FNexusEventMessage>(GunnerNativeGameplayTags::TAG_Input_Drop, {});
				SlotIndexProperty->Tick();
			}


			Test->TestTrue(Prefix(TEXT("슬롯 아이템이 제거되었습니다")), SlotManagerComponent->IsSlotEmpty(EGunnerSlotType::Primary));
			if (!Test->TestTrue(Prefix(TEXT("슬롯이 비활성화 되었습니다.")), SlotManagerComponent->GetCurrentSlotType() != EGunnerSlotType::Primary))
			{
				Test->AddInfo(Prefix(FString::Printf(TEXT("슬롯 인덱스: %f"), SlotIndexProperty->GetDynamicValue())));
				Test->AddInfo(Prefix(FString::Printf(TEXT("슬롯 인덱스(스태틱): %f"), SlotIndexProperty->GetStaticValue())));
			}

			const FNexusActionDefContainer& ActionDefs = ActionComponent->GetActionDefs();
			Test->TestFalse(Prefix(TEXT("Persistent액션이 제거되었습니다")), ActionDefs.Items.ContainsByPredicate([](const FNexusActionDef& Element)
			{
				return Element.ActionClass == UGunnerTestActionActivatePrimary::StaticClass();
			}));
			Test->TestFalse(Prefix(TEXT("Transient액션이 제거되었습니다")), ActionDefs.Items.ContainsByPredicate([](const FNexusActionDef& Element)
			{
				return Element.ActionClass == UGunnerTestActionTransient::StaticClass();
			}));


			UNexusProperty* BulletProperty = ActionComponent->GetProperty(GunnerNativeGameplayTags::TAG_Property_Weapon_Bullet);
			UNexusProperty* MagazineBulletProperty = ActionComponent->GetProperty(GunnerNativeGameplayTags::TAG_Property_Weapon_MagazineBullet);

			if (IsServer(WorldContext))
			{
				BulletProperty->Tick();
				MagazineBulletProperty->Tick();
			}

			Test->TestNotEqual(Prefix(TEXT("슬롯 아이템의 총알 수가 업데이트 되었습니다")), BulletProperty->GetDynamicValue(), PreDropBulletCount);
			Test->TestNotEqual(Prefix(TEXT("슬롯 아이템의 탄알집의 총알 수가 업데이트 되었습니다")), MagazineBulletProperty->GetDynamicValue(), PreDropMagazineBulletCount);
		}
		Test->AddInfo(Prefix(TEXT("Test_SlotManager_Drop() 완료")));
	}

	void CheckAquire(APawn* Pawn)
	{
		UNexusActionComponent* ActionComponent = UNexusActionComponent::GetActionComponentFromActor(Pawn);
		check(ActionComponent);

		UGunnerSlotManagerComponent* SlotManagerComponent = GetSlotManagerComponent(Pawn);
		check(SlotManagerComponent);

		AGunnerTestGun* TestGun = Cast<AGunnerTestGun>(SlotManagerComponent->GetSlotItemByType(GetDefault<AGunnerTestGun>()->GetSlotType()));

		if (!Test->TestNotNull(Prefix(TEXT("슬롯 아이템이 추가되었습니다")), TestGun))
		{
			return;
		}
		Test->TestNotEqual(Prefix(TEXT("획득한 슬롯 아이템이 아직 장착되지 않았습니다")), TestGun->GetSlotType(), SlotManagerComponent->GetCurrentSlotType());
		Test->TestNotEqual(Prefix(TEXT("획득한 아이템의 이전 오너와 현재 오너가 다릅니다")), Cast<APawn>(TestGun->LastAgentActor), Pawn);


		const FNexusActionDefContainer& ActionDefs = ActionComponent->GetActionDefs();
		Test->TestTrue(Prefix(TEXT("Persistent액션이 추가되었습니다")), ActionDefs.Items.ContainsByPredicate([](const FNexusActionDef& Element)
		{
			return Element.ActionClass == UGunnerTestActionActivatePrimary::StaticClass();
		}));
	}

	void Test_SlotManager_ServerEnablePickup()
	{
		if (IsServer(WorldContext))
		{
			int32 Count = 0;
			for (TActorIterator<AActor> It(WorldContext.World(), AGunnerTestSlotItemPickup::StaticClass()); It; ++It)
			{
				AGunnerTestSlotItemPickup* Pickup = Cast<AGunnerTestSlotItemPickup>(*It);
				Pickup->SetPickupEnabled(true);
				Count++;
			}
			Test->TestTrue(Prefix(TEXT("서버에서 Pickup을 활성화했습니다")), Count == 3);
			Test->AddInfo(Prefix(TEXT("Test_SlotManager_ServerEnablePickup() 완료")));
		}
	}

	void Test_SlotManager_Acquire()
	{
		for (FConstPlayerControllerIterator Iterator = WorldContext.World()->GetPlayerControllerIterator(); Iterator; ++Iterator)
		{
			APlayerController* PC = Iterator->Get();
			if (!PC || !PC->GetPawn() || !PC->GetPawn()->IsA(AGunnerTestPawn::StaticClass()))
			{
				continue;
			}

			APawn* Pawn = PC->GetPawn();
			CheckAquire(Pawn);
		}

		Test->AddInfo(Prefix(TEXT("Test_SlotManager_Acquire() 완료")));
	}

	void Test_SlotManger_CheckPreDropProperties()
	{
		for (FConstPlayerControllerIterator Iterator = WorldContext.World()->GetPlayerControllerIterator(); Iterator; ++Iterator)
		{
			APlayerController* PC = Iterator->Get();
			if (!PC || !PC->GetPawn() || !PC->GetPawn()->IsA(AGunnerTestPawn::StaticClass()))
			{
				continue;
			}

			APawn* Pawn = PC->GetPawn();
			UNexusActionComponent* ActionComponent = UNexusActionComponent::GetActionComponentFromActor(Pawn);
			check(ActionComponent);

			if (IsServer(WorldContext))
			{
				UNexusProperty* BulletProperty = ActionComponent->GetProperty(GunnerNativeGameplayTags::TAG_Property_Weapon_Bullet);
				BulletProperty->Tick();
				UNexusProperty* MagazineBulletProperty = ActionComponent->GetProperty(GunnerNativeGameplayTags::TAG_Property_Weapon_MagazineBullet);
				MagazineBulletProperty->Tick();
			}


			UNexusProperty* BulletProperty = ActionComponent->GetProperty(GunnerNativeGameplayTags::TAG_Property_Weapon_Bullet);
			Test->TestEqual(Prefix(TEXT("슬롯 아이템의 총알 수가 같습니다")), BulletProperty->GetDynamicValue(), PreDropBulletCount);

			UNexusProperty* MagazineBulletProperty = ActionComponent->GetProperty(GunnerNativeGameplayTags::TAG_Property_Weapon_MagazineBullet);
			Test->TestEqual(Prefix(TEXT("슬롯 아이템의 탄알집의 총알 수가 같습니다")), MagazineBulletProperty->GetDynamicValue(), PreDropMagazineBulletCount);
		}
		Test->AddInfo(Prefix(TEXT("Test_SlotManger_CheckPreDropProperties() 완료")));
	}


	UGunnerSlotManagerComponent* GetSlotManagerComponent(AActor* Actor)
	{
		if (Actor)
		{
			if (UGunnerSlotManagerComponent* SlotManagerComponent = Actor->GetComponentByClass<UGunnerSlotManagerComponent>())
			{
				return SlotManagerComponent;
			}

			if (const IGunnerSlotManagerInterface* SlotManagerInterface = Cast<IGunnerSlotManagerInterface>(Actor))
			{
				return SlotManagerInterface->GetSlotManagerComponent();
			}
		}
		return nullptr;
	}

	static bool IsServer(const FWorldContext& WorldContext) { return WorldContext.PIEInstance == 0; }
	FString Prefix(const FString& Msg) const { return FString::Printf(TEXT("[PIE_%d]"), WorldContext.PIEInstance) + ": " + Msg; }

private:
	FWorldContext WorldContext;
	FAutomationTestBase* Test = nullptr;

	float PreDropBulletCount = 5.0f;
	float PreDropMagazineBulletCount = 10.0f;
};


class FGunnerTest : public FAutomationTestBase
{
public:
	typedef void (FGunnerTestCollection::*TestFunc)();

	FGunnerTest(const FString& InName)
		: FAutomationTestBase(InName, false)
	{
		AddTest(&FGunnerTestCollection::Test_SlotManager_AddItem, TEXT("SlotManager.TestSlotManager"), 0.0f, 0.2f);
		AddTest(&FGunnerTestCollection::Test_SlotManager_ActivateItem, TEXT("SlotManager.TestSlotManager"), 0.0f, 0.2f);
		AddTest(&FGunnerTestCollection::Test_SlotManager_ChangeBullet, TEXT("SlotManager.TestSlotManager"), 0.0f, 0.2f);
		AddTest(&FGunnerTestCollection::Test_SlotManager_Drop, TEXT("SlotManager.TestSlotManager"), 0.0f, 0.2f);
		AddTest(&FGunnerTestCollection::Test_SlotManager_ServerEnablePickup, TEXT("SlotManager.TestSlotManager"));
		AddTest(&FGunnerTestCollection::Test_SlotManager_Acquire, TEXT("SlotManager.TestSlotManager"), 0.2f, 0.2f);
		AddTest(&FGunnerTestCollection::Test_SlotManager_ActivateItem, TEXT("SlotManager.TestSlotManager"), 0.0f, 0.2f);
		AddTest(&FGunnerTestCollection::Test_SlotManger_CheckPreDropProperties, TEXT("SlotManager.TestSlotManager"), 0.0f, 0.2f);
	}

	virtual uint32 GetTestFlags() const override { return EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter; }
	virtual FString GetBeautifiedTestName() const override { return TEXT("System.Gunner.GunnerTest"); }
	virtual uint32 GetRequiredDeviceNum() const override { return 1; }


	virtual void GetTests(TArray<FString>& OutBeautifiedNames, TArray<FString>& OutTestCommands) const override
	{
		for (const auto& [TestFuncName, TestContext] : TestContextMap)
		{
			OutBeautifiedNames.Add(TestFuncName);
			OutTestCommands.Add(TestFuncName);
		}
	}

	virtual bool RunTest(const FString& Parameters) override
	{
		// find the matching test


		FTestContext* TestContextPtr = TestContextMap.Find(Parameters);
		if (!TestContextPtr)
		{
			return false;
		}


		FLevelEditorModule& LevelEditorModule = FModuleManager::Get().GetModuleChecked<FLevelEditorModule>(TEXT("LevelEditor"));

		ULevelEditorPlaySettings* PlaySettings = GetMutableDefault<ULevelEditorPlaySettings>();
		PlaySettings->SetPlayNumberOfClients(3);
		PlaySettings->SetPlayNetMode(EPlayNetMode::PIE_ListenServer);

		FRequestPlaySessionParams SessionParams;
		SessionParams.DestinationSlateViewport = LevelEditorModule.GetFirstActiveViewport();
		SessionParams.EditorPlaySettings = PlaySettings;
		SessionParams.GameModeOverride = AGunnerTestGameMode::StaticClass();
		SessionParams.GlobalMapOverride = TEXT("/Game/Maps/GunnerTestMap");
		GUnrealEd->RequestPlaySession(SessionParams);

		// 맵 로드 딜레이
		ADD_LATENT_AUTOMATION_COMMAND(FWaitLatentCommand(2.0f));


		for (int i = 0; i < TestContextPtr->FunctionPtrs.Num(); ++i)
		{
			if (TestContextPtr->InitialDelays[i] > 0.0f)
			{
				ADD_LATENT_AUTOMATION_COMMAND(FWaitLatentCommand(TestContextPtr->InitialDelays[i]));
			}

			TestFunc FunctionPtr = TestContextPtr->FunctionPtrs[i];

			// 서버 테스트 코드 실행
			ADD_LATENT_AUTOMATION_COMMAND(FGunnerCallableTest([this, FunctionPtr]()
				{
				FGunnerTestCollection Tester(GetWorldContext(0), this);
				(Tester.*FunctionPtr)();
				}));


			// 리플리케이션 딜레이
			if (TestContextPtr->ReplicationDelays[i] > 0.0f)
			{
				ADD_LATENT_AUTOMATION_COMMAND(FWaitLatentCommand(TestContextPtr->ReplicationDelays[i]));
			}


			// 클라이언트 테스트 코드 실행
			ADD_LATENT_AUTOMATION_COMMAND(FGunnerCallableTest([this, FunctionPtr]()
				{
				FGunnerTestCollection Tester(GetWorldContext(1), this);
				(Tester.*FunctionPtr)();
				}));
			ADD_LATENT_AUTOMATION_COMMAND(FGunnerCallableTest([this, FunctionPtr]()
				{
				FGunnerTestCollection Tester(GetWorldContext(2), this);
				(Tester.*FunctionPtr)();
				}));
		}


		ADD_LATENT_AUTOMATION_COMMAND(FEndPlayMapCommand());


		return true;
	}

private:
	void AddTest(const TestFunc& InTestFunction, const FString& InTestName, float InitialDelay = 0.0f, float ReplicationDelay = 0.0f)
	{
		FTestContext& TestContext = TestContextMap.FindOrAdd(InTestName);
		TestContext.InitialDelays.Add(InitialDelay);
		TestContext.FunctionPtrs.Add(InTestFunction);
		TestContext.ReplicationDelays.Add(ReplicationDelay);
	}


	FWorldContext GetWorldContext(int32 PIEInstance)
	{
		const TIndirectArray<FWorldContext>& WorldContexts = GEditor->GetWorldContexts();
		for (const auto& WorldContext : WorldContexts)
		{
			if (WorldContext.WorldType == EWorldType::PIE && WorldContext.PIEInstance == PIEInstance)
			{
				return WorldContext;
			}
		}
		checkNoEntry();
		return FWorldContext();
	}

private:
	struct FTestContext
	{
		TArray<float> InitialDelays;
		TArray<TestFunc> FunctionPtrs;
		TArray<float> ReplicationDelays;
	};

	TMap<FString, FTestContext> TestContextMap;
};

namespace
{
	FGunnerTest GunnerTestInstance(TEXT("GunnerTest"));
}

#endif // WITH_EDITOR
