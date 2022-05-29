/*
** server.c -- a stream socket server demo
*/

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include "stack.hpp"
#include <iostream>
#include <sys/mman.h>

using namespace std;

//declaration of the start_routine method
void* start_routine(void* new_fd);
my_namespace::Stack* stack;



#define MAXDATASIZE 1024

#define PORT "3490"  // the port users will be connecting to

#define BACKLOG 10   // how many pending connections queue will hold

void sigchld_handler(int s)
{
    // waitpid() might overwrite errno, so we save and restore it:
    int saved_errno = errno;

    while(waitpid(-1, NULL, WNOHANG) > 0);

    errno = saved_errno;
}


// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int which_command(char * input, char * command){
    for(int i = 0; i < strlen(input); i++){
        if(input[i] != command[i]){
            return 0;
        }
    }
    return 1;
}

void handle(char input[MAXDATASIZE], int client){

    if(which_command((char*)"POP", input) == 1){
        string res = stack->pop();
        cout << "OUTPUT: " << res << " has been poped out of the stack\n";
        char * message = (char *)calloc(MAXDATASIZE, sizeof(char));
        for(int i = 0; i < res.size(); i++){
            message[i] = res.at(i);
        }
        message[res.size()] = '\0';
        send(client ,message ,strlen(message) + 1, 0);
        free(message);
    }

    else if(which_command((char*)"PUSH", input) == 1){
        
        string res;
        int i;
        for(i = 0; i < strlen(input) && input[i] != ' ' && input[i] != '\n' && input[i] != '\0'; i++){
            ;//just reach to the wanted value to push
        }
        i++;
        for(; i < strlen(input) && input[i] != '\0' && input[i] != '\n'; i++){
            res.push_back(input[i]);
        }
        res.push_back('\0');
        int success = stack->push(res);
        if (success == 1){
            char * message = (char*)"the wanted value has been pushed succesfully to the stack";
            send(client ,message ,strlen(message) + 1, 0);
        }
                
    }

    else if(which_command((char*)"TOP", input) == 1){
        string res = stack->top();
        cout << "OUTPUT:" << res << " is the top element in the stack\n";
        char * message = (char *)calloc(MAXDATASIZE, sizeof(char));
        for(int i = 0; i < res.size(); i++){
            message[i] = res.at(i);
        }
        message[res.size()] = '\0';
        send(client ,message ,strlen(message) + 1, 0);
        free(message);
    }
}

int main(void)
{
    //int fd = my_namespace::Stack::create_file();
    stack = (my_namespace::Stack*)mmap(0, 2000, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_SHARED | MAP_ANON, -1, 0);
    int sockfd, new_fd;  // listen on sock_fd, new connection on new_fd
    struct addrinfo hints, *servinfo, *p;
    struct sockaddr_storage their_addr; // connector's address information
    socklen_t sin_size;
    struct sigaction sa;
    int yes=1;
    char s[INET6_ADDRSTRLEN];
    int rv;

    pthread_t thread_pool[BACKLOG]; // TID array by index, 10 members
    bool active_threads[BACKLOG]; // array of booleans
    //int thread_cursor = 0;
    memset(thread_pool, 0, sizeof(thread_pool)); // zero out all cells
    memset(active_threads, 0, sizeof(active_threads));

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE; // use my IP

    if ((rv = getaddrinfo(NULL, PORT, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    // loop through all the results and bind to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                p->ai_protocol)) == -1) {
            perror("server: socket");
            continue;
        }

        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes,
                sizeof(int)) == -1) {
            perror("setsockopt");
            exit(1);
        }

        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("server: bind");
            continue;
        }

        break;
    }

    freeaddrinfo(servinfo); // all done with this structure

    if (p == NULL)  {
        fprintf(stderr, "server: failed to bind\n");
        exit(1);
    }

    if (listen(sockfd, BACKLOG) == -1) {
        perror("listen");
        exit(1);
    }

    sa.sa_handler = sigchld_handler; // reap all dead processes
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    if (sigaction(SIGCHLD, &sa, NULL) == -1) {
        perror("sigaction");
        exit(1);
    }

    printf("server: waiting for connections...\n");

    while(1) {  // main accept() loop
        sin_size = sizeof their_addr;
        new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
        if (new_fd == -1) {
            perror("accept");
            continue;
        }

        inet_ntop(their_addr.ss_family,
            get_in_addr((struct sockaddr *)&their_addr), s, sizeof(s));
        printf("server: got connection from %s\n", s);

        /* 

        instead of using the fork method we will use pthread_create
        the pthread_create method is going to start a new thread using the 
        start_routine methon which we implement below the main method

        if (!fork()) { // this is the child process
            close(sockfd); // child doesn't need the listener
            if (send(new_fd, "Hello, world!", 13, 0) == -1)
                perror("send");
            close(new_fd);
            exit(0);
        }
        close(new_fd);  // parent doesn't need this
        */
        pthread_t new_thread; // new thread to create
        /* 
            pthread_create gets the pointer to the new thread, the wanted attributes for 
            the new thread (NULL is default attributes), the start_routine method to invoke the thread
            and new_fd which is the argument to insert to the start_routine method

        */
        pthread_create(&new_thread, NULL, &start_routine, (void *)&new_fd);
    }
 
    return 0;
}

void * start_routine(void * new_fd){
    
    int client = *(int*)new_fd;
    int success;
    char message[MAXDATASIZE];
    while(1){
        success = recv(client, message, MAXDATASIZE, 0);
        if (success == -1 || success == 0){
            printf("we are done with this client\n\n");
            break;
        }
        printf("handelling client's new request\n");
        handle(message, client);
        memset(message, 0, MAXDATASIZE);
    }
    close(client);
    pthread_exit(NULL);
    return NULL;
}