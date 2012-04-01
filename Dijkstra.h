/*
	Dijkstra by Ji Zhu

*/

#include "globalhelper.h"
#include "topology.h"


int insertIntoIntArray(int value, int* array, int* pMax)
{
	int i=0;
	for(i=0; i<*pMax; i++){
		if (value == array[i]) return i;
	}
	array[i] = value; // insert
	*pMax +=1;	// update the total number of nodes inserted.
	return i;
}

int searchIntArray(int value, int* array, int max)
{
	int i;
	for(i=0;i<max;i++)
	{
		if(array[i]==value) return i;
	}
	return -1;
}

float add(float a, float b)
{
	if (a > -0.1 && b> -0.1) return a+b;
	else return -1;
}

// if a>b, return 1, else if a<=b return 0;
bool compare(float a, float b)
{
	if (a > -0.1 && b> -0.1) return (a>b) ? 1:0;
	if (b <= -0.1) return 0;
	return 1;
}

bool isInf(float a)
{
	return (a<=-0.5)?1:0;
}

bool reArrange(int place, int value, int* array, int max)
{
	int i;
	for(i=0;i<max;i++)
	{
		if(array[i]==value)
		{
			array[i] = array[place];
			array[place] = value;
			return 1;
		}
	}
	return 0;
}

bool switchvalue(void* p, void* q, int byte)
{
	void * temp = malloc(byte);
	memcpy(temp,p,byte);
	memcpy(p,q,byte);
	memcpy(q,temp,byte);
	free(temp);
}

float fetchLinkCost(int node1, int node2, struct LINK* pLink, int numLinks)
{
	int i;
	for(i=0;i<numLinks;i++)
	{
		if (node1 == pLink[i].nodeAddr[0] && node2 == pLink[i].nodeAddr[1]) return pLink[i].cost;
		if (node1 == pLink[i].nodeAddr[1] && node2 == pLink[i].nodeAddr[0]) return pLink[i].cost;
	}
	return -1;
}

// implement Dijkstra Algo, input: src, numOfLinks in pLINK
// output: the PATH list to all dest node, the *resultPathNum shows the number of paths in path list
struct PATH* calDijkstra(int* resultPathNum, int src, int numLinks, struct LINK* pLink)
{
	if (numLinks == 0 || pLink==NULL || src < 0) return NULL;
	
	int numNodes = 0,i=0,j=0,k=0;
	int lenNodeList = numLinks;
	int *nodeList = (int*)malloc(sizeof(int)*numLinks);
	memset(nodeList,0,sizeof(int)*numLinks);
	int *tempNodeList = (int*)malloc(sizeof(int)*numNodes);
	memset(tempNodeList,0,sizeof(int)*numNodes);
	int tempNodeListNum = 0;
	struct PATH* paths =NULL;
	float * dist = (float*) malloc(sizeof(float)*numNodes);
	int * previous = (int*)malloc(sizeof(int)*numNodes);
	int * Q = (int*) malloc(sizeof(int)*numNodes);
	int numQ = 0;
	float alt;
	int u;
	
	
	// fetch the number of nodes from the link information
	for(i=0;i<numLinks;i++)
	{
		insertIntoIntArray(pLink[i].nodeAddr[0], nodeList, &numNodes);
		insertIntoIntArray(pLink[i].nodeAddr[1], nodeList, &numNodes);
	}
	if(searchIntArray(src,nodeList,numNodes) < 0) return NULL;
	else reArrange(0, src, nodeList, numNodes);  // put the src in the first position of nodeList
	/* debug
	printf("Dijkstra: node list is ");
	printARRAY(nodeList,numNodes,'i');
	*/
	for(i=0;i<numNodes;i++)
	{
		dist[i] = -1;      // dist is infinity
		previous[i] = -1; // undefined previous pointer
		Q[i] = 0;         // All points are in Q set
	}
	dist[0] = 0;
	numQ = numNodes;
	//for(i=1;i<numNodes;i++) dist[i] = fetchLinkCost(nodeList[0],nodeList[i], pLink,numLinks);
	//printf("Dijkstra numQ: %d\n",numQ);
	while(numQ>0)
	{
		// u := vertex in Q with smallest distance in dist[] ;
		u=-1;
		for (i=0;i<numNodes;i++)
		{
			if(Q[i]==0)
			{
				if(u==-1) u=i;
				else if( compare(dist[u],dist[i]) ) u=i;
			}
		}
		if(isInf(dist[u])) break;
		//printf("Dijkstra: u is %d\n", u);
		// remove u from Q
		Q[u] = 1;
		numQ--;
		// for each link contains u in Q
		for(i=0;i<numNodes;i++)
		{
			if(Q[i]==0)  // Add if nodeList[i] is the neighbour of nodeList[u]
			{
				alt = add(fetchLinkCost(nodeList[u],nodeList[i],pLink,numLinks), dist[u]);
				if(compare(dist[i],alt))
				{
					//update
					dist[i] = alt;
					previous[i] = u;
				}
			}
		}
	}
	/*debug
	printf("Dijkstra: dist is ");
	printARRAY(dist, numNodes,'f');
	printf("Dijkstra: previous is ");
	printARRAY(previous, numNodes,'i');
	*/
	// write the result dist and previous into path information
	paths = allocPATH(numNodes-1); // allocate storage for paths
	for (i=0;i<numNodes-1;i++){
		j = i+1;
		paths[i].dest = nodeList[j];
		paths[i].cost = dist[j];
		u = j;
		tempNodeListNum = 0;
		while(u != -1){
			tempNodeList[tempNodeListNum] = nodeList[u];
			tempNodeListNum++;
			u = previous[u];
		}
		if(tempNodeList[tempNodeListNum-1] == src)
		{
			paths[i].pathNodes = (int*) malloc(sizeof(int)*tempNodeListNum);
			for(k = 0;k<tempNodeListNum;k++){
				paths[i].pathNodes[k] = tempNodeList[tempNodeListNum-k-1];
			}
			paths[i].numHops = tempNodeListNum;
			paths[i].nexthop = paths[i].pathNodes[1];
		}
		/*debug
		printf("Dijkstra path to %d is ",paths[i].dest);
		printPATH(&paths[i]);
		*/
		
	}
	*resultPathNum = numNodes-1;
	
	free(tempNodeList);
	free(dist);
	free(previous);
	free(Q);
	free(nodeList);
	return paths;
}





// find the shortest path to dest, stores the result in "result"
// input: result: a pointer to a single PATH
int CalculateShortestPath(struct PATH* result, int dest,struct NetworkTopoStruct* Topo)
{
	if (result==NULL) return;
	int pathNum=0;
	int i=0;
	struct PATH* DijResult = calDijkstra(&pathNum,Topo->myaddr,Topo->linkNum, Topo->Links);
	for (i=0;i<pathNum;i++)
	{
		if(DijResult[i].dest == dest)
		{
			copyPATH(result,&DijResult[i]);
			printf("CalculateShortestPath: result path to %d is",dest);
			printPATH(result); // debug
			break;
		}
	}
	
	freePATH(DijResult,pathNum);
	if(i==pathNum) {
		printf("ERR CalculateShortestPath: %d not found in the Dij Result\n",dest);
		return 0;
	}
	else if(isInf(result->cost))
	{
		printf("CalculateShortestPath: no finite cost routing to %d\n",dest);
		return -1;
	}
	else return 1;
}
