# IRC 서버 구현을 위한 규칙

## 1. 필수 메시지

### 1.1 환영 메시지 (Welcome message)
클라이언트가 성공적으로 접속했을 때 보내는 환영 메시지입니다.
```plaintext
:<server> 001 <nickname> :Welcome to the Internet Relay Network <nickname>
```

### 1.2 호스트 정보 (Your host)
클라이언트가 접속한 서버의 정보를 알려줍니다.
```plaintext
:<server> 002 <nickname> :Your host is <servername>, running version <version>
```

### 1.3 서버 생성 정보 (Server creation)
서버가 언제 생성되었는지 알려줍니다.
```plaintext
:<server> 003 <nickname> :This server was created <date>
```

### 1.4 서버 정보 (Server info)
서버의 버전과 설정 정보를 클라이언트에게 전달합니다.
```plaintext
:<server> 004 <nickname> <servername> <version> <user_modes> <chan_modes>
```

### 1.5 서버의 지원 명령어 (Server capabilities)
클라이언트가 서버에서 지원하는 명령어들을 나열하여 제공합니다.
```plaintext
:<server> 005 <nickname> <support details> :are supported by this server
```

## 2. 필수 오류 메시지

### 2.1 닉네임 충돌 (ERR_NICKNAMEINUSE)
클라이언트가 사용하려는 닉네임이 이미 사용 중일 때 반환합니다.
```plaintext
:<server> 433 <nickname> :Nickname is already in use
```

### 2.2 유효하지 않은 닉네임 (ERR_ERRONEUSNICKNAME)
클라이언트가 유효하지 않은 형식의 닉네임을 사용하려 할 때 반환합니다.
```plaintext
:<server> 432 <nickname> :Erroneous nickname
```

### 2.3 닉네임 또는 사용자 정보 부족 (ERR_NONICKNAMEGIVEN, ERR_NOTREGISTERED)
클라이언트가 `NICK` 또는 `USER` 명령어를 사용하지 않고 채널에 참여하려 할 때 반환합니다.
```plaintext
:<server> 431 :No nickname given
:<server> 451 <nickname> :You have not registered
```

### 2.4 명령어 부족 (ERR_NEEDMOREPARAMS)
명령어에 필요한 매개변수가 부족할 때 반환합니다.
```plaintext
:<server> 461 <command> :Not enough parameters
```

### 2.5 사용자 권한 부족 (ERR_NOPRIVILEGES, ERR_CHANOPRIVSNEEDED)
클라이언트가 권한이 부족한 명령어를 실행하려 할 때 반환합니다.
```plaintext
:<server> 481 :Permission Denied- You're not an IRC operator
:<server> 482 <channel> :You're not channel operator
```

## 3. 예외 처리

### 3.1 유효하지 않은 닉네임 사용
클라이언트가 유효하지 않은 닉네임을 사용할 때 이를 감지하고 `ERR_ERRONEUSNICKNAME` 메시지를 반환합니다.
```cpp
if (!isValidNickname(nickname)) {
    send_message(client_fd, ":irc.example.com 432 " + nickname + " :Erroneous nickname");
}
```

### 3.2 닉네임 중복
클라이언트가 이미 사용 중인 닉네임을 사용할 때 이를 감지하고 `ERR_NICKNAMEINUSE` 메시지를 반환합니다.
```cpp
if (isNicknameInUse(nickname)) {
    send_message(client_fd, ":irc.example.com 433 " + nickname + " :Nickname is already in use");
}
```

### 3.3 닉네임 또는 사용자 정보 부족
클라이언트가 `NICK` 또는 `USER` 명령어를 사용하지 않고 `JOIN` 명령어를 실행하려 할 때 `ERR_NONICKNAMEGIVEN` 또는 `ERR_NOTREGISTERED` 메시지를 반환합니다.
```cpp
if (!hasGivenNickOrUserInfo(client_fd)) {
    send_message(client_fd, ":irc.example.com 451 " + nickname + " :You have not registered");
}
```

### 3.4 명령어 부족
클라이언트가 명령어에 필요한 매개변수를 제공하지 않았을 때 `ERR_NEEDMOREPARAMS` 메시지를 반환합니다.
```cpp
if (params.empty()) {
    send_message(client_fd, ":irc.example.com 461 " + command + " :Not enough parameters");
}
```

### 3.5 권한 부족
클라이언트가 운영자 권한이 필요한 명령어를 실행하려 할 때 `ERR_NOPRIVILEGES` 또는 `ERR_CHANOPRIVSNEEDED` 메시지를 반환합니다.
```cpp
if (!isOperator(client_fd)) {
    send_message(client_fd, ":irc.example.com 481 :Permission Denied- You're not an IRC operator");
}
```

### 3.6 클라이언트 연결 종료
클라이언트의 연결이 종료되었을 때 이를 감지하고 자원을 정리합니다.
```cpp
if (bytes_received == 0) {
    close(client_fd);
    // 클라이언트 목록에서 제거
}
```

### 3.7 네트워크 오류
네트워크 오류 발생 시 적절한 로그를 남기고 자원을 정리합니다.
```cpp
if (bytes_received == -1) {
    perror("recv");
    close(client_fd);
    // 클라이언트 목록에서 제거
}
```

## 4. 추가적인 규칙 및 예외 처리 (RFC 1459 참고)

### 4.1 PING 및 PONG
서버는 클라이언트의 연결 상태를 확인하기 위해 `PING` 메시지를 보내야 합니다. 클라이언트는 `PONG` 메시지로 응답해야 합니다.
```plaintext
PING :<server>
PONG :<server>
```

### 4.2 서버와 클라이언트 간의 동기화
서버는 클라이언트가 `NICK` 및 `USER` 명령어를 보내고 성공적으로 등록되었음을 확인한 후에만 다른 명령어를 허용해야 합니다.
```cpp
if (!isRegistered(client_fd)) {
    send_message(client_fd, ":irc.example.com 451 " + nickname + " :You have not registered");
}
```

### 4.3 유효하지 않은 명령어 (ERR_UNKNOWNCOMMAND)
클라이언트가 지원하지 않는 명령어를 보낼 때 `ERR_UNKNOWNCOMMAND` 메시지를 반환합니다.
```plaintext
:<server> 421 <command> :Unknown command
```

### 4.4 채널 관련 오류
- **채널 존재하지 않음 (ERR_NOSUCHCHANNEL)**
  클라이언트가 존재하지 않는 채널에 접근하려 할 때 반환합니다.
  ```plaintext
  :<server> 403 <channel> :No such channel
  ```

- **채널 운영자 권한 없음 (ERR_CHANOPRIVSNEEDED)**
  클라이언트가 채널 운영자 권한이 필요한 명령어를 실행하려 할 때 반환합니다.
  ```plaintext
  :<server> 482 <channel> :You're not channel operator
  ```

### 4.5 서버의 공지 메시지 (NOTICE)
서버는 중요한 공지사항을 클라이언트에게 `NOTICE` 메시지로 전달할 수 있습니다.
```plaintext
:<server> NOTICE <nickname> :<message>
```

### 4.6 QUIT 메시지
클라이언트가 연결을 종료할 때 `QUIT` 메시지를 보내야 합니다.
```plaintext
QUIT :<message>
```

## 5. 서버 구현 시 고려해야 할 추가 사항

### 5.1 멀티 클라이언트 지원
서버는 동시에 여러 클라이언트를 처리할 수 있어야 합니다. 이를 위해 비차단(non-blocking) 소켓과 `poll()` 또는 `select()`와 같은 다중화(multiplexing) 기법을 사용해야 합니다.

### 5.2 인증 및 보안
서버는 클라이언트가 연결할 때 비밀번호를 요구하고 검증해야 합니다. 이는 서버와 클라이언트 간의 보안을 강화하는 데 필요합니다.

### 5.3 명령어 처리
서버는 IRC 프로토콜의 기본 명령어(`NICK`, `USER`, `JOIN`, `PRIVMSG`, `PART`, `QUIT` 등)를 처리할 수 있어야 합니다. 추가적으로 채널 운영자 명령어(`KICK`, `INVITE`, `TOPIC`, `MODE` 등)도 구현해야 합니다.

### 5.4 메시지 브로드캐스팅
클라이언트가 채널에 보낸 메시지는 해당 채널의 모든 클라이언트에게 브로드캐스트 되어야 합니다.

### 5.5 로그 관리
서버는 클라이언트의 활동과 오류를 기록(log)하여 유지보수와 디버깅에 도움을 줄 수 있어야 합니다.

### 5.6 리소스 관리
서버는 메모리 누수 없이 안정적으로 실행되어야 하며, 클라이언트의 연결 해제 시 자원을 적절히 해제해야 합니다.

graph TD
    A[서버 시작] --> B[소켓 생성]
    B --> C[서버 주소 구조체 설정]
    C --> D[소켓 바인딩]
    D --> E[연결 대기 (listen)]
    E --> F[클라이언트 연결 수락 (accept)]
    
    F --> G[클라이언트 연결 처리]
    
    subgraph "클라이언트 연결 처리"
        G --> H[클라이언트 인증]
        H --> I{인증 성공?}
        I -->|예| J[명령어 수신 대기]
        I -->|아니오| K[연결 종료]
        
        J --> L{명령어 유형 확인}
        L -->|NICK| M[닉네임 설정]
        L -->|USER| N[사용자 정보 설정]
        L -->|JOIN| O[채널 참가]
        L -->|PRIVMSG| P[개인 메시지 처리]
        L -->|PART| Q[채널 떠남]
        L -->|QUIT| R[연결 종료]
        L -->|기타| S[기타 명령어 처리]

        M --> T[응답 메시지 전송]
        N --> T
        O --> T
        P --> T
        Q --> T
        S --> T

        T --> J
        R --> K

        K --> U[클라이언트 목록에서 제거]
        U --> V[자원 해제]
        V --> E
    end

    G --> K
