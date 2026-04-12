# 넥서스 액션 시스템 (Nexus Action System)

## 개요

넥서스 액션 시스템은 언리얼 엔진 기반의 멀티플레이어 게임을 위한 커스텀 프레임워크입니다.

이 시스템은 멀티플레이어 게임 개발에서 발생하는 **네트워크 코드 중복**, **클라이언트 예측 시 발생하는 부작용**, **애니메이션 몽타주 동기화** 등의 문제를 해결하기 위해 설계되었습니다.

---

## 목차

- [문제점 1: 반복적인 네트워크 통신 코드](#문제점-1-반복적인-네트워크-통신-코드)
- [문제점 2: 예측이 야기하는 속성 동기화의 복잡성](#문제점-2-예측이-야기하는-속성-동기화의-복잡성)
- [문제점 3: 액션의 실행 역할 한계로 인한 애니메이션 몽타주 시각적 불일치](#문제점-3-액션의-실행-역할role-한계로-인한-애니메이션-몽타주-시각적-불일치)

---

## 문제점 1: 반복적인 네트워크 통신 코드

### 배경

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

### 해결 방법: 행동의 추상화 및 네트워크 통신 코드와의 의존성 제거

넥서스 액션 시스템은 이러한 문제를 해결하기 위해 **'액션(Action)'** 이라는 개념을 도입하여 게임 내 행동을 추상화하고, 네트워크 통신 과정을 프레임워크가 관리하도록 하였습니다.

**1. 행동의 추상화**

게임 내 모든 행동(스킬 사용, 아이템 획득 등)을 액션이라는 단위로 추상화하였습니다. 더 이상 기능마다 `UFUNCTION(Server, Reliable)` 같은 RPC 함수를 개별적으로 정의할 필요가 없습니다. 대신, 각 액션 클래스 안에 해당 기능의 핵심 게임 로직만 구현하고, 미리 시스템이 정의한 통신 방식을 열거형을 통해 지정할 수 있게 하였습니다.

<img width="764" height="420" alt="Image" src="https://github.com/user-attachments/assets/56d427dd-724e-41e0-a6c1-80b6700478f1" />


> 네트워크 동기화 과정을 추상화하여 열거형을 통해 고를 수 있게 함
> <img width="988" height="253" alt="Image" src="https://github.com/user-attachments/assets/703ba637-bd31-4630-91d3-7f59c7b4ccf7" />
> [소스 코드 보기](https://github.com/kijin-Kim/NewGunner/blob/825f993a8b092475c1b38158856e2e60aa98119a/Plugins/NexusAction/Source/NexusAction/Public/Action/NexusAction.h#L20-L27)

```cpp
UCLASS()
class GUNNER_API UAction_Fire : public UNexusAction
{
    GENERATED_BODY()
public:
    // 이 함수 안에 사격 행동의 핵심 로직만 구현합니다.
    // 네트워크 통신은 프레임워크가 따로 처리합니다.
    virtual void OnTriggerAction() override;
};
```

**2. 네트워크 통신 코드의 중앙화**

액션이 실행될 때, 넥서스 액션 시스템 프레임워크가 복잡한 네트워크 통신 과정을 직접 관리합니다. 클라이언트에서 액션 실행을 요청하면, 시스템은 이 액션이 지정한 통신 방식에 따라 서버/클라이언트와 통신합니다. 직접 RPC 함수를 호출하거나 데이터 복제를 위한 코드를 작성하지 않고, 넥서스 액션 시스템이 프레임워크 차원에서 총괄 관리합니다.

[NexusActionComponent.cpp L624–L670](https://github.com/kijin-Kim/NewGunner/blob/825f993a8b092475c1b38158856e2e60aa98119a/Plugins/NexusAction/Source/NexusAction/Private/Action/NexusActionComponent.cpp#L624-L670)

> [!NOTE]
> **시스템에서 제공하는 세 가지 실행 방식**
>
> **① 한 쪽에서만 실행 (`LocalOnly` / `ServerOnly`)**  
> 서버나 클라이언트 중 한 곳에서만 실행되며, 다른 쪽과 통신이 필요 없는 방식입니다.
>
> <img width="1440" height="294" alt="Image" src="https://github.com/user-attachments/assets/1e860f19-f652-4442-8142-b35d1202de29" />
>
> **② 클라이언트 예측 실행 (`LocalPredicted`)**  
> 클라이언트가 먼저 실행하고 서버에 알려주는 방식입니다.
>
> <img width="2403" height="645" alt="Image" src="https://github.com/user-attachments/assets/443016a6-ec83-4015-9955-31b41b3bc044" />
>
> **③ 서버 승인 실행 (`ServerAuthoritative`)**  
> 클라이언트가 서버에 실행 허가를 요청하고, 서버가 승인한 후에 실행되는 방식입니다.
>
> <img width="2886" height="645" alt="Image" src="https://github.com/user-attachments/assets/5e297927-d118-4694-8da6-f0b66f02c29f" />


---

## 문제점 2: '예측'이 야기하는 속성 동기화의 복잡성

### 배경

멀티플레이어 게임에서 플레이어에게 즉각적인 반응성을 제공하는 건 몰입도를 높이는 핵심입니다. 네트워크 지연은 피할 수 없으니, 플레이어가 스킬을 쓰거나 행동할 때 클라이언트에서 그 결과를 '예측'해 즉시 보여주는 것은 이러한 지연을 가려주고 부드러운 사용자 경험을 제공하는 데 필수적입니다.

하지만 언리얼 엔진이 속성값을 자동으로 복제(Replicate)해주더라도, '예측'을 실제로 도입하고 구현하는 순간 복제만으로는 해결할 수 없는 복잡하고 어려운 문제들이 발생합니다.

**① '덮어쓰기(Override)' 문제**

클라이언트가 속성 변화를 예측해 화면에 먼저 적용했을 때, 이후 서버로부터 확정된 속성 값이 도착하면 클라이언트의 임시 값을 서버의 권위 있는 값으로 덮어쓰는 방식을 어떻게 처리할지가 큰 난관입니다. 단순히 덮어쓰면 클라이언트가 예측한 중간 과정의 정보가 사라지거나, 다른 예측 로직과의 충돌로 잘못된 값이 적용될 위험이 있습니다.

예를 들어, 클라이언트는 연사 입력에 즉각 반응하기 위해 총알 수를 예측적으로 감소시킵니다 (30발 → 29발 → ... → 25발). 이 도중 서버로부터 총알이 29발이라는 복제 값이 도착하면, 예측된 25발을 무작정 29발로 덮어써야 합니다. 그 결과 총알이 갑자기 늘어나거나 서버와 불일치가 발생합니다.

<img width="1173" height="807" alt="Image" src="https://github.com/user-attachments/assets/3f85127c-39eb-464a-9455-829d63a44eba" />

> `#1` 클라이언트가 총알 개수를 30개 → 29개로 줄이고 서버에 알림  
> `#2` 클라이언트가 예측하여 25개까지 줄어든 상황에서, 서버로부터 이전에 보냈던 29개 정보가 도착  
> `#3` 클라이언트 총알 갯수가 덮어쓰여짐

**② '되돌릴 수 없는(Undoable)' 문제**

클라이언트가 예측적으로 총알을 감소시켰는데 서버가 이 사격 행위를 거절했을 경우, 클라이언트에서 줄어든 총알 수를 원래대로 되돌리는 로직이 필요합니다. 서버는 변수가 변경될 때만 클라이언트에게 알리므로 요청이 거절되어도 클라이언트는 이를 알 수 없습니다.

<img width="1188" height="807" alt="Image" src="https://github.com/user-attachments/assets/c3bbf783-0d29-4dc2-b0ee-650a7a8f2a93" />

> `#1` 클라이언트가 총알을 30개 → 29개로 줄이고 서버에 변경 요청  
> `#2` 서버가 이 요청을 거절하고 서버에서는 총알을 30개로 유지

또한 클라이언트가 여러 단계의 계산을 수행할 때 중간 과정을 저장하지 않으면 이전 상태로 되돌릴 수 없습니다. 예를 들어:

- 초기값: 30
- 예측 계산 ①: 30 - 2 = **28**
- 예측 계산 ②: 28 ÷ 2 = **14**

계산 ①이 서버에서 거절되어 되돌려야 할 경우, 클라이언트는 중간 과정을 저장하지 않았으므로 올바른 결과(15)를 얻지 못하고 `14 + 2 = 16`이라는 잘못된 결과를 냅니다.

### 해결 방법: SideEffect와 PredictionTag 개념 도입

넥서스 액션 시스템은 **SideEffect**와 **PredictionTag** 두 가지 개념을 도입해 위 문제를 해결하였습니다.

**1. SideEffect 개념 도입**

SideEffect는 체력 감소, 마나 회복 등 특정 속성(Nexus Property)에 대한 모든 종류의 변화를 정의하고 캡슐화한 데이터 단위입니다. SideEffect는 어떤 속성을, 얼마만큼, 어떤 방식으로 변화시킬지에 대한 모든 규칙을 포함합니다. 복잡한 수치 계산이나 네트워크 동기화 코드를 직접 작성하는 대신, 해당 규칙이 정의된 SideEffect를 '적용(Apply)'하기만 하면 됩니다. 이 SideEffect 자체는 서버에서 다시 적용되고 실행한 클라이언트로 복제됩니다.


**2. PredictionTag를 활용한 속성 변화의 프레임워크 관리**

PredictionTag는 클라이언트의 예측된 행동과 서버의 확정된 행동을 일대일로 연결하는 고유한 식별자입니다. 시스템은 이 태그를 활용하여 속성 변화의 모든 동기화 과정을 프레임워크 차원에서 관리합니다.

- **Step 1 — 클라이언트: PredictionTag 생성 및 예측적 SideEffect 적용**  
  플레이어가 액션을 실행하려고 하면, 클라이언트는 고유한 PredictionTag를 생성합니다. 클라이언트는 서버의 응답을 기다리지 않고 속성 변화를 즉시 예측 적용합니다. 이때 즉시 적용(Instant) SideEffect라 하더라도 클라이언트 측에서는 PredictionTag와 연결된 **무한 지속 SideEffect**로 취급하여 임시로 관리합니다. 동시에, PredictionTag와 함께 액션 실행 요청을 서버로 보냅니다.

- **Step 2 — 서버: 검증 및 확정 SideEffect 적용**  
  서버는 액션의 유효성을 검증하고, 유효하다고 판단되면 권위 있는 SideEffect를 적용합니다. 이 확정된 SideEffect 정보는 PredictionTag와 함께 해당 클라이언트로 복제됩니다.

- **Step 3 — 클라이언트: 예측 SideEffect 제거 및 속성 재계산**  
  클라이언트가 서버로부터 PredictionTag를 복제받으면, 해당 태그와 연결된 임시 무한 지속 SideEffect를 제거합니다. 이는 예측의 성공 여부와 관계없이 항상 수행됩니다.
  - **Override 문제 해결**: 서버로부터 받은 복제 값을 '최종 값'이 아닌 '기준 값'으로 취급하고, 여기에 클라이언트의 예측 델타를 재합산하여 최종 표시 값을 계산합니다. 서버 값으로 일방적으로 덮어쓰는 대신 최신 서버 값 위에 예측 변화를 재조정합니다.
  - **Undo 문제 해결**: 예측적 SideEffect를 항상 임시로 적용하고, 성공 여부와 상관없이 항상 제거하므로 자연스럽게 되돌리기가 보장됩니다.
  - **재적용 방지 (Redo Prevention)**: 서버에서 복제된 권위 있는 SideEffect가 적용되기 전에 예측 임시 SideEffect를 먼저 제거하여, 동일한 SideEffect가 두 번 적용되는 것을 방지합니다.

---

## 문제점 3: 액션의 실행 역할(Role) 한계로 인한 애니메이션 몽타주 시각적 불일치

### 배경

멀티플레이어 게임에서 캐릭터 애니메이션은 모든 클라이언트에게 동일하게 보여야 합니다. 그러나 액션은 실행되는 주체의 네트워크 역할(ROLE)에 따라 로직이 실행되는 범위가 제한됩니다.

예를 들어 `LocalPredicted` 액션은 `ROLE_AutonomousProxy`(자신의 클라이언트)와 서버에서만 실행됩니다. 따라서 액션 내부에서 몽타주를 직접 재생하면, `ROLE_SimulatedProxy`(다른 클라이언트)에서는 해당 애니메이션이 재생되지 않습니다.

이 문제를 해결하기 위해 매번 개별적인 멀티캐스트 RPC를 작성하는 것은 비효율적이며 개발 부담을 크게 가중시킵니다.

### 해결: 서버 권위 기반의 애니메이션 몽타주 프레임워크 관리

넥서스 액션 시스템은 `NexusAnimMontagePlayerComponent`를 통해 이 문제를 해결합니다. 이 컴포넌트는 서버의 정확한 몽타주 재생 상태를 클라이언트에 복제하고, 클라이언트가 이를 기준으로 자신의 애니메이션을 지속적으로 보정하여 모든 플레이어에게 일관된 시각적 경험을 제공합니다.

**1. 서버의 몽타주 상태 업데이트 및 복제**

서버는 캐릭터가 몽타주를 재생하면, 해당 몽타주 정보(애셋, 재생 속도, 현재 재생 위치, 섹션 이름, 재생 상태 등)를 매 프레임 `FNexusAnimMontageRepData` 구조체에 업데이트합니다. 이 구조체는 언리얼 엔진의 복제 시스템을 통해 모든 `ROLE_SimulatedProxy` 클라이언트에 전달됩니다.

- [NexusAnimMontagePlayerComponent.h L73–L74](https://github.com/kijin-Kim/NewGunner/blob/825f993a8b092475c1b38158856e2e60aa98119a/Plugins/NexusAction/Source/NexusAction/Public/Animation/NexusAnimMontagePlayerComponent.h#L73-L74)
- [NexusAnimMontagePlayerComponent.cpp L84–L96](https://github.com/kijin-Kim/NewGunner/blob/825f993a8b092475c1b38158856e2e60aa98119a/Plugins/NexusAction/Source/NexusAction/Private/Animation/NexusAnimMontagePlayerComponent.cpp#L84-L96)

**2. 클라이언트의 몽타주 상태 수신 및 보정**

클라이언트는 `ReplicatedAnimMontageData`가 복제되면 `OnRep_ReplicatedAnimMontage` 함수를 통해 서버의 몽타주 데이터와 로컬 재생 상태(`FNexusLocalAnimMontageData`)를 비교합니다. 다음 조건 중 하나라도 해당되면 즉시 보정합니다:

- 몽타주 애셋(`AnimMontage`) 또는 `AnimMontageInstanceID`가 다른 경우
- 서버는 정지/일시정지 상태인데 클라이언트는 재생 중인 경우
- 재생 속도 또는 섹션 이름이 다른 경우
- 재생 위치(`Position`) 오차가 `MONTAGE_POSITION_DELTA_TOLERANCE` (0.3f) 이상인 경우

보정 작업에는 몽타주 재시작, 중지, 특정 위치/속도/섹션으로 점프 등이 포함됩니다.

[NexusAnimMontagePlayerComponent.cpp L119–L169](https://github.com/kijin-Kim/NewGunner/blob/825f993a8b092475c1b38158856e2e60aa98119a/Plugins/NexusAction/Source/NexusAction/Private/Animation/NexusAnimMontagePlayerComponent.cpp#L119-L169)

**3. `AnimMontageInstanceID`를 활용한 정밀한 동기화**

동일한 몽타주가 짧은 시간 내에 반복 재생되는 시나리오(예: 빠른 연타 공격)에서는 몽타주 애셋만으로 각 재생 시도를 구분하기 어렵습니다. `FNexusAnimMontageRepData` 구조체에는 `uint8` 타입의 `AnimMontageInstanceID`가 포함되어 함께 복제됩니다. 서버는 `PlayMontage` 호출 시마다 이 ID를 증가시키고, 클라이언트는 이를 비교하여 정확히 어느 재생 인스턴스에 해당하는지 파악해 오류 없는 동기화를 수행합니다.

[NexusAnimMontagePlayerComponent.cpp L37–L51](https://github.com/kijin-Kim/NewGunner/blob/825f993a8b092475c1b38158856e2e60aa98119a/Plugins/NexusAction/Source/NexusAction/Private/Animation/NexusAnimMontagePlayerComponent.cpp#L37-L51)

### 결과

> 모든 클라이언트에 애니메이션 몽타주가 잘 출력되는 모습

[https://github.com/user-attachments/assets/YOUR_VIDEO_ASSET_ID
](https://github.com/user-attachments/assets/61cff391-160a-42ba-b0cc-4ead2a76db5c
)
<!-- 영상은 GitHub 이슈 텍스트창에 mp4를 드래그앤드롭하여 생성된 URL로 교체하세요. -->
<!-- 예시: <video src="https://github.com/user-attachments/assets/xxxx" controls width="700"></video> -->
