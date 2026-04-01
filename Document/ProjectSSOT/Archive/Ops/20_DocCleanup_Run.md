# ProjectSSOT 정리 실행표

> 역할: `Document/ProjectSSOT` 문서 정리의 기준표이자 실행 로그  
> 작성일시(Asia/Seoul): 2026-03-20 10:19  
> 기준 선언:
> - `BP_ModularVehicle`는 현재 기준 구현이 아니다.
> - 현재 기준 구현은 `BP_CFVehiclePawn` + `ACFVehiclePawn` + `UCFVehicleDriveComp` + `UCFWheelSyncComp` + `UCFVehicleData` 조합이다.
> - 문서 링크/경로는 실제 존재하는 경로를 우선한다.
> - 완료된 과도기 문서는 Archive로 내린다.

---

## 처리 기준
- **유지**: 현재 기준 운영에 계속 필요한 문서
- **수정 유지**: 현재 기준으로 내용을 갱신한 뒤 계속 유지할 문서
- **Archive 이동**: 완료된 작업/과도기/역사 기록 문서
- **축소 유지**: 남기되 기준 문서가 아니라 보조 색인으로만 유지할 문서

---

## 처리표

| 파일 | 판정 | 조치 | 비고 |
|---|---|---|---|
| `README.md` | 수정 유지 | 현재 기준 운영 안내로 재작성 | 활성 문서/Archive 구조 재정리 |
| `ProjectSSOT_OpMap.md` | 축소 유지 | README 보조 색인으로 재작성 | README를 단일 운영 기준으로 고정 |
| `00_Handover.md` | 수정 유지 | 현재 기준 상태 문서로 재작성 | 레거시 `BP_ModularVehicle` 상세 제거 |
| `01_Roadmap.md` | 수정 유지 | C++ 중심 유지/안정화 로드맵으로 재작성 | 전환보다 운영/검증 우선 |
| `02_Conventions.md` | 수정 유지 | 현재 기준 규칙 보강 | 구형 기준 문서화 금지 명시 |
| `07_TestMap_Notes.md` | 유지 | 변경 없음 | 테스트맵 메모 템플릿 |
| `08_P0_Verification.md` | 수정 유지 | `BP_CFVehiclePawn` 기준 검증 문서로 재작성 | 최종 판정 기준선 통일 |
| `12_CPP_TransitionPlan.md` | 수정 유지 | 파일명은 유지, 내용은 현재 C++ 코어 운영 계획으로 재작성 | 실존 경로 우선 반영 |
| `16_CPP_DecisionLog.md` | 수정 유지 | 현재 C++ 코어 결정 로그로 재작성 | 역사 기록과 현재 결정을 분리 |
| `18_DriveState_Tuning.md` | 유지 | 변경 없음 | 현재 활성 튜닝 가이드 |
| `19_DriveState_CoreChecklist.md` | 유지 | 변경 없음 | 현재 활성 코어 검증 체크 |
| `04_WheelSync_BP.md` | Archive 이동 | `Archive/LegacyBP/` 이관 완료 | 구형 BP 기준 절차 |
| `09_CPP_Transition_Link.md` | Archive 이동 | `Archive/CPP/` 이관 완료 | 과도기 입구 문서 |
| `11_WheelSnapshotFix.md` | Archive 이동 | `Archive/LegacyBP/` 이관 완료 | 과거 조사/핫픽스 기록 |
| `13_CPP_P1_Start.md` | Archive 이동 | `Archive/CPP/` 이관 완료 | 완료된 전환 단계 문서 |
| `14_CPP_P2_Design.md` | Archive 이동 | `Archive/CPP/` 이관 완료 | 완료된 전환 단계 문서 |
| `14_CPP_P2_Start.md` | Archive 이동 | `Archive/CPP/` 이관 완료 | 완료된 전환 단계 문서 |
| `15_CPP_P2_DebugPipe.md` | Archive 이동 | `Archive/CPP/` 이관 완료 | 과도기 디버그 파이프 기록 |
| `15_CPP_P2_HelperCmp.md` | Archive 이동 | `Archive/CPP/` 이관 완료 | 과도기 helper 비교 기록 |
| `17_CPP_P2_SingleAxisYawTest.md` | Archive 이동 | `Archive/CPP/` 이관 완료 | 과도기 단일 축 테스트 기록 |
| `SSOT_20260225.md` | Archive 이동 | `Archive/Checkpoints/` 이관 완료 | 체크포인트 로그 |
| `SSOT_20260303.md` | Archive 이동 | `Archive/Checkpoints/` 이관 완료 | 체크포인트 로그 |
| `Archive_Batch01_Run.md` | Archive 이동 | `Archive/Ops/` 이관 완료 | 과거 정리 실행 기록 |
| `Archive_Move_Check.md` | Archive 이동 | `Archive/Ops/` 이관 완료 | 과거 이동 점검표 |

---

## 실행 상태
- [x] 처리표 작성
- [x] 활성 문서 재작성
- [x] Archive 하위 구조 정리
- [x] 완료 문서 이동
- [x] 최종 링크/경로 검증
