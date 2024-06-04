import telnetlib
import threading
import time

# 서버와 연결할 호스트와 포트 번호를 설정합니다.
HOST = "127.0.0.1"
PORT = 4000
NICK = "t0"
USER = "t0"
CHANNEL = "#t"

# 메시지를 수신하여 출력하는 함수입니다.
def receive_messages(tn):
	try:
		while True:
			response = tn.read_until(b"\r\n", timeout=1)
			if response:
				print(response.decode('utf-8'))
	except EOFError:
		print("Server closed the connection.")
		exit(0)
	except Exception as e:
		print(f"An error occurred: {e}")

def main():
	try:
		# 텔넷 연결을 설정합니다.
		tn = telnetlib.Telnet(HOST, PORT)
		print(f"Connected to {HOST}:{PORT}")

		# 자동으로 NICK, USER, JOIN 명령어 전송
		tn.write(f"NICK {NICK}\r\n".encode('utf-8'))
		time.sleep(0.5)
		tn.write(f"USER {USER} 0 * :Real Name\r\n".encode('utf-8'))
		time.sleep(0.5)
		tn.write(f"JOIN {CHANNEL}\r\n".encode('utf-8'))
		time.sleep(0.5)

		# 메시지 수신 스레드를 시작합니다.
		receiver_thread = threading.Thread(target=receive_messages, args=(tn,))
		receiver_thread.daemon = True
		receiver_thread.start()

		# 사용자로부터 수동으로 명령어를 입력받습니다.
		while True:
			command = input("")
			if command.lower() == 'exit':
				break
			tn.write(command.encode('utf-8') + b"\r\n")
			print(f"Sent: {command}")

		tn.close()
	except Exception as e:
		print(f"An error occurred: {e}")

if __name__ == "__main__":
	main()
