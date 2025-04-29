#include "EngineUtils.h"
#include "GunnerItemPickup_Test.h"
#include "GunnerItem_Test.h"
#include "GunnerTestGameMode.h"
#include "LevelEditor.h"
#include "UnrealEdGlobals.h"
#include "Action/NexusActionComponent.h"

#include "GunnerTestPawn.h"
#include "Editor/UnrealEdEngine.h"
#include "Gunner/Item/GunnerInventoryManagerComponent.h"
#include "Gunner/Item/GunnerItemDef.h"
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
		UObject* LoadedItemDef = StaticLoadObject(UGunnerItemDef::StaticClass(), nullptr, TEXT("/Game/Developers/kijin/Test/ID_TestItem.ID_TestItem"));
		TestItemDef = Cast<UGunnerItemDef>(LoadedItemDef);
		check(TestItemDef);
	}

	~FGunnerTestCollection()
	{
	}

	void Test_InventoryManager_AddItem()
	{
		for (FConstPlayerControllerIterator Iterator = WorldContext.World()->GetPlayerControllerIterator(); Iterator; ++Iterator)
		{
			APlayerController* PC = Iterator->Get();
			if (!PC || !PC->GetPawn() || !PC->GetPawn()->IsA(AGunnerTestPawn::StaticClass()))
			{
				continue;
			}


			APawn* Pawn = PC->GetPawn();

			UGunnerInventoryManagerComponent* InventoryManagerComponent = UGunnerInventoryManagerComponent::GetInventoryManagerComponentFromActor(Pawn);
			const TArray<AGunnerItem*>& InventoryItems = InventoryManagerComponent->GetInventoryItems();
			Test->TestTrue(Prefix(TEXT("아이템 추가 확인")), InventoryItems.ContainsByPredicate([this](const AGunnerItem* Item)
			{
				return Item->GetClass() == TestItemDef->ItemClass;
			}));

			UNexusActionComponent* ActionComponent = UNexusActionComponent::GetActionComponentFromActor(Pawn);
			check(ActionComponent);
			const FNexusActionDefContainer& ActionDefs = ActionComponent->GetActionDefs();
			Test->TestTrue(Prefix(TEXT("Acquired 액션 추가 확인")), ActionDefs.Items.ContainsByPredicate([this](const FNexusActionDef& Element)
			{
				return TestItemDef->AcquiredActionClasses.Contains(Element.ActionClass);
			}));
		}
		Test->AddInfo(Prefix(TEXT("Test_InventoryManager_AddItem() 완료")));
	}


	void Test_InventoryManager_RemoveItem()
	{
		for (FConstPlayerControllerIterator Iterator = WorldContext.World()->GetPlayerControllerIterator(); Iterator; ++Iterator)
		{
			APlayerController* PC = Iterator->Get();
			if (!PC || !PC->GetPawn() || !PC->GetPawn()->IsA(AGunnerTestPawn::StaticClass()))
			{
				continue;
			}

			APawn* Pawn = PC->GetPawn();
			UGunnerInventoryManagerComponent* InventoryManagerComponent = UGunnerInventoryManagerComponent::GetInventoryManagerComponentFromActor(Pawn);
			check(InventoryManagerComponent);
			

			if (IsServer(WorldContext))
			{
				const TArray<AGunnerItem*>& InventoryItems = InventoryManagerComponent->GetInventoryItems();
				AGunnerItem* const* TestItem = InventoryItems.FindByPredicate([this](const AGunnerItem* Item)
				{
					return Item->GetClass() == TestItemDef->ItemClass;
				});
				InventoryManagerComponent->AuthRemoveItem(*TestItem);
			}

			const TArray<AGunnerItem*>& InventoryItems = InventoryManagerComponent->GetInventoryItems();
			Test->TestFalse(Prefix(TEXT("아이템 제거 확인")), InventoryItems.ContainsByPredicate([this](const AGunnerItem* Item)
			{
				return Item->GetClass() == TestItemDef->ItemClass;
			}));

			UNexusActionComponent* ActionComponent = UNexusActionComponent::GetActionComponentFromActor(Pawn);
			check(ActionComponent);
			const FNexusActionDefContainer& ActionDefs = ActionComponent->GetActionDefs();
			Test->TestFalse(Prefix(TEXT("Acquired 액션 제거 확인")), ActionDefs.Items.ContainsByPredicate([this](const FNexusActionDef& Element)
			{
				return TestItemDef->AcquiredActionClasses.Contains(Element.ActionClass);
			}));
		}
		Test->AddInfo(Prefix(TEXT("Test_InventoryManager_RemoveItem() 완료")));
	}


	static bool IsServer(const FWorldContext& WorldContext) { return WorldContext.PIEInstance == 0; }
	FString Prefix(const FString& Msg) const { return FString::Printf(TEXT("[PIE_%d]"), WorldContext.PIEInstance) + ": " + Msg; }

private:
	UGunnerItemDef* TestItemDef;

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
		AddTest(&FGunnerTestCollection::Test_InventoryManager_AddItem, TEXT("TestInventory"), 0.0f, 0.2f);
		AddTest(&FGunnerTestCollection::Test_InventoryManager_RemoveItem, TEXT("TestInventory"), 0.0f, 0.2f);
	}

	virtual uint32 GetTestFlags() const override { return EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter; }
	virtual FString GetBeautifiedTestName() const override { return TEXT("Gunner.GunnerTest"); }
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
