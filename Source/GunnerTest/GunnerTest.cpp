#include "GunnerTestGameMode.h"
#include "GunnerTestGun.h"
#include "LevelEditor.h"
#include "UnrealEdGlobals.h"
#include "Action/NexusAction.h"
#include "Action/NexusActionComponent.h"

#include "GunnerTestPawn.h"
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


	void Test_SlotManager_AddItemAndActivate()
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
			AGunnerTestGun* TestGun = Cast<AGunnerTestGun>(SlotManagerComponent->GetSlotItemByType(EGunnerSlotType::Primary));
			if (!Test->TestNotNull(Prefix(TEXT("슬롯에 아이템이 추가되었습니다.")), TestGun))
			{
				return;
			}

			UNexusActionComponent* ActionComponent = UNexusActionComponent::GetActionComponentFromActor(Pawn);
			check(ActionComponent);
			const FNexusActionDefContainer& ActionDefs = ActionComponent->GetActionDefs();

			Test->TestTrue(Prefix(TEXT("Persistent액션이 추가되었습니다")), ActionDefs.Items.ContainsByPredicate([](const FNexusActionDef& Element)
			{
				return Element.ActionClass == UGunnerTestActionActivatePrimary::StaticClass();
			}));

			if (IsServer(WorldContext))
			{
				ActionComponent->SendEventToSelf<FNexusEventMessage>(GunnerNativeGameplayTags::TAG_Input_ActivateSlot_Primary, {});
				UNexusProperty* SlotIndexProperty = ActionComponent->GetProperty(GunnerNativeGameplayTags::TAG_Property_SlotIndex);
				SlotIndexProperty->Tick();
			}

			//슬롯 아이템 비교
			Test->TestEqual(Prefix(TEXT("활성화된 슬롯 아이템이 같습니다")), Cast<AGunnerTestGun>(SlotManagerComponent->GetCurrentSlotItem()), TestGun);
			Test->TestEqual(Prefix(TEXT("활성화된 슬롯 아이템의 타입이 같습니다")), TestGun->GetSlotType(), EGunnerSlotType::Primary);
			Test->TestTrue(Prefix(TEXT("Transient액션이 추가되었습니다")), ActionDefs.Items.ContainsByPredicate([](const FNexusActionDef& Element)
			{
				return Element.ActionClass == UGunnerTestActionTransient::StaticClass();
			}));
		}
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
			
			

			Test->AddInfo(Prefix(FString::Printf(TEXT("슬롯 인덱스: %f"), SlotIndexProperty->GetDynamicValue())));
			Test->AddInfo(Prefix(FString::Printf(TEXT("슬롯 인덱스(스태틱): %f"), SlotIndexProperty->GetStaticValue())));
			Test->TestTrue(Prefix(TEXT("슬롯 아이템이 제거되었습니다")), SlotManagerComponent->IsSlotEmpty(EGunnerSlotType::Primary));
			Test->TestTrue(Prefix(TEXT("슬롯이 비활성화 되었습니다.")), SlotManagerComponent->GetCurrentSlotType() != EGunnerSlotType::Primary);

			const FNexusActionDefContainer& ActionDefs = ActionComponent->GetActionDefs();
			Test->TestFalse(Prefix(TEXT("Persistent액션이 제거되었습니다")), ActionDefs.Items.ContainsByPredicate([](const FNexusActionDef& Element)
			{
				return Element.ActionClass == UGunnerTestActionActivatePrimary::StaticClass();
			}));
			Test->TestFalse(Prefix(TEXT("Transient액션이 제거되었습니다")), ActionDefs.Items.ContainsByPredicate([](const FNexusActionDef& Element)
			{
				return Element.ActionClass == UGunnerTestActionTransient::StaticClass();
			}));
		}
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
};


class FGunnerTest : public FAutomationTestBase
{
public:
	typedef void (FGunnerTestCollection::*TestFunc)();

	FGunnerTest(const FString& InName)
		: FAutomationTestBase(InName, false)
	{
		AddTest(&FGunnerTestCollection::Test_SlotManager_AddItemAndActivate, TEXT("SlotManager.TestSlotManager"), 0.0f, 1.0f);
		AddTest(&FGunnerTestCollection::Test_SlotManager_Drop, TEXT("SlotManager.TestSlotManager"), 0.0f, 3.0f);
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
		SessionParams.GlobalMapOverride = TEXT("/NexusAction/Maps/NexusActionTestMap");
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
