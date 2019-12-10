//
//  kdtree.c
//  
//
//  Created by Bigdata LAB on 2019/11/05.
//

#include <cstdio>
#include <stack>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <queue>
#include <sys/time.h>
#include <unistd.h>
#define MAX_DIM 2
#define COMPARE(a, b) ((a > b) ? a : b)

using namespace std;

// kdtree의 node 자료구조 정의.
struct kd_node_t
{
    double x[MAX_DIM];
    struct kd_node_t *left, *right;
};


struct point{
    double x;
    double y;
};

struct Rect{

    double min_x, min_y;
    double max_x, max_y;
};

struct candidate_node{

    struct kd_node_t current_node;
    struct Rect rec;
};

priority_queue<double> rQ; // kNN resultQ

int result = 0;
int count = 0;
// 거리함수 정의.
inline double dist(struct kd_node_t *a, struct kd_node_t *b, int dim)
{
    double t, d = 0;
    while (dim--)
    {
        t = a->x[dim] - b->x[dim];
        d = d + t * t;
    }
   
    return d;
}

// time function
void getElapsedTime(struct timeval Tstart, struct timeval Tend)
{
    Tend.tv_usec = Tend.tv_usec - Tstart.tv_usec;
    Tend.tv_sec  = Tend.tv_sec - Tstart.tv_sec;
    Tend.tv_usec += (Tend.tv_sec*1000000);

    printf("Elapsed Time: %lf sec\n", Tend.tv_usec / 1000000.0);
}
// printf node
void print_kd_node(struct kd_node_t *p)
{

    printf("(x, y) = (");

    for (int i = 0 ; i < MAX_DIM;i++)
    {
        printf("%f, ",p->x[i]);
    }
    printf(")\n");
}


// swap 함수 정의.
inline void swap(struct kd_node_t *x, struct kd_node_t *y)
{
    double tmp[MAX_DIM];
    memcpy(tmp, x->x, sizeof(tmp));
    memcpy(x->x, y->x, sizeof(tmp));
    memcpy(y->x, tmp, sizeof(tmp));
}

// 중앙값 찾는 함수 정의. 중앙값은 kdtree의 node split point를 찾을 때 사용된다.
struct kd_node_t* find_median(struct kd_node_t *start, struct kd_node_t *end, int idx)
{
    if (end <= start) return NULL;
    if (end == start + 1)
        return start;
    
    struct kd_node_t *p, *store, *med;
    med = start + (end - start) / 2;
    double pivot;
    
    while (1)
    {
        pivot = med->x[idx];
        
        swap(med, end-1);
        for(store = p = start; p < end; p++)
            if(p->x[idx] < pivot)
            {
                if(p != store)
                    swap(p, store);
                store++;
            }
        swap(store, end - 1);
        
        if(store->x[idx] == med->x[idx])
            return med;
        if (store > med) end = store;
        else start = store;
    }
}

// recursion으로 kdtree를 build하는 함수.
struct kd_node_t* make_kdtree(struct kd_node_t *t, int len, int i, int dim)
{
    struct kd_node_t *n;
    
    if(!len) return NULL;
    
    if((n = find_median(t, t+len, i)))
    {
        i = (i + 1) % dim;
        n->left = make_kdtree(t, n-t, i, dim);
        n->right = make_kdtree(n+1, t+len-(n+1), i, dim);
    }
    
    return n;
}

void rangeQuery(struct kd_node_t *p, struct kd_node_t *cur, double radius, int rank)
{
    //range query의 질의 조건인 질의 포인트와 질의 반경
    //
    // You will start with root node.
    
    struct kd_node_t *n;
    
    if( dist(p, cur, MAX_DIM) <= radius)
    {
       //print_kd_node(cur); 
       result++;
    }


    if( rank % 2 == 1 )//odd rank
    {
        if(cur->x[0] >= p->x[0]) // left
        {
            if(cur->left != NULL)
                rangeQuery(p, cur->left, radius,rank+1);

            if(cur->x[0] <= p->x[0] + radius)
            {
                if(cur->right != NULL)
                    rangeQuery(p, cur->right, radius, rank+1);
            }
        }
        else //        if(cur->x[0] < p->x[0]) // right
        {
            if(cur->right != NULL)
                rangeQuery(p, cur->right, radius, rank+1);

            if(cur->x[0] >= p->x[0] - radius)
            {
                if(cur->left != NULL)
                    rangeQuery(p, cur->left, radius,rank+1);
            }
        }
    }
    else // even rank
    {
        if(cur->x[1] >= p->x[1]) // down
        {
            if(cur->left != NULL)
                rangeQuery(p, cur->left, radius,rank+1);

            if(cur->x[1] <= p->x[1] + radius)
            {
                if(cur->right != NULL)
                    rangeQuery(p, cur->right, radius, rank+1);
            }
        }
        else //if(cur->x[1] < p->x[1]) // up
        {
            if(cur->right != NULL)
                rangeQuery(p, cur->right, radius, rank+1);

            if(cur->x[1] >= p->x[1] - radius)
            {
                if(cur->left != NULL)
                    rangeQuery(p, cur->left, radius,rank+1);
            }
        }
    }
}

void kNNQuery(struct kd_node_t *p, struct kd_node_t *cur, double radius, int rank, int K)
{
    //kNN query의 질의조건인 질의 포인트와 최근접이웃 개수
    //radius로 rangeQuery 하여 K개를 찾으면 이중 제일 먼걸로 r을 업데이트후, 계속 rangeQuery 반복하여 
    struct kd_node_t *n;
    
    double distt = dist(p, cur, MAX_DIM) ;
    if( distt  <= radius)
    {
       rQ.push(distt); // result queue push
       //print_kd_node(cur); 
    
       if (rQ.size() == K)
       {
            radius = rQ.top();//pq front . dist

            while(!rQ.empty())
            {
                rQ.pop();
            }
       }
    }


    if( rank % 2 == 1 )//odd rank
    {
        if(cur->x[0] >= p->x[0]) // left
        {
            if(cur->left != NULL)
                kNNQuery(p, cur->left, radius,rank+1,K);

            if(cur->x[0] <= p->x[0] + radius)
            {
                if(cur->right != NULL)
                    kNNQuery(p, cur->right, radius, rank+1,K);
            }
        }
        else //        if(cur->x[0] < p->x[0]) // right
        {
            if(cur->right != NULL)
                kNNQuery(p, cur->right, radius, rank+1,K);

            if(cur->x[0] >= p->x[0] - radius)
            {
                if(cur->left != NULL)
                    kNNQuery(p, cur->left, radius,rank+1,K);
            }
        }
    }
    else // even rank
    {
        if(cur->x[1] >= p->x[1]) // down
        {
            if(cur->left != NULL)
                kNNQuery(p, cur->left, radius,rank+1,K);

            if(cur->x[1] <= p->x[1] + radius)
            {
                if(cur->right != NULL)
                    kNNQuery(p, cur->right, radius, rank+1,K);
            }
        }
        else //if(cur->x[1] < p->x[1]) // up
        {
            if(cur->right != NULL)
                kNNQuery(p, cur->right, radius, rank+1,K);

            if(cur->x[1] >= p->x[1] - radius)
            {
                if(cur->left != NULL)
                    kNNQuery(p, cur->left, radius,rank+1,K);
            }
        }
    }
}

//recursive
void brutequery(struct kd_node_t *p, struct kd_node_t *cur, double radius)
{
    
    // You will start with root node.

    if( dist(p, cur, MAX_DIM) <= radius)
    {
       //print_kd_node(cur); 
        result++;
    }

    if(cur->left != NULL)
        brutequery(p, cur->left, radius);

    if(cur->right != NULL)
        brutequery(p, cur->right, radius);

}

int main(void)
{

    struct kd_node_t *a = (struct kd_node_t *)malloc(sizeof(struct kd_node_t)*1000001) ;
    struct kd_node_t input;
    struct kd_node_t *input_use;
    struct kd_node_t *root;
    double rad;
    int k = 1;
    int N = 1000000;

 //   printf("N = ");
 //   scanf("%d", &N);
    
    for (int i = 0 ; i < N;i++)
    {
        //printf("(x, y) = %d\n ",i);
        scanf("%lf, %lf",&a[i].x[0], &a[i].x[1]);
    }
    root = make_kdtree(a, N, 0, MAX_DIM);

    input.x[0] = 250;
    input.x[1] = 250;


    printf("[------Range query------]\n");
    printf("input point (x, y) : %lf, %lf\n ",input.x[0],input.x[1]);
 //   scanf("%lf %lf",&input.x[0], &input.x[1]);
 //
    
    struct timeval Tstart, Tend; //time value

    
    for(int i = 1 ; i <= 10 ; i++)
    {
        rad = 10*i;
        printf("input radius : %.2lf\n", rad);
        input_use = &input;
        printf("-----------Output-----------\n");
        		
        gettimeofday(&Tstart, NULL); 
        rangeQuery(input_use,root,rad*rad,1); 
		gettimeofday(&Tend, NULL);
		getElapsedTime(Tstart, Tend);

        printf(" %d points are founded.\n", result);
        result = 0;

    }

    printf("\n[------brute query------]\n");
    for(int i = 1 ; i <= 10 ; i++)
    {
        rad = 10*i;
        printf("input radius : %.2lf\n", rad);
        input_use = &input;
        printf("-----------Output-----------\n");

        gettimeofday(&Tstart, NULL); 
        brutequery(input_use,root,rad*rad);
		gettimeofday(&Tend, NULL);
		getElapsedTime(Tstart, Tend);

        printf(" %d points are founded.\n", result);
        result = 0;

    }
    printf("\n[------kNN query------]\n");
    
    for(int i = 1 ; i <= 10 ; i++)
    {
        k = 1*i;
   
        printf("[K = %d] \n",k);
        gettimeofday(&Tstart, NULL); 
        kNNQuery(input_use,root,1000000,1,k);
        gettimeofday(&Tend, NULL);
		getElapsedTime(Tstart, Tend);
        puts("");

    }

    return 0;
}
