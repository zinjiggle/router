


#ifndef _socketsupport_h
#define _socketsupport_h


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include <arpa/inet.h>
#include "globalhelper.h"

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

// send message in TCP socket
int sendmessage(int sockfd, char* send)
{
	// deal with the ending '/n', delete too many \n, and add \n if there is not
	int numbytes = 0;
	char sendbuf[MAXDATASIZE];
	strcpy(sendbuf,send);
	int ending = strlen(sendbuf)-1;
	int i;
	for (i=ending;i>=0;i=i-1){
		if (sendbuf[i] != '\n') break;
	}
	strcpy(sendbuf+i+1,"\n");
	
	
	
	if ((numbytes = write(sockfd,sendbuf,strlen(sendbuf))) == -1) {
		perror("send error");
		exit(1);
	}
	fprintf(stdout,"Send: %s",sendbuf);
	return numbytes;
}


// receive information in TCP port
int receive(int sockfd, char* buf)
{
        int numbytes;
        if ((numbytes = recv(sockfd, buf, MAXDATASIZE-1, 0)) == -1) {
	    perror("recv");
	    exit(1);
	}
        buf[numbytes] = '\0';
        fprintf(stdout,"%s",buf);

        return numbytes;
}

// read one line information in TCP port
int receiveOneLine(FILE* socket_stream_in, char* buf)
{
	if ( fgets(buf,MAXDATASIZE-1,socket_stream_in)!=NULL ){
		        printf("%s",buf);
		        return 1;
	}
	else {return 0;}
		
}

// wait until you get the wait message information
void waitForMessage(FILE* socket_stream_in, char* buf,const char* waitMessage)
{
	do{
		if ( fgets(buf,MAXDATASIZE-1,socket_stream_in)!=NULL ){
		        printf("%s",buf);
		        if(strncmp(buf,waitMessage,strlen(waitMessage))==0) break;
		}
	}while(1);
}

int udpTalkTo(char *hostname, char* SERVERPORT, const char* message, int bytes)
{
	int sockfd;
	struct addrinfo hints, *servinfo, *p;
	int rv;
	int numbytes;
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_DGRAM;

	if ((rv = getaddrinfo(hostname, SERVERPORT, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}

	// loop through all the results and make a socket
	for(p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype,
				p->ai_protocol)) == -1) {
			perror("talker: socket");
			continue;
		}

		break;
	}

	if (p == NULL) {
		fprintf(stderr, "talker: failed to bind socket\n");
		return 2;
	}

	if ((numbytes = sendto(sockfd, message, bytes, 0,
			 p->ai_addr, p->ai_addrlen)) == -1) {
		perror("talker: sendto");
		exit(1);
	}

	freeaddrinfo(servinfo);

	//printf("talker: sent %d bytes to %s at port %s\n", numbytes, hostname, SERVERPORT);
	close(sockfd);
	return 0;
}

int openUDPListenningSocket(char* MYPORT)
{
	int sockfd;
	struct addrinfo hints, *servinfo, *p;
	int rv;
	int numbytes;
	struct sockaddr_storage their_addr;
	char buf[MAXDATASIZE];
	socklen_t addr_len;
	char s[INET6_ADDRSTRLEN];
	

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC; // set to AF_INET to force IPv4
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_flags = AI_PASSIVE; // use my IP

	if ((rv = getaddrinfo(NULL, MYPORT, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}

	// loop through all the results and bind to the first we can
	for(p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype,
				p->ai_protocol)) == -1) {
			perror("listener: socket");
			continue;
		}

		if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
			close(sockfd);
			perror("listener: bind");
			continue;
		}

		break;
	}

	if (p == NULL) {
		fprintf(stderr, "listener: failed to bind socket\n");
		return 2;
	}

	freeaddrinfo(servinfo);

	printf("listener in port: %s : waiting to recvfrom...\n", MYPORT);


	return sockfd;
}


// receive one packet from udp sockfd, stores the information in buf, return the length of bytes received.
int receiveUDPMessage(int sockfd, char* buf)
{	
	int numbytes;
	socklen_t addr_len;
	struct sockaddr_storage their_addr;
	addr_len = sizeof their_addr;
	char s[INET6_ADDRSTRLEN];
	if ((numbytes = recvfrom(sockfd, buf, MAXDATASIZE-1 , 0,
		(struct sockaddr *)&their_addr, &addr_len)) == -1) {
		perror("recvfrom");
		exit(1);
	}
	/*
	printf("listener: got packet from %s\n",
		inet_ntop(their_addr.ss_family,
			get_in_addr((struct sockaddr *)&their_addr),
			s, sizeof s));
	printf("listener: packet is %d bytes long\n", numbytes); */
	buf[numbytes] = '\0';
	//printf("listener: packet contains \"%s\"\n", buf);
	return numbytes;
}

int openTCPSocket(char * hostname, char* hostport)
{
	int sockfd;
	struct addrinfo hints, *servinfo, *p;
	int rv;
	char s[INET6_ADDRSTRLEN];

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	if ((rv = getaddrinfo(hostname, hostport, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return -1;
	}
	// loop through all the results and connect to the first we can
	for(p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype,
				p->ai_protocol)) == -1) {
			perror("client: socket");
			continue;
		}

		if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
			close(sockfd);
			perror("client: connect");
			continue;
		}

		break;
	}
	if (p == NULL) {
		fprintf(stderr, "client: failed to connect\n");
		return -2;
	}
	inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr),
			s, sizeof s);
	// printf("client: connecting to %s\n", s);

	freeaddrinfo(servinfo); // all done with this structure
	return sockfd;
	
}


#endif
