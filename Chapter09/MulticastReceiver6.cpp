#include "../Common.h"

#define MULTICASTIP "FF12::1:2:3:4"
#define LOCALPORT   9000
#define BUFSIZE     512

int main(int argc, char *argv[])
{
	int retval;

	// 소켓 생성
	SOCKET sock = socket(AF_INET6, SOCK_DGRAM, 0);
	if (sock == INVALID_SOCKET) err_quit("socket()");

	// SO_REUSEADDR 옵션 설정
	int optval = 1;
	retval = setsockopt(sock, SOL_SOCKET, SO_REUSEADDR,
		&optval, sizeof(optval));
	if (retval == SOCKET_ERROR) err_quit("setsockopt()");

	// bind()
	struct sockaddr_in6 localaddr;
	memset(&localaddr, 0, sizeof(localaddr));
	localaddr.sin6_family = AF_INET6;
	localaddr.sin6_addr = in6addr_any;
	localaddr.sin6_port = htons(LOCALPORT);
	retval = bind(sock, (struct sockaddr *)&localaddr, sizeof(localaddr));
	if (retval == SOCKET_ERROR) err_quit("bind()");

	// 멀티캐스트 그룹 가입
	struct ipv6_mreq mreq;
	inet_pton(AF_INET6, MULTICASTIP, &mreq.ipv6mr_multiaddr);
	mreq.ipv6mr_interface = 0;
	retval = setsockopt(sock, IPPROTO_IPV6, IPV6_JOIN_GROUP,
		&mreq, sizeof(mreq));
	if (retval == SOCKET_ERROR) err_quit("setsockopt()");

	// 데이터 통신에 사용할 변수
	struct sockaddr_in6 peeraddr;
	socklen_t addrlen;
	char buf[BUFSIZE + 1];

	// 멀티캐스트 데이터 받기
	while (1) {
		// 데이터 받기
		addrlen = sizeof(peeraddr);
		retval = recvfrom(sock, buf, BUFSIZE, 0,
			(struct sockaddr *)&peeraddr, &addrlen);
		if (retval == SOCKET_ERROR) {
			err_display("recvfrom()");
			break;
		}

		// 받은 데이터 출력
		buf[retval] = '\0';
		char addr[INET6_ADDRSTRLEN];
		inet_ntop(AF_INET6, &peeraddr.sin6_addr, addr, sizeof(addr));
		printf("[UDP/%s:%d] %s\n", addr, ntohs(peeraddr.sin6_port), buf);
	}

	// 멀티캐스트 그룹 탈퇴
	retval = setsockopt(sock, IPPROTO_IPV6, IPV6_LEAVE_GROUP,
		&mreq, sizeof(mreq));
	if (retval == SOCKET_ERROR) err_quit("setsockopt()");

	// 소켓 닫기
	close(sock);
	return 0;
}
