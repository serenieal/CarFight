# CarFight Audio Resource Plan

- Version: 0.2.0
- Date: 2026-07-24
- Status: Rejected / Superseded / Do Not Execute
- Former Feature ID: `CF-FQ-024`
- Superseding Decision: `CF-PDL-0009 — CarFight는 게임 사운드를 지원하지 않는다`
- Active Parent Plan: `Document/Plan/CombatFxAudio/ImplementationDesign.md`

---

## 1. 현재 상태

이 문서는 과거 사운드 리소스 확보 계획의 경로를 보존하기 위한 폐기 안내 문서다.

CarFight는 게임 사운드를 지원하지 않으므로 다음 작업을 수행하지 않는다.

```text
- AI 효과음 생성
- 외부 효과음 다운로드
- SoundWave 원본 확보와 편집
- SoundCue 또는 MetaSound Source 제작
- Sound Attenuation 제작
- 사운드 라이선스 조사와 크레딧 목록 작성
- UE 프로젝트로 WAV 반입
- 발사음, 피격음, 파괴음, 차량음, UI음과 BGM 제작
```

기존에 검토했던 ElevenLabs, Stable Audio, Freesound와 기타 사운드 생성·확보 경로는 더 이상 CarFight 작업 후보가 아니다.

---

## 2. 구현 해석

```text
- CF-FQ-024는 전투 FX 전용 기능이다.
- 대표 구현 계획은 같은 디렉터리의 ImplementationDesign.md다.
- CombatFxAudio는 레거시 디렉터리명이며 Audio 구현을 의미하지 않는다.
- 이 문서를 작업 체크포인트, 리소스 목록 또는 구매·다운로드 지시로 사용하지 않는다.
```

---

## 3. 보존 이유

파일 경로를 즉시 삭제하지 않는 이유는 과거 문서 링크와 결정 이력을 깨지 않기 위해서다.
이 문서의 기존 사운드 제작 세부 내용은 현재 제품 방향과 충돌하므로 활성 본문에서는 제거했다.

---

## 4. Changelog

### v0.2.0 - 2026-07-24

```text
- 사용자 결정에 따라 CarFight 게임 사운드 리소스 확보 계획을 Rejected로 전환했다.
- AI 생성, 외부 다운로드, SoundWave 반입, MetaSound와 Attenuation 제작 범위를 전부 폐기했다.
- 파일은 과거 링크 호환을 위한 폐기 안내 경로로만 보존한다.
```

### v0.1.0 - 2026-07-16

```text
- 과거 P0 전투 사운드 리소스 확보 계획 최초 작성.
- 이 버전의 실행 계획은 v0.2.0과 CF-PDL-0009로 대체됐다.
```

---

## 5. Migration

### v0.2.0 적용 안내

```text
- 이 문서를 기반으로 사운드 자산을 생성하거나 다운로드하지 않는다.
- 기존 로컬 AudioVault 또는 미반입 후보가 있더라도 CarFight 저장소와 콘텐츠에 연결하지 않는다.
- 신규 작업은 Niagara 기반 시각 FX 계획만 사용한다.
- 제품 방향이 다시 명시적으로 변경되기 전에는 이 계획을 재활성화하지 않는다.
```
