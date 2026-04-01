# MyDocument 운영 안내

> 역할: `Document/ProjectSSOT/MyDocument/` 문서의 **역할과 사용 순서**를 고정한다.  
> 마지막 정리(Asia/Seoul): 2026-03-27

---

## 이 폴더의 역할
`MyDocument`는 CarFight의 **북극성 문서 묶음**이다.

즉 이 폴더는 아래 내용을 위해 본다.
- 프로젝트가 최종적으로 어떤 게임과 차량 구조를 목표로 하는가
- 현재 작업이 왜 필요한가
- 지금 만드는 코어가 장기적으로 어디에 붙는가

이 폴더는 **현재 실행 계획 문서 묶음이 아니다.**

---

## 이 폴더를 읽을 때의 주의
이 폴더의 문서는 장기 방향과 상위 구상을 설명한다.

따라서 아래 항목은 이 폴더만 보고 바로 현재 실행 계획으로 해석하지 않는다.
- 과거 과도기 자산명
- 아직 도입되지 않은 최종 구조
- 미래 전환을 전제로 한 조립 / 파괴 / 자동화 파이프라인

예:
- `BP_ModularVehicle` 관련 표현은 역사적 / 구상적 문맥이 포함될 수 있다.
- `CMVS / Cluster Union / Geometry Collection`은 현재 실제 구현이 아니라 **최종 방향**이다.

---

## 현재 실행 기준 문서
현재 실제 작업은 아래 `ProjectSSOT` 루트 문서를 기준으로 진행한다.

1. `Document/ProjectSSOT/03_VisionAlign.md`
   - 최종 방향
   - 현재 기준선
   - 현재 구조 갭
2. `Document/ProjectSSOT/00_Handover.md`
   - 현재 실제 기준선
   - 임시 운영 편차
3. `Document/ProjectSSOT/01_Roadmap.md`
   - 현재 해야 할 일
   - 작업 순서
4. `Document/ProjectSSOT/08_P0_Verification.md`
   - 현재 기준선 검증
   - DriveState 코어 체크
   - 첫 차량 튜닝 기준
5. `Document/ProjectSSOT/16_CPP_DecisionLog.md`
   - 유지 구조
   - 교체 구조
   - 운영 판단

---

## 읽는 순서
### 프로젝트 방향을 먼저 볼 때
1. `MyDocument/CarFight개발기획서.md`
2. `MyDocument/22_Project_BigPicture.md`
3. `Document/ProjectSSOT/03_VisionAlign.md`

### 실제 작업에 들어갈 때
1. `Document/ProjectSSOT/03_VisionAlign.md`
2. `Document/ProjectSSOT/00_Handover.md`
3. `Document/ProjectSSOT/01_Roadmap.md`
4. `Document/ProjectSSOT/08_P0_Verification.md`
5. `Document/ProjectSSOT/16_CPP_DecisionLog.md`

---

## 한 줄 규칙
`MyDocument`는 **북극성**,  
`ProjectSSOT` 루트는 **현재 실행 기준**으로 본다.
