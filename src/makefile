all: sssp_serial sssp_blocking sssp_lockfree

sssp_serial: sssp_serial.cpp Timer.h simplegraph.h serialqueue.h
	g++ -g -O3 $< -o $@

bq_test: test.cpp blocking_queue.h
	g++ -g -std=c++11 -O3 $< -o $@

lf_test: test2.cpp lockfree_queue.h
	g++ -g -std=c++11 -O3 $< -o $@

sssp_blocking: sssp_blocking.cpp Timer.h simplegraph.h blocking_queue.h
	g++ -g -std=c++11 -pthread -O3 $< -o $@

sssp_lockfree: sssp_lockfree.cpp Timer.h simplegraph.h lockfree_queue.h
	g++ -g -std=c++11 -pthread -O3 $< -o $@
