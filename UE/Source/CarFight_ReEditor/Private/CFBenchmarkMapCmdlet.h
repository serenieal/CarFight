// Copyright (c) CarFight. All Rights Reserved.
//
// File: CFBenchmarkMapCmdlet.h
// Version: v1.0.0
// Date: 2026-09-01
// Description: CarFight 전용 주행 성능 테스트 맵 M_VehicleBenchmark의 최소 편집 가능한 골격을 생성하는 Editor-only one-shot commandlet입니다.
// Changelog:
// - v1.0.0: 빈 월드, 500m x 40m TestRoad, VehicleBenchmarkStart PlayerStart, 500m EndPreview TargetPoint, 기본 Directional/Sky Light를 생성하고 exact map package로 저장.
// Migration:
// - 기존 M_VehicleBenchmark가 존재하면 덮어쓰지 않고 fail-closed합니다.
// - Vehicle Builder benchmark source는 이 commandlet이 자동 변경하지 않습니다. USER가 맵 길이를 확정한 뒤 별도 전환합니다.

#pragma once

#include "CoreMinimal.h"
#include "Commandlets/Commandlet.h"
#include "CFBenchmarkMapCmdlet.generated.h"

/** 전용 Vehicle Benchmark 맵의 최소 편집 골격을 생성합니다. */
UCLASS()
class CARFIGHT_REEDITOR_API UCFBenchmarkMapCommandlet : public UCommandlet
{
	GENERATED_BODY()

public:
	// Headless Editor commandlet 실행 속성을 준비합니다.
	UCFBenchmarkMapCommandlet();

	// 새 M_VehicleBenchmark를 생성하고 기존 Asset이 없을 때만 저장합니다.
	virtual int32 Main(const FString& Params) override;
};
