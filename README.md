# KrkrExtractV2 (ForCxdecV2) 동적 툴 모음
&emsp;&emsp;Wamsoft(Hxv4 2021.11+) 암호화 해제/문자열<->Hash 추출에 적용

## 환경
&emsp;&emsp;시스템: Windows 7 SP1 x64

&emsp;&emsp;IDE: Visual Sudio 2022

&emsp;&emsp;컴파일러: MSVC2022 x86

## 이전 버전과의 차이점
&emsp;&emsp;1. 코드 재구성, 약간 덜 엉망진창으로 보임

&emsp;&emsp;2. 기능 모듈을 다른 Dll로 분할

&emsp;&emsp;3. 많은 버그 수정

## 사용 방법
&emsp;&emsp;1. `CxdecExtractorLoader.exe`, `CxdecExtractor.dll`, `CxdecExtractorUI.dll`, `CxdecStringDumper.dll`을 동일한 디렉토리에 유지

&emsp;&emsp;2. 게임이 Wamsoft KrkrZ Hxv4 암호화 유형이고 암호화 인증이 제거되었는지 확인

&emsp;&emsp;3. 게임 exe를 `CxdecExtractorLoader.exe`로 드래그하여 시작하고 모듈 선택 대화 상자를 팝업

&emsp;&emsp;4. `해제 모듈 로드`를 선택하고 해제 대화 상자를 팝업한 다음 `xxx.xp3`를 상자 안으로 드래그하여 해제

&emsp;&emsp;&emsp;4.1 `게임 디렉토리\Extractor_Output\`가 출력 디렉토리이며, `xxx 폴더`의 패키지 리소스와 `xxx.alst` 파일 테이블을 포함

&emsp;&emsp;&emsp;4.2 `도구 디렉토리\Extractor.log`는 로그 정보

&emsp;&emsp;5. `문자열 Hash 추출 모듈 로드`를 선택하여 게임 실행 시 문자열 Hash 매핑 테이블을 자동으로 추출

&emsp;&emsp;&emsp;5.1 `게임 디렉토리\StringHashDumper_Output\`가 출력 디렉토리

&emsp;&emsp;&emsp;5.2 `DirectoryHash.log`는 폴더 경로 Hash 매핑 테이블

&emsp;&emsp;&emsp;5.3 `FileNameHash.log`는 파일 이름 Hash 매핑 테이블

&emsp;&emsp;&emsp;5.4 `Universal.log`는 일반 정보(Hash 암호화 매개변수)

&emsp;&emsp;6. `Key 추출 모듈 로드`를 선택(기능은 아직 구현되지 않음)

&emsp;&emsp;7. 도구는 관리자 권한을 신청하여 UAC 권한 상승을 팝업하지 않으므로 게임과 도구를 C 드라이브에 두지 마십시오.

&emsp;&emsp;8. 오류 제목의 팝업 오류가 발생하면 위의 단계를 확인하십시오.

## 일반적인 문제
&emsp;&emsp;Q: 왜 리소스 파일 이름이 없습니까?

&emsp;&emsp;A: 패키지 안에 원래 파일 이름이 없습니다.

&emsp;&emsp;Q: 해제 대화 상자에서 일괄 드래그 앤 드롭 해제를 지원합니까?

&emsp;&emsp;A: 지원하지 않으며, 단일 패키지만 개별적으로 드래그하여 추출할 수 있습니다.

&emsp;&emsp;Q: 해제 응답 상자가 해제 시 응답이 없습니까?

&emsp;&emsp;A: 다중 스레드 지원을 하지 않았으므로 천천히 완료될 때까지 기다리십시오.

&emsp;&emsp;Q: Hash 매핑 테이블을 한 번에 모두 추출할 수 있습니까?

&emsp;&emsp;A: 아니요, 이름이 스크립트 안에 여기저기 흩어져 있고 완전하지 않습니다.

&emsp;&emsp;Q: Win7 이외의 시스템과 호환됩니까?

&emsp;&emsp;A: 이론적으로는 호환되지만 테스트하지 않았으므로 문제가 있어도 모릅니다.

## 유사 도구 추천

* KrkrExtractV2 (ForCxdecV2)(본 도구)&emsp; 유형: 동적 &emsp; 해제: 일회성 &emsp;파일 이름: 런타임

* [KrkrDump](https://github.com/crskycode/KrkrDump)&emsp; 유형: 동적 &emsp; 해제: 런타임 &emsp;파일 이름: 런타임

* [GARBro](https://github.com/crskycode/GARbro)&emsp; 유형: 정적(수동으로 채워야 함) &emsp; 해제: 일회성 &emsp;파일 이름: 없음
