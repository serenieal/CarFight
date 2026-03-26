# Delete 대기 폴더

> 역할: 현재 도구 제약 때문에 **즉시 삭제/이동하지 못한 루트 스텁 파일**의 정리 대기 위치  
> 마지막 정리(Asia/Seoul): 2026-03-20

---

## 목적
이 폴더는 `Document/ProjectSSOT/` 루트에 남아 있는 임시 스텁 파일을,
삭제 또는 실제 이동 도구가 확보되기 전까지 **정리 대기 상태**로 관리하기 위한 임시 폴더다.

현재는 다음 제약이 있다.
- 쓰기/덮어쓰기는 가능
- 하지만 안전한 파일 이동/삭제 도구는 없다

따라서 지금 단계에서는:
- 삭제 대상 목록을 이 폴더에서 관리하고
- 루트에 남아 있는 스텁 파일은 후속 정리 대상으로 본다.

---

## 현재 Delete 대상(루트 스텁)
- `04_WheelSync_BP.md`
- `09_CPP_Transition_Link.md`
- `11_WheelSnapshotFix.md`
- `13_CPP_P1_Start.md`
- `14_CPP_P2_Design.md`
- `14_CPP_P2_Start.md`
- `15_CPP_P2_DebugPipe.md`
- `15_CPP_P2_HelperCmp.md`
- `17_CPP_P2_SingleAxisYawTest.md`
- `SSOT_20260225.md`
- `SSOT_20260303.md`
- `Archive_Batch01_Run.md`
- `Archive_Move_Check.md`

---

## 현재 원칙
- 활성 문서는 루트의 현재 기준 문서를 우선한다.
- Archive 이관 안내는 `21_Archive_Index.md`를 우선 본다.
- 위 루트 스텁 파일은 최종적으로 삭제하거나 실제 이동할 대상이다.

---

## 후속 작업 원칙
삭제/이동 가능한 도구가 확보되면 아래 순서로 처리한다.
1. 루트 스텁 파일 실제 이동 또는 삭제
2. `Delete/README.md` 목록 갱신
3. `21_Archive_Index.md`와 충돌 없는지 확인
