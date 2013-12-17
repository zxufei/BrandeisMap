XUFEI ZHAO
======================================
Welcome to brandeis map project.
The project is well ansi c code and can be complied with
gcc -ansi -Wall -o Map Map.c
and execute with ./Map
==========================================
How my code works in Map.c

Graph adjacency list data structure:

I create a struct graph node. The graph node has three members: 
int vertex; //to store vertex index
int edgeIndex;// to store edge
struct graphNode *next;//next pointer point to the other graph Node.

I create a array of graphNode named map. Every map[i] store the vertex i and a next pointer point to a linked list of the all adjacency vertices j such that (i,j) is an edge and store edges in linked node member edgeIndex.

Heap data structure:

I design a heap node for heap data structure. There are two members for head node.
int vertex;//to store vertex index
int cost;//the cost associated to specific vertex for heap to sort, the smallest cost would be first in heap data structure.

There are three member functions for heap 
insertHeap(int vertex, int cost);
deleteMin();
updateHeap(int index, int cost);//given the heap index to update specific cost.

For updateHeap function, I also design a vertexToHeapMap array to record every vertice that map to the location in heap. Every insert, delete, swap operations to heap would affect location of vertice in vertexToHeapMap. As a result, when I want to update specific vertex’ cost, I just search vertexToHeapMap and get the index in heap.

Dijkstra algorithm:
First, begin vertex would be inserted into heap, and then search the adjacent map to retrieve all the adjacent vertices. Each vertices would be computed the smallest cost from beginning vertex.(if TimeFlag is true, the cost must be the time else it would be the distance). 
I define prevVertex data structure to store the previous vertex and edges index that reach the minimum cost path from the beginning vertex.
Then go back through the prevVertex data structures to create result linked list path. 
Finally, using PringLeg functions to print out result.
 



