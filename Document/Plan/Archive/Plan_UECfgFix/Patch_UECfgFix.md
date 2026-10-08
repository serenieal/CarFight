# UE Config 경로 불일치 패치안

- 문서 버전: v1.0.0
- 작성일: 2026-04-10
- 대상 시스템: GoPyMCP UE Runner
- 대상 이슈: `ue.list_assets` 호출 시 `ue runner configuration missing` 발생
- 작성 목적: 초보자도 바로 수정할 수 있도록 원인, 수정 위치, 코드 예시, 검증 절차를 한 문서에 정리

---

## 1. 문제 요약

현재 `diag.ue_env`는 정상적으로 UE 설정 파일을 읽는다.

확인된 설정 파일:

```text
D:\Work\CarFight_git\GoPyMCP\Config\gopymcp_ue.json
```

실제 설정 내용:

```json
{
  "editor_cmd": "D:\\UE_5.7\\Engine\\Binaries\\Win64\\UnrealEditor-Cmd.exe",
  "project_file": "D:\\Work\\CarFight_git\\UE\\CarFight_Re.uproject",
  "commandlet": "AssetDump",
  "workdir": "D:\\Work\\CarFight_git\\UE",
  "extra_args": ""
}
```

그런데 `ue.list_assets`를 호출하면 아래 오류가 발생한다.

```text
ue runner configuration missing
```

겉으로 보면 설정이 없는 것처럼 보이지만, 실제 원인은 **설정 파일을 읽는 기준 경로가 서로 다르기 때문**이다.

---

## 2. 근본 원인

### 2.1 진단 코드가 보는 설정 경로

`diag.ue_env`는 Python adapter 쪽 진단 로직을 사용한다.
이 로직은 `GoPyMCP` 루트를 기준으로 아래 파일을 읽는다.

```text
D:\Work\CarFight_git\GoPyMCP\Config\gopymcp_ue.json
```

즉, 진단 쪽은 현재 올바른 설정 파일을 보고 있다.

### 2.2 실제 UE 런타임이 보는 설정 경로

반면, `ue.list_assets`는 Go core의 UE runner를 사용한다.
이쪽은 `ResolveUECommandConfig()` 안에서 `os.Getwd()` 기준으로 기본 설정 파일을 찾는다.

현재 bridge 로그에서 확인된 실제 작업 디렉터리:

```text
D:\Work\CarFight_git\GoPyMCP\Workspace
```

현재 코드가 기본적으로 찾는 파일:

```text
D:\Work\CarFight_git\GoPyMCP\Workspace\Config\gopymcp_ue.json
```

하지만 실제 파일은 여기 있다.

```text
D:\Work\CarFight_git\GoPyMCP\Config\gopymcp_ue.json
```

즉,

- 진단 코드: `GoPyMCP\Config\gopymcp_ue.json`
- 런타임 코드: `GoPyMCP\Workspace\Config\gopymcp_ue.json`

로 기준이 달라서, 진단은 성공하고 런타임은 실패한다.

---

## 3. 실패가 발생하는 정확한 조건

### 3.1 에러 정의 파일

파일:

```text
GoPyMCP/Workspace/core/go/internal/ue_runner/ue_errors.go
```

핵심 에러:

```go
var ErrUEConfigurationMissing = errors.New("ue runner configuration missing")
```

### 3.2 실제 실패 분기

파일:

```text
GoPyMCP/Workspace/core/go/internal/ue_runner/ue_config.go
```

핵심 조건:

```go
if strings.TrimSpace(resolvedConfig.EditorCmd) == "" || strings.TrimSpace(resolvedConfig.ProjectFile) == "" {
    return UECommandConfig{}, ErrUEConfigurationMissing
}
```

즉, 코어가 설정 파일을 못 읽어서

- `EditorCmd`
- `ProjectFile`

를 채우지 못하면 바로 이 오류가 난다.

---

## 4. 수정 목표

이번 패치의 목표는 단순하다.

### 목표 1
`diag.ue_env`와 `ue.list_assets`가 **같은 설정 파일을 기준으로 동작**하게 만든다.

### 목표 2
현재 bridge 실행 작업 디렉터리가 `Workspace` 여도,
실제 설정 파일인 `GoPyMCP\Config\gopymcp_ue.json` 을 정상 인식하게 만든다.

### 목표 3
기존 환경변수 override 흐름은 깨지지 않게 유지한다.

---

## 5. 권장 수정 방식

가장 안전하고 작은 수정은 아래 방식이다.

### 권장안
`resolveDefaultUEConfigPaths()` 에서 기본 탐색 경로를 하나만 보지 말고,
아래 둘 다 보도록 만든다.

1. 현재 작업 디렉터리 기준
2. 현재 작업 디렉터리의 상위 디렉터리 기준

즉, 현재처럼 bridge CWD가

```text
D:\Work\CarFight_git\GoPyMCP\Workspace
```

일 때,
다음 두 경로를 모두 후보로 넣는다.

```text
D:\Work\CarFight_git\GoPyMCP\Workspace\Config\gopymcp_ue.json
D:\Work\CarFight_git\GoPyMCP\Config\gopymcp_ue.json
```

이렇게 하면 지금 구조를 거의 건드리지 않으면서 문제를 해결할 수 있다.

---

## 6. 수정 대상 파일

### 수정 파일

```text
GoPyMCP/Workspace/core/go/internal/ue_runner/ue_config.go
```

### 수정 대상 함수

```go
func resolveDefaultUEConfigPaths() []string
```

---

## 7. 수정 전 코드 설명

현재 함수는 대략 이런 흐름이다.

```go
func resolveDefaultUEConfigPaths() []string {
    candidatePaths := []string{}

    currentWorkingDirectory, workingDirectoryError := os.Getwd()
    if workingDirectoryError == nil && strings.TrimSpace(currentWorkingDirectory) != "" {
        candidatePaths = append(candidatePaths, filepath.Join(currentWorkingDirectory, "Config", "gopymcp_ue.json"))
    }

    userHomeDirectory, userHomeError := os.UserHomeDir()
    if userHomeError == nil && strings.TrimSpace(userHomeDirectory) != "" {
        candidatePaths = append(candidatePaths, filepath.Join(userHomeDirectory, ".gopymcp", "ue.json"))
    }

    return candidatePaths
}
```

이 구조의 문제는 **현재 작업 디렉터리 바로 아래 Config만 찾는다는 점**이다.

지금처럼 bridge CWD가 `...GoPyMCP\Workspace` 인 경우,
실제 파일이 한 단계 위인 `...GoPyMCP\Config` 에 있어도 못 찾는다.

---

## 8. 수정 후 코드안

아래는 바로 적용 가능한 권장 패치안이다.

```go
package ue_runner

import (
    "encoding/json"
    "errors"
    "os"
    "path/filepath"
    "strings"
)

// resolveDefaultUEConfigPaths는 기본 탐색 설정 파일 경로 목록을 반환한다.
func resolveDefaultUEConfigPaths() []string {
    // candidatePaths는 순서대로 시도할 UE 설정 파일 후보 목록이다.
    candidatePaths := []string{}

    // currentWorkingDirectory는 현재 Go core 프로세스의 작업 디렉터리다.
    currentWorkingDirectory, workingDirectoryError := os.Getwd()
    if workingDirectoryError == nil && strings.TrimSpace(currentWorkingDirectory) != "" {
        // 현재 작업 디렉터리 바로 아래 Config를 먼저 본다.
        candidatePaths = append(
            candidatePaths,
            filepath.Join(currentWorkingDirectory, "Config", "gopymcp_ue.json"),
        )

        // parentDirectory는 현재 작업 디렉터리의 상위 폴더다.
        parentDirectory := filepath.Dir(currentWorkingDirectory)
        if strings.TrimSpace(parentDirectory) != "" && parentDirectory != currentWorkingDirectory {
            // 상위 디렉터리 아래 Config도 함께 본다.
            candidatePaths = append(
                candidatePaths,
                filepath.Join(parentDirectory, "Config", "gopymcp_ue.json"),
            )
        }
    }

    // userHomeDirectory는 사용자 홈 디렉터리다.
    userHomeDirectory, userHomeError := os.UserHomeDir()
    if userHomeError == nil && strings.TrimSpace(userHomeDirectory) != "" {
        // 사용자 전역 설정 파일도 계속 지원한다.
        candidatePaths = append(
            candidatePaths,
            filepath.Join(userHomeDirectory, ".gopymcp", "ue.json"),
        )
    }

    return candidatePaths
}
```

---

## 9. 왜 이 수정이 안전한가

### 9.1 기존 동작을 거의 유지한다
기존 1순위 후보였던

```text
[current working directory]\Config\gopymcp_ue.json
```

은 그대로 유지된다.

즉, 기존 사용자가 이미 `Workspace\Config` 아래에 파일을 둔 경우 동작이 바뀌지 않는다.

### 9.2 현재 프로젝트 구조도 수용한다
지금 프로젝트는 실제 설정 파일이

```text
GoPyMCP\Config\gopymcp_ue.json
```

에 있으므로, 상위 폴더 탐색만 추가하면 바로 인식할 수 있다.

### 9.3 환경변수 override를 깨지 않는다
`ResolveUECommandConfig()` 의 뒤쪽에는 아래 흐름이 있다.

- `GOPYMCP_UE_CONFIG`
- `GOPYMCP_UE_EDITOR_CMD`
- `GOPYMCP_UE_PROJECT_FILE`
- `GOPYMCP_UE_COMMANDLET`
- `GOPYMCP_UE_WORKDIR`
- `GOPYMCP_UE_EXTRA_ARGS`

즉, 이번 패치는 **기본 탐색 경로만 보강**하는 것이므로,
기존 env override 체계는 그대로 유지된다.

---

## 10. 추가 권장 개선안

이번 패치만으로도 문제는 해결될 가능성이 높다.
하지만 구조적으로 더 안정적으로 만들고 싶다면 아래 개선도 고려할 수 있다.

### 개선안 A
`resolveDefaultUEConfigPaths()` 가
`GoPyMCP root` 를 명시적으로 계산해서

```text
GoPyMCP\Config\gopymcp_ue.json
```

을 직접 보게 만들기

### 개선안 B
`diag.ue_env` 와 `ResolveUECommandConfig()` 가
같은 설정 해석 helper를 공유하게 리팩터링하기

### 개선안 C
실패 diagnostics에 아래 정보를 함께 넣기

- 코어가 실제로 시도한 config 후보 목록
- 최종 선택된 config path
- `EditorCmd` / `ProjectFile` 비어 있었는지 여부

이렇게 해두면 다음에 비슷한 문제가 생겨도 훨씬 빨리 찾을 수 있다.

---

## 11. 적용 순서

### 1단계
`ue_config.go` 수정

### 2단계
빌드 또는 런타임 재시작

### 3단계
아래 순서로 재검증

1. `diag.ue_env`
2. `ue.list_assets`
3. 필요하면 `ue.dump_asset_details_safe`

---

## 12. 검증 체크리스트

### 12.1 설정 확인
`diag.ue_env` 에서 아래가 유지되는지 본다.

- `loaded_config_path = D:\Work\CarFight_git\GoPyMCP\Config\gopymcp_ue.json`
- `cmd_exists = true`
- `uproject_exists = true`
- `commandlet = AssetDump`

### 12.2 핵심 성공 조건
`ue.list_assets` 호출 시 더 이상 아래 메시지가 나오지 않아야 한다.

```text
ue runner configuration missing
```

### 12.3 다음 단계 실패는 정상이다
이번 패치 후 만약 다른 오류가 나오면,
그건 이제 설정 단계는 통과했다는 뜻이다.
예를 들면:

- UE 실행 자체 실패
- AssetDump commandlet 실패
- plugin 누락
- timeout
- project load 문제

즉, 이번 패치의 성공 기준은 **configuration missing 제거**다.

---

## 13. 임시 우회 방법

코드를 바로 고치기 전에 분석이 맞는지 빨리 검증하고 싶다면,
환경변수로 아래를 명시 지정해도 된다.

```text
GOPYMCP_UE_CONFIG=D:\Work\CarFight_git\GoPyMCP\Config\gopymcp_ue.json
```

이 상태에서 `ue.list_assets` 를 다시 실행했을 때
`ue runner configuration missing` 이 사라지면,
이번 문서의 원인 분석은 맞다고 봐도 된다.

다만 이 방법은 **임시 확인용**이고,
근본 해결은 `ue_config.go` 수정이 더 적절하다.

---

## 14. 초보자용 한 줄 요약

지금 오류는 **설정이 없는 게 아니라, 프로그램이 설정 파일을 잘못된 폴더에서 찾고 있어서 생긴 문제**다.

따라서 해결 방법은

- `Workspace\Config` 만 보지 말고
- 그 상위인 `GoPyMCP\Config` 도 같이 보게

코드를 고치는 것이다.

---

## 15. 최종 권장안

이번 이슈는 아래 방식으로 처리하는 것을 권장한다.

### 최종 권장안
- `ue_config.go`
- `resolveDefaultUEConfigPaths()`
- 상위 디렉터리 `Config/gopymcp_ue.json` 탐색 추가

### 이유
- 수정 범위가 작다
- 기존 구조를 거의 안 깨뜨린다
- 지금 실제 프로젝트 구조와 맞다
- `diag.ue_env` 와 `ue.list_assets` 의 불일치를 빠르게 해소할 수 있다

---

## 16. 후속 버전 관리 메모

### v1.0.0
- 최초 패치안 문서 작성
- 원인 분석, 수정 코드 예시, 검증 절차 포함

### 다음 버전에서 추가 가능
- 실제 적용 diff 기록
- 적용 후 테스트 결과
- `ue.dump_asset_details_safe` 재검증 로그
