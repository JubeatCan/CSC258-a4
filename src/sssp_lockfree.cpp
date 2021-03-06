/* -*- mode: c++ -*- */

#include <stdio.h>
#include <assert.h>
#include <limits.h>
#include <thread>
#include <cstring>
#include <mutex>
#include <atomic>
#include "simplegraph.h"
#include "lockfree_queue.h"
#include "Timer.h"
typedef SimpleCSRGraph<unsigned int, std::atomic<int> > SimpleCSRGraphUII;

const int INF = INT_MAX;
int numofthreads;
SimpleCSRGraphUII input;
LockfreeQueue sq;
std::atomic<int> check[170] = {};

void sssp_init(unsigned int src) {
  for(int i = 0; i < input.num_nodes; i++) {
    input.node_wt[i] = (i == src) ? 0 : INF;
  }
}

void sssp(int t) {
  bool changed = false;
  int node;
  bool flag;
  while(true){
    flag = true;
    while(sq.pop(node)) {
      for(unsigned int e = input.row_start[node]; e < input.row_start[node + 1]; e++) {

        unsigned int dest = input.edge_dst[e];
        int distance = input.node_wt[node].load( std::memory_order_relaxed )  + input.edge_wt[e];

        for(;;){
        int prev_distance = input.node_wt[dest];
        
        if(prev_distance <= distance) {
          break;
        }else if(input.node_wt[dest].compare_exchange_weak(prev_distance, distance)){
          changed = true;
          if(!sq.push(dest)) {
	          fprintf(stderr, "ERROR: Out of queue space.\n");
	          exit(1);
	        }
          break;
        }
      }
      }
   
    }
    check[t] = 1;
    for(int i = 0; i < numofthreads; i++)
    {
      if(!check[i].load())
      {
        flag = false;
        break;
      }
    }
    if(flag)
    {
      //printf("%d thread returns.",t);
      return;

    }
    else
      continue;
    
  }
    
  }

void write_output(SimpleCSRGraphUII &g, const char *out) {
  FILE *fp;

  fp = fopen(out, "w");
  if(!fp) {
    fprintf(stderr, "ERROR: Unable to open output file '%s'\n", out);
    exit(1);
  }

  for(int i = 0; i < g.num_nodes; i++) {
    int r;
    if(g.node_wt[i] == INF) {
      r = fprintf(fp, "%d INF\n", i);
    } else {
      r = fprintf(fp, "%d %d\n", i, g.node_wt[i].load());
    }

    if(r < 0) {
      fprintf(stderr, "ERROR: Unable to write output\n");
      exit(1);
    }
  }
}

int main(int argc, char *argv[])
{
  if(argc != 4) {
    fprintf(stderr, "Usage: %s inputgraph outputfile numofthreads\n", argv[0]);
    exit(1);
  }

  char *pEnd;
  numofthreads = strtol(argv[3],&pEnd,10);
  if( numofthreads > 160 )
  {
    fprintf(stderr, "Too many errors.");
    exit(1);
  }



  if(!input.load_file(argv[1])) {
    fprintf(stderr, "ERROR: failed to load graph\n");
    exit(1);
  }

  printf("Loaded '%s', %u nodes, %u edges\n", argv[1], input.num_nodes, input.num_edges);

  /* if you want to use dynamic allocation, go ahead */
  sq.initialize(); // should be enough ...

  ggc::Timer t("sssp");

  int src = 0;

  /* no need to parallelize this */
  sssp_init(src);
  std::thread threads[numofthreads];

  t.start();
  sq.push(src);
  for(int i = 0; i < numofthreads; i++)
  {
    threads[i] = std::thread(sssp,i);
  }
  for(int i = 0; i < numofthreads; i++)
  {
    threads[i].join();
  }
  t.stop();

  printf("Total time: %u ms\n", t.duration_ms());

  write_output(input, argv[2]);

  printf("Wrote output '%s'\n", argv[2]);
  return 0;
}
