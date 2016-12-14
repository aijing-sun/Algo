/*
	 Assignment2 Sample Solution
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <math.h>
#include <string.h>

#define MAX_MAX_X 1000
#define MAX_MAX_Y 1000
#define MAX_NUM_PT 10000
#define INPUT_FILE_ERROR_MSG "File structure error."
#define White 0
#define Black 1

#ifndef max
    #define max(a,b) ((a) > (b) ? (a) : (b))
#endif

//Define Point object
struct point {
	int x, y, color, parent, distotree;
};

//Global Variables:
int x, y, max_x, max_y, num_pt, num_ins, hasWhitePoint, curMin, curPt, treeLen, ip, jp, ix, iy, ipx, ipy, jx, jy, jpx, jpy;
char progName[31], infileName[31], ofileName[31];
bool fs, dupCheck[1001][1001];
FILE *infile, *ofile;
struct point points[10000];

//Function Declarations:
void initDup();
void inputError();
void get_no_dup_point();
void readAndCheck();
void invalCmd();
void geneRamdIns();
void printNonCmtLine();
void checknumPTError(int d);
void inputfile();
void checkMaxXY(int d);
void computeMST();
int distance(struct point a, struct point b);


//MST Core:
void computeMST(){
	//open infile stream
	infile = fopen(infileName, "a");
	int i;

	//check if -o option exists
	int cmp = strcmp(ofileName, "stdout");
	if(cmp!=0){
		//print to file
		ofile = fopen(ofileName, "w");
	}else{
		//print to stdout
		ofile = stdout;
	}


	//Choose our root point 0
	points[0].color = Black;
	fprintf(ofile, "Choosing point 0 as the root ...\n");
	//Compute all d(0, i)
	for(i=1; i<num_pt; i++){
		points[i].distotree = distance(points[0], points[i]);
		points[i].parent = 0;
		fprintf(ofile, "Point %d has a distance %d to the tree, parent 0;\n", i, points[i].distotree);
	}

	//Append this comment line to both input and output
	fprintf(infile, "#Edges of the MST by Prim’s algorithm:\n");
	fprintf(ofile, "#Edges of the MST by Prim’s algorithm:\n");

	treeLen = 0;
	hasWhitePoint = num_pt - 1;
	while(hasWhitePoint--){
		fs = true;
		for(i=0; i<num_pt; i++){
			if(points[i].color == White){
				if(fs){
					curMin = points[i].distotree;
					curPt = i;
					fs = false;
				}
				else if(points[i].distotree < curMin){
					//change curMin and pt index
					curMin = points[i].distotree;
					curPt = i;
				}
				else if(points[i].distotree == curMin){
					//break tie
					ip = points[i].parent;	jp = points[curPt].parent;
					ix = points[i].x;	iy = points[i].y;
					ipx = points[ip].x;	ipy = points[ip].y;
					jx = points[curPt].x;	jy = points[curPt].y;
					jpx = points[jp].x;	jpy = points[jp].y;

					if(abs(iy - ipy) > abs(jy - jpy)){
						curPt = i;
					}
					else if( (abs(iy - ipy) == abs(jy - jpy)) && (max(ix, ipx) >= max(jx, jpx)) ){
						curPt = i;
					}
				}
			}
		}
		//color curPt Black...
		points[curPt].color = Black;
		fprintf(ofile, "%d %d %d\n", points[curPt].parent, curPt, points[curPt].distotree);
		fprintf(infile, "%d %d %d\n", points[curPt].parent, curPt, points[curPt].distotree);
		treeLen += points[curPt].distotree;

		//update distance to Tree for All white points
		for(i=0; i<num_pt; i++){
			if(points[i].color == White){
				if(distance(points[i], points[curPt]) < points[i].distotree ){
					points[i].distotree = distance(points[i], points[curPt]);
					points[i].parent = curPt;
				}
				//Update parent of i if needed: Same tie breaking rule
				if(distance(points[i], points[curPt]) == points[i].distotree ){

					//break tie
					ip = curPt;	jp = points[i].parent;
					ix = points[i].x;	iy = points[i].y;
					ipx = points[ip].x;	ipy = points[ip].y;
					jx = points[i].x;	jy = points[i].y;
					jpx = points[jp].x;	jpy = points[jp].y;

					if(abs(iy - ipy) > abs(jy - jpy)){
						points[i].parent = curPt;
					}
					else if( (abs(iy - ipy) == abs(jy - jpy)) && (max(ix, ipx) >= max(jx, jpx)) ){
						points[i].parent = curPt;
					}

				}
			}
		}
	}

	//Print MST Length
	fprintf(ofile, "The total length of the MST is %ld.\n", treeLen);

	//close filestreams
	fclose(infile);
	fclose(ofile);
}


//Function compute and return the distance of two points
int distance(struct point a, struct point b){
	return abs(a.x-b.x) + abs(a.y-b.y);
}

//func1
int main(int argc, char *argv[])
{
	// Get program name
	strcpy(progName, argv[0]);
	// The first thing to do is to check command line input
	if (argc == 1) {
		//go to function 5 for generating random instances
		geneRamdIns();
	}else if (argc ==3) {
		//argv[1] must be '-i' and argv[2] must be test file name
		if (strcmp(argv[1],"-i")!=0) {
			invalCmd();
		}else {
			strcpy(infileName, argv[2]);
			strcpy(ofileName, "stdout"); //later used for checking where we should print
			readAndCheck();
		}
	}else if (argc ==5) {
		//argv[1] or argv[3] must be '-i' or '-o'
		if (strcmp(argv[1], "-i")==0 && strcmp(argv[3], "-o") == 0) {
			strcpy(infileName, argv[2]);
			strcpy(ofileName, argv[4]);
			readAndCheck();
		}else if (strcmp(argv[1], "-o") == 0 && strcmp(argv[3], "-i") == 0) {
			strcpy(infileName, argv[4]);
			strcpy(ofileName, argv[2]);
			readAndCheck();
		}else {
			invalCmd();
		}
	}else {
		invalCmd();
	}
	return 0;
}

//func2 && 3
void readAndCheck()
{
	int counter = 0; //latter used for ordering non-commented line
	char c, str[64];
	infile = fopen(infileName, "r");
	if (infile==NULL) {
		printf("Input File Does Not Exist\n");
		exit(1);
	}
	while (fgets(str, 64, infile)) {
		c = str[0];
		if (c!='#' && c!='\n') {
			counter++;
			if (counter==1) {
				if (sscanf(str, "%d %d %c", &max_x, &max_y, &c)!=2) {
					inputError();
				}
				checkMaxXY(1);
				//Initialize duplicate checking table
				initDup();
			}else if (counter==2) {
				if(sscanf(str, "%d %c", &num_pt, &c)!=1){
					inputError();
				}
				//check num_pt error
				if (num_pt<1 || num_pt>MAX_NUM_PT) {
					printf("NUM_PT outside Max/Min NUM_PT range\n");
					fclose(infile);
					exit(1);
				}
				checknumPTError(1);
			}else {
				//counter is at least 3 now.
				//store points in points table
				if (sscanf(str, "%d %d %c", &points[counter-3].x, &points[counter-3].y, &c)!=2) {
					inputError();
				}
				x = points[counter-3].x;
				y = points[counter-3].y;
				// Initialize id, parent and color
				points[counter-3].color = White;
				points[counter-3].parent = -1;

				if(x>max_x || y>max_y || x<0 || y<0){
					printf("Input point value out side [0,max_x] and/or [0,max_y]\n");
					fclose(infile);
					exit(1);
				}
				//we need check for duplicates
				if(dupCheck[x][y]){
					printf("Input has duplicate points\n");
					fclose(infile);
					exit(1);
				}
				dupCheck[x][y] = true; //point now seen
				if((counter-2)>num_pt){
					inputError();
				}
			}
		}
	}
	if((counter-2) <num_pt){
		inputError();
	}
	fclose(infile);
	//Compute MST
	//printNonCmtLine();
	computeMST();
}

//Func4
void printNonCmtLine(){
	//check if -o option exists
	int cmp = strcmp(ofileName, "stdout");
	int i;
	if(cmp!=0){
		//print to file
		ofile = fopen(ofileName, "w");
	}else{
		ofile = stdout;
		//print to stdout
	}
	fprintf(ofile, "%d %d\n%d\n", max_x, max_y, num_pt);
	for(i=0; i<num_pt; i++){
		fprintf(ofile, "%d %d\n", points[i].x, points[i].y);
	}
	fclose(ofile);
}

//Called when Cmd is invalid
void invalCmd(){
	printf(">%s [-i inputfile [-o outputfile]]\n", &progName[2]);
	exit(1);
}

//Func5
void geneRamdIns(){
	char insName[31];
	FILE *wfile;
	int i;

	printf("Generating random instances ...\n");
	printf("Enter the circuit board size max_x max_y: ");
	if(scanf("%d%d", &max_x, &max_y)!=2){
		printf("Invalid Input of max_x or max_y\n");
		exit(1);
	}
	checkMaxXY(0);

	printf("Enter the number of points num_pt: ");
	if(scanf("%d", &num_pt)!=1){
		printf("Invalid Input of num_pt\n");
		exit(1);
	}
	//check num_pt error
	if(num_pt<1 || num_pt>MAX_NUM_PT){
		printf("NUM_PT outside Max/Min NUM_PT range\n");
		exit(1);
	}
	checknumPTError(0);
	printf("Enter the number of random instances to be generated: ");
	if(scanf("%d", &num_ins)!=1){
		printf("Invalid Input of num_ins\n");
		exit(1);
	}
	//We seed to make sure no repetious instances being generated
	srand(time(NULL));
	for(i=1; i<=num_ins; i++){
        int j;
		sprintf(insName, "instance%d_%d.txt", num_pt, i);
		wfile = fopen(insName, "w");
		//initialize duplicates check table
		initDup();
		fprintf(wfile, "#%s\n#area [0, max_x] x [0, max_y]\n%d %d\n#number of points num_pt\n%d\n#coordinates\n", insName, max_x, max_y, num_pt);
		for(j=0; j<num_pt; j++){
			// we want a new and its not duplicate
			get_no_dup_point();
			fprintf(wfile, "%d %d\n", x, y);
		}
		fprintf(wfile, "#end of instance\n");

		fclose(wfile);
		printf("instance%d_%d.txt generated", num_pt, i);
		if(i!=num_ins){
			printf("\n");
		}
	}
	printf(" ... done!\n");
}

//We set all entries for dupCheck to false, meaning point not seen yet
void initDup(){
    int i,j;
	for(i=0; i<=max_x;i++){
		for(j=0; j<=max_y;j++){
			dupCheck[i][j] = false;
		}
	}
}

//Try to generate a new point
void get_no_dup_point(){
	do{
		x = rand() % (max_x+1);
		y = rand() % (max_y+1);
	}while(dupCheck[x][y]);
	dupCheck[x][y] = true;
}

//Check Impossible num_pt error
void checknumPTError(int d){
	if(num_pt > ((max_x+1)*(max_y+1))){
		printf("The board is too small to hold %d points\n", num_pt);
		if(d==1){
			fclose(infile);
		}
		exit(1);
	}
}

void inputError(){
	printf("%s\n",INPUT_FILE_ERROR_MSG);
	fclose(infile);
	exit(1);
}

void checkMaxXY(int d){
	if(max_x > MAX_MAX_X || max_y > MAX_MAX_Y || max_x < 0 || max_y <0){
		printf("Input MAX_X/MAX_Y out side range\n");
		if(d==1){
			fclose(infile);
		}
		exit(1);
	}
}
//END OF ASSIGNMENT 2
