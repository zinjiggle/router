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


// input: nodeNum, probability of a link
int main(int argc, char *argv[])
{
	// input: nodeNum, probability of a link between two nodes, maximam cost
	int	nodeNum = atoi(argv[1]);
	float	prob = atof(argv[2]);
	int	maxCost = 50;
	
	int** graph = (int**)malloc(sizeof(int*)*nodeNum);
    int i,j;
    int linkNum = 0;
    int src,dest;
    FILE *pFile;
    for(i=0;i<nodeNum;i++)
        graph[i] = (int*) malloc(sizeof(int)*nodeNum);
    srand ( time(NULL) );
	
	for(i=0;i<nodeNum;i++)
		for(j=0;j<nodeNum;j++){
			if(rand()%100<=prob*100) {
				graph[i][j] = rand()%(maxCost-1)+1;
				linkNum++;
			}
			else graph[i][j]=-1;
		}
	
	fprintf(stdout,"Num of links: %d\n", linkNum);
	pFile = fopen("Random_Graph.xml","w");
	fprintf(pFile,"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
	fprintf(pFile,"<Simulation>\n");
	fprintf(pFile,"\t<Topology>\n");
	
	for(i=0;i<nodeNum;i++)
		for(j=i+1;j<nodeNum;j++){
			if(graph[i][j]>0){
				fprintf(pFile,"\t\t<Link>\n");
				fprintf(pFile,"\t\t\t<node>%d</node>\n",i+1);
				fprintf(pFile,"\t\t\t<node>%d</node>\n",j+1);
				fprintf(pFile,"\t\t\t<cost>%d</cost>\n",graph[i][j]);
				fprintf(pFile,"\t\t</Link>\n");
			}
		}
	fprintf(pFile,"\t</Topology>\n");
	
	// events, 5 random message, 2 random link change, 2 random link failure
	char* message[5] = {
	"aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa",
	"bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb",
	"ccccccccccccccccccccccccccccccc",
	"ddddddddddddddddddddddddddddddd",
	"eeeeeeeeeeeeeeeeeeeeeeeeeeeeeee"
	};
	fprintf(pFile,"\t<Events>\n");
	for(i=0;i<5;i++){
		src = rand()%nodeNum+1;
		while((dest = rand()%nodeNum+1) ==src){};
		fprintf(pFile,"\t\t<Message>\n");
		fprintf(pFile,"\t\t\t<src>%d</src>\n",src);
		fprintf(pFile,"\t\t\t<dst>%d</dst>\n",dest);
		fprintf(pFile,"\t\t\t<data>%s</data>\n",message[i]);
		fprintf(pFile,"\t\t</Message>\n");
	}
	
	fprintf(pFile,"\t</Events>\n");
	
	fprintf(pFile,"</Simulation>\n");
	fclose(pFile);
    
    pFile = fopen("ports.txt","w");
    int initUdp = rand()%2000 + 4000;
    for(i=0;i<nodeNum;i++)
    {
        fprintf(pFile,"%d\n",initUdp+i);
    }
    fclose(pFile);
    
    
	return 0;
}

