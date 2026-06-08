# ProjectRuntimeConfig

## 문서 목적
이 문서는 현재 프로젝트에서 `ProjectRuntimeConfig` 기능이 실제로 어떤 일을 하는지, 그리고 그 기능이 어떤 설정 파일 구성으로 동작하는지를 기록한다.
이 문서는 미래 설계나 개선 계획이 아니라, **현재 확인된 설정 상태**를 기준으로 작성한다.

## 문서 범위
이 문서에서 말하는 `ProjectRuntimeConfig` 기능은 아래 요소를 묶어서 본다.

- 핵심 설정 파일:
  - `UE/Config/DefaultEngine.ini`
  - `UE/Config/DefaultInput.ini`
  - `UE/Config/DefaultGame.ini`
- 보조 설정 파일:
  - `UE/Config/DefaultEditor.ini`
- 현재 영향 범위:
  - 프로젝트 기본 시작 맵
  - 렌더링 파이프라인
  - 기본 RHI / 셰이더 타깃
  - 입력 백엔드 기본 클래스
  - 기본 축 민감도 / DeadZone
  - 일부 공통 UI 입력 처리

즉, 현재 기준 `ProjectRuntimeConfig`는 **프로젝트가 어떤 맵으로 시작하고, 어떤 렌더링 환경 위에서 실행되며, 어떤 입력 백엔드를 기본으로 사용하는지를 결정하는 런타임 환경 설정 기능**으로 본다.

## 이 기능이 현재 실제로 하는 일
현재 구현 기준 `ProjectRuntimeConfig`의 핵심 역할은 **프로젝트가 어떤 맵으로 부팅되는지, 어떤 그래픽 파이프라인과 하드웨어 타깃을 사용하는지, 어떤 입력 시스템 클래스를 기본으로 쓰는지, 그리고 입력 축이 어떤 기본 감도/DeadZone으로 처리되는지를 ini 설정으로 고정하는 것**이다.

이 기능은 단순히 ini 값을 저장해두는 것이 아니다.
현재 프로젝트에서 이 설정 묶음은 아래 다섯 가지 역할을 한다.

### 1. 프로젝트 시작 맵을 결정한다
현재 `DefaultEngine.ini`의 `[/Script/EngineSettings.GameMapsSettings]` 기준 시작 맵은 아래와 같다.

- `GameDefaultMap=/Game/Maps/TestMap.TestMap`
- `EditorStartupMap=/Game/Maps/TestMap.TestMap`

현재 의미:
- 에디터에서 프로젝트를 열면 기본적으로 `TestMap`으로 시작한다.
- 게임 기본 시작 맵도 현재 `TestMap`이다.

즉 현재 `ProjectRuntimeConfig`는 **프로젝트의 기본 진입 맵을 `TestMap`으로 고정하는 부팅 설정 기능**을 가진다.

다만 현재 `TestMap` 자체는 특별한 기능 검증 환경이 아니라, 사실상 기본 평지 맵에 가깝다.
따라서 이 설정은 현재 **임시 검증 진입점 설정**의 성격이 강하다.

### 2. 프로젝트의 기본 렌더링 파이프라인을 결정한다
현재 `DefaultEngine.ini`의 `[/Script/Engine.RendererSettings]` 기준, 프로젝트는 아래 렌더링 성향을 사용한다.

현재 핵심 값:
- `r.AllowStaticLighting=False`
- `r.GenerateMeshDistanceFields=True`
- `r.DynamicGlobalIlluminationMethod=1`
- `r.ReflectionMethod=1`
- `r.SkinCache.CompileShaders=True`
- `r.RayTracing=True`
- `r.RayTracing.RayTracingProxies.ProjectEnabled=True`
- `r.Substrate=True`
- `r.Substrate.ProjectGBufferFormat=0`
- `r.Shadow.Virtual.Enable=1`
- `r.DefaultFeature.AutoExposure.ExtendDefaultLuminanceRange=True`
- `r.DefaultFeature.LocalExposure.HighlightContrastScale=0.8`
- `r.DefaultFeature.LocalExposure.ShadowContrastScale=0.8`

현재 의미를 정리하면 아래와 같다.

#### A. 정적 라이팅 비활성
- `r.AllowStaticLighting=False`

즉 현재 프로젝트는 **전통적인 정적 라이트맵 기반 파이프라인보다 동적 라이팅 중심 설정**을 택하고 있다.

#### B. Lumen 계열 GI / Reflection 사용
- `r.DynamicGlobalIlluminationMethod=1`
- `r.ReflectionMethod=1`

현재 프로젝트는 **Lumen GI / Lumen Reflection 기반 현대 렌더링 파이프라인**을 전제로 한다.

#### C. Ray Tracing 프로젝트 활성
- `r.RayTracing=True`
- `r.RayTracing.RayTracingProxies.ProjectEnabled=True`

즉 현재 설정은 **DX12 + RayTracing 가능 환경**을 전제로 한 고사양 렌더링 설정이다.

#### D. Substrate 사용
- `r.Substrate=True`

즉 현재 머티리얼 파이프라인도 **Substrate 사용 프로젝트**로 설정돼 있다.

#### E. Virtual Shadow Map 사용
- `r.Shadow.Virtual.Enable=1`

즉 그림자 시스템도 **Virtual Shadow Map 활성 상태**다.

따라서 현재 `ProjectRuntimeConfig`는 단순 그래픽 옵션 모음이 아니라,
**프로젝트 전체가 어떤 렌더링 기술 스택 위에 서 있는지를 결정하는 렌더링 환경 기능**이다.

### 3. 기본 플랫폼/RHI/셰이더 타깃을 결정한다
현재 `DefaultEngine.ini`의 플랫폼 설정 기준, 프로젝트는 아래 하드웨어 타깃 성향을 가진다.

#### Windows
- `DefaultGraphicsRHI=DefaultGraphicsRHI_DX12`
- `+D3D12TargetedShaderFormats=PCD3D_SM6`
- `+D3D11TargetedShaderFormats=PCD3D_SM5`

현재 의미:
- 기본 RHI는 DX12다.
- 현재 주력 셰이더 타깃은 SM6다.
- D3D11 SM5 흔적은 있지만 기본 방향은 DX12/SM6이다.

#### Linux / Mac
- Linux: `+TargetedRHIs=SF_VULKAN_SM6`
- Mac: `+TargetedRHIs=SF_METAL_SM6`

즉 현재 설정상 멀티플랫폼 타깃 값은 존재하지만,
프로젝트의 실질 기본 방향은 **Desktop / Maximum / 고사양 셰이더 타깃**이다.

#### Hardware Targeting
- `TargetedHardwareClass=Desktop`
- `AppliedTargetedHardwareClass=Desktop`
- `DefaultGraphicsPerformance=Maximum`
- `AppliedDefaultGraphicsPerformance=Maximum`

즉 현재 프로젝트는 **데스크탑 고성능 하드웨어 전제**로 설정돼 있다.

따라서 현재 `ProjectRuntimeConfig`는 **프로젝트의 기본 하드웨어 성격과 셰이더/RHI 기준을 고정하는 플랫폼 런타임 설정 기능**도 가진다.

### 4. 입력 백엔드와 축 기본값을 결정한다
현재 `DefaultInput.ini`는 InputAction 매핑 자체보다는,
**입력 백엔드와 축 입력의 기본 처리 특성**을 설정하는 역할을 한다.

현재 핵심 값:
- `DefaultPlayerInputClass=/Script/EnhancedInput.EnhancedPlayerInput`
- `DefaultInputComponentClass=/Script/EnhancedInput.EnhancedInputComponent`
- `bEnableLegacyInputScales=True`
- `bEnableMotionControls=True`
- `bEnableMouseSmoothing=True`
- `bEnableFOVScaling=True`
- `DefaultViewportMouseCaptureMode=CapturePermanently_IncludingInitialMouseDown`
- `DefaultViewportMouseLockMode=LockOnCapture`

즉 현재 프로젝트는 **Enhanced Input를 기본 입력 백엔드**로 사용한다.
이건 이미 `Input.md`에서 본 `IMC_Vehicle_Default`와도 연결된다.

#### 현재 축 기본 설정
대표 예시:
- `Gamepad_LeftX/LeftY/RightX/RightY` DeadZone = `0.25`
- `MouseX/MouseY/Mouse2D` Sensitivity = `0.07`
- `MouseWheelAxis` Sensitivity = `1.0`
- `Gamepad_LeftTriggerAxis`, `Gamepad_RightTriggerAxis` Sensitivity = `1.0`

현재 의미:
- 게임패드 스틱은 기본 DeadZone 0.25로 운영된다.
- 마우스 축은 0.07 감도로 운영된다.

#### XR/VR 축 기본값도 등록돼 있음
현재 `DefaultInput.ini`에는 아래 계열 축 설정도 다수 존재한다.
- Vive
- MixedReality
- OculusTouch
- ValveIndex

즉 현재 프로젝트는 기본 입력 설정 수준에서 이미 **XR/VR 컨트롤러 축까지 허용하는 입력 환경**을 깔고 있다.

중요한 점은,
현재 실제 차량 입력 기능은 `Input.md`에서 정리한 대로 `Enhanced Input + IMC_Vehicle_Default`가 중심이지만,
`DefaultInput.ini`는 그보다 더 아래층에서 **축 기본 민감도와 백엔드 클래스를 정하는 기반 설정**이라는 점이다.

### 5. 일부 공통 UI/프로젝트 일반 설정을 결정한다
현재 `DefaultGame.ini`에는 두 가지 정도가 눈에 띈다.

#### CommonUI Accept Key 처리
- `[/Script/CommonUI.CommonUISettings]`
- `CommonButtonAcceptKeyHandling=TriggerClick`

즉 현재 CommonUI 사용 시 Accept 입력은 **TriggerClick 방식**으로 처리되도록 설정돼 있다.

#### 일반 프로젝트 식별값
- `[/Script/EngineSettings.GeneralProjectSettings]`
- `ProjectID=...`

이건 동작 기능보다는 프로젝트 식별 정보에 가깝다.

## 현재 기준 기능의 성격 정리
현재 구현을 종합하면 `ProjectRuntimeConfig`는 아래 역할을 가진다.

1. **프로젝트 부팅 설정 기능**
   - 기본 게임 시작 맵과 에디터 시작 맵을 고정함

2. **렌더링 환경 설정 기능**
   - Lumen / RayTracing / Substrate / VSM 기반 렌더링 방향을 고정함

3. **플랫폼/RHI 타깃 설정 기능**
   - Desktop / Maximum / DX12 / SM6 중심 설정을 고정함

4. **입력 백엔드 설정 기능**
   - Enhanced Input 기본 클래스와 축 기본 처리값을 고정함

5. **공통 UI/기반 시스템 설정 기능**
   - CommonUI 기본 Accept 동작 같은 일부 공통 설정을 고정함

따라서 현재 이 기능은 단순한 `Config 파일 모음`이 아니라,
**프로젝트가 어떤 환경에서 시작되고, 어떤 그래픽/입력 스택 위에서 동작하는지를 결정하는 현재 런타임 환경 기능**이라고 보는 것이 맞다.

## 현재 동작 방식
현재 `ProjectRuntimeConfig`는 아래 방식으로 동작한다.

### 1. 엔진 부팅 시 ini 설정이 기본값으로 로드된다
현재 이 기능은 별도 C++ 런타임 함수가 아니라,
엔진이 각 `Default*.ini`를 읽어 기본 프로젝트 설정으로 반영하는 구조다.

즉 현재 이 기능은 **런타임 중 매 프레임 계산되는 기능이 아니라, 프로젝트 실행 환경을 먼저 결정하는 초기 환경 설정 기능**이다.

### 2. 입력 기능과의 연결 방식
`Input.md`에서 정리한 입력 기능은 실제로 `IMC_Vehicle_Default`와 `ACFVehiclePawn` 바인딩을 통해 동작한다.
하지만 그 아래에는 현재 `ProjectRuntimeConfig`가 제공하는 값이 깔려 있다.

예:
- EnhancedInput 기본 클래스 지정
- 마우스/게임패드/XR 축 민감도와 DeadZone
- 모션 컨트롤 사용 허용

즉 현재 `Input` 기능은 독립이 아니라,
**`ProjectRuntimeConfig`가 제공하는 입력 백엔드 기본 설정 위에서 동작**한다.

### 3. 카메라/렌더링 기능과의 연결 방식
`VehicleCamera` 문서에서 본 카메라 기능은 현재 SpringArm/FOV/AimTrace를 계산하지만,
실제 화면 표현 품질과 충돌 시 시야 인상은 아래 프로젝트 환경 설정의 영향을 받는다.

예:
- Lumen
- RayTracing
- Virtual Shadow Map
- Auto Exposure / Local Exposure
- DX12 / SM6

즉 현재 `ProjectRuntimeConfig`는 **개별 차량 기능을 직접 계산하지 않지만, 그 기능이 어떤 화면 품질과 플랫폼 스택 위에서 보이게 되는지를 정하는 상위 환경 기능**이다.

## 현재 표시 조건 / 실행 조건
현재 `ProjectRuntimeConfig`가 의미를 가지는 조건은 아래와 같다.

- 프로젝트가 현재 `UE/Config/Default*.ini`를 기준 설정으로 부팅할 것
- 런타임/에디터가 해당 ini를 정상 로드할 것
- 플랫폼이 현재 지정된 RHI/셰이더 타깃을 지원할 것

특히 현재 렌더링 설정은:
- DX12
- SM6
- Lumen
- RayTracing
- Substrate

중심이기 때문에,
하드웨어나 드라이버가 이 조합을 지원하지 못하면 실제 런타임 체감이 달라질 수 있다.

즉 현재 `ProjectRuntimeConfig`는 **설정상 기준 환경을 정의하는 기능**이지,
모든 환경에서 동일 결과를 보장하는 추상 기능은 아니다.

## 현재 자산 / 클래스 역할
### `DefaultEngine.ini`
- 종류: 프로젝트 엔진 설정 파일
- 현재 역할: 기본 맵, 렌더러, RHI, 하드웨어 타깃, UI/엔진 리다이렉트 설정의 중심 파일

### `DefaultInput.ini`
- 종류: 프로젝트 입력 설정 파일
- 현재 역할: Enhanced Input 기본 클래스와 축 기본값 설정의 중심 파일

### `DefaultGame.ini`
- 종류: 프로젝트 게임 설정 파일
- 현재 역할: CommonUI 처리와 일반 프로젝트 설정 보조 파일

### `DefaultEditor.ini`
- 종류: 에디터 전용 설정 파일
- 현재 역할: 에디터 프리뷰/뷰포트 관련 설정 중심
- 현재 문서 기준 해석: 런타임 핵심 기능보다는 에디터 표시 보조 설정에 가깝다.

## 현재 생성 및 연결 구조
현재 설정 연결 구조는 아래와 같다.

1. 프로젝트 실행/에디터 시작
2. `DefaultEngine.ini`, `DefaultInput.ini`, `DefaultGame.ini` 기본값 로드
3. 엔진이 기본 맵 / 렌더러 / RHI / 입력 클래스 / 축 설정을 적용
4. 이후 차량 런타임 기능(`Input`, `VehicleCamera`, `VehicleDrive` 등)이 그 환경 위에서 동작

현재 구조 해석:
- `ProjectRuntimeConfig`는 개별 차량 기능보다 상위 계층의 환경 기능이다.
- 직접 값을 계산하지는 않지만, 다른 기능들이 어떤 기본 환경 위에서 실행되는지 정한다.

## 현재 기능 책임
현재 구현 기준에서 `ProjectRuntimeConfig`의 책임은 아래와 같다.

- 프로젝트 기본 시작 맵을 지정한다.
- 렌더링 기술 스택의 기본 방향을 지정한다.
- 플랫폼/RHI/셰이더 타깃의 기본 방향을 지정한다.
- Enhanced Input 기본 클래스를 지정한다.
- 기본 축 민감도와 DeadZone을 지정한다.
- 모션 컨트롤/XR 축 기본 허용 환경을 제공한다.
- 일부 CommonUI 기본 입력 처리 방식을 지정한다.

## 현재 기준 비책임 항목
현재 구현상 `ProjectRuntimeConfig`의 직접 책임으로 보지 않는 항목은 아래와 같다.

- 실제 InputAction 매핑 구성 자체
  - 현재 액션/키 매핑은 `Input` 기능 책임
- 차량 주행 상태 계산 자체
  - `VehicleDrive` 책임
- 휠 시각 동기화 계산 자체
  - `WheelSync` 책임
- 카메라 자유 조준 계산 자체
  - `VehicleCamera` 책임
- 테스트맵의 검증 구조 자체
  - 현재 `TestMap`은 환경 진입점일 뿐, 별도 기능 검증 환경 책임은 아직 약함

즉 현재 `ProjectRuntimeConfig`는 계산기라기보다,
**프로젝트 전체 런타임 환경을 고정하는 기반 설정 기능**에 가깝다.

## 현재 문서 기준의 핵심 결론
현재 `ProjectRuntimeConfig` 기능은,

**프로젝트가 어떤 맵으로 시작하고, 어떤 렌더링 기술과 하드웨어 타깃 위에서 실행되며, 어떤 입력 백엔드와 축 기본값을 사용하는지를 ini 설정으로 고정하는 현재 런타임 환경 기능**이다.

이 문서에서 가장 중요하게 봐야 할 현재 역할은 다음 한 줄로 요약할 수 있다.

> `ProjectRuntimeConfig`는 현재 프로젝트의 차량 기능들이 어떤 시작 맵, 어떤 그래픽 스택, 어떤 입력 기반 위에서 실행되는지를 결정하는 현재 상태 기능이다.

## 현재 문서에서 미확인인 항목
아래는 아직 이 문서에서 확정하지 않은 내용이다.

- AndroidFileServer 설정이 현재 실제 개발 루프에서 사용되는지 여부
- Linux/Mac 타깃 설정이 현재 실제 빌드 파이프라인에서 사용되는지 여부
- `DefaultEditor.ini`의 대형 프리뷰 프로필 설정이 현재 팀 작업에서 어느 정도 중요한지 여부
- 추가 플랫폼별/사용자별 override ini가 실제로 별도 운용 중인지 여부

## 문서 갱신 조건
아래 변경이 생기면 이 문서를 함께 갱신한다.

- `GameDefaultMap` / `EditorStartupMap` 변경
- 렌더링 파이프라인 핵심 값(Lumen, RayTracing, Substrate, VSM) 변경
- 기본 RHI / 셰이더 타깃 변경
- `DefaultPlayerInputClass` / `DefaultInputComponentClass` 변경
- 주요 축 민감도 / DeadZone 정책 변경
- CommonUI 기본 처리 방식 변경

## 문서 버전 관리
- 현재 문서 버전: `1.0.0`
- 문서 상태: `Initial`
- 관리 원칙:
  - 이 문서는 한 번 작성하고 끝내는 문서가 아니라, 기능의 현재 상태가 바뀌면 함께 갱신한다.
  - 기능 설명 본문이 바뀌면 체인지로그도 같이 갱신한다.
  - 구현 변경 없이 표현만 다듬은 경우와, 기능 이해에 영향을 주는 내용 변경을 구분해서 기록한다.

### 버전 증가 기준
- `Major`
  - 기능 해석 자체가 바뀌는 수준의 대규모 재작성
  - 문서 범위가 다른 기능 묶음까지 확장되거나 재정의될 때
- `Minor`
  - 현재 기능 설명에 중요한 항목이 추가될 때
  - 새로운 표시 항목, 동작 조건, 연결 구조가 확인되어 본문 의미가 확장될 때
- `Patch`
  - 오탈자 수정
  - 표현 명확화
  - 근거 보강
  - 본문 의미는 유지한 채 설명 정밀도만 올라갈 때

## 체인지로그
### v1.0.0 - 2026-04-23
- `ProjectRuntimeConfig` 문서 최초 작성
- `DefaultEngine.ini`, `DefaultInput.ini`, `DefaultGame.ini` 기준으로 현재 런타임 환경 설정 정리
- 기본 맵, 렌더링 파이프라인, RHI/셰이더 타깃, 입력 백엔드와 축 기본값 중심으로 본문 작성
- `DefaultEditor.ini`는 현재 런타임 핵심보다 에디터 보조 설정 성격이 강함을 구분해 기록

## 마지막 확인 기준
- 확인 일시: 2026-04-23
- 확인 근거:
  - `UE/Config/DefaultEngine.ini`
  - `UE/Config/DefaultInput.ini`
  - `UE/Config/DefaultGame.ini`
  - `UE/Config/DefaultEditor.ini`
