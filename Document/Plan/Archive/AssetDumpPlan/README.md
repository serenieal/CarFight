# AssetDump Plan Migration Notice

- 문서 버전: v2.0
- 최근 갱신일: 2026-07-14
- 문서 상태: Deprecated / External Repository Boundary
- 역할: 잘못 생성된 CarFight 측 AssetDump 체크포인트의 폐기 및 이관 안내

---

## 1. 현재 기준

AssetDump는 CarFight의 하위 기능 Plan이 아니라 별도 Git 저장소와 별도 문서체계를 가진 독립 도구 프로젝트다.

```text
독립 저장소: UE/Plugins/ue-assetdump
문서 진입점: UE/Plugins/ue-assetdump/Documents/Document_Entry.md
활성 작업: UE/Plugins/ue-assetdump/Documents/ActiveWork.md
Plan 색인: UE/Plugins/ue-assetdump/Documents/Plan/README.md
```

이 문서는 더 이상 CarFight의 Active Plan, 세션 체크포인트 또는 상태 판단 기준으로 사용하지 않는다.
`Document/ActiveWork.md`와 `Document/Plan/README.md`에서도 공식 연결을 제거했다.

---

## 2. 금지 사항

- AssetDump 내부 구현 단계와 릴리스 상태를 CarFight `ActiveWork.md`에 등록하지 않는다.
- AssetDump TaskSource, Codex 계약과 검증 결과를 CarFight Plan으로 복사하지 않는다.
- CarFight FeatureQueue ID를 AssetDump 내부 작업에 사용하지 않는다.

CarFight가 AssetDump의 공개 출력 계약에 의존할 경우 CarFight 문서에는 계약명, 사용 위치와 요구 버전만 기록한다.

---

## 3. Changelog

### v2.0 - 2026-07-14

- 기존 CarFight AssetDump 작업 체크포인트를 폐기 안내 문서로 전환.
- AssetDump 활성 상태와 계획을 독립 저장소 문서체계로 완전히 이관.
- CarFight ActiveWork와 Plan Index에서 공식 연결 제거.

### v1.0 - 2026-07-14

- CarFight 문서체계 안에 AssetDump 체크포인트를 생성했으나 독립 저장소 경계 원칙에 어긋나 v2.0에서 폐기.

---

## 4. Migration

- 이전 문서의 작업 상태는 `UE/Plugins/ue-assetdump/Documents/ActiveWork.md`로 이동했다.
- 상세 설계와 검증 기준은 기존 `UE/Plugins/ue-assetdump/Documents/Plan/AssetIntelligencePlan/`에서 계속 관리한다.
- 이 파일은 경계 오류의 이력을 남기기 위한 migration stub이며 새 작업에서는 읽지 않는다.
