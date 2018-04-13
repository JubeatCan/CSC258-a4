#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <mutex>
#include <condition_variable>
#include <deque>
#include <assert.h>

#include "lockfree_queue.h"

using namespace std;

int main()
{
        LockfreeQueue n;
            n.initialize();
                n.push(2);
                    n.push(3);
                        n.push(4);
                            n.dump();
                                int m;
                                    n.pop(m);

                                        n.pop(m);
                                            n.push(100);
                                                cout << m << endl;
                                                    n.dump();
                                                    //
}
