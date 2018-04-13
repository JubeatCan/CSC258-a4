#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <mutex>
#include <condition_variable>
#include <deque>
#include <assert.h>
#include <atomic>
#include <stddef.h>
#include <stdint.h>
#include <malloc.h>

// typedef struct pointer_t{
//         node_t* ptr;
//         unsigned int count;
// }pointer_t;

// typedef struct node_t{
//         int value;
//         pointer_t next;
// }node_t;


class LockfreeQueue{
    public:

        struct node_t;
        
        

        struct pointer_t{
            node_t *ptr;
            uint64_t count;
            pointer_t()
            {
                ptr = NULL;
                count = 0;
            }
            pointer_t(node_t *p, uint64_t c)
            {
                ptr = p;
                count = c;
            }
            pointer_t(pointer_t const & p)
            {
                ptr = p.ptr;
                count = p.count;
            }
        }__attribute__( (packed,aligned(16)) );

        struct node_t{
            int value;
            volatile pointer_t next;
            void init(int val)
            {
                value = val;
                next.ptr = NULL;
                next.count = 0;
            }
        };

        /*
         *  Reference:
         *  https://gist.github.com/linuxholic/d0ba7035efd6b206cff2bfa5fab79e1b
         */
        static inline bool CAS(pointer_t volatile *addr, pointer_t &old_value, pointer_t &new_value)
        {
            bool ret;
            __asm__ __volatile__(
                "lock cmpxchg16b %1;\n"
                "sete %0;\n"
                :"=m"(ret),"+m" (*(volatile pointer_t *) (addr))
                :"a" (old_value.ptr), "d" (old_value.count), "b" (new_value.ptr), "c" (new_value.count)
            );
            return ret;
        }

        void initialize()
        {
            node_t *node = (node_t *)memalign(16, sizeof(node_t));
            node->init(0);
            Head.count = Tail.count = 0;
            Head.ptr = Tail.ptr = node;
        }

        int push(int item)
        {
            node_t *node = (node_t *)memalign(16, sizeof(node_t));
            node->init(item);
            //node->value = value;
            //node->next.ptr = NULL;
            pointer_t tail(NULL, 0);
            pointer_t next(NULL, 0);
            while(true)
            {
                //pointer_t tail = Tail;
                //pointer_t next = tail.ptr->next;
                tail.ptr = this->Tail.ptr;
                tail.count = this->Tail.count;
                next.ptr = tail.ptr->next.ptr;
                next.count = tail.ptr->next.count;

                if(tail.ptr == this->Tail.ptr && tail.count == this->Tail.count)
                {
                    if(next.ptr == NULL)
                    {
                        pointer_t temp(node,next.count+1);
                        //temp.ptr = (node_t *)node;
                        //temp.count = next.count+1;
                        node->next.count = temp.count;
                        // if(CAS((long long *)&(tail.ptr->next),*(long long *)&next,(long long *)pointer_t{node,next.count+1}))
                        //     break;
                        // else{
                        //     break;
                        // }
                        if(CAS(&(this->Tail.ptr->next), next, temp)){
                            break;
                        }
                    }
                    else{
                        pointer_t temp(next.ptr, tail.count+1);
                        node->next.count = temp.count;
                        CAS(&(this->Tail), tail, temp);
                    }
                    // if(next.ptr == NULL)
                    // {
                    //     if(atomic_compare_exchange_weak(&(tail.ptr->next),next,pointer_t(node,next.count+1)))
                    //     {

                    //     }
                    // }
                }
            }
            pointer_t temp(node,tail.count+1);
            CAS(&(this->Tail), tail, temp);
            return 1;
        }

        int pop(int &item)
        {
            pointer_t tail(NULL, 0);
            pointer_t head(NULL, 0);
            pointer_t next(NULL, 0);
            while(true)
            {
                head.ptr = this->Head.ptr;
                head.count = this->Head.count;
                tail.ptr = this->Tail.ptr;
                tail.count = this->Tail.count;
                next.ptr = (head.ptr)->next.ptr;
                next.count = (head.ptr)->next.count;
                if ((head.ptr == this->Head.ptr) && (head.count == this->Head.count))
                {
                    if(head.ptr == tail.ptr){
                        if(next.ptr == NULL)
                            return false;
                        pointer_t temp(next.ptr, tail.count+1);
                        CAS(&(this->Tail), tail, temp);
                    }
                    else{
                        item = next.ptr->value;
                        pointer_t temp(next.ptr, head.count+1);
                        if(CAS(&(this->Head), head, temp))
                            break;
                    }
                }
            }
            free(head.ptr);
            return true;

        }

        void dump()
        {
            pointer_t next;
            next.ptr = (Head.ptr)->next.ptr;
            int i = 0;
            while(next.ptr!=NULL)
            {
                std::cout << "Q" << i << ": " << next.ptr->value << std::endl;
                i++;
                next.ptr = next.ptr->next.ptr;
            }
        }

        
    private:
        volatile pointer_t Head;
        volatile pointer_t Tail;
};
