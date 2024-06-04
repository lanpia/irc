# 과제요구사항
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

### 명령어 형식
모든 명령어는 특정 형식을 따라야 합니다. 예를 들어, NICK, USER, JOIN, PRIVMSG 등 주요 명령어는 아래와 같은 형식을 가집니다.
- **NICK**: `NICK <nickname>`
- **USER**: `USER <username> <hostname> <servername> <realname>`
- **JOIN**: `JOIN <channel>`
- **PRIVMSG**: `PRIVMSG <target> :<message>`
- **QUIT**: `QUIT :<message>`

### 메시지 포맷
IRC 메시지는 `CR LF` (캐리지 리턴과 라인 피드)로 끝납니다. 메시지는 최대 512자까지 가능합니다.
```irc
<prefix> <command> <params> :<trailing>

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
