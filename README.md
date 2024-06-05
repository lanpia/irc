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
