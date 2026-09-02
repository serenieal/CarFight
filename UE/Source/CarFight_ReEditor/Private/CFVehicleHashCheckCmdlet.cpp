// Copyright (c) CarFight. All Rights Reserved.
//
// File: CFVehicleHashCheckCmdlet.cpp
// Version: v1.0.0
// Date: 2026-09-02
// Description: FCFVehicleSnapshotBuilder authority를 사용한 saved VehicleData DefinitionHash read-only preflight 구현입니다.
// Changelog:
// - v1.0.0: exact object path load, expected MD5-format validation, current DefinitionHash build, exact mismatch fail-closed 및 bounded PASS log를 추가.
// Migration:
// - Asset package dirty/save를 호출하지 않습니다.
// - hash algorithm을 복제하지 않고 existing Data Authoring Snapshot authority를 직접 재사용합니다.

#include "CFVehicleHashCheckCmdlet.h"

#include "CFVehicleData.h"
#include "DataAuthoring/CFVehicleSnapshotBuilder.h"
#include "DataAuthoring/CFVehicleSnapshotTypes.h"
#include "Misc/Char.h"
#include "Misc/Parse.h"

DEFINE_LOG_CATEGORY_STATIC(LogCFVehicleHashCheck, Log, All);

namespace CFVehicleHashCheck
{
	// Process exit code taxonomy입니다.
	enum class EExitCode : int32
	{
		Success = 0,
		ArgumentInvalid = 121,
		VehicleDataLoadFailed = 122,
		SnapshotFailed = 123,
		HashMismatch = 124
	};

	// Snapshot DefinitionHash current format인 32자리 hex 문자열인지 검사합니다.
	bool IsDefinitionHashFormatValid(const FString& Hash)
	{
		if (Hash.Len() != 32)
		{
			return false;
		}

		for (const TCHAR Character : Hash)
		{
			if (!FChar::IsHexDigit(Character))
			{
				return false;
			}
		}
		return true;
	}
}

// Headless Editor read-only commandlet 실행 속성을 준비합니다.
UCFVehicleHashCheckCommandlet::UCFVehicleHashCheckCommandlet()
{
	IsClient = false;
	IsServer = false;
	IsEditor = true;
	LogToConsole = true;
	ShowErrorCount = true;
}

// VehicleData를 load하고 current DefinitionHash를 계산해 expected hash와 exact 비교합니다.
int32 UCFVehicleHashCheckCommandlet::Main(const FString& Params)
{
	using namespace CFVehicleHashCheck;

	// Caller가 검증하려는 saved UCFVehicleData exact object path입니다.
	FString VehicleDataPath;
	if (!FParse::Value(*Params, TEXT("CFVehicleData="), VehicleDataPath) || VehicleDataPath.IsEmpty())
	{
		UE_LOG(LogCFVehicleHashCheck, Error, TEXT("CF_VEHICLE_HASH_CHECK_ARGUMENT_FAIL reason=MissingVehicleData"));
		return static_cast<int32>(EExitCode::ArgumentInvalid);
	}

	// Caller가 이미 승인/기록한 expected semantic DefinitionHash입니다.
	FString ExpectedDefinitionHash;
	if (!FParse::Value(*Params, TEXT("CFExpectedDefinitionHash="), ExpectedDefinitionHash)
		|| !IsDefinitionHashFormatValid(ExpectedDefinitionHash))
	{
		UE_LOG(
			LogCFVehicleHashCheck,
			Error,
			TEXT("CF_VEHICLE_HASH_CHECK_ARGUMENT_FAIL reason=InvalidExpectedHash value=%s"),
			*ExpectedDefinitionHash);
		return static_cast<int32>(EExitCode::ArgumentInvalid);
	}

	// Disk에서 읽은 current saved VehicleData입니다.
	UCFVehicleData* VehicleData = LoadObject<UCFVehicleData>(nullptr, *VehicleDataPath);
	if (!VehicleData)
	{
		UE_LOG(
			LogCFVehicleHashCheck,
			Error,
			TEXT("CF_VEHICLE_HASH_CHECK_LOAD_FAIL path=%s"),
			*VehicleDataPath);
		return static_cast<int32>(EExitCode::VehicleDataLoadFailed);
	}

	// Existing Builder/Data Authoring 공용 Definition semantic snapshot입니다.
	FCFVehicleDefinitionSnapshot DefinitionSnapshot;
	// Snapshot build failure의 bounded 원인입니다.
	FString SnapshotError;
	if (!FCFVehicleSnapshotBuilder::BuildDefinitionSnapshot(*VehicleData, DefinitionSnapshot, SnapshotError))
	{
		UE_LOG(
			LogCFVehicleHashCheck,
			Error,
			TEXT("CF_VEHICLE_HASH_CHECK_SNAPSHOT_FAIL path=%s reason=%s"),
			*VehicleDataPath,
			*SnapshotError);
		return static_cast<int32>(EExitCode::SnapshotFailed);
	}

	// Current saved Target이 caller expected identity와 정확히 다르면 benchmark/evidence 실행을 차단합니다.
	if (DefinitionSnapshot.DefinitionHash != ExpectedDefinitionHash)
	{
		UE_LOG(
			LogCFVehicleHashCheck,
			Error,
			TEXT("CF_VEHICLE_HASH_CHECK_MISMATCH path=%s expected=%s actual=%s"),
			*VehicleDataPath,
			*ExpectedDefinitionHash,
			*DefinitionSnapshot.DefinitionHash);
		return static_cast<int32>(EExitCode::HashMismatch);
	}

	UE_LOG(
		LogCFVehicleHashCheck,
		Display,
		TEXT("CF_VEHICLE_HASH_CHECK_PASS path=%s definition_hash=%s field_count=%d"),
		*VehicleDataPath,
		*DefinitionSnapshot.DefinitionHash,
		DefinitionSnapshot.SortedFields.Num());

	return static_cast<int32>(EExitCode::Success);
}
