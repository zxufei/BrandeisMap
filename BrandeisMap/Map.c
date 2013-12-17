/*Brandeis Map Program*/
/*NAME AND DATE GOES HERE.*/
/*XUFEI ZHAO NOV.23 2013*/

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>

/***************************************************************************************/
/*Global parameters.                                                                   */
/***************************************************************************************/

/*Path to the map folder (that is prepended to all file names).*/
char PATH[100] = "";/*/Users/xufeizhao/Documents/*/

/*File names*/
#define FileVertices    "MapDataVertices.txt" /*Map vertex data.*/
#define FileEdges       "MapDataEdges.txt"    /*Map edge data.*/
#define FilePath        "Route.txt"           /*Computed route points - feet.*/
#define FilePathCropped "RouteCropped.txt"    /*Computed route points - cropped pixels.*/

/*Limits*/
#define MaxString 100       /*Maximum length of any input string.*/
#define MaxLabel 5          /*Maximum length of a location label (includes ending \0).*/
#define MaxVertex 175       /*Maximum number of vertices.*/
#define MaxEdge   600       /*Maximum number of edges.*/
#define UndefinedIndex -1   /*All array insides start at 0; -1 is like a nil pointer.*/
#define InfiniteCost  10000 /*Cost of an edge that does not exist.*/


/***************************************************************************************/
/*MAP DATA                                                                             */
/***************************************************************************************/

/*Vertex data.*/
int  nV=0;                        /*Number of vertices.*/
int  Vindex[MaxVertex];           /*Vertex index.*/
char Vlabel[MaxVertex][MaxLabel]; /*Vertex label - each at most 3 chars plus the \0.*/
int  VX[MaxVertex];               /*Vertex X-coord. in feet from upper left corner.*/
int  VY[MaxVertex];               /*Vertex Y-coord. in feet from upper left corner.*/
char *Vname[MaxVertex];           /*Vertex name - pointer to an allocated string.*/

/*Edge data.*/
int  nE=0;              /*Number of edges.*/
int  Eindex[MaxEdge];   /*Edge index.*/
int  Estart[MaxEdge];   /*Edge start (a vertex number).*/
int  Eend[MaxEdge];     /*Edge end (a vertex number).*/
int  Elength[MaxEdge];  /*Edge length in feet.*/
int  Eangle[MaxEdge];   /*Edge angle in degrees clockwise from north.*/
char Edir[MaxEdge][6];  /*Edge direction - one of North, NE, East, ..., NW*/
char Ecode[MaxEdge];    /*S = straight, U = up hill, D = down hill.*/
char *Ename[MaxEdge];   /*Edge name - pointer to an allocated string.*/

/*User input for current request.*/
int Begin, Finish; /*Vertex indices of begin and finish locations.*/
int BoardFlag;     /*1 if have skateboard*/
int TimeFlag;      /*1 to minimize time instead of distance*/

/***************************************************************************************/
/*FUNCTION Time(i) TO RETURN THE NUMBER OF SECONDS TO TRAVERSE AN EDGE i               */
/***************************************************************************************/
#include "MapTime.h" /*To be used by Dijkstra's algorithm.*/

/***************************************************************************************/
/*INPUT / OUTPUT FUNCTIONS                                                             */
/***************************************************************************************/
#include "MapInputData.h" /*Used by GetVertices, GetEdges (called by main).*/
#include "MapInputUser.h" /*Used by GetRequest (called by main).*/
#include "MapOutput.h"    /*PrintLeg and PrintSummary functions used by Dijkstra.*/



/***************************************************************************************/
/*GRAPH ADJACENCY LIST DATA STRUCTURE                                                  */
/***************************************************************************************/
struct graphNode{
    int vertex;
    int edgeIndex;
    struct graphNode * next;
};
struct graphNode * map;  /*Adjacent list data structure. Array of graphNode*/

void initMap(){
    map = (struct graphNode*)malloc(sizeof(struct graphNode)*nV);
    int i=0;
    for(i=0; i<nV; i++){
        map[i].vertex = Vindex[i];
        map[i].next = NULL;/*address of graphNode*/
    }
}

void createAdjList(){
    struct graphNode * ptr=NULL,*ptNext=NULL;
    int startVertex,endVertex;
    int i=0;
    for (i=0; i<nE; i++){
        ptr = (struct graphNode*)malloc(sizeof(struct graphNode));
        startVertex = Estart[Eindex[i]];
        endVertex = Eend[Eindex[i]];
        ptr->edgeIndex = Eindex[i];
        ptr->vertex = endVertex;
        ptr->next = NULL;
        if (map[startVertex].next == NULL){
            map[startVertex].next = ptr;
        }
        else{
            ptNext = map[startVertex].next;
            while (ptNext->next!=NULL) {
                ptNext = ptNext->next;
            }
            ptNext->next = ptr;
        }
    }
}

void printAdjList(int n){
    
    struct graphNode* ptr;
    int i=0;
    for (i=0; i<n; i++){
        printf("  map[%3d]:",i);
        ptr = map[i].next;
        while (ptr!=NULL) {
            printf(" %3d,%3d",ptr->vertex,ptr->edgeIndex);
            ptr = ptr->next;
        }
        printf("\n");
    }
    printf("\n");
}

void testAdj(){
    initMap();
    createAdjList();
    printAdjList(nV);
}

/***************************************************************************************/
/*HEAP DATA STRUCTURE                                                                  */
/***************************************************************************************/
struct heapNode{
    int vertex;
    int cost;
};
struct heapNode * heap;           /*heap data structure*/
int nextRB;                       /*record next location of new heap node*/
int vertexToHeapMap[MaxVertex];   /*record vertex number map to such vertex's location in heap*/

void initVHMap(){
    int i=0;
    for (i=0; i<MaxVertex; i++){
        vertexToHeapMap[i]=UndefinedIndex;
    }
}

void initHeap(){
    nextRB = 0;
    heap = (struct heapNode*)malloc(sizeof(struct heapNode)*nV);
    int i=0;
    for (i=0; i<nV; i++) {
        heap[i].vertex = UndefinedIndex;
        heap[i].cost = InfiniteCost;
    }
}

/*swap location of two given index of heap node*/
void swapHeap(int index1, int index2){
    struct heapNode tmp = heap[index1];
    heap[index1] = heap[index2];
    heap[index2] = tmp;
    /*also change location record*/
    vertexToHeapMap[heap[index1].vertex] = index1;
    vertexToHeapMap[heap[index2].vertex] = index2;
}

void percup(int index){
    while (index > 0 && heap[index].cost < heap[(index-1)/2].cost) {
        swapHeap(index, (index-1)/2);
        index = (index-1)/2;
        
    }
}

void insertHeap(int vertex,int cost){
    if (nextRB >= nV) printf("ERROR: Heap is full.");
    else{
        struct heapNode n;
        n.vertex = vertex;
        n.cost = cost;
        heap[nextRB] = n;
        
        vertexToHeapMap[vertex] = nextRB;/*record location*/
        
        percup(nextRB);
        nextRB++;
    }
}

void percdown(int index){
    int min_index;
    while (!(heap[2*index+1].vertex==-1 && heap[2*index+2].vertex==-1)
           && (heap[index].cost > heap[2*index+1].cost || heap[index].cost > heap[2*index+2].cost)) {
        if (heap[2*index+1].vertex !=-1 && heap[2*index+2].vertex != -1) {
            if (heap[2*index+1].cost < heap[2*index+2].cost) {
                min_index = 2*index+1;
            }
            else {
                min_index = 2*index+2;
            }
        }
        else if (heap[2*index+1].vertex!=-1){
            min_index = 2*index+1;
        }
        else{
            min_index = 2*index+2;
        }
        swapHeap(index, min_index);
        index = min_index;
    }
}

void deleteMin(){
    if (nextRB <= 0) printf("ERROR: heap is empty.\n");
    else{
        nextRB--;
        swapHeap(0, nextRB);
        vertexToHeapMap[heap[nextRB].vertex] = UndefinedIndex;/*delete record*/
        heap[nextRB].vertex=UndefinedIndex;
        heap[nextRB].cost = InfiniteCost;
        percdown(0);
    }
}

void updateHeap(int index,int cost){
    heap[index].cost = cost;/*only would become minimum*/
    percup(index);
}

void testHeap(){
    initHeap();
    initVHMap();
    insertHeap(5, 10);
    insertHeap(6, 9);
    insertHeap(7, 11);
    int i=0;
    for (i=4; i>0; i--) {
        insertHeap(i, i);
    }
    updateHeap(vertexToHeapMap[7],2);
    updateHeap(vertexToHeapMap[5], 6);
    for (i=0; i<11; i++){
        if (nextRB>0){
        printf("vertex:%d cost:%d\n",heap[0].vertex,heap[0].cost);
        deleteMin();
        }
    }
    
}

/***************************************************************************************/
/*Dijkstra's Single-Source Shortest Path Algorithm                                     */
/***************************************************************************************/

/*Do Dijkstra algorithm, reverse back edges, print path with PrintLeg.*/
int * cost;
int * visited;
struct prevVertex{
    int pre;
    int edge;
};
struct prevVertex *previous;/*Record previous of vertex and corresponding edges*/

void initDijstra(){
    cost = (int*)malloc(sizeof(int)*nV);
    visited = (int *)malloc(sizeof(int)*nV);
    previous = (struct prevVertex*)malloc(sizeof(struct prevVertex)*nV);
    int i=0;
    for (i=0; i<nV; i++){
        cost[i] = InfiniteCost;
        visited[i] = 0;
        previous[i].pre = UndefinedIndex;
        previous[i].edge = UndefinedIndex;
    }
}
/*according to the ecode to determine speed value*/
/*
double speedSelect(char c){
    double speed = 0.0;
    switch (c) {
        case 'f':
            speed = WalkSpeed;
            break;
        case 'd':
            speed = WalkSpeed*WalkFactorD;
            break;
        case 'u':
            speed = WalkSpeed*WalkFactorU;
            break;
        case 's':
            speed = WalkSpeed*StepFactorU;
            break;
        case 't':
            speed = WalkSpeed*StepFactorD;
            break;
        case 'b':
            speed = WalkSpeed*BridgeFactor;
            break;
        case 'F':
            if (BoardFlag) speed = WalkSpeed*SkateFactorF;
            else speed = WalkSpeed;
            break;
        case 'U':
            if (BoardFlag) speed = WalkSpeed*SkateFactorU;
            else speed = WalkSpeed*WalkFactorU;
            break;
        case 'D':
            if (BoardFlag) speed = WalkSpeed*SkateFactorD;
            else speed = WalkSpeed*WalkFactorD;
            break;
        default:
            break;
    }
    return speed;
}*/

void Dijkstra() {
    initDijstra();
    initMap();
    createAdjList();
    initHeap();
    initVHMap();
    
    /*Begin dijstra*/
    cost[Begin]=0;
    insertHeap(Begin, 0);
    while (nextRB > 0) {
        struct heapNode u = heap[0];
        deleteMin();
        visited[u.vertex] = 1;
        
        struct graphNode* v = map[u.vertex].next;
        while (v) {
            int vEdgeIndex = v->edgeIndex;
            int vVertext = v->vertex;
            int alt=0;
            if (!TimeFlag) {
                alt = cost[u.vertex] + Elength[vEdgeIndex];
            }else{
          /*      int speed = speedSelect(Ecode[vEdgeIndex]);*/
          /*      alt = cost[u.vertex] + ((int)(((double)Elength[vEdgeIndex]*60)/speed+0.5));*/
                alt = cost[u.vertex] + Time(vEdgeIndex);
            }
            if (alt < cost[vVertext] && !visited[vVertext]){
                cost[vVertext] = alt;
                previous[vVertext].pre = u.vertex;
                previous[vVertext].edge = vEdgeIndex;
                if (vertexToHeapMap[vVertext] == UndefinedIndex) {
                    insertHeap(vVertext, cost[vVertext]);
                }
                else{
                    updateHeap(vertexToHeapMap[vVertext], cost[vVertext]);
                }
            }
            v = v->next;
        }
    }
    /*reverse back edge*/
    int endIndex = Finish;
    /*printf("%d\n",endIndex);*/
    struct paths{
        int edge;
        struct paths * next;
    };
    struct paths * result = NULL;
    while (previous[endIndex].pre != UndefinedIndex) {
        /*create a stack like structure to reverse previous edge*/
        struct paths * tmp = (struct paths*)alloca(sizeof(struct paths));
        tmp->edge = previous[endIndex].edge;
        tmp->next = result;
        result = tmp;
        endIndex = previous[endIndex].pre;
    }

    
    /*print the path with PrintLeg. */
    while (result) {
        PrintLeg(result->edge, Time(result->edge));
        result = result ->next;
    }
        
}


/***************************************************************************************/
/*START OF PROGRAM                                                                     */
/***************************************************************************************/
int main() {
    GetVertices();
    GetEdges();
    while (GetRequest())
    {RouteOpen(); Dijkstra(); RouteClose();}
    return(0);
}
