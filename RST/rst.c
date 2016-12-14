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

#ifndef min
    #define min(a,b) ((a) <= (b) ? (a) : (b))
#endif



// copy from tutorial
struct node {
	int index; /* the order in the instance file */
	int x; /* x coordinate */
	int y; /* y coordinate */
	int parent; /* parent in the tree when added */
	int num_children; /* has value 0 -- 8 */
	int child[8];
	int overlap_hv; /* total overlap when horizontal then vertical */
	int overlap_vh; /* total overlap when the other way */
};

/*
 *  hl : current point head towards the horizontal left
 *  hr : current point head towards the horizontal right
 *  vp : current point head towards the vertical up
 *  vd : current point head towards the vertical down
 *
 */
enum DIRECTION{hl, hr, vp, vd};

/*
 * here edge is based on the current point
 * so the edge direction is viewed from the current point.
 * current point means the point that is being computed for
 * its max overlap.
 * Refer to 5th item of the assignment3 tutorial pdf.
 */
struct edge {
//    int x; /* either the right point coordinate value of h edge */
//    int y; /* or the up point of v edge    */
    int len; /* the length of the edge */
    enum DIRECTION direction; /* the direction of this edge based on current point*/
};

//Global Variables:
struct node nodes[10000];
FILE *infile, *ofile;
char progName[31], infileName[31], ofileName[31];
int reverse_dfs_order[10000];
int dfs_counter = 0;
int num_pt;
int x, y, max_x, max_y, num_ins;
bool dupCheck[1001][1001];
int totalMST = 0;
int debug = -1; // use for debug

// functions declaration
void inputError();
void get_no_dup_point();
void readAndCheck();
void invalCmd();
void geneRamdIns();
void printNonCmtLine();
void checknumPTError(int d);
void inputfile();
void printDFS(struct node root);
void printSecondNode(int index);
void checkMaxXY(int d);
void initDup();

// the core function for assignment 3
void calcMaxOverlap();
int calcCurrentOverlap(struct edge edges[],int len,int debug);
int enumCombtion(struct node sub_root, int lay_out );


//func1 copy from assignment 2
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



//func2 && 3  modified from assignment 2
void readAndCheck()
{
	int parent_ind, child_ind, dist;
	int tmp_child_num;
	int counter = 0; //latter used for ordering non-commented line
	char c, str[64];
	int i;
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
				// if num_pt == 1 then exit
				if (num_pt <= 1 || num_pt>MAX_NUM_PT) {
					printf("NUM_PT outside Max/Min NUM_PT range\n");
					fclose(infile);
					exit(1);
				}
				checknumPTError(1);
			}else if (counter >= 3 && counter < 3 + num_pt){
				//counter is at least 3 now.
				//store nodes in nodes table
				if (sscanf(str, "%d %d %c", &nodes[counter-3].x, &nodes[counter-3].y, &c)!=2) {
					inputError();
				}
				x = nodes[counter-3].x;
				y = nodes[counter-3].y;
				// Initialize node members
				nodes[counter-3].index = counter - 3;
				nodes[counter-3].parent = -1;
				nodes[counter-3].num_children = 0;
				for (i = 0; i < 8; i++)
					nodes[counter-3].child[i] = -1;
				nodes[counter-3].overlap_hv = -1;
				nodes[counter-3].overlap_vh = -1;


				if(x>max_x || y>max_y || x<0 || y<0){
					printf("Input point value out side [0,max_x] and/or [0,max_y]\n");
					fclose(infile);
					exit(1);
				}
				//we need check for duplicates
				if(dupCheck[x][y]){
					printf("Input has duplicate nodes\n");
					fclose(infile);
					exit(1);
				}
				dupCheck[x][y] = true; //point now seen
				if((counter-2)>num_pt){
					inputError();
				}
			} else { //TODO read edges info
				if (sscanf(str, "%d %d %d %c", &parent_ind, &child_ind, &dist, &c) != 3) {
					inputError();
				}
				// update parent and child nodes
				tmp_child_num = nodes[parent_ind].num_children;
				nodes[parent_ind].child[tmp_child_num] = child_ind;
				nodes[parent_ind].num_children = tmp_child_num + 1;
				nodes[child_ind].parent = parent_ind;
				totalMST += dist;


			}
		}
	}

	// print the first set of lines in the output
	printSecondNode(1);

	// print the second set of lines in the output
	printDFS(nodes[0]);

    // print the third set of lines in the output
    calcMaxOverlap();

	fclose(infile);
	fclose(ofile);
	//Compute MST
	//printNonCmtLine();
	//computeMST();
}

// print the second set recursively
// use depth-first-search
void printDFS(struct node root){

    int i;
	// store the reverse dfs order
	dfs_counter++;
	reverse_dfs_order[num_pt - dfs_counter] = root.index;


	fprintf(ofile, "%d, %d", root.index, root.num_children);
	for (i = 0; i < root.num_children - 1; i++){
		fprintf(ofile, ", %d", root.child[i]);
	}
	// append the last child node index
	if (root.num_children == 0) {
        fprintf(ofile, "\n");
	} else {
        fprintf(ofile, ", %d\n", root.child[i]);
	}

	// if no child then just pass
	for (i = 0; i < root.num_children; i++){
		printDFS(nodes[root.child[i]]);
	}

	if (root.index == 0)
        fprintf(ofile, "\n");
}


void printSecondNode(int index){

    int i;
	//open infile stream
	infile = fopen(infileName, "a");

	//check if -o option exists
	int cmp = strcmp(ofileName, "stdout");
	if(cmp!=0){
		//print to file
		ofile = fopen(ofileName, "w");
	}else{
		//print to stdout
		ofile = stdout;
	}


	fprintf(ofile, "%d\n", nodes[index].index);
	fprintf(ofile, "%d\n", nodes[index].x);
	fprintf(ofile, "%d\n", nodes[index].y);
	fprintf(ofile, "%d\n", nodes[index].parent);
	fprintf(ofile, "%d\n", nodes[index].num_children);
	for (i = 0; i < nodes[index].num_children; i++){
		fprintf(ofile, "%d ", nodes[index].child[i]);
	}
	fprintf(ofile, "\n");
	fprintf(ofile, "%d\n", nodes[index].overlap_hv);
	fprintf(ofile, "%d\n", nodes[index].overlap_vh);
	fprintf(ofile, "\n");
}


// calculate the max overlap of the tree
// by the reverse DFS order.
void calcMaxOverlap(){
    int i;
    int cur_ind;
    int parent_ind;

    for (i = 0; i < num_pt; i++){
        cur_ind = reverse_dfs_order[i];
        if (nodes[cur_ind].num_children == 0) {
            nodes[cur_ind].overlap_hv = 0;
            nodes[cur_ind].overlap_vh = 0;
        } else {
            parent_ind = nodes[cur_ind].parent;
           // int cur_node_max_overlap;
            if (parent_ind >= 0){
                if (nodes[cur_ind].x == nodes[parent_ind].x ||
                    nodes[cur_ind].y == nodes[parent_ind].y){
                    //cur_node_max_overlap = enumCombtion(nodes[cur_ind], -1);
                    nodes[cur_ind].overlap_hv = enumCombtion(nodes[cur_ind], -1);
                    nodes[cur_ind].overlap_vh = nodes[cur_ind].overlap_hv;
                } else {
                    nodes[cur_ind].overlap_hv = enumCombtion(nodes[cur_ind], 0);
                    nodes[cur_ind].overlap_vh = enumCombtion(nodes[cur_ind], 1);
                }
            } else { // reach tree root node
                nodes[cur_ind].overlap_hv = enumCombtion(nodes[cur_ind], 0);
                nodes[cur_ind].overlap_vh = nodes[cur_ind].overlap_hv;
            }



        }
        if (debug > 0) {
            printf("node index:%d, overlap_hv:%d, overlap_vh:%d\n",cur_ind,nodes[cur_ind].overlap_hv,nodes[cur_ind].overlap_vh);

        }
    }
    printf("total MST length:%d\n",totalMST);
    fprintf(ofile, "The total overlap is %d\n", nodes[0].overlap_hv);
	fprintf(ofile, "The reduction rate is %.2f\n", (float)nodes[0].overlap_hv / totalMST);


}


// Enumerate the combinations of how edges are laid, and compute the maximum overlap
// from the view of current point.
//      sub_root means the current point we mentioned all the time.
//      layout : 0 is hv; 1 is vh; -1 is none and should never happen
// Refer to the solution in 5th item of assignment 3 tutorial pdf.
int enumCombtion(struct node sub_root, int lay_out ) {

    // For enumerate all the combinations, we use a bitmap.
    // When there are n child, then number of combinations are pow(2,n)
    // so we use a i ranging from [0, pow(2,n)-1],
    // then use bit operator '&', we can know how the corresponding edge
    // is laid out.
    // For example, if n=2,i=2, the lowest 8 bit of i is 00000010,
    // so we can see bit value of child 1 is 0, the one of child 2 is 1.
    // The value '0' of the bit means the parent first horizontally then vertically reaches the child
    // The value '1' of the bit means the parent first vertically then horizontally reaches the child
    int bitmap[8] = {1, 2, 4, 8, 16, 32, 64, 128};
    int num_children = sub_root.num_children;
    int i;
    int counter = 0; // count how much edges between sub_root and his children
    int maxOverlap = 0;
    int tmpOverlap = 0;
    struct edge edges[9];

    // add the edge between sub_root and its parent
    // only when sub_root node is not the tree root node
    if (sub_root.index > 0){
        //
        int parent_ind = sub_root.parent;
        if (sub_root.y == nodes[parent_ind].y ){
            edges[num_children].len = abs(sub_root.x - nodes[parent_ind].x);
            if (sub_root.x > nodes[parent_ind].x){
                edges[num_children].direction = hl;
            } else if (sub_root.x < nodes[parent_ind].x){
                edges[num_children].direction = hr;
            } else {
                printf("wrong in the child edges directions!\n");
                exit(1);
            }
        }
        else if (sub_root.x == nodes[parent_ind].x){
            edges[num_children].len = abs(sub_root.y - nodes[parent_ind].y);
            if (sub_root.y > nodes[parent_ind].y){
                edges[num_children].direction = vd;
            } else if (sub_root.y < nodes[parent_ind].y){
                edges[num_children].direction = vp;
            } else {
                printf("wrong in the child edges directions!\n");
                exit(1);
            }
        } else {// 0 is hv; 1 is vh
            if (lay_out == 0){
                edges[num_children].len = abs(sub_root.y - nodes[parent_ind].y);
                if (sub_root.y > nodes[parent_ind].y){
                    edges[num_children].direction = vd;
                } else if (sub_root.y < nodes[parent_ind].y){
                    edges[num_children].direction = vp;
                } else {
                    printf("wrong in the child edges directions!\n");
                    exit(1);
                }

            } else if (lay_out == 1) {
                edges[num_children].len = abs(sub_root.x - nodes[parent_ind].x);
                if (sub_root.x > nodes[parent_ind].x){
                    edges[num_children].direction = hl;
                } else if (sub_root.x < nodes[parent_ind].x){
                    edges[num_children].direction = hr;
                } else {
                    printf("wrong in the child edges directions!\n");
                    exit(1);
                }
            }
        }
    }


    // 0 is hv; 1 is vh
    for (i = 0; i < (int)pow(2,num_children); i++) {

        tmpOverlap = 0;
        for (counter = 0; counter < num_children; counter++) {

            int child_ind = sub_root.child[counter];


                // root and child on the same straight line
                if (sub_root.y == nodes[child_ind].y){
                        if (debug > 0 && sub_root.index == 4 && lay_out == 0 && i == 3 ) {

                                printf("counter:%d  3 1here i:%d!!!!!!!!!\n",counter,i);
                         }
                    edges[counter].len = abs(sub_root.x - nodes[child_ind].x);
                    if (sub_root.x > nodes[child_ind].x){
                        edges[counter].direction = hl;
                    } else if (sub_root.x < nodes[child_ind].x){
                        edges[counter].direction = hr;
                    } else {
                        printf("wrong in the child edges directions!\n");
                        exit(1);
                    }

                    // the hv and vh of edge(root,child) have the same value
                    tmpOverlap += nodes[child_ind].overlap_hv;
                    if (nodes[child_ind].overlap_hv != nodes[child_ind].overlap_vh) {
                        // this should never execute
                        printf("hv and vh should be the same!\n");
                        exit(1);
                    }

                } else if (sub_root.x == nodes[child_ind].x){
                         if (debug > 0 && sub_root.index == 4 && lay_out == 0 && i == 3 ) {

                                printf("counter:%d  3 2here i:%d!!!!!!!!!\n",counter,i);
                         }
                    edges[counter].len = abs(sub_root.y - nodes[child_ind].y);
                    if (sub_root.y > nodes[child_ind].y){
                        edges[counter].direction = vd;
                    } else if (sub_root.y < nodes[child_ind].y){
                        edges[counter].direction = vp;
                    } else {
                        printf("wrong in the child edges directions!\n");
                        exit(1);
                    }

                     // the hv and vh of edge(root,child) have the same value
                    tmpOverlap += nodes[child_ind].overlap_hv;
                    if (nodes[child_ind].overlap_hv != nodes[child_ind].overlap_vh) {
                        // this should never execute
                        printf("hv and vh should be the same!\n");
                        exit(1);
                    }

                } else {
                    if ((bitmap[counter] & i) == bitmap[counter]) { // vh
                        if (debug > 0 && sub_root.index == 4 && lay_out == 0 && i == 3 ) {

                                printf("counter:%d  3 3here i:%d!!!!!!!!!\n",counter,i);
                         }
                        edges[counter].len = abs(sub_root.y - nodes[child_ind].y);
                        if (sub_root.y > nodes[child_ind].y){
                            edges[counter].direction = vd;
                        } else if (sub_root.y < nodes[child_ind].y){
                            edges[counter].direction = vp;
                        } else {
                            printf("wrong in the child edges directions!\n");
                            exit(1);
                        }

                        tmpOverlap += nodes[child_ind].overlap_vh;

                    } else if ((bitmap[counter] & i) == 0) { //hv
                         if (debug > 0 && sub_root.index == 4 && lay_out == 0 && i == 3 ) {

                                printf("counter:%d  3 4here i:%d!!!!!!!!!\n",counter,i);
                         }

                        edges[counter].len = abs(sub_root.x - nodes[child_ind].x);
                        if (sub_root.x > nodes[child_ind].x){
                            edges[counter].direction = hl;
                        } else if (sub_root.x < nodes[child_ind].x){
                            edges[counter].direction = hr;
                        } else {
                            printf("wrong in the child edges directions!\n");
                            exit(1);
                        }

                        tmpOverlap += nodes[child_ind].overlap_hv;
                    } else {
                        printf("wrong in bitmap!\n");
                        exit(1);
                    }


                }

               // counter++;
        }

         // 0 is hv; 1 is vh
        if (debug > 0 && sub_root.index == 4 && lay_out == 0) {
            int hh = 0;
            for (;hh < num_children; hh++){
                printf("edge(4-%d) len:%d dir:%d\n",sub_root.child[hh],edges[hh].len,edges[hh].direction);
            }
            printf("edge(%d-4) len:%d dir:%d\n",sub_root.parent,edges[hh].len,edges[hh].direction);
            printf("\n");
        }

        // whether is the root node of the tree
        if (sub_root.index > 0){
            maxOverlap = max(maxOverlap,tmpOverlap + calcCurrentOverlap(edges, num_children + 1, debug));
        } else {
            maxOverlap = max(maxOverlap,tmpOverlap + calcCurrentOverlap(edges, num_children, debug));
        }


    }

    return maxOverlap;

}


// from the point of sub_root point, for a certain combination,
// we can find the overlap is easily calculated.
int calcCurrentOverlap(struct edge edges[],int len,int debug){
    struct edge tmp_edges[4]; /* store current point's four direction's edge length.
                                    actually we can just use array : int tmp_edges[4];
                                    at this case, the index of array still means direction,
                                    but the content of the array stores edge length.
                                */
    int i;
    int direction;
    int overlap = 0;

    // init edges
    for (i = 0; i < 4; i++) {
        tmp_edges[i].len = 0;
    }

    for (i = 0; i < len; i++) {
        direction = edges[i].direction;

        if (direction >= 4 || direction < 0) {
            printf("direction wrong!\n");
            exit(1);
        }

        if (tmp_edges[direction].len == 0) {
                tmp_edges[direction].len = edges[i].len;
        } else {
                overlap += min(edges[i].len,tmp_edges[direction].len);
                tmp_edges[direction].len = max(edges[i].len,tmp_edges[direction].len);
        }


    }
    if (debug == 4){
        for (i = 0; i < len; i++){
            printf("edge dir:%d len:%d\n",edges[i].direction,edges[i].len);
        }

      //  printf("edge tmp len:%d\n",tmp_edges[2].len);
        printf("overlap %d\n",overlap);
        printf("\n");
    }


    return overlap;

}



//void paint(struct edge edges[],int len){
//    struct edge tmp_edges[18]; /* at most 18 edges since child is at most 8*/
//    int i,j;
//    int counter = 0;
//    int overlap = 0;
//    for (i = 0; i < len; i++) {
//        for (j = 0; j < counter; j++) {
//            if (edges[i].x == tmp_edges[j].x && edges[i].y == tmp_edges[j].y){
//                    // this two edge in the same line of the grid network
//                    overlap += min(edges[i].len, tmp_edges[j].len);
//                    tmp_edges[j].len = max(edges[i].len, tmp_edges[j].len);
//
//            } else if (edges[i].x != tmp_edges[j].x && edges[i].y == tmp_edges[j].y)
//                    if (edges[i].direction == h) {
//                        int max_x = max(edges[i].x, tmp_edges[j].x);
//                        int max_x_ind;
//                        if (edges[i].x > tmp_edges[j].x){
//                            max_x_ind = i;
//                        }
//                    }
//        }
//    }
//
//
//
//
//}


// below functions copy from assign 2
void checkMaxXY(int d){
	if(max_x > MAX_MAX_X || max_y > MAX_MAX_Y || max_x < 0 || max_y <0){
		printf("Input MAX_X/MAX_Y out side range\n");
		if(d==1){
			fclose(infile);
		}
		exit(1);
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
		printf("The board is too small to hold %d nodes\n", num_pt);
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

//Called when Cmd is invalid
void invalCmd(){
	printf(">%s [-i inputfile [-o outputfile]]\n", &progName[2]);
	exit(1);
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




//Func5
void geneRamdIns(){

	char insName[31];
	FILE *wfile;
    int i, j;
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
		sprintf(insName, "instance%03d_%03d.txt", num_pt, i);
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
		printf("instance%03d_%03d.txt generated", num_pt, i);
		if(i!=num_ins){
			printf("\n");
		}
	}
	printf(" ... done!\n");
}
