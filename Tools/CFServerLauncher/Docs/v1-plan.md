# CFServerLauncher v1.0 구현 메모

작성 기준일: 2026-05-29

## 1. 기준 문서

상위 SSOT 문서 위치:

```text
Document/ProjectSSOT/Plan/CFServerLauncher
```

구현은 다음 문서를 기준으로 진행한다.

- `Roadmap.md`
- `Plan.md`
- `Design.md`
- `ConfigSpec.md`
- `RunLogSpec.md`
- `UISpec.md`
- `CheckList.md`

## 2. 구현 범위

이 폴더의 WPF 프로젝트는 v1.0 최소 범위만 담당한다.

- 설정 JSON 저장/불러오기
- 실행 인자 미리보기
- 서버 외부 프로세스 시작
- 런처가 시작한 서버 프로세스 종료
- `-AbsLog` 파일 tail 표시

## 3. 제외 범위

- Unreal C++ 빌드
- Cook/Stage 자동화
- RCON
- 게임 내부 관리자 명령
- 웹 대시보드
- 복잡한 서버 모니터링
