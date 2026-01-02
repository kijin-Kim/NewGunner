# 넥서스 액션 시스템 기술 문서

# 개요

넥서스 액션 시스템은 언리얼 엔진 기반의 멀티플레이어 게임을 위한 커스텀 프레임워크입니다. 

이 시스템은 멀티플레이어 게임 개발에서 발생하는 네트워크 코드 중복, 클라이언트 예측시 발생하는 부작용, 애니메이션 몽타주 동기화 등의 문제를 해결하기 위해 설계되었습니다.

# 문제점: 반복적인 네트워크 통신 코드

---

## 배경

---

멀티플레이어 게임을 개발할 때마다 '클라이언트 요청 → 서버 확인 → 실행'이라는 유사한 네트워크 통신 코드를 반복해서 작성하는 것은 흔한 문제입니다. 이는 코드 중복을 야기하고 유지보수를 어렵게 하며, 개발 효율성을 떨어뜨립니다. 

```cpp
UFUNCTION(Server, Reliable)
void Server_A() { GameLogic_A(); Client_A(); }
UFUNCTION(Client, Reliable)
void Client_A() { GameLogic_A(); }

UFUNCTION(Server, Reliable)
void Server_B() { GameLogic_B(); Client_B(); }
UFUNCTION(Client, Reliable)
void Client_B() { GameLogic_B(); }
```

## 해결 방법: 행동의 추상화및 네트워크 통신 코드와의 의존성 제거

---

넥서스 액션 시스템은 이러한 문제를 해결하기 위해 '액션'이라는 개념을 도입하여 게임 내 행동을 추상화하고,  네트워크 통신 과정을 프레임워크가 관리하도록 하였습니다.

1. 행동의 추상화:
    - 게임 내 모든 행동(스킬 사용, 아이템 획득 등)을 액션이라는 단위로 추상화하였습니다. 더 이상 기능마다 `UFUNCTION(Server, Reliable)` 같은 RPC 함수를 개별적으로 정의할 필요가 없고,  대신, 각 액션 클래스 안에 해당 기능의 핵심 게임 로직만 구현하고, 미리 시스템이 정의한 통신 방식을 열거형을 통해 지정할 수 있게 하였습니다.
        
        ![네트워크 동기화 과정을 추상화하여 열거형을 통해 고를 수 있게 함
        ‣](image.png)
        
        네트워크 동기화 과정을 추상화하여 열거형을 통해 고를 수 있게 함
        ‣
        
    
    ```cpp
    
    UCLASS()
    class GUNNER_API UAction_Fire: public UNexusAction
    {
        GENERATED_BODY()
    public:
        // 이 함수 안에 사격 행동의 핵심 로직만 구현합니다.
        // 네트워크 통신은 프레임워크가 따로 처리합니다.
        virtual void OnTriggerAction() override;
    };
    
    ```
    
2. 네트워크 통신 코드의 중앙화: 
    - 액션이 실행될 때, 넥서스 액션 시스템 프레임워크가 복잡한 네트워크 통신 과정을 직접 관리합니다. 클라이언트에서 액션 실행을 요청하면, 시스템은 이 액션이 지정한 통신 방식에 따라 서버/클라이언트와 통신합니다.
    - 즉, 직접 RPC 함수를 호출하거나 데이터 복제를 위한 코드를 작성하지 않고, 넥서스 액션 시스템이 액션이 지정한 통신 방식에 따라 프레임워크 차원에서 총괄 관리합니다.
    - 
        
        [https://github.com/kijin-Kim/NewGunner/blob/825f993a8b092475c1b38158856e2e60aa98119a/Plugins/NexusAction/Source/NexusAction/Private/Action/NexusActionComponent.cpp#L624-L670](https://github.com/kijin-Kim/NewGunner/blob/825f993a8b092475c1b38158856e2e60aa98119a/Plugins/NexusAction/Source/NexusAction/Private/Action/NexusActionComponent.cpp#L624-L670)
        

<aside>

💡 시스템에서 제공하는 세 가지 실행 방식이 있습니다:

1. 한 쪽에서만 실행: 서버나 클라이언트 중 한 곳에서만 실행되며, 다른 쪽과 통신이 필요 없는 방식입니다 (`LocalOnly`, `ServerOnly`)
    
    ![각각 클라이언트와 서버에서 독자적으로 실행합니다.](LocalOnlyServerOnly.png)
    
    각각 클라이언트와 서버에서 독자적으로 실행합니다.
    
2. 클라이언트 예측 실행: 클라이언트가 먼저 실행하고 서버에 알려주는 방식입니다. (`LocalPredicted`)
    
    ![클라이언트에서 실행 후 서버에 RPC를 보낸 후 서버에서도 동일하게 실행합니다.](LocalPredicted.png)
    
    클라이언트에서 실행 후 서버에 RPC를 보낸 후 서버에서도 동일하게 실행합니다.
    
3. 서버 승인 실행: 클라이언트가 서버에 실행 허가를 요청하고, 서버가 승인한 후에 실행되는 방식입니다. (`ServerAuthoritative`)
    
    ![클라이언트에서 액션을 실행하기 전 서버로 RPC를 보낸 후, 서버에서 다시 승인 RPC를 받은 후 실행합니다.](ServerAuthoritative.png)
    
    클라이언트에서 액션을 실행하기 전 서버로 RPC를 보낸 후, 서버에서 다시 승인 RPC를 받은 후 실행합니다.
    
</aside>

# 문제점: '예측'이 야기하는 속성 동기화의 복잡성

---

## 배경

---

멀티플레이어 게임에서 플레이어에게 즉각적인 반응성을 제공하는 건 몰입도를 높이는 핵심입니다. 네트워크 지연은 피할 수 없으니, 플레이어가 스킬을 쓰거나 행동할 때 클라이언트에서 그 결과를 '예측'해 즉시 보여주는 것은 이러한 지연을 가려주고 부드러운 사용자 경험을 제공하는 데 필수적입니다. 

하지만 언리얼 엔진이 속성값을 자동으로 복제(Replicate)해주더라도, 이러한 '예측'을 실제로 도입하고 구현하는 순간 복제만으로는 해결할 수 없는 복잡하고 어려운 문제들이 발생합니다.

1. '덮어쓰기(Override)' 문제
    
    클라이언트가 네트워크 지연을 극복하려 속성 변화를 '예측'해 화면에 먼저 적용했을 때, 이후 서버로부터 확정된 속성 값이 도착하면 클라이언트의 임시 값을 서버의 권위 있는 값으로 '덮어쓰는(Override)' 방식을 어떻게 처리할지가 큰 난관입니다. 단순히 덮어쓰면 클라이언트가 예측한 중간 과정의 정보가 사라지거나, 다른 예측 로직과의 충돌로 잘못된 값이 적용될 위험이 있습니다.
    
    예를들어 플레이어가 총을 연사하여 총알을 발사한다고 가정해 봅시다. 클라이언트는 플레이어의 입력에 즉각 반응하기 위해 총알이 발사되는 순간 탄창의 남은 총알 수를 '예측적으로' 감소시킵니다 (예: 30발 -> 29발 -> 28발 $\dots$→ 25발). 그런데 만약 이 도중에 서버가 같은 총알(Bullet) 변수를 29로 바꿨다는 정보가 복제되면, 이 29발이라는 서버 값을 받아 자신의 예측된 25발을 무작정 '덮어써야(Override)' 합니다. 이 경우, 클라이언트는 자신이 발사한 총알에 대한 정보(총알 수 감소 예측)를 잃고 총알이 갑자기 늘어나거나, 이후 발사된 총알 수가 서버와 불일치하게 되는 문제가 발생할 수 있습니다.
    
    ![#1: 클라이언트가 총알 개수를 30개에서 29개로 줄이고 이를 서버에 알림.
    #2: 클라이언트가 총알을 예측하여 25개까지 줄어든 상황에서, 서버로부터 이전에 보냈던 29개라는 정보가 도착.
    #3: 클라이언트 총알 갯수가 덮어쓰여짐](Override.png)
    
    #1: 클라이언트가 총알 개수를 30개에서 29개로 줄이고 이를 서버에 알림.
    #2: 클라이언트가 총알을 예측하여 25개까지 줄어든 상황에서, 서버로부터 이전에 보냈던 29개라는 정보가 도착.
    #3: 클라이언트 총알 갯수가 덮어쓰여짐
    
2. '되돌릴 수 없는(Undoable)' 문제
    
    또 다른 시나리오로 클라이언트가 예측적으로 총알을 발사하여 총알을 감소시켰는데, 서버에서 어떤 이유로 이 사격 행위가 실패했다고 판단할 수 있습니다. 이 경우, 클라이언트에서 이미 줄어들었던 총알 수를 원래대로 '되돌리는(Undo)' 로직이 필요합니다.
    
    이러한 '되돌리기'는 예측의 정확한 시점과 상태를 파악해야 하므로 매우 까다롭습니다. 예를 들어, 클라이언트가 총알을 30개에서 29개로 줄이고 서버에 변경을 요청했는데 서버가 거절했다고 가정해보겠습니다. 서버는 변수가 변경될 때만 클라이언트에게 알리므로, 요청이 거절되면 클라이언트는 이를 알 수 없습니다.
    
    ![#1: 클라이언트가 총알을 30개에서 29개로 줄이고 서버에 이 변경을 요청.
    #2: 서버가 이 요청을 거절하고 서버에서는 총알을 30개로 유지.](Undo.png)
    
    #1: 클라이언트가 총알을 30개에서 29개로 줄이고 서버에 이 변경을 요청.
    #2: 서버가 이 요청을 거절하고 서버에서는 총알을 30개로 유지.
    
    서버가 거절 정보를 보낸다 해도 또 다른 문제가 있습니다. 클라이언트가 여러 단계의 계산을 수행할 때 중간 과정을 저장하지 않아 이전 상태로 되돌릴 수 없기 때문입니다.
    
    다음과 같은 간단한 예시를 살펴보겠습니다:
    
    - 초기값은 30입니다
    - 클라이언트가 두 가지 계산을 예측적으로 수행합니다:
        1. 30에서 2를 뺍니다 (= 28)
        2. 28을 2로 나눕니다 (= 14)
    - 클라이언트는 두 번째 계산(2로 나누기)의 중간 과정을 저장하지 않습니다. 따라서 첫 번째 계산(2를 빼는 것)이 실패하면, 클라이언트는 올바른 결과(15)를 계산할 수 없습니다. 대신 14에 2를 더해 16이라는 잘못된 결과가 나옵니다.
    

## 해결 방법: SideEffect와 PredictionTag 개념 도입

---

넥서스 액션 시스템은 SideEffect와 PredictionTag(예측 태그)라는 두 가지 개념을 도입해 상기 문제를 해결하였습니다.

1. SideEffect 개념 도입:
SideEffect는 체력 감소, 마나 회복 등 특정 속성(Nexus Property)에 대한 모든 종류의 변화를 정의하고 캡슐화한 데이터 단위입니다. SideEffect는 어떤 속성을, 얼마만큼, 어떤 방식으로 변화시킬지에 대한 모든 규칙을 포함합니다. 복잡한 수치 계산이나 네트워크 동기화 코드를 직접 작성하는 대신, 해당 규칙이 정의된 SideEffect를 '적용(Apply)'하기만 하면 됩니다. SideEffect 자체가 속성 변화의 복잡한 세부 사항을 추상화하므로, '무엇을 변화시킬지'에 집중할 수 있습니다. 이 SideEffect 자체는 서버에서 다시 적용되고 실행한 클라이언트로 복제됩니다.
    
    ![SideEffect를 블루프린트에서 설정하는 예시
    ](image%201.png)
    
    SideEffect를 블루프린트에서 설정하는 예시
    
2. PredictionTag를 활용한 속성 변화의 프레임워크 관리:
PredictionTag는 넥서스 액션 시스템에서 클라이언트의 예측된 행동과 서버의 확정된 행동을 일대일로 연결하는 고유한 식별자입니다. 클라이언트에서 액션이 실행되어 속성 변화가 유발될 때, 시스템은 이 특정 행동을 식별하기 위한 고유한 PredictionTag를 생성합니다. 시스템은 이 태그를 활용하여 속성 변화의 모든 동기화 과정을 프레임워크 차원에서 관리합니다.

    - Step 1: 클라이언트에서 PredictionTag 생성 및 예측적 SideEffect 적용 (임시 '무한 지속' 취급):
    플레이어가 액션을 실행하려고 하면, 클라이언트는 해당 액션에 대한 고유한 PredictionTag를 생성합니다. 이 태그는 현재 클라이언트에서 발생한 '예측적 행동'을 식별하는 ID 역할을 합니다. 클라이언트는 서버의 응답을 기다리지 않고 이 PredictionTag를 사용하여 속성 변화를 자신의 화면에 '예측적으로' 즉시 적용합니다 (예: 마나가 즉시 감소).
    이때, 즉시 적용되는(Instant) SideEffect라 하더라도, 클라이언트 측에서는 PredictionTag와 연결된 무한 지속 SideEffect로 취급되어 관리됩니다. 이는 예측된 변화량(델타)이 속성의 최종 값에 임시로 영향을 미치도록 합니다. 동시에, 클라이언트는 이 PredictionTag와 함께 액션 실행 요청을 서버로 보냅니다.
    - Step 2: 서버에서의 검증 및 확정 SideEffect 적용:
    서버는 클라이언트로부터 받은 PredictionTag와 함께 액션 실행 요청을 받습니다. 서버는 액션의 유효성을 검증하고, 유효하다고 판단되면 실제 게임 로직을 실행하여 권위 있는 SideEffect를 적용합니다. 그리고 이 확정된 SideEffect 정보는 PredictionTag와 함께 액션을 실행한 클라이언트로 복제됩니다.
    - Step 3: 클라이언트의 예측 SideEffect 제거 및 속성 재계산:
    예측을 수행한 클라이언트가 서버로부터 해당 PredictionTag를 복제받으면, 해당 PredictionTag와 연결된 자신의 예측적 임시 '무한 지속' SideEffect를 제거합니다. 이는 예측의 성공 여부와 관계없이 일어납니다.
        - '덮어쓰기(Override)' 문제 해결 - 델타 예측 및 기준 값 정합: 속성(Nexus Property)은 일반적으로 서버에서 복제(Replicated)되는 UPROPERTY 형태로 관리됩니다. 클라이언트가 예측적으로 자신의 속성을 변경했을 때, 서버로부터 받은 속성 값(Replicated Value)은 최종 값이 아닌 계산의 ‘기준 값'으로 취급됩니다. 클라이언트는 이 서버로부터 온 기준 값에 자신의 예측적인 변화(델타)를 다시 합산하여 최종적으로 보여줄 속성 값을 계산합니다. 이는 서버의 확정 값으로 클라이언트의 예측 값을 일방적으로 '덮어쓰는' 것이 아니라, 최신 서버 값을 기반으로 클라이언트에서 예측된 변화를 재조정하는 방식으로 'Override' 문제를 해결합니다.
        - ‘되돌릴 수 없는’ 문제 해결: 클라이언트는 예측적으로 SideEffect를 임시로 적용하고, 또 예측의 성공 여부와 상관없이 항상 클라이언트의 임시 예측적 임시 '무한 지속' SideEffect를 제거하기 때문에 이 문제는 자연스럽게 해결됩니다. 항상 되돌릴 수 있는 형태로 실행하고, 예측의 성공 여부와 상관 없이 항상 클라이언트의 예측 행동을 되돌린다고 볼 수 있습니다.
        - '재적용 방지' (Redo Prevention): 서버에서 복제되어 온 권위 있는 SideEffect가 클라이언트의 속성에 적용되기 전, 예측한 임시 SideEffect를 제거함으로써, 동일한 SideEffect가 두 번 적용되는 것을 방지합니다.
    

# 문제점: 액션의 실행 역할(Role) 한계로 인한 애니메이션 몽타주 시각적 불일치

---

## 배경

---

멀티플레이어 게임에서 캐릭터 애니메이션은 플레이어에게 중요한 시각적 피드백을 제공하며, 모든 클라이언트에게 동일하게 보여야 합니다. 액션 시스템의 실행 역할(Role) 특성 때문에,  액션 내부에서 재생하는 애니메이션 몽타주가 필요한 모든 클라이언트에게 동기화되지 않는 문제가 발생합니다.

액션은 실행되는 주체의 네트워크 역할(ROLE)에 따라 로직이 실행되는 범위가 제한됩니다. 이 기본적인 한계 때문에, 액션 내부에서 몽타주를 직접 재생할 경우 다음과 같은 시각적 불일치 문제들이 발생합니다. 예를 들어 [`LocalPredicted`](https://www.notion.so/213dfaf5a3f58146ab02f1a5006f57e7?pvs=21) 액션은 클라이언트(`ROLE_AutonomousProxy`)와 서버에서만 실행되기 때문에 내부 로직에서 애니메이션 몽타주를 실행하면 별다른 처리없이는 다른 클라이언트(`ROLE_SimulatedProxy`) 에서는 애니메이션 몽타주가 실행되지 않습니다.

이러한 문제들을 해결하기 위해 매번 서로 다른 애니메이션 동기화를 위한 개별적인 멀티캐스트 RPC(Remote Procedure Call)를 날리는 것은 비효율적이며, 개발 부담을 크게 가중시킵니다.

## 해결: 서버 권위 기반의 애니메이션 몽타주 프레임워크 관리

---

넥서스 액션 시스템은 `NexusAnimMontagePlayerComponent`를 통해 위에서 언급된 `NexusAction`의 실행 역할(Role) 한계로 인한 애니메이션 몽타주 시각적 불일치 문제를 서버 권위 기반의 프레임워크 관리로 해결합니다. 이 컴포넌트는 서버의 정확한 몽타주 재생 상태를 클라이언트에 복제하고, 클라이언트가 이를 기준으로 자신의 애니메이션을 지속적으로 보정함으로써 모든 플레이어에게 일관되고 자연스러운 시각적 경험을 제공합니다.

1. 서버의 몽타주 상태 업데이트 및 복제:
서버는 모든 캐릭터의 애니메이션 몽타주 재생에 대한 권위를 가집니다. 캐릭터가 몽타주를 재생하면, 서버는 해당 몽타주 정보(애셋, 재생 속도, 현재 재생 위치, 활성화된 섹션 이름, 재생 상태 등)를 매 프레임마다`FNexusAnimMontageRepData` 구조체에 업데이트합니다. 이 구조체는 언리얼 엔진의 복제(Replication) 시스템을 통해 주체가 아닌 모든 클라이언트(`ROLE_SimulatedProxy`)에 전달됩니다.  

[https://github.com/kijin-Kim/NewGunner/blob/825f993a8b092475c1b38158856e2e60aa98119a/Plugins/NexusAction/Source/NexusAction/Public/Animation/NexusAnimMontagePlayerComponent.h#L73-L74](https://github.com/kijin-Kim/NewGunner/blob/825f993a8b092475c1b38158856e2e60aa98119a/Plugins/NexusAction/Source/NexusAction/Public/Animation/NexusAnimMontagePlayerComponent.h#L73-L74)

[https://github.com/kijin-Kim/NewGunner/blob/825f993a8b092475c1b38158856e2e60aa98119a/Plugins/NexusAction/Source/NexusAction/Private/Animation/NexusAnimMontagePlayerComponent.cpp#L84-L96](https://github.com/kijin-Kim/NewGunner/blob/825f993a8b092475c1b38158856e2e60aa98119a/Plugins/NexusAction/Source/NexusAction/Private/Animation/NexusAnimMontagePlayerComponent.cpp#L84-L96)

1. 클라이언트의 몽타주 상태 수신 및 보정:
클라이언트는 서버로부터 `ReplicatedAnimMontageData`가 복제되면 `OnRep_ReplicatedAnimMontage` 함수를 통해 수신된 서버의 몽타주 데이터와 현재 로컬에서 재생 중인 몽타주 정보(`FNexusLocalAnimMontageData`)를 비교합니다. 만약 몽타주 애셋 자체(`AnimMontage`)나 `AnimMontageInstanceID`가 서버와 다르거나, 몽타주가 서버에서 정지/일시정지 상태인데 클라이언트는 그렇지 않거나, 재생 속도/섹션 이름이 다르거나, 또는 재생 위치(`Position`)가 `MONTAGE_POSITION_DELTA_TOLERANCE` (0.3f) 이상의 오차를 보인다면, 클라이언트는 즉시 서버의 권위 있는 데이터를 기준으로 로컬 몽타주 재생 상태를 보정합니다. 이는 몽타주를 새로 재생시키거나, 중지시키거나, 특정 재생 위치/속도/섹션으로 점프시키는 등의 작업을 포함합니다. 

[https://github.com/kijin-Kim/NewGunner/blob/825f993a8b092475c1b38158856e2e60aa98119a/Plugins/NexusAction/Source/NexusAction/Private/Animation/NexusAnimMontagePlayerComponent.cpp#L119-L169](https://github.com/kijin-Kim/NewGunner/blob/825f993a8b092475c1b38158856e2e60aa98119a/Plugins/NexusAction/Source/NexusAction/Private/Animation/NexusAnimMontagePlayerComponent.cpp#L119-L169)

1. `AnimMontageInstanceID`를 활용한 정밀한 동기화:
동일한 몽타주가 짧은 시간 내에 반복적으로 재생될 수 있는 시나리오(예: 빠른 연타 공격)에서, 단순히 몽타주 애셋만으로는 각 재생 시도를 정확하게 구분하기 어렵습니다. 이를 해결하기 위해 `FNexusAnimMontageRepData` 구조체에는 `AnimMontageInstanceID`라는 `uint8` 타입의 고유한 인스턴스 식별자가 포함되어 함께 복제됩니다. 서버는 몽타주 재생 시 `PlayMontage` 함수 내에서 이 ID를 증가시켜 동일한 몽타주라도 다른 재생 시도임을 명확히 구분합니다. 클라이언트는 `OnRep_ReplicatedAnimMontage`에서 이 `AnimMontageInstanceID`를 비교하여 서버로부터 받은 몽타주 데이터가 현재 로컬에서 재생 중인 몽타주의 어떤 특정 '인스턴스'에 해당하는지 정확히 파악하고, 이를 통해 더욱 정밀하고 오류 없는 동기화를 수행합니다. 

[https://github.com/kijin-Kim/NewGunner/blob/825f993a8b092475c1b38158856e2e60aa98119a/Plugins/NexusAction/Source/NexusAction/Private/Animation/NexusAnimMontagePlayerComponent.cpp#L37-L51](https://github.com/kijin-Kim/NewGunner/blob/825f993a8b092475c1b38158856e2e60aa98119a/Plugins/NexusAction/Source/NexusAction/Private/Animation/NexusAnimMontagePlayerComponent.cpp#L37-L51)

## 결과

---

[모든 클라이언트에 애니메이션 몽타주가 잘 출력되는 모습](AnimMontageReplication_(2).mp4)

모든 클라이언트에 애니메이션 몽타주가 잘 출력되는 모습