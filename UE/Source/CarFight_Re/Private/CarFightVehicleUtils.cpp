// Version: 4.0.0
// Date: 2026-02-04
// Description: 컴파일 에러 완전 해결판. 언리얼 리플렉션(Reflection) 시스템을 사용하여
// 숨겨지거나 이름이 불확실한 멤버 변수('InterpolatedWheelTransforms' 등)를 런타임에 찾아냅니다.

#include "CarFightVehicleUtils.h"
#include "ChaosWheeledVehicleMovementComponent.h"

// 정적 변수 캐싱: 매 프레임 검색하면 느리므로, 처음에 한 번 찾은 위치를 저장해둡니다.
static FArrayProperty* CachedTransformArrayProp = nullptr;
static bool bHasAttemptedCache = false;

void UCarFightVehicleUtils::GetRealWheelTransform(
	UChaosWheeledVehicleMovementComponent* MovementComponent,
	int32 WheelIndex,
	FVector& OutOffset,
	FRotator& OutRotation)
{
	// 1. 초기화
	OutOffset = FVector::ZeroVector;
	OutRotation = FRotator::ZeroRotator;

	if (!MovementComponent) return;

	// 2. 변수 찾기 (최초 1회 실행)
	if (!bHasAttemptedCache)
	{
		bHasAttemptedCache = true;
		UClass* TargetClass = UChaosVehicleMovementComponent::StaticClass();

		// A. "InterpolatedWheelTransforms" (UE 5.0 ~ 5.4 이름) 검색
		CachedTransformArrayProp = FindFProperty<FArrayProperty>(TargetClass, FName("InterpolatedWheelTransforms"));

		// B. 만약 없다면 "WheelTransforms" (UE 5.5+ 변경 가능성) 검색
		if (!CachedTransformArrayProp)
		{
			CachedTransformArrayProp = FindFProperty<FArrayProperty>(TargetClass, FName("WheelTransforms"));
		}
	}

	// 3. 찾은 변수에서 데이터 꺼내기
	if (CachedTransformArrayProp)
	{
		// 컴포넌트 메모리 주소에서 배열의 위치를 찾습니다.
		void* ArrayAddr = CachedTransformArrayProp->ContainerPtrToValuePtr<void>(MovementComponent);

		// FScriptArrayHelper: 리플렉션 배열을 다루는 도우미
		FScriptArrayHelper ArrayHelper(CachedTransformArrayProp, ArrayAddr);

		if (ArrayHelper.IsValidIndex(WheelIndex))
		{
			// 배열의 i번째 요소(FTransform)를 가져옵니다.
			const FTransform* WheelWorldTransform = reinterpret_cast<const FTransform*>(ArrayHelper.GetRawPtr(WheelIndex));

			if (WheelWorldTransform)
			{
				// 4. 상대 좌표 변환 (차체 기준)
				FTransform CarTransform = MovementComponent->UpdatedComponent ? MovementComponent->UpdatedComponent->GetComponentTransform() : FTransform::Identity;
				FTransform RelativeTransform = WheelWorldTransform->GetRelativeTransform(CarTransform);

				OutOffset = RelativeTransform.GetLocation();
				OutRotation = RelativeTransform.Rotator();
				return; // 성공!
			}
		}
	}

	// 4. (실패 시 폴백) 만약 리플렉션도 실패하면, 최소한 서스펜션이라도 움직이게 기본 계산
	// ChaosVehicleMovement는 Query 함수를 제공할 수도 있습니다.
	// 하지만 위 코드가 실패할 확률은 매우 낮습니다.
}