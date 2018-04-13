#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <mutex>
#include <condition_variable>
#include <deque>
#include <assert.h>

class node_t{
    public:
        int value;
        node_t* next;
};

class BlockingQueue{
    private:
        node_t* head;
        node_t* tail;
        mutable std::mutex H_lock;
        mutable std::mutex T_lock;
        int itemsNum;
        //int maxSize;
    public:
        void initialize(unsigned int maxqsize){
           // maxSize = maxqsize;
            itemsNum = 0;
            node_t* node = new node_t();
            node->next = NULL;
            head = tail = node;
            H_lock.unlock();
            T_lock.unlock();
        };

        int push(int item){
            // if (itemsNum+1 > maxSize)
            // {
            //     return 0;
            // }
            node_t* node = new node_t();
            node->value = item;
            node->next = NULL;
            T_lock.lock();

            tail->next = node;
            tail = node;
            itemsNum++;

            T_lock.unlock();
            return 1;
        };

        int pop(int &item){
            H_lock.lock();
            node_t* node = head;
            node_t* new_head = node->next;
            if(new_head == NULL)
            {
                H_lock.unlock();
                return 0;
            }
            item = new_head->value;
            head = new_head;

            H_lock.unlock();

            delete node;
            return 1;
        };

        void dump()
        {
            node_t* node = head->next;
            int i = 0;
            while(node != NULL)
            {
                std::cout << "Q" << i << ": " << node->value << std::endl;
                i++;
                node = node->next;
            }
        };
};