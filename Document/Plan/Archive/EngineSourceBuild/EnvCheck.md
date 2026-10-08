# Source Build 환경 점검

문서 버전: v0.1
작성 기준일: 2026-05-22
대상 프로젝트: CarFight

이 문서는 UE 5.7 Source Build 전환 전 환경 점검 체크리스트다.

## 범위

- Epic GitHub 소스 접근 확인
- Visual Studio, Windows SDK, Git, 디스크 공간 확인
- 기존 `.uproject` 백업과 EngineAssociation 기록
- Server Target 존재 여부 확인

## 제외

- C++ 코드 작성
- Target.cs 작성
- GameMode 작성
- PlayerController 작성
- 무기, 체력, 대미지, 스코어, 리스폰, 로비, 매치메이킹 작업

## 기준 경로

| 항목 | 경로 |
|---|---|
| 프로젝트 루트 | `D:\Work\CarFight_git` |
| 프로젝트 파일 | `D:\Work\CarFight_git\UE\CarFight_Re.uproject` |
| 현재 런처 엔진 | `D:\UE_5.7` |
| 추천 Source Build 엔진 | `D:\UE_5.7_Source` |

## 체크리스트

| 체크 | 항목 | 기준 |
|---|---|---|
| [ ] | Epic Games 계정 | 로그인 가능 |
| [ ] | GitHub 연결 | Epic 계정과 GitHub 계정 연결 완료 |
| [ ] | UnrealEngine 소스 접근 | `EpicGames/UnrealEngine` 저장소 접근 가능 |
| [ ] | Visual Studio | UE 5.7 빌드 가능 구성 설치 |
| [ ] | C++ 도구 | Desktop development with C++ 설치 |
| [ ] | Windows SDK | UE 5.7 빌드에 필요한 SDK 설치 |
| [ ] | Git | `git --version` 확인 가능 |
| [ ] | 디스크 공간 | Source Build와 Intermediate 산출물 저장 가능 |
| [ ] | Git 상태 | 전환 전 변경사항 확인 |
| [ ] | `.uproject` 백업 | 전환 전 파일 백업 완료 |
| [ ] | EngineAssociation 기록 | 전환 전 값 기록 완료 |
| [ ] | Server Target | `CarFight_ReServer` Target 존재 여부 확인 |

## 전환 전 기록 양식

```text
전환 전 EngineAssociation:
전환 전 정상 실행 엔진:
전환 전 마지막 정상 커밋:
Source Build 설치 경로:
UE 5.7 소스 브랜치/태그:
Visual Studio 버전:
Windows SDK 버전:
```

Server Target이 없으면 이번 작업에서는 Target.cs를 작성하지 않고, Server 빌드 검증 보류로 기록한다.
