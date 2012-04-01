//
//  topology.h
//  
//
//  Created by Zhu Ji on 3/27/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#ifndef _topology_h
#define _topology_h

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


struct LINK{
    int nodeAddr[2]; // node1 and node2
    float cost;     // link cost
};

struct PATH{
    int dest;  // destination node
    int nexthop; // the next hop node
    float cost;  // the cost to the destination
    int numHops; // the number of hops, including src itself.
    int* pathNodes; // the pathNodes: src node1 node2 node3 ...dest
};

void printPATH(struct PATH * p)
{
	int i;
	printf("dest:%d, nexthop:%d, cost: %.0f, numhops:%d,\n",p->dest,p->nexthop,p->cost,p->numHops);
	printf("pathNodes: ");
	for(i=0;i<p->numHops;i++) printf("%d ",p->pathNodes[i]);
	printf("\n");
}

void printARRAY(void* p, int num, char type)
{
	int i;
	int * qint;
	float * qfloat;
	switch(type)
	{
	case 'i':
		 qint = (int*) p;
		for(i=0;i<num;i++) printf("%d ",qint[i]);
		printf("\n");
		break;
	case 'f':
		 qfloat = (float *) p;
		for(i=0;i<num;i++) printf("%.0f ",qfloat[i]);
		printf("\n");
		break;
	default:
		break;
	}
}

struct NEIGHBOUR{
    int addr;
    char host[30];
    char udpport[30];
    float cost;
};


struct NetworkTopoStruct{
	int nodeNum;
	int myaddr;  // my address, returned by the server.
    
    	int pathNum;
    	int maxPathNum;
    	struct PATH* Paths;      // stores all the paths calculated by the router.
	
	int neighNum;
	int maxNeighNum;
	struct NEIGHBOUR* Neighs;   // stores the neigh information, maximum MAXDATASIZE number of neighbours
	
	int linkNum;
	int maxLinkNum;
	struct LINK* Links;    // stores the link cost information
};

void clean(char* buf)
{
	buf[0] = '\0';
}

struct PATH* allocPATH(int size)
{
	struct PATH *p = (struct PATH*) malloc(sizeof(struct PATH)*size);
	memset(p,0,sizeof(struct PATH)*size);
	return p;
}

void freePATH(struct PATH* p, int size)
{
	if (p==NULL) return;
	int i;
	for (i=0;i<size;i++)
	{
		if(p[i].pathNodes != NULL) free(p[i].pathNodes);
	}
	free(p);
}

void copyPATH(struct PATH* dest, struct PATH* src)
{
	memcpy(dest,src,sizeof(struct PATH));
	if(src->numHops!=0){
		dest->pathNodes = (int*) malloc(sizeof(int)*src->numHops);
		memcpy(dest->pathNodes,src->pathNodes,sizeof(int)*src->numHops);
	}
}




struct NetworkTopoStruct* initNetworkTopo()
{
	struct NetworkTopoStruct* p = (struct NetworkTopoStruct*) malloc(sizeof(struct NetworkTopoStruct));
    memset(p,0,sizeof(struct NetworkTopoStruct));
	p->nodeNum = 1;
    
    p->maxPathNum = MAXDATASIZE;
    p->maxNeighNum = MAXDATASIZE;
    p->maxLinkNum = MAXDATASIZE;
    
    p->Paths = (struct PATH*)malloc(sizeof(struct PATH)*p->maxPathNum);
    memset(p->Paths,0,sizeof(struct PATH)*p->maxPathNum);
    /*
    for(i=0;i<p->maxPathNum;i++){
        p->Paths[i].maxHopNodes = MAXHOPNUM;
        p->Paths[i].pathNodes = (int*) malloc(sizeof(int)*p->Paths[i].maxHopNodes);
        memset(p->Paths[i].pathNodes,0,sizeof(int)*p->Paths[i].maxHopNodes);
    }*/
    
    p->Neighs =(struct NEIGHBOUR*)malloc(sizeof(struct NEIGHBOUR)*p->maxNeighNum);
    memset(p->Neighs,0, sizeof(struct NEIGHBOUR)*p->maxNeighNum);
    
    p->Links = (struct LINK*) malloc(sizeof(struct LINK)*p->maxLinkNum);
    memset(p->Links,0,sizeof(struct LINK)*p->maxLinkNum);
	
	return p;
}

void freeTopo(struct NetworkTopoStruct* p)
{
    int i;
    free(p->Links);
    free(p->Neighs);
    for(i=0;i<p->maxPathNum;i++){
        if(!p->Paths[i].pathNodes) free(p->Paths[i].pathNodes);
    }
    free(p->Paths);
    free(p);
}

// result and buf must have already been allocated
char* fetchToken(const char *buf, int num, char* result)
{
	char temp[MAXDATASIZE];
	strcpy(temp,buf);
	char* pch = strtok(temp," \n");
	int i = 0;
	for(i=0;i<num;i++)
	{
		if (pch != NULL){
			pch = strtok(NULL," \n");
		}
		else return 0;
	}
	strcpy(result,pch);
	return result;
}

void storesMyAddress(char* buf,struct NetworkTopoStruct* p)
{
	char rv[20];
	if(!isStringContain(buf,"ADDR")){
		perror("ADDR Err: not ADDR info");
		exit(1);
	}
	else{
		p->myaddr = atoi(fetchToken(buf,1,rv));
	}
}

void* addNewSize(int *max,void** array, int unitSize)
{
    int oldSize = unitSize * (*max);
    int oldMax = *max;
    void* temp = *array;
    *max = *max + ONETIMEALLOC;
    int newSize = unitSize*(*max);
    void* newArray = malloc(newSize);
    memset(newArray,0,newSize);
    if(oldSize>0) memcpy(newArray,*array,oldSize);   // this already does initialization;
    *array = newArray;
    return temp;
    
}

void addPATHSize(struct NetworkTopoStruct* p)
{
    printf("HERE: add path size, new paths not initialized");
    int oldMax = p->maxPathNum;
    int i;
    struct PATH* temp =(struct PATH*)addNewSize(&p->maxPathNum,(void**)&p->Paths,sizeof(struct PATH));
    // free temp
    for(i=0;i<oldMax; i++){
        if(!temp[i].pathNodes) free(temp[i].pathNodes);
    }
    free(temp);
}

void addNEISize(struct NetworkTopoStruct* p){
    printf("HERE: add NEIGHBOUR List size");
    struct NEIGHBOUR* temp =(struct NEIGHBOUR*)addNewSize(&p->maxNeighNum,(void**)&p->Neighs,sizeof(struct NEIGHBOUR));
    free(temp);
}

void addLINKSize(struct NetworkTopoStruct* p){
    printf("HERE: add LINK size");
    struct LINK* temp =(struct LINK*)addNewSize(&p->maxLinkNum,(void**)&p->Links,sizeof(struct LINK));
    free(temp);
}

/*
void addHOPSize(struct PATH* pa){
    printf("HERE: add HOP size");
    int* temp = (int*)addNewSize(&pa->maxHopNodes,(void**) &pa->pathNodes,sizeof(int));
    free(temp);
}
*/

bool isNodeInLink(int addr, struct LINK link)
{
	if(addr == link.nodeAddr[0] || addr == link.nodeAddr[1]) return 1;
	else return 0;
}


bool isStringContain(const char* src, const char* dst)
{
	if(strncmp(src,dst,strlen(dst))==0) return 1;
	else return 0;
}

// inserts "addr localhost updport cost" into Neighs
void insertNeighInfo(struct NEIGHBOUR newNei, struct NetworkTopoStruct* p)
{
	//printf("HERE: Insert New Neigh Info!!\n");
	if (p->neighNum + 5 > p-> maxNeighNum) addNEISize(p);  // allocate new size if not enough.
	
	int i,copyPos=-1;
	for (i=0;i<p->neighNum;i++){
		if (newNei.addr == p->Neighs[i].addr){
			printf("Notice: Neigh Info Already Stores!\n");
			copyPos = i;
		}
	}
	if (copyPos == -1){
		copyPos = p->neighNum;
		p->neighNum++;
	}
	printf("HERE: New Neigh Info Is Inserted Into %d\n",copyPos);
	// insert
	p->Neighs[copyPos] = newNei;
}

void sort2(int* p)
{
    int k;
    if(p[0]>p[1]){
        k = p[1]; p[1]=p[0];p[0]=k;
    }
}

/* return: 0 - link is not stored
	   1 - link is already stored
*/
bool isLinkStored(struct LINK newLink, struct NetworkTopoStruct* p){
	int i;
	sort2(newLink.nodeAddr);
	for (i=0;i<p->linkNum;i++){
		if (memcmp(newLink.nodeAddr,p->Links[i].nodeAddr,sizeof(int)*2)==0){
			//printf("isLinkStored: found the same nodes, newlink cost: %.0f Stored cost: %.0f\n",newLink.cost, p->Links[i].cost);
			if( abs(newLink.cost - p->Links[i].cost) < 0.01){
				//printf("isLinkStored: found the same cost\n");
				return 1;
			}
		}
	}
	return 0;
}

void insertLinkInfo(struct LINK newLink, struct NetworkTopoStruct* p)
{
    	//printf("insertLinkInfo: Insert New Link Info!!\n");
    	sort2(newLink.nodeAddr);  // sort the information
    	if (p->linkNum + 5 > p->maxLinkNum) addLINKSize(p);
    	int i,copyPos=-1;
	for (i=0;i<p->linkNum;i++){
		if (memcmp(newLink.nodeAddr,p->Links[i].nodeAddr,sizeof(int)*2)==0){
			if( abs(newLink.cost - p->Links[i].cost) < 0.01){
				printf("insertLinkInfo: Link Info Already Exists!\n");
				return;
			}
			else{
				printf("insertLinkInfo: Link Cost Will be Update!\n");
			}
			copyPos = i;
		}
	}
	if (copyPos == -1){
		copyPos = p->linkNum;
		p->linkNum++;
	}
	printf("%d insertLinkInfo: Inserted Into %d\n",p->myaddr,copyPos);
	p->Links[copyPos] = newLink;
}


int storesNeighFromControlInfo(const char* buf,struct NetworkTopoStruct* networkTopo)
{
	// fetch the neigh information from buf and stores into the networkTopo, print err if there are some
	// storage form: addr localhost udpport cost
	char rv[50];
    struct NEIGHBOUR   newNei;
    struct LINK        newLink;
	if(!isStringContain(buf,"NEIGH")){
		perror("storeNeigh Err: not neigh info");
		exit(1);
	}
	else {
        // update neigh information
        newNei.addr = atoi(fetchToken(buf,1,rv));
        strcpy(newNei.host,fetchToken(buf,2,rv));
        if (isStringContain(newNei.host,"localhost")) strcpy(newNei.host,"127.0.0.1");
        strcpy(newNei.udpport,fetchToken(buf,3,rv));
        newNei.cost = atof(fetchToken(buf,4,rv));
        insertNeighInfo(newNei,networkTopo);
        
        // update link information
        newLink.nodeAddr[0] = networkTopo->myaddr;
        newLink.nodeAddr[1] = newNei.addr;
        sort2(newLink.nodeAddr);
        newLink.cost = newNei.cost;
        insertLinkInfo(newLink,networkTopo);
	}
	
	return 1;
	
}

int fetchNeiIdByAddr(struct NetworkTopoStruct* p, int addr)
{
    int i;
    for(i=0;i<p->neighNum;i++){
        if(p->Neighs[i].addr == addr) return i;
    }
    return -1;
}

struct NEIGHBOUR* fetchNeiPointerByLink(struct LINK newLink, struct NetworkTopoStruct *p)
{
	int addr;
	int neiID;
	if(p->myaddr == newLink.nodeAddr[0] || p->myaddr == newLink.nodeAddr[1]){
            addr = (p->myaddr == newLink.nodeAddr[0]) ? (newLink.nodeAddr[1]):(newLink.nodeAddr[0]);
            if ((neiID = fetchNeiIdByAddr(p,addr))>=0){
            	return &p->Neighs[neiID];
            }
        }
	return NULL;
}

// "LINKCOST node1 node2 cost"
struct LINK storesLinkCostChangeFromControlInfo(const char*buf, struct NetworkTopoStruct* p)
{
    char rv[50];
    struct LINK        newLink;
    struct NEIGHBOUR	newNei;
    struct NEIGHBOUR *  pNei;
    int         addr;
    int         neiID;
	if(!isStringContain(buf,"LINKCOST")){
		perror("LINKCOST Err: not LINKCOST info");
		exit(1);
	}
	else
    {
        newLink.nodeAddr[0] = atoi(fetchToken(buf,1,rv));
        newLink.nodeAddr[1] = atoi(fetchToken(buf,2,rv));
        sort2(newLink.nodeAddr);
        newLink.cost = atof(fetchToken(buf,3,rv));
        insertLinkInfo(newLink,p);  // insert links
        if((pNei=fetchNeiPointerByLink(newLink,p))!=NULL)
        {
        	newNei = *pNei;
        	newNei.cost = newLink.cost;
        	insertNeighInfo(newNei,p);
        }
        else{
        	perror("ERR: LINKCOST nodes are not found in Neighbours");
                exit(1);
        }
    }
    return newLink;
}






#endif
