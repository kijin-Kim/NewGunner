#include "NexusActionTestPawn.h"
#include "LevelEditor.h"
#include "NexusNativeGameplayTag.h"
#include "NexusTestAction.h"
#include "NexusTestGameMode.h"
#include "UnrealEdGlobals.h"
#include "Action/NexusAction.h"
#include "Action/NexusActionComponent.h"
#include "Editor/UnrealEdEngine.h"
#include "Misc/AutomationTest.h"
#include "Tests/AutomationCommon.h"
#include "Tests/AutomationEditorCommon.h"

// These test codes are from AbilitySystemComponentTests.cpp 


IMPLEMENT_MODULE(FDefaultModuleImpl, NexusActionTests)

#if WITH_EDITOR


DEFINE_LATENT_AUTOMATION_COMMAND_ONE_PARAMETER(FNexusActionCallableTest, TFunction<void()>, Func);

bool FNexusActionCallableTest::Update()
{
	Func();
	return true;
}




class FNexusActionTestCollection
{
public:
	FNexusActionTestCollection(const FWorldContext& InWorldContext, FAutomationTestBase* InTest)
		: WorldContext(InWorldContext), Test(InTest)
	{
	}

	~FNexusActionTestCollection() = default;

	void Test_ClientTriggerEventually_ServerAddAndRemoteTrigger()
	{
		if (!IsServer(WorldContext))
		{
			return;
		}
		
		for (FConstPlayerControllerIterator Iterator = WorldContext.World()->GetPlayerControllerIterator(); Iterator; ++Iterator)
		{
			APlayerController* PC = Iterator->Get();
			if (!PC || !PC->GetPawn() || !PC->GetPawn()->IsA(ANexusActionTestPawn::StaticClass()))
			{
				continue;
			}

			UNexusActionComponent* ActionComponent = UNexusActionComponent::GetActionComponentFromActor(PC->GetPawn());
			check(ActionComponent)

			if (IsServer(WorldContext))
			{
				FNexusActionDefHandle Handle = ActionComponent->AuthAddAction(UNexusTestAction::StaticClass());
				ActionComponent->TryTriggerAction(Handle);
			}
		}
	}

	void Test_ClientTriggerEventually_ClientCheckNotRepOrTriggered()
	{
		if (IsServer(WorldContext))
		{
			return;
		}

		for (FConstPlayerControllerIterator Iterator = WorldContext.World()->GetPlayerControllerIterator(); Iterator; ++Iterator)
		{
			APlayerController* PC = Iterator->Get();
			if (!PC || !PC->GetPawn() || !PC->GetPawn()->IsA(ANexusActionTestPawn::StaticClass()))
			{
				continue;
			}

			UNexusActionComponent* ActionComponent = UNexusActionComponent::GetActionComponentFromActor(PC->GetPawn());
			check(ActionComponent)
			UNexusAction* TriggeringActionInstance = nullptr;
			for (const auto& [Handle, Instance] : ActionComponent->GetLocalActionInstanceMap())
			{
				if (Instance && Instance->IsA(UNexusTestAction::StaticClass()))
				{
					TriggeringActionInstance = Instance;
					break;
				}
			}

			Test->TestFalse(Prefix(TEXT("로컬 액션이 아직 리플리케이트/실행 되지 않았습니다")), TriggeringActionInstance && TriggeringActionInstance->IsTriggering());
		}
	}

	void Test_ClientTriggerEventually_ClientCheckRepAndTriggered()
	{
		if (IsServer(WorldContext))
		{
			return;
		}


		for (FConstPlayerControllerIterator Iterator = WorldContext.World()->GetPlayerControllerIterator(); Iterator; ++Iterator)
		{
			APlayerController* PC = Iterator->Get();
			if (!PC || !PC->GetPawn() || !PC->GetPawn()->IsA(ANexusActionTestPawn::StaticClass()))
			{
				continue;
			}

			UNexusActionComponent* ActionComponent = UNexusActionComponent::GetActionComponentFromActor(PC->GetPawn());
			check(ActionComponent)
			UNexusAction* TriggeringActionInstance = nullptr;
			for (const auto& [Handle, Instance] : ActionComponent->GetLocalActionInstanceMap())
			{
				if (Instance && Instance->IsA(UNexusTestAction::StaticClass()))
				{
					TriggeringActionInstance = Instance;
					break;
				}
			}
			Test->TestTrue(Prefix(TEXT("로컬 액션이 리플리케이트/실행 되었습니다")), TriggeringActionInstance && TriggeringActionInstance->IsTriggering());
		}
	}

	static bool IsServer(const FWorldContext& WorldContext) { return WorldContext.PIEInstance == 0; }
	FString Prefix(const FString& Msg) const { return FString::Printf(TEXT("[PIE_%d]"), WorldContext.PIEInstance) + ": " + Msg; }

private:
	FWorldContext WorldContext;
	FAutomationTestBase* Test = nullptr;
};


class FNexusActionTest : public FAutomationTestBase
{
public:
	typedef void (FNexusActionTestCollection::*TestFunc)();

	FNexusActionTest(const FString& InName)
		: FAutomationTestBase(InName, false)
	{
		AddTest(&FNexusActionTestCollection::Test_ClientTriggerEventually_ServerAddAndRemoteTrigger, TEXT("ClientTriggerEventually"), 0.0f, 0.0f);
		AddTest(&FNexusActionTestCollection::Test_ClientTriggerEventually_ClientCheckNotRepOrTriggered, TEXT("ClientTriggerEventually"), 0.0f, 0.0f);
		AddTest(&FNexusActionTestCollection::Test_ClientTriggerEventually_ClientCheckRepAndTriggered, TEXT("ClientTriggerEventually"), 1.0f, 0.0f);
	}

	virtual uint32 GetTestFlags() const override { return EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter; }
	virtual FString GetBeautifiedTestName() const override { return TEXT("NexusAction"); }
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
		SessionParams.GameModeOverride = ANexusTestGameMode::StaticClass();
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
			ADD_LATENT_AUTOMATION_COMMAND(FNexusActionCallableTest([this, FunctionPtr]()
				{
				FNexusActionTestCollection Tester(GetWorldContext(0), this);
				(Tester.*FunctionPtr)();
				}));


			// 리플리케이션 딜레이
			if (TestContextPtr->ReplicationDelays[i] > 0.0f)
			{
				ADD_LATENT_AUTOMATION_COMMAND(FWaitLatentCommand(TestContextPtr->ReplicationDelays[i]));
			}


			// 클라이언트 테스트 코드 실행
			ADD_LATENT_AUTOMATION_COMMAND(FNexusActionCallableTest([this, FunctionPtr]()
				{
				FNexusActionTestCollection Tester(GetWorldContext(1), this);
				(Tester.*FunctionPtr)();
				}));
			ADD_LATENT_AUTOMATION_COMMAND(FNexusActionCallableTest([this, FunctionPtr]()
				{
				FNexusActionTestCollection Tester(GetWorldContext(2), this);
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
	FNexusActionTest NexusActionTestInstance(TEXT("NexusActionTest"));
}

#endif // WITH_EDITOR
