#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define INT_MAX 1000000
#define FILE_SIZE 30000
#define MAX_STATION_NAME_LENGTH 30

//Edges are the connections between stations
typedef struct Edge {
    char* name;
    double distance;
    double bestTime;
    double peakTime;
    double interPeakTime;
    struct Edge *next;
} Edge;

//Nodes are the stations themselves
typedef struct Node {
    char* name;
    int numberOfLines;
    int changeHere;
    char** lines;
    char* path;
    char* currentLine;
    double displacement; // Value used to compare shortest displacement
    double distanceDisplacement;  // Keeps track of other variables to print out at the end
    double timeDisplacementBest;
    double timeDisplacementPeak;
    double timeDisplacementInterPeak;
    int visited;
    Edge *edgeHead;
    struct Node *next;
} Node;

char* concatonate3Strings(char* destination,char* source1, char* source2) {
    char* newstring = malloc(strlen(destination)+strlen(source1)+strlen(source2)+1);
    strcpy(newstring,destination);
    if (strcmp(destination,"") != 0) {
        strcat(newstring,source1);
    }
    strcat(newstring,source2);
    return newstring;
}

Node* createNode(char* name,char** lines,int numberOfLines) {
    Node *node = (Node*) malloc(sizeof(Node));
    node->name = strdup(name);
    node->numberOfLines = numberOfLines;
    node->changeHere = 0;
    node->lines = lines;
    node->path = NULL;
    node->currentLine = "";
    node->displacement = INT_MAX;
    node->distanceDisplacement = 0;
    node->timeDisplacementBest = 0;
    node->timeDisplacementPeak = 0;
    node->timeDisplacementInterPeak = 0;
    node->visited = 0;
    node->edgeHead = NULL;
    return node;
}

Edge* createEdge(char* name, double distance, double bestTime, double peakTime, double interPeakTime) {
    Edge *edge = (Edge*) malloc(sizeof(Edge));
    edge->name = strdup(name);
    edge->distance = distance;
    edge->bestTime = bestTime;
    edge->peakTime = peakTime;
    edge->interPeakTime = interPeakTime;
    return edge;
}

Node* appendNode(Node *node,Node *head) {
    node->next = head;
    head = node;
    return head;
}

Edge* appendEdge(Edge *edge,Edge *head) {
    edge->next = head;
    head = edge;
    return head;
}

char* readFile(char* directory) {
    FILE *file;
    char *rawText = malloc(FILE_SIZE),fileText[FILE_SIZE];
    file = fopen(directory,"r");
    while (fgets(fileText,FILE_SIZE,file) != NULL) {
        strcat(rawText,fileText);
    }
    fclose(file);

    return rawText;
}

//The lines are a string of text delimited by '/'. This function gets the number of lines.
int getListSize(char* text,char* delimeter) {
    int size = 0;
    char* textCopy = strdup(text);
    char* item = strtok_r(textCopy,delimeter,&textCopy);
    while (item != NULL) {
        size++;
        item = strtok_r(textCopy,delimeter,&textCopy);
    }
    return size;
}

//Creates a list of strings with the stations
char** getLineList(char* lineText,int size) {
    char **lineList = malloc(size), *line;
    int i;
    for (i = 0; i < size; i++) {
        line = strtok_r(lineText,"/",&lineText);
        lineList[i] = malloc(strlen(line));
        lineList[i] = strdup(line);
    }
    return lineList;
}

//Converts the string containing the distance to a double
Edge* initialiseEdges(char* edgeText,Edge* edgeHead) {
    char *name, *edgeData = strtok_r(edgeText,",",&edgeText);
    double distance,bestTime,peakTime,interPeakTime;
    while (edgeData != NULL) {
        name = edgeData;
        distance = strtod(strtok_r(edgeText,",",&edgeText),NULL); //The data is delimited by ','. strtok_r seperates the string by the delimiter.
        bestTime = strtod(strtok_r(edgeText,",",&edgeText),NULL);
        peakTime = strtod(strtok_r(edgeText,",",&edgeText),NULL);
        interPeakTime = strtod(strtok_r(edgeText,",",&edgeText),NULL);
        edgeHead = appendEdge(createEdge(name,distance,bestTime,peakTime,interPeakTime),edgeHead);
        edgeData = strtok_r(edgeText,",",&edgeText);
    }
    return edgeHead;
}

Node* initialiseNode(char* newNode,Node* nodeHead) {
    char *name = strtok_r(newNode,",",&newNode),*lines = strtok_r(newNode,",",&newNode);
    int size = getListSize(lines,"/");
    char** linelist = getLineList(lines,size);
    Node *node = createNode(name,linelist,size);
    node->edgeHead = initialiseEdges(newNode,node->edgeHead);
    nodeHead = appendNode(node,nodeHead);
    return nodeHead;
}

//Creates the node from the data file
Node* createLists(char* rawText,Node* nodeHead) {
    char* newNode;
    newNode = strtok_r(rawText,"\n",&rawText);
    while (newNode != NULL) {
        nodeHead = initialiseNode(newNode,nodeHead);
        newNode = strtok_r(rawText,"\n",&rawText);
    }
    return nodeHead;
}

Node* getNodeByName(char* name,Node* head) {
    Node* currentNode = head;
    while (currentNode != NULL) {
        if (strcmp(name,currentNode->name) == 0) {
            return currentNode;
        }
        currentNode = currentNode->next;
    }
    return NULL;
}

//Initialises the current line(s) that is being taken
void setLineList(Node* node) {
    int i;
    for (i = 0; i < node->numberOfLines; i++) {
        char *newPath = strdup(node->currentLine);
        node->currentLine = concatonate3Strings(newPath,",",node->lines[i]);
    }
}

void initiateStartingNode(Node* startingNode) {
    startingNode->displacement = 0;
    startingNode->path = "";
    startingNode->currentLine = "";
    setLineList(startingNode);
}

//Checks if the two stations share a line
int checkIfElement(char** lineList,int size,char* element) {
    int i, isElement = 0;
    for (i = 0; i < size; i++) {
        if (strcmp(element,lineList[i]) == 0) {
            isElement = 1;
        }
    }
    return isElement;
}

//Adds another line onto the current line if two stations share multiple lines
void addSharedStations(Node* connectedNode,char* line) {
    char* lineCopy = strdup(connectedNode->currentLine);
    connectedNode->currentLine = concatonate3Strings(lineCopy,",",line);
}

//Takes the current line of the visited node and compares it with the lines of the connected node.
//compares them and sees if they share a line.
int setCurrentLine(Node* connectedNode, Node* visitedNode) {
    char* currentLine = strdup(visitedNode->currentLine);
        char *line = strtok_r(currentLine,",",&currentLine);
        int hasSharedStation = 0;
        connectedNode->currentLine = "";
        while (line != NULL) {
            if (checkIfElement(connectedNode->lines,connectedNode->numberOfLines,line) == 1) {
                addSharedStations(connectedNode,line);
                hasSharedStation = 1;
            }
            line = strtok_r(currentLine,",",&currentLine);
        }
    return hasSharedStation;
}

//Sets the new line if the current line being taken is not in the new station
void setNewLine(Node* connectedNode, Node* visitedNode) {
    int i,j;
    for (i = 0; i < connectedNode->numberOfLines; i++) {
        for (j = 0; j < visitedNode->numberOfLines; j++) {
            if (strcmp(connectedNode->lines[i],visitedNode->lines[j]) == 0) {
                addSharedStations(connectedNode,connectedNode->lines[i]);
            }
        }
    }
}

//Checks if you need to change line
int checkSharedStations(Node* connectedNode, Node* visitedNode) {
    int hasSharedStation = setCurrentLine(connectedNode,visitedNode);
    if (hasSharedStation == 0) {
        setNewLine(connectedNode,visitedNode);
        connectedNode->changeHere = 1;
    }
    else {
        connectedNode->changeHere = 0;
    }
}

//Compares the displacement and sets the time taken
void compareEdgesDistance(Node* connectedNode,Edge* currentEdge,Node* visitedNode) {
    double distanceFromStart = currentEdge->distance+visitedNode->displacement;
    if (connectedNode->displacement > distanceFromStart) {
        checkSharedStations(connectedNode,visitedNode);
        connectedNode->displacement = distanceFromStart;
        connectedNode->timeDisplacementBest = visitedNode->timeDisplacementBest + currentEdge->bestTime;
        connectedNode->timeDisplacementPeak = visitedNode->timeDisplacementPeak + currentEdge->peakTime;
        connectedNode->timeDisplacementInterPeak = visitedNode->timeDisplacementInterPeak + currentEdge->interPeakTime;
        free(connectedNode->path);
        connectedNode->path = concatonate3Strings(visitedNode->path,",",visitedNode->name);
    }

}

//Compares the displacement and sets the distance travelled
void compareEdgesTime(Node* connectedNode,Edge* currentEdge, Node* visitedNode,double currentDistance) {
    double timeFromStart = currentDistance + visitedNode->displacement;
    if (connectedNode->displacement > timeFromStart) {
        checkSharedStations(connectedNode,visitedNode);
        connectedNode->displacement = timeFromStart;
        connectedNode->distanceDisplacement = visitedNode->distanceDisplacement + currentEdge->distance;
        free(connectedNode->path);
        connectedNode->path = concatonate3Strings(visitedNode->path,",",visitedNode->name);
    }
}

//Calls different functions depending on the option the user chooses
void manageEdges(Node* visitedNode,Node* nodeHead,int comparisonType) {
    Edge* currentEdge = visitedNode->edgeHead;
    while (currentEdge != NULL) {
        Node* connectedNode = getNodeByName(currentEdge->name,nodeHead);
        if (connectedNode->visited != 1 && connectedNode != NULL) {
            if (comparisonType == 1) {
                compareEdgesDistance(connectedNode,currentEdge,visitedNode);
            } 
            else if (comparisonType == 2) {
                compareEdgesTime(connectedNode,currentEdge,visitedNode,currentEdge->bestTime);
            }
            else if (comparisonType == 3) {
                compareEdgesTime(connectedNode,currentEdge,visitedNode,currentEdge->peakTime);
            }
            else if (comparisonType == 4) {
                compareEdgesTime(connectedNode,currentEdge,visitedNode,currentEdge->interPeakTime);
            }
        }
        currentEdge = currentEdge->next;
    }
}

//Finds the next node with the shortest displacement which hasn't been visited already
Node* getNextNode(Node* nodeHead) {
    Node* currentNode = nodeHead;
    Node* selectedNode = NULL;
    int shortestDisplacement = INT_MAX;
    while (currentNode != NULL) {
        if (currentNode->displacement < shortestDisplacement && currentNode->visited != 1) {
            selectedNode = currentNode;
            shortestDisplacement = currentNode->displacement;
        }
        currentNode = currentNode->next;
    }
    return selectedNode;
}

//Uses Djikstra's shortest path algorithm
void findShortestPath(Node* visitedNode,Node* nodeHead,int comparisonType) {
    initiateStartingNode(visitedNode);
    while (visitedNode != NULL) {
        visitedNode->visited = 1;
        manageEdges(visitedNode,nodeHead,comparisonType);
        visitedNode = getNextNode(nodeHead);
    }
}

//Prints out instructions to change lines where needed
void printLineChange(char* currentLine,char* previousStation,Node* stationNode) {
    if (stationNode->changeHere == 1) {
        printf("the %s line to %s.\nThen from %s, take ",currentLine,previousStation,previousStation);
    }
}

char* checkIfLineChange(char* currentLine,char* previousStation,char* station,char* path,Node* nodeHead) {
    while (station != NULL) {
        Node* stationNode = getNodeByName(station,nodeHead);
        printLineChange(currentLine,previousStation,stationNode);
        currentLine = stationNode->currentLine;
        previousStation = stationNode->name;
        station = strtok_r(path,",",&path);
    }
    return currentLine;
}

void printOtherData(int comparisonType,Node* destination) {
    if (comparisonType == 1) {
        printf("\nDistance: %fkm\nBest case time: %f mins\nPeak time: %f mins\nInter-peak time: %f mins\n",destination->displacement,destination->timeDisplacementBest,destination->timeDisplacementPeak,destination->timeDisplacementInterPeak);
    }
    else {
        printf("\nDistance: %fkm\nTime: %f mins\n",destination->distanceDisplacement,destination->displacement);
    }
}

//Function to start printing the instructions to get from station to station.
void printPath(Node* destinationNode,Node* nodeHead,int comparisonType) {
    char* path = concatonate3Strings(strdup(destinationNode->path),",",destinationNode->name);

    char* station = strtok_r(path,",",&path);
    printf("\nFrom %s take ",station);  //seperate case for the starting station.
    Node* stationNode = getNodeByName(station,nodeHead);
    char* currentLine = stationNode->currentLine, * previousStation = stationNode->name;
    station = strtok_r(path,",",&path);

    currentLine = checkIfLineChange(currentLine, previousStation, station, path, nodeHead);

    printf("the %s line to %s.\n",strtok_r(currentLine,",",&currentLine),destinationNode->name); //seperate case for destination station
    printOtherData(comparisonType,destinationNode);
}

//Capitalises the first letter and turns the rest of the letters to lower case
char* title(char* name) {
    int i;
    name[0] = toupper(name[0]);
    for (i = 1; i < strlen(name); i++) {
        name[i] = tolower(name[i]);
        if (name[i] == '\n') { //removes the trailing \n from fgets
            name[i] = '\0';
        }
    }
    return name;
}

char* normaliseInput(char* stationName) {
    char *normalisedString = calloc(1,MAX_STATION_NAME_LENGTH), *subName;
    subName = strtok_r(stationName," ",&stationName);
    while (subName != NULL) {
        subName = title(subName);
        if (strcmp(subName,"And") == 0) { //Converts the string And into & as the names in the data file use &.
            strcat(normalisedString,"&");
        }
        else {
            strcat(normalisedString,subName);
        }
        subName = strtok_r(stationName," ",&stationName); //splits the string by spaces
        if (subName != NULL) {
            strcat(normalisedString," ");
        }
    }
    return normalisedString;
}

Node* getStationName(Node* nodeHead) {
    Node* station = NULL;
    char stationName[MAX_STATION_NAME_LENGTH];
    while (station == NULL) {
        fflush(stdin);
        fgets(stationName,MAX_STATION_NAME_LENGTH,stdin);
        char* normalisedInput = normaliseInput(stationName);
        station = getNodeByName(normalisedInput,nodeHead);
        free(normalisedInput);
        if (station == NULL) {
            printf("Station not found.\nInput station:\n");
        }
    }
    return station;
}

//gets the data the user wants to compare.
//1: distance
//2: uninterrupted time
//3: peak time (07:00-10:00)
//4: inter-peak time (10:00-16:00)
int getComparisonType(void) {
    int comparisonType = 0;
    while (comparisonType == 0) {
        printf("Input the number corresponding to what data you want to use.\n1: Distance\n2: Uninterrupted travel time\n3: Peak travel time (07:00-10:00)\n4: Inter-Peak travel time (10:00-16:00)\n");
        scanf("%i",&comparisonType);
        getchar();
        if (!(1 <= comparisonType && comparisonType <= 4)) {
            comparisonType = 0;
        }
    }
    return comparisonType;
}

int main(void) {
    Node* nodeHead = NULL;
    int comparisonType = getComparisonType();
    chdir(".."); //Goes back one directory
    nodeHead = createLists(readFile("./stations.txt"),nodeHead); //gets the data file
    printf("Input start point: \n");
    findShortestPath(getStationName(nodeHead),nodeHead,comparisonType);
    printf("Input end point: \n");
    printPath(getStationName(nodeHead),nodeHead,comparisonType);
    return 0;
}