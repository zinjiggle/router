/*
** router.c -- self build router.c
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <time.h>

#include <arpa/inet.h>

#include "globalhelper.h"

#include "socketsupport.h"

#include "Dijkstra.h"

#include "topology.h"

/*
MessageTypes:
1: send message required by the server
2: forwarded message: 2/HOPNUM/src/dest/...../Message
3: BroadCast: Original link cost Info, forward link cost generated from the NEIGH info 3/node1/node2/cost
4: BroadCast: Link cost change Info, forward link cost generated from the LINKCOST info 4/node1/node2/cost
*/
int byte2int(char* p)
{
	int result = 0;  // initialize;
	char* rv = (char*)&result;
	rv[0] = p[1];
	rv[1] = p[0];
	return result;
}

struct MESSAGE
{
	int msgType; //1 or 2 or 3 or 4
	int dest;    // forward destination
	int src;     // source node
	int hopNum;  // number of hops, including the src and the dest
	int* path;   // the forward path information
	char message[MAXDMESSAGESIZE]; // real message inside
	struct LINK linkinfo;         //  the link cost control info
};

struct MESSAGE* allocMSG()
{
	struct MESSAGE *p = (struct MESSAGE*) malloc(sizeof(struct MESSAGE));
	memset(p,0,sizeof(struct MESSAGE));
	return p;
}

void freeMSG(struct MESSAGE *p)
{
	if(p->path != NULL) free(p->path);
	free(p);
}



int isControlInfo(char* buf, const char* control)
{
	if(strncmp(buf,control,strlen(control))==0) return 1;
	else return 0;
}

void printMESSAGE(char* message)
{
	int msgType;
	msgType = (int)message[0];
	int dest;
	int* paths;
	int hopNum;
	int i;
	int* nodes;
	float* cost;
	switch(msgType){
	case 1:
		printf("printMESSAGE messagetype: %d ", msgType);
		//memcpy(1+( (char*)&dest ),&message[1],1);
		dest = byte2int(&message[1]);
    		printf("dest: %d message: %s\n",dest,&message[3]);
		return;
	case 2:
		printf("printMESSAGE messagetype: %d ", msgType);
		memcpy(&hopNum,&message[1],sizeof(int));
		printf("hopNum: %d ",hopNum);
		paths = (int*)&message[1+sizeof(int)];
		printf("Paths: ");
		for(i=0;i<hopNum;i++){
			printf("%d ",paths[i]);
		}
		printf("message: %s\n",&message[1+sizeof(int)+sizeof(int)*hopNum]);
		return;
	case 3:
	case 4:
		printf("printMESSAGE messagetype: %d ", msgType);
		printf("node1: %d node2: %d cost: %.0f \n", *(int*)(message+1), *(int*)(message+1+sizeof(int)), *(float*)(message+1+sizeof(int)*2));
		return;
	default: 
		printf("printMESSAGE: Unknown MESSAGE\n");
		return;
	}
}

// return 0 if message type is not 1,2,3,4
int fetchReceivedMessage(struct MESSAGE* p, char* buf)
{
	p->msgType = (int)buf[0];
	int* path=NULL;
	int i;
	switch(p->msgType){
	case 1:
		p->dest = byte2int(&buf[1]);
    		strcpy(p->message, &buf[3]);
    		printf("%d fetchReceivedMessage: ",pGlobalTopo->myaddr);
    		printMESSAGE(buf);
    		//printf("fetchReceivedMessage: after transformation: type: %d dest: %d message: %s\n",p->msgType,p->dest,p->message);
		return 1;
	case 2:
		path = (int*)(buf+1);
		p->hopNum = path[0];
		p->path = (int*)malloc(sizeof(int)*p->hopNum);
		memcpy(p->path,&path[1],sizeof(int)*p->hopNum);
		strcpy(p->message,buf+1+sizeof(int)*(p->hopNum+1));
		p->src = p->path[0];
		p->dest = p->path[p->hopNum-1];
		//printf("fetchReceivedMessage: %d %d %s\n",p->msgType,p->dest,p->message);
		printf("%d fetchReceivedMessage: ",pGlobalTopo->myaddr);
    		printMESSAGE(buf);
		return 2;
	case 3:
		memcpy(&p->linkinfo,&buf[1],sizeof(struct LINK));
		//printf("fetchReceivedMessage: %d %d %d %.0f\n",p->msgType,p->linkinfo.nodeAddr[0],p->linkinfo.nodeAddr[1],p->linkinfo.cost);
		//printf("fetchReceivedMessage: ");
    		//printMESSAGE(buf);
		return 3;
	case 4:
		memcpy(&p->linkinfo,&buf[1],sizeof(struct LINK));
		//printf("fetchReceivedMessage: %d %d %d %.0f\n",p->msgType,p->linkinfo.nodeAddr[0],p->linkinfo.nodeAddr[1],p->linkinfo.cost);
		//printf("fetchReceivedMessage: ");
    		//printMESSAGE(buf);
		return 4;
	default: 
		printf("Unknown UDP fetchReceivedMessage\n");
		return 0;
	}
}



// generate type 2 message.
char* generateForWardingMessage(int* MSGLen, struct PATH* shortest, struct MESSAGE* pMSG)
{
	// generate forwarding message: type 2
    	int messageLen = 0;
    	int numbytes = 0;
    	char* newMessage;
    	char* pos=NULL;
    	switch(pMSG->msgType){
    	case 1:
    		messageLen = 1+sizeof(int)+sizeof(int)*shortest->numHops+strlen(pMSG->message);
    		newMessage = (char*)malloc(sizeof(char)*messageLen);
    		memset(newMessage,0,sizeof(char)*messageLen);
    		newMessage[0] = 2;
    		memcpy(&newMessage[1],&shortest->numHops,sizeof(int));
    		memcpy(&newMessage[1+sizeof(int)],shortest->pathNodes,sizeof(int)*shortest->numHops);
    		strcpy(&newMessage[1+sizeof(int)+sizeof(int)*shortest->numHops],pMSG->message);
    		*MSGLen = messageLen;
    		printf("generateForWardingMessage 1: message is ");
    		printMESSAGE(newMessage);
    		return newMessage;
    	case 2:
    		messageLen = 1+sizeof(int)+sizeof(int)*pMSG->hopNum+strlen(pMSG->message);
    		newMessage = (char*)malloc(sizeof(char)*messageLen);
    		memset(newMessage,0,sizeof(char)*messageLen);
    		newMessage[0] = 2;
    		memcpy(&newMessage[1],&pMSG->hopNum,sizeof(int));
    		memcpy(&newMessage[1+sizeof(int)],pMSG->path,sizeof(int)*pMSG->hopNum);
    		strcpy(&newMessage[1+sizeof(int)+sizeof(int)*pMSG->hopNum],pMSG->message);
    		*MSGLen = messageLen;
    		printf("generateForWardingMessage 2: message is ");
    		printMESSAGE(newMessage);
    		return newMessage;
    	default:
    		return NULL;
    	
    	}
    	
    	
    	
}

bool broadCastLinkInfo(int messagetype, struct LINK link, struct NetworkTopoStruct* Topo)
{
	char buf[MAXDATASIZE];
	int length = 0;
	int i;
	length = 1+sizeof(struct LINK);
	buf[0] = (char)messagetype;
	memcpy(&buf[1],&link, sizeof(struct LINK));
	buf[length] = '\0';
	char broadresult[MAXDATASIZE];
	char temp[MAXDATASIZE];
	memset(broadresult,'\0',MAXDATASIZE);
	
	for(i=0;i<Topo->neighNum;i++)
	{
		if(isInf(Topo->Neighs[i].cost)) continue; // do not broadcast link info through links with infinite cost
		if(isNodeInLink(Topo->Neighs[i].addr,link)) continue; // do not broadcast link info to nodes the nodes in the link
		else{
			if(udpTalkTo(Topo->Neighs[i].host,Topo->Neighs[i].udpport,buf,length) != 0)
    			{
    				perror("ERR broadCastLinkInfo: udp talking error.");
    				exit(1);
    			}
    			else
    			{
    				sprintf(temp,"%d ", Topo->Neighs[i].addr);
    				strcat(broadresult,temp);
    			}
		}
	}
	if(strlen(broadresult)>0)
		printf("%d broadCastLinkInfo: %d <%d %d %.0f> through Neighs: %s\n",Topo->myaddr,messagetype,link.nodeAddr[0],link.nodeAddr[1],link.cost,broadresult);
	return 1;
}


void updateLinkChange(struct LINK linkinfo, struct NetworkTopoStruct *Topo)
{
	int addr;
	struct NEIGHBOUR Nei;
	struct NEIGHBOUR* pNei = NULL;
	// update the link database
	insertLinkInfo(linkinfo, Topo);
	// update the neighbour database if necessary
	if(( pNei = fetchNeiPointerByLink(linkinfo, Topo) ) !=NULL){
		Nei = *pNei;
		Nei.cost = linkinfo.cost;
		insertNeighInfo(Nei,Topo);
	}
}


void dealWithUDPMessage(char* receivebuf, int length, struct NetworkTopoStruct* p, int tcpsockfd)
{
    struct MESSAGE* pMSG = allocMSG();
    char* newMessage = NULL;
    char* buf = (char*) malloc(sizeof(char)*(length+10));
    memcpy(buf,receivebuf,sizeof(char)*(length+2));
    int newMSGLen = 0;
    int neiID=0;
    int i;
    char sendbuf[MAXDATASIZE];
    struct PATH * shortestPath = allocPATH(1);
    /*
    printf("dealWithUDPMessage: new packet received: ");
    printMESSAGE(buf);
    */
    if (fetchReceivedMessage(pMSG, buf)==0) {
    	perror("ERR in dealWithUDPMessage: fetch received message error !\n");
    	return;
    }
    
    switch(pMSG->msgType){
    case 1:
    	if(CalculateShortestPath(shortestPath, pMSG->dest,p) != 1)
    	{
    		printf("dealWithUDPMessage: no route exist!\n");
    		sprintf(sendbuf,"DROP %s",pMSG->message);
    		sendmessage(tcpsockfd,sendbuf);
    		receive(tcpsockfd,buf);
    		break;
    	}
    	newMessage = generateForWardingMessage(&newMSGLen,shortestPath,pMSG);
    	// send log information
    	sprintf(sendbuf,"LOG FWD %d %s",shortestPath->nexthop,pMSG->message);
    	sendmessage(tcpsockfd,sendbuf);
    	receive(tcpsockfd,buf); // This is LOG OK
    	// send newMessage to the neighbour
    	if((neiID = fetchNeiIdByAddr(p,shortestPath->nexthop))<0){
    		perror("ERR dealWithUDPMessage case 1: neigh id not found\n");
    		exit(1);
    	}
    	printf("dealWithUDPMessage: sending %d bytes with %d messages to %d through %d\n",newMSGLen,strlen(pMSG->message),pMSG->dest,p->Neighs[neiID].addr);
    	if(udpTalkTo(p->Neighs[neiID].host,p->Neighs[neiID].udpport,newMessage,newMSGLen) != 0)
    	{
    		perror("ERR dealWithUDPMessage case 1: udp talking error.\n");
    		exit(1);
    	}
    	break;
    case 2:
    	// find myself in the path list, 
    	for(i=0;i<pMSG->hopNum;i++){
    		if(pMSG->path[i] == p->myaddr) break;
    	}
    	//if not in the list, err
    	if(i==pMSG->hopNum){
    		perror("ERR dealWithUDPMessage: src not in path list\n");
    		break;
    	}
    	// if in the list but not the last one, forward and send the log information
    	if( i < pMSG->hopNum-1 ){
    		// send log information
    		sprintf(sendbuf,"LOG FWD %d %s",pMSG->path[i+1],pMSG->message);
    		sendmessage(tcpsockfd,sendbuf);
    		receive(tcpsockfd,buf); // This is LOG OK received
    		// generate forward message
		newMessage = generateForWardingMessage(&newMSGLen,NULL,pMSG);
    		if((neiID = fetchNeiIdByAddr(p,pMSG->path[i+1])) < 0){
    			perror("ERR dealWithUDPMessage case 2: neigh id not found\n");
    			exit(1);
    		}
    		// forward the information
    		printf("dealWithUDPMessage: sending %d bytes with %d messages to %d through %d\n",newMSGLen,strlen(pMSG->message),pMSG->dest,p->Neighs[neiID].addr);
    		if(udpTalkTo(p->Neighs[neiID].host,p->Neighs[neiID].udpport,newMessage,newMSGLen) != 0)
    		{
    			perror("ERR dealWithUDPMessage case 2: udp talking error.\n");
    			exit(1);
    		}
    	}
    	else{     // else, report received the list
    		sprintf(sendbuf,"RECEIVED %s",pMSG->message);
    		sendmessage(tcpsockfd,sendbuf);
    	}
    	break;
    case 3:
    case 4:
    	// check whether the link cost is already stored
    	if(!isLinkStored(pMSG->linkinfo,p)){  //if it is not stored.
    		// stores it and broadcast it
    		updateLinkChange(pMSG->linkinfo,p);
    		if(!broadCastLinkInfo(pMSG->msgType, pMSG->linkinfo, p)){
    			printf("ERR dealWithUDPMessage: broadcastlinkinfo\n");
    		}
    	}
    	break;
    default:
    	break;
    }
    
    freeMSG(pMSG);
    freePATH(shortestPath,1);
    if(newMessage != NULL) free(newMessage);
    
}




int main(int argc, char *argv[])
{
	if (argc != 4) {
	    fprintf(stderr,"Usage: ./router localhost TCP UDP\n");
	    exit(1);
	}
	int tcpsockfd,udplistener, numbytes;  
	int i;
	struct LINK newLink;
	char buf[MAXDATASIZE];
	char* pBuf = NULL;
	char sendbuf[MAXDATASIZE];
	char tempbuf[MAXDATASIZE];
	struct timeval tval;			// time out for select()
	fd_set readfds;				// read fds for select()
	int ret;
	FILE *socket_stream_in;
	struct NetworkTopoStruct* networkTopo = initNetworkTopo();
	pGlobalTopo = networkTopo;
	/* initialize random seed: */
  	srand ( time(NULL) );
	// open tcp
	tcpsockfd = openTCPSocket(argv[1],argv[2]);
	socket_stream_in = fdopen(tcpsockfd, "r");
	// open the udp listener
	udplistener = openUDPListenningSocket(argv[3]);
	// get my address
	sendmessage(tcpsockfd,"HELO\n");
	numbytes = recv(tcpsockfd, buf, MAXDATASIZE-1, 0);
	buf[numbytes] = '\0';
	pBuf = buf+5;
	printf("manager replied with address %s",pBuf);
	storesMyAddress(buf,networkTopo); // Stores my address into the database.
	// register the host with udpport
	sprintf(sendbuf,"HOST localhost %s\n",argv[3]);
	sendmessage(tcpsockfd,sendbuf);
	receive(tcpsockfd,buf);  // this is OK
	// retrive the neighbour links
	sendmessage(tcpsockfd,"NEIGH?\n");
	do{
		if ( fgets(buf,MAXDATASIZE-1,socket_stream_in)!=NULL ){
		        printf("%s",buf);
		        if (isControlInfo(buf,"NEIGH")){
		        	storesNeighFromControlInfo(buf,networkTopo);		       // stores the neigh information
		        }
		        if (isControlInfo(buf,"DONE")) break;
		}
			
	}while(1);
	sendmessage(tcpsockfd,"READY\n");
	receive(tcpsockfd,buf);   // This is OK
	// Until here, the handshake is done
	// LOG Information
	sendmessage(tcpsockfd,"LOG ON\n");
	receive(tcpsockfd,buf);  // This is LOG ON replied from the server
	// initialize select() to do multiplexing I/O
	tval.tv_sec = 0;
	tval.tv_usec = 80000;
	FD_ZERO(&readfds);
	FD_SET(tcpsockfd,&readfds);  // input from the tcp
	FD_SET(udplistener,&readfds); // input from the udp
	
	//Broadcast the neighbour links to all nodes to set up the topology of the network
	for(i=0;i<networkTopo->linkNum;i++)
	{
		broadCastLinkInfo(3,networkTopo->Links[i],networkTopo);
	}
	
	// the loop for the select(), and dealing with the input from keyboard and the input from socket.
	while( (ret=select(max(tcpsockfd,udplistener)+1, &readfds, NULL, NULL, NULL)) >=0 )
	{
		if (FD_ISSET(tcpsockfd,&readfds))
		{
			if (receiveOneLine(socket_stream_in,buf)){
				// update and acknowledge link cost
				if (isControlInfo(buf,"LINKCOST")){
					// fetch the cost
					fetchToken(buf,3,tempbuf); // the cost is in tempbuf
					sprintf(sendbuf,"COST %s OK\n",tempbuf);
					sendmessage(tcpsockfd,sendbuf);
					newLink = storesLinkCostChangeFromControlInfo(buf,networkTopo);  // save the link cost change in the data structure and broadcast the change
					broadCastLinkInfo(4,newLink,networkTopo);
				}
				//break if received end
				if (isControlInfo(buf,"END")){
					sleep((rand()%200+100)/300);
					sendmessage(tcpsockfd,"BYE\n");
					break;	
				}
			}
		}
		if (FD_ISSET(udplistener,&readfds))
		{
			if((numbytes = receiveUDPMessage(udplistener,buf)) >0){
				dealWithUDPMessage(buf,numbytes, networkTopo,tcpsockfd);   // forward message or deal with control information between routers
			}
			
		}
		FD_ZERO(&readfds);
		FD_SET(tcpsockfd,&readfds);  // got message from the tcp
		FD_SET(udplistener,&readfds); // got message from the udp server input

	}
	if (ret==-1){
		perror("select()");		
		exit(1);
	}
	freeTopo(networkTopo); // release the networkTopo
	close(udplistener);
	close(tcpsockfd);

	return 0;
}

