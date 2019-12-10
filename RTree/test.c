/**
   rtree lib usage example app.
*/

#include <cstdio>
#include <time.h>
#include <sys/time.h>
#include <unistd.h>
#include <queue>
#include <functional>
extern "C"{

#include "rtree.h"
}
#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#define innum 1000000
/*
RTREEMBR rects[] = {
    { {0, 0, 0, 2, 2, 0} },      
    { {5, 5, 0, 7, 7, 0} },
    { {8, 5, 0, 9, 6, 0} },
    { {7, 1, 0, 9, 2, 0} }
};
*/


RTREEMBR rects[innum];/* xmin, ymin, xmax, ymax (for 3 dimensional RTree) */

struct compare{
    bool operator()(RTREEBRANCH const& s1, RTREEBRANCH const& s2)
    {
        return s1.mind > s2.mind;
    };
};


double rad;
int cnt = 0;
int nrects = sizeof(rects) / sizeof(rects[0]);
RTREEMBR search_rect = {
    {90, 90,110, 110}   /* search will find above rects that this one overlaps */
};

using namespace std;

priority_queue< RTREEBRANCH, vector<RTREEBRANCH>, compare > min_heap;

void getElapsedTime(struct timeval Tstart, struct timeval Tend)
{
    Tend.tv_usec = Tend.tv_usec - Tstart.tv_usec;
    Tend.tv_sec  = Tend.tv_sec - Tstart.tv_sec;
    Tend.tv_usec += (Tend.tv_sec*1000000);

    printf("Elapsed Time: %lf sec\n", Tend.tv_usec / 1000000.0);
}

int MySearchCallback(int id, void* arg) 
{
    /* Note: -1 to make up for the +1 when data was inserted */
   // fprintf (stdout, "Hit data mbr %d \n", id-1);
    return 1; /* keep going */
}

//rectangle is a point ld(x,y) , ru(x,y)
void inputRTree()
{
    double x, y;
    for(int i = 0 ; i < innum ;i++)
    {
        scanf("%lf, %lf",&x,&y);

        rects[i].bound[0] = x; 
        rects[i].bound[1] = y; 
        rects[i].bound[2] = x; 
        rects[i].bound[3] = y;
    }
}

double dist_RT_and_point(RTREEMBR a,double  x,double y)
{
    double d = 0, t;
    t = (a.bound[1] - x);
    d += t*t;
    t = (a.bound[2] - y);
    d += t*t;

    //printf("%lf %lf\n",a->bound[1],a->bound[2]);
    return d;
}

void bruteSearch(RTREENODE *node, double x, double y, double radius)
{
    
    if( node->level > 0)
    {
        for(int i = 0; i < MAXCARD; i++)
        {
            if(node->branch[i].child)
            {

                bruteSearch(node->branch[i].child, x,y,radius);
            }
        }

    }
    else//leaf
    {
        for(int i = 0; i < MAXCARD ;i++)
        {
            if(node->branch[i].child)
            {
              if ( dist_RT_and_point(node->branch[i].mbr,x,y) <= radius )
                  cnt++;
            }
        }

    }
}


double min_dist(RTREEMBR a, double x, double y)
{
    double xmin = a.bound[0], ymin = a.bound[1], xmax = a.bound[2], ymax = a.bound[3];

    if(x >= xmin && x <= xmax)
    {
        if( y >= ymin && y<= ymax)
        {
            //in
            return 0;
        }
        else
        {
            //out only x
            //
            //
            return MIN( (y-ymin)*(y-ymin), (y-ymax)*(y-ymax) );
        }
    }
    else
    {
        if( y >= ymin && y<= ymax)
        {
            //in
            return MIN( (x-xmin)*(x-xmin), (x-xmax)*(x-xmax) );;
        }
        else
        {
            //out only x
            //
            //
            return MIN (  MIN( (y-ymin)*(y-ymin) + (x-xmin)*(x-xmin) , (y-ymax)*(y-ymax) + (x-xmin)*(x-xmin) ), MIN ( (y-ymin)*(y-ymin) + (x-xmax)*(x-xmax) , (y-ymax)*(y-ymax) + (x-xmax)*(x-xmax) ) );
        }
    }


}

void RangeQuery(RTREENODE *node, double x, double y, double radius)
{
   

    if( node->level > 0)
    {
        for(int i = 0; i < MAXCARD; i++)
        {
            if(node->branch[i].child)
            {
                double xmin = node->branch[i].mbr.bound[0], xmax = node->branch[i].mbr.bound[2];
                double ymin = node->branch[i].mbr.bound[1], ymax = node->branch[i].mbr.bound[3];
                //printf("%lf %lf %lf %lf\n",xmin,ymin,xmax,ymax);
               // printf("%lf %lf %lf %lf\n",node->branch[i].mbr.bound[0],node->branch[i].mbr.bound[1],node->branch[i].mbr.bound[2],node->branch[i].mbr.bound[3]);
                if( rad < (xmin-x) || 
                        rad < (x-xmax) || 
                        rad < (ymin-y) || 
                        rad < (y-ymax)
                        )
                    ;
                else
                    RangeQuery(node->branch[i].child, x,y,radius);
            }
        }

    }
    else
    {
        for(int i = 0; i < MAXCARD ;i++)
        {
            if(node->branch[i].child)
            {
              if ( dist_RT_and_point(node->branch[i].mbr,x,y) <= radius )
                  cnt++;
            }
        }

    }
}


void kNNQuery(RTREENODE *node, double x, double y, int K)
{

    // child 를 heap에다 전부넣고, pop하면서 k개를 찾는다.    

    if( node->level > 0)
    { 
   
        for(int i = 0; i < MAXCARD; i++)
        {
            if(node->branch[i].child)
            {
                node->branch[i].mind = min_dist(node->branch[i].mbr,x,y); 
                min_heap.push(node->branch[i]);
            }
        }// min heap에 노드 넣음.

        RTREEBRANCH br = min_heap.top();
        min_heap.pop();
        
        kNNQuery(br.child, x,y,K);
    }
    else // leaf
    {
        for(int i = 0; i < MAXCARD; i++)
        {
            if(node->branch[i].child)
            {
                node->branch[i].mind = min_dist(node->branch[i].mbr,x,y); 
                min_heap.push(node->branch[i]);
            }
        }// min heap에 노드 넣음.

        for(int i = 0 ; i < K ; i++)
        {
            RTREEBRANCH br = min_heap.top();
            min_heap.pop();
        }
    }

}


int main()
{
    RTREENODE* root = RTreeCreate();
    
    int i, nhits;
    struct timeval Tstart, Tend; //time value

    double xx = 250.00000 , yy = 250.00000;
    fprintf (stdout, "nrects = %d \n", nrects);
   
    inputRTree();

    /* Insert all the testing data rects */


    for(i=0; i<nrects; i++){
        RTreeInsertRect(&rects[i],  /* the mbr being inserted */
                        i+10,        /* i+1 is mbr ID. ID MUST NEVER BE ZERO */
                        &root,        /* the address of rtree's root since root can change undernieth*/
                        0            /* always zero which means to add from the root */
            );
    }

/*
    gettimeofday(&Tstart, NULL);
    nhits = RTreeSearch(root, &search_rect, MySearchCallback, 0);
    gettimeofday(&Tend, NULL);
    getElapsedTime(Tstart, Tend);

    fprintf (stdout, "Search resulted in %d hits \n", nhits);
*/

    printf("\n============bruteSearch=============\n");

    rad = 5;
        printf(">> R = %lf\n",rad);
    gettimeofday(&Tstart, NULL);
    bruteSearch(root,xx,yy,rad*rad);
    gettimeofday(&Tend, NULL);
    getElapsedTime(Tstart, Tend);
    printf("Search resulted in %d hits. \n\n",cnt);
    cnt = 0;

    for(int i = 1; i <= 5 ; i++)
    {
        rad = i*10;
        printf(">> R = %lf\n",rad);
        gettimeofday(&Tstart, NULL);
        bruteSearch(root,xx,yy,rad*rad);
        gettimeofday(&Tend, NULL);
        getElapsedTime(Tstart, Tend);
        printf("Search resulted in %d hits. \n\n",cnt);
        cnt = 0;
    }

    for(int i = 1; i <= 5 ; i++)
    {
        rad = i*100;
        printf(">> R = %lf\n",rad);
        gettimeofday(&Tstart, NULL);
        bruteSearch(root,xx,yy,rad*rad);
        gettimeofday(&Tend, NULL);
        getElapsedTime(Tstart, Tend);
        printf("Search resulted in %d hits. \n\n",cnt);
        cnt = 0;
    }

    printf("\n============RangeQuery=============\n");


    rad = 5;
        printf(">> R = %lf\n",rad);
    gettimeofday(&Tstart, NULL);
    RangeQuery(root,xx,yy,rad*rad);
    gettimeofday(&Tend, NULL);
    getElapsedTime(Tstart, Tend);
    printf("Search resulted in %d hits. \n",cnt);
    cnt = 0;

    for(int i = 1; i <= 5 ; i++)
    {
        rad = i*10;
        printf(">> R = %lf\n",rad);
        gettimeofday(&Tstart, NULL);
        RangeQuery(root,xx,yy,rad*rad);
        gettimeofday(&Tend, NULL);
        getElapsedTime(Tstart, Tend);
        printf("Search resulted in %d hits. \n\n",cnt);
        cnt = 0;
    }

    for(int i = 1; i <= 5 ; i++)
    {
        rad = i*100;
        printf(">> R = %lf\n",rad);
        gettimeofday(&Tstart, NULL);
        RangeQuery(root,xx,yy,rad*rad);
        gettimeofday(&Tend, NULL);
        getElapsedTime(Tstart, Tend);
        printf("Search resulted in %d hits. \n\n",cnt);
        cnt = 0;
    }

    printf("\n============kNNQuery=============\n");
      
    int k = 1;

    printf("K = %d\n",k);   
    gettimeofday(&Tstart, NULL);
    kNNQuery(root,xx,yy,k);
    gettimeofday(&Tend, NULL);
    getElapsedTime(Tstart, Tend);

    for (int i = 1 ; i <= 10;i++)
    {
        k = i*10;
        printf("K = %d\n\n",k);   
        gettimeofday(&Tstart, NULL);
        kNNQuery(root,xx,yy,k);
        gettimeofday(&Tend, NULL);
        getElapsedTime(Tstart, Tend);

    }
    RTreeDestroy (root);

    return 0;
}
