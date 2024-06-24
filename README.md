# 과제요구사항
INVITE
클라이언트를 채널에 초대합니다.
형식: INVITE <nickname> <channel>
이거하기
## 채널
KICK - Eject a client from the channel
INVITE - Invite a client to a channel
TOPIC - Change or view the channel topic
MODE - Change the channel’s mode:
- i: Set/remove Invite-only channel
- t: Set/remove the restrictions of the TOPIC command to channeloperators
- k: Set/remove the channel key (password)
- o: Give/take channel operator privilege
## 서버
## 클라이언트
## 기타

### RFC 1459 준수
IRC 프로토콜은 RFC 1459에 정의되어 있습니다. 이는 클라이언트와 서버 간의 통신에 필요한 표준입니다. 주요 명령어와 메시지 형식, 연결 및 인증 절차 등을 포함합니다.

### 1. 소개
- IRC는 실시간으로 텍스트 메시지를 교환할 수 있는 인터넷 프로토콜입니다.
- 클라이언트와 서버 간의 통신 규약을 정의합니다.
- 다중 사용자 채팅, 개인 메시지, 파일 전송 등을 지원합니다.

### 2. 메시지 형식
- 모든 IRC 메시지는 텍스트 기반이며 `CR LF` (캐리지 리턴과 라인 피드)로 끝납니다.
- 최대 메시지 길이는 512자입니다 (CR LF 포함).

### 메시지 포맷
IRC 메시지는 `CR LF` (캐리지 리턴과 라인 피드)로 끝납니다. 메시지는 최대 512자까지 가능합니다.
```irc
<prefix> <command> <params> :<trailing>
```

- `<prefix>`: 선택 사항으로, 메시지의 출처를 나타냅니다.
- `<command>`: 수행할 동작을 나타내는 명령어입니다.
- `<params>`: 명령어와 관련된 매개변수입니다.
- `:<trailing>`: 메시지의 나머지 부분으로, 보통 공백이 포함된 텍스트입니다.

### 3. 주요 명령어

#### NICK
- 클라이언트의 닉네임을 설정합니다.
- 형식: `NICK <nickname>`

#### USER
- 클라이언트의 사용자 정보를 설정합니다.
- 형식: `USER <username> <hostname> <servername> <realname>`

#### JOIN
- 클라이언트가 채널에 참여합니다.
- 형식: `JOIN <channel>{,<channel>}`

#### PART
- 클라이언트가 채널을 떠납니다.
- 형식: `PART <channel>{,<channel>}`

#### PRIVMSG
- 특정 클라이언트 또는 채널에 메시지를 보냅니다.
- 형식: `PRIVMSG <receiver>{,<receiver>} :<message>`

#### QUIT
- 클라이언트가 서버 연결을 종료합니다.
- 형식: `QUIT :<message>`

#### TOPIC
- 채널의 주제를 설정하거나 조회합니다.
- 형식: `TOPIC <channel> :<topic>` (주제 설정)
- 형식: `TOPIC <channel>` (주제 조회)

#### KICK
- 클라이언트를 채널에서 강퇴합니다.
- 형식: `KICK <channel> <user> [:<comment>]`

#### INVITE
- 클라이언트를 채널에 초대합니다.
- 형식: `INVITE <nickname> <channel>`

#### MODE
- 사용자나 채널의 모드를 설정하거나 조회합니다.
- 형식: `MODE <target> <modes>`

### 4. 응답 코드
서버는 다양한 응답 코드를 사용하여 클라이언트에 상태와 오류를 알립니다. 몇 가지 예는 다음과 같습니다:
- `001`: 환영 메시지
- `433`: 닉네임이 이미 사용 중임

### 참고할 사이트

1. **RFC 1459 원문**:
   - [RFC 1459: Internet Relay Chat Protocol](https://tools.ietf.org/html/rfc1459)

2. **IRC 프로토콜 관련 자료**:
   - [IRC 프로토콜 개요](https://www.irchelp.org/irchelp/rfc/)
   - [Wikipedia: Internet Relay Chat](https://en.wikipedia.org/wiki/Internet_Relay_Chat)

3. **IRC 관련 라이브러리 및 도구**:
   - [libircclient](http://www.ulduzsoft.com/libircclient/): C 언어로 작성된 IRC 클라이언트 라이브러리
   - [HexChat](https://hexchat.github.io/): 오픈 소스 IRC 클라이언트

4. **GitHub 프로젝트 예제**:
   - [simple-ircd](https://github.com/jrosdahl/simple-ircd): 간단한 IRC 서버 구현 예제
   - [ircd-hybrid](https://github.com/ircd-hybrid/ircd-hybrid): IRC 서버 소프트웨어

----
# 평가지요구사항
## 채널
## 서버
## 클라이언트
----
# 슬랙확인
## 채널
## 서버
## 클라이언트
----
# 블로그등등드으드으으
## 채널
## 서버
## 클라이언트
