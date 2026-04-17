# 📘 Web Proxy 서버 구축을 연습하는 장소입니다

내용 없음
```
webproxy_lab_docker/
├── .devcontainer/
│   ├── devcontainer.json      # VSCode에서 컨테이너 환경 설정
│   └── Dockerfile             # C 개발 환경 이미지 정의
│
├── .vscode/
│   ├── launch.json            # 디버깅 설정 (F5 실행용)
│   └── tasks.json             # 컴파일 자동화 설정
│
├── webproxy-lab
│   ├── tiny                    # tiny 웹 서버 구현 폴더
│   │  ├── cgi-bin              # tiny 웹 서버를 테스트하기 위한 동적 컨텐츠를 구현하기 위한 폴더
│   │  ├── home.html            # tiny 웹 서버를 테스트하기 위한 정적 HTML 파일
│   │  ├── tiny.c               # tiny 웹 서버 구현 파일
│   │  └── Makefile             # tiny 웹 서버를 컴파일하기 위한 파일
│   ├── Makefile                # proxy 웹 서버를 컴파일하기 위한 파일
│   └── proxy.c                 # proxy 웹 서버 구현 파일
│
└── README.md  # 설치 및 사용법 설명 문서
```
---


