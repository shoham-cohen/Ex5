#include <fcntl.h>
#include <string>
#include <stdio.h>
#include <mutex>

using namespace std;

namespace my_namespace{

    class Node{

        public:
            string data;
            Node *next;
            Node(); //default constructor
            Node(string & data); //constructor 
            Node *next_node(){
                return this->next;
            }
            string &get_data(){
                return this->data;
            }
    };

    class Stack{
    
        public:
            Node *head;
            size_t length;
            Stack(); //default and only constructor
            int push(string data);
            string pop();
            string top();
            string top_helper();
            int create_file();
    };
}