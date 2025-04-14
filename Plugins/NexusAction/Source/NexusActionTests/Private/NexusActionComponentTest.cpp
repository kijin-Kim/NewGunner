#include "ActionSystemTestPawn.h"
#include "Action/NexusAction.h"
#include "Action/NexusActionComponent.h"
#include "GameFramework/PlayerState.h"
#include "Misc/AutomationTest.h"
#include "Tests/AutomationEditorCommon.h"

IMPLEMENT_MODULE(FDefaultModuleImpl, NexusActionTests)

class UTestAction : public UNexusAction
{
public:
	UTestAction()
	{
		ActionNetMethod = ENexusActionNetMethod::LocalOnly;
	}
};

class FActionSystemTestSuite
{
public:
	FActionSystemTestSuite(UWorld* InWorld, FAutomationTestBase* InTest)
		: World(InWorld), Test(InTest)
	{
		// Set up basic actor hierarchy
		TestPawn = World->SpawnActor<AActionSystemTestPawn>();
		TestController = World->SpawnActor<APlayerController>();
		TestController->Possess(TestPawn);
		TestActionComponent = UNexusActionComponent::GetActionComponentFromActor(TestPawn);
	}

	~FActionSystemTestSuite()
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

		FNexusActionDef ActionDef(TestPawn, UTestAction::StaticClass());
		TestActionComponent->AuthAddAction(ActionDef);
		TestActionComponent->TryTriggerAction(ActionDef.Handle, FNexusEventMessage());
		FNexusActionDef* ResultDef = TestActionComponent->FindActionDefByHandle(ActionDef.Handle);

		if (!Test->TestNotNull(TEXT("액션 인스턴스가 유효합니다."), ResultDef->ActionInstance.Get()))
		{
			return;
		}
		Test->TestTrue(TEXT("액션이 트리거 되었습니다."), ResultDef->ActionInstance->IsTriggering());
	}

private:
	APlayerController* TestController = nullptr;
	APawn* TestPawn = nullptr;
	UNexusActionComponent* TestActionComponent = nullptr;
	UWorld* World = nullptr;
	FAutomationTestBase* Test = nullptr;
};

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FActionSystemLocalTest, "NexusActionTests.Action.LocalOnlyTrigger", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FActionSystemLocalTest::RunTest(const FString& Parameters)
{
	UWorld *World = UWorld::CreateWorld(EWorldType::Game, false);
	FWorldContext &WorldContext = GEngine->CreateNewWorldContext(EWorldType::Game);
	WorldContext.SetCurrentWorld(World);

	FURL URL;
	//World->GetGameInstanceChecked<UGameInstance>()->EnableListenServer(true);
	World->InitializeActorsForPlay(URL);
	World->BeginPlay();

	uint64 InitialFrameCounter = GFrameCounter;
	{
		FActionSystemTestSuite Tester(World, this);
		Tester.Test_TriggerLocalOnlyAction();
	}
	GFrameCounter = InitialFrameCounter;

	GEngine->DestroyWorldContext(World);
	World->DestroyWorld(false);

	return true;

}
