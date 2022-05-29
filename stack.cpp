#include <stdio.h>
#include <string>
#include "stack.hpp"
#include <iostream>
#include <mutex>
#include <cstring>

using namespace std;
using namespace my_namespace;

int fd;
struct flock locker;


int my_namespace::Stack::create_file()
{
    fd = open("locker.txt", O_WRONLY | O_CREAT);

    if (fd == -1)
    {
        printf("Error Number % d\n", errno);

        perror("Failed");
    }
    memset(&locker, 0, sizeof(locker));
    return fd;
}



//default constractor
my_namespace::Node::Node(){
    this -> next = NULL;
}

my_namespace::Node::Node(string &data){
    this -> next = NULL;
    this -> data = data;
}

my_namespace::Stack::Stack(){
    this -> head = new Node();
    this -> length = 0;
}

int my_namespace::Stack::push(string data){
    locker.l_type = F_WRLCK;
    fcntl(fd, F_SETLKW, &locker);
    //data cant be longer than 1024 bytes
    if(data.size() > 1024){
        locker.l_type = F_UNLCK;
        fcntl(fd, F_SETLKW, &locker);
        throw runtime_error("cant handle with string that longer than 1024 chars");
    }
    Node *new_node = new Node(data);
    new_node -> next = this -> head;
    this -> head = new_node;
    this -> length += 1;
    locker.l_type = F_UNLCK;
    fcntl(fd, F_SETLKW, &locker);
    return 1;
}

string my_namespace::Stack::pop(){
    Node *temp = this->head;
    this-> head = temp->next_node();
    string res = temp->get_data();
    this->length -= 1;
    return res;
}

string my_namespace::Stack::top(){
    locker.l_type = F_WRLCK;
    fcntl(fd, F_SETLKW, &locker);
    string res = this->top_helper();
    locker.l_type = F_UNLCK;
    fcntl(fd, F_SETLKW, &locker);
    return res;
}

string my_namespace::Stack::top_helper(){
    return this->head->data;
}