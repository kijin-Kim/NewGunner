#include "ActionSystemTestPawn.h"
#include "LevelEditor.h"
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

class UTestAction : public UNexusAction
{
public:
	UTestAction()
	{
		ActionNetMethod = ENexusActionNetMethod::LocalOnly;
	}
};

class FActionSystemTestCollection
{
public:
	FActionSystemTestCollection(UWorld* InWorld, FAutomationTestBase* InTest)
		: World(InWorld), Test(InTest)
	{
		// Set up basic actor hierarchy
		TestPawn = World->SpawnActor<AActionSystemTestPawn>();
		TestController = World->SpawnActor<APlayerController>();
		TestController->Possess(TestPawn);
		TestActionComponent = UNexusActionComponent::GetActionComponentFromActor(TestPawn);
	}

	~FActionSystemTestCollection()
	{
		if (TestPawn)
		{
			World->EditorDestroyActor(TestPawn, false);
		}
	}

	void Test_TriggerLocalOnlyAction()
	{
		if (!Test->TestTrue(TEXT("액션 컴포넌트가 셋업 되었습니다."), TestActionComponent->IsSetupCompleted()))
		{
			return;
		}

		FNexusActionDefHandle Handle = TestActionComponent->AuthAddAction(UTestAction::StaticClass(), TestPawn);
		TestActionComponent->TryTriggerAction(Handle);
		UNexusAction* ActionInstance = TestActionComponent->FindActionInstanceByHandle(Handle);
		if (!Test->TestNotNull(TEXT("액션 인스턴스가 유효합니다."), ActionInstance))
		{
			return;
		}
		Test->TestTrue(TEXT("액션이 트리거 되었습니다."), ActionInstance->IsTriggering());
	}

private:
	APlayerController* TestController = nullptr;
	APawn* TestPawn = nullptr;
	UNexusActionComponent* TestActionComponent = nullptr;
	UWorld* World = nullptr;
	FAutomationTestBase* Test = nullptr;
};


class FActionSystemTest : public FAutomationTestBase
{
public:
	typedef void (FActionSystemTestCollection::*TestFunc)();

	FActionSystemTest(const FString& InName)
		: FAutomationTestBase(InName, false)
	{
		// Add test functions here
		AddTest(&FActionSystemTestCollection::Test_TriggerLocalOnlyAction, TEXT("TriggerLocalOnlyAction"));
	}

	virtual uint32 GetTestFlags() const override { return EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter; }
	virtual FString GetBeautifiedTestName() const override { return TEXT("System.ActionSystem.ActionSystemComponent"); }
	virtual uint32 GetRequiredDeviceNum() const override { return 1; }


	virtual void GetTests(TArray<FString>& OutBeautifiedNames, TArray<FString>& OutTestCommands) const override
	{
		for (const FString& TestFuncName : TestFunctionNames)
		{
			OutBeautifiedNames.Add(TestFuncName);
			OutTestCommands.Add(TestFuncName);
		}
	}

	virtual bool RunTest(const FString& Parameters) override
	{
		// find the matching test
		TestFunc TestFunction = nullptr;
		for (int32 i = 0; i < TestFunctionNames.Num(); ++i)
		{
			if (TestFunctionNames[i] == Parameters)
			{
				TestFunction = TestFunctions[i];
				break;
			}
		}

		if (!TestFunction)
		{
			return false;
		}


		FLevelEditorModule& LevelEditorModule = FModuleManager::Get().GetModuleChecked<FLevelEditorModule>(TEXT("LevelEditor"));

		ULevelEditorPlaySettings* PlaySettings = GetMutableDefault<ULevelEditorPlaySettings>();
		PlaySettings->SetPlayNumberOfClients(2);
		PlaySettings->SetPlayNetMode(EPlayNetMode::PIE_ListenServer);

		FRequestPlaySessionParams SessionParams;
		SessionParams.DestinationSlateViewport = LevelEditorModule.GetFirstActiveViewport();
		SessionParams.EditorPlaySettings = PlaySettings;
		GUnrealEd->RequestPlaySession(SessionParams);


		ADD_LATENT_AUTOMATION_COMMAND(FWaitLatentCommand(3.0f));

		GUnrealEd->RequestEndPlayMap();


		//
		// UWorld* World = UWorld::CreateWorld(EWorldType::Game, false);
		// FWorldContext& WorldContext = GEngine->CreateNewWorldContext(EWorldType::Game);
		// WorldContext.SetCurrentWorld(World);
		//
		// FURL URL;
		// //World->GetGameInstanceChecked<UGameInstance>()->EnableListenServer(true);
		// World->InitializeActorsForPlay(URL);
		// World->BeginPlay();
		//
		// uint64 InitialFrameCounter = GFrameCounter;
		// {
		// 	FActionSystemTestCollection Tester(World, this);
		// 	(Tester.*TestFunction)();
		// }
		// GFrameCounter = InitialFrameCounter;
		//
		// GEngine->DestroyWorldContext(World);
		// World->DestroyWorld(false);

		return true;
	}

private:
	void AddTest(const TestFunc& InTestFunction, const FString& InTestName)
	{
		TestFunctions.Add(InTestFunction);
		TestFunctionNames.Add(InTestName);
	}

private:
	TArray<TestFunc> TestFunctions;
	TArray<FString> TestFunctionNames;
};

namespace
{
	FActionSystemTest ActionSystemTestInstance(TEXT("ActionSystemTest"));
}

#endif // WITH_EDITOR
