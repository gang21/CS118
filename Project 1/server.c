#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <arpa/inet.h>
#include <sys/socket.h>



/**
 * Project 1 starter code
 * All parts needed to be changed/added are marked with TODO
 */

#define BUFFER_SIZE 1024
#define DEFAULT_SERVER_PORT 8081
#define DEFAULT_REMOTE_HOST "131.179.176.34"
#define DEFAULT_REMOTE_PORT 5001

struct server_app {
    // Parameters of the server
    // Local port of HTTP server
    uint16_t server_port;

    // Remote host and port of remote proxy
    char *remote_host;
    uint16_t remote_port;
};

// The following function is implemented for you and doesn't need
// to be change
void parse_args(int argc, char *argv[], struct server_app *app);

// The following functions need to be updated
void handle_request(struct server_app *app, int client_socket);
void serve_local_file(int client_socket, const char *path);
void proxy_remote_file(struct server_app *app, int client_socket, const char *path);
char *read_file(char *filename);
int find_length(char *filename);

// The main function is provided and no change is needed
int main(int argc, char *argv[])
{
    struct server_app app;
    int server_socket, client_socket;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len;
    int ret;

    parse_args(argc, argv, &app);

    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(app.server_port);

    // The following allows the program to immediately bind to the port in case
    // previous run exits recently
    int optval = 1;
    setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));

    if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(server_socket, 10) == -1) {
        perror("listen failed");
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d\n", app.server_port);

    while (1) {
        client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &client_len);
        if (client_socket == -1) {
            perror("accept failed");
            continue;
        }
        
        printf("Accepted connection from %s:%d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
        handle_request(&app, client_socket);
        close(client_socket);
    }

    close(server_socket);
    return 0;
}

void parse_args(int argc, char *argv[], struct server_app *app)
{
    int opt;

    app->server_port = DEFAULT_SERVER_PORT;
    app->remote_host = NULL;
    app->remote_port = DEFAULT_REMOTE_PORT;

    while ((opt = getopt(argc, argv, "b:r:p:")) != -1) {
        switch (opt) {
        case 'b':
            app->server_port = atoi(optarg);
            break;
        case 'r':
            app->remote_host = strdup(optarg);
            break;
        case 'p':
            app->remote_port = atoi(optarg);
            break;
        default: /* Unrecognized parameter or "-?" */
            fprintf(stderr, "Usage: server [-b local_port] [-r remote_host] [-p remote_port]\n");
            exit(-1);
            break;
        }
    }

    if (app->remote_host == NULL) {
        app->remote_host = strdup(DEFAULT_REMOTE_HOST);
    }
}

void handle_request(struct server_app *app, int client_socket) {
    char buffer[BUFFER_SIZE];
    ssize_t bytes_read;

    // Read the request from HTTP client
    // Note: This code is not ideal in the real world because it
    // assumes that the request header is small enough and can be read
    // once as a whole.
    // However, the current version suffices for our testing.
    bytes_read = recv(client_socket, buffer, sizeof(buffer) - 1, 0);
    if (bytes_read <= 0) {
        return;  // Connection closed or error
    }

    buffer[bytes_read] = '\0';
    // copy buffer to a new string
    char *request = malloc(strlen(buffer) + 1);
    strcpy(request, buffer);

    printf("\nI am now requesting this: %s\n\n", request);

    // TODO: Parse the header and extract essential fields, e.g. file name
    // Hint: if the requested path is "/" (root), default to index.html

    //parsing the string
    char file_name[] = "index.html";        // default
    char *manipulate_request = malloc(strlen(request)+1);
    strcpy(manipulate_request, request);
    char *get = strtok(manipulate_request, " ");       // GET request
    char *file = strtok(NULL, " ");         // file name
    char *manipulate_file = malloc(strlen(file)+1);
    strcpy(manipulate_file, file);
    char *http = strtok(NULL, "\n");        // HTTP version
    char *name = strtok(manipulate_file, ".");
    char *extension = strtok(NULL, "\0"); 
    

    //check for get request
    if (strcmp(get, "GET") == 0) {
        // TODO: Implement proxy and call the function under condition
        // specified in the spec
        // if (need_proxy(...)) {
        //    proxy_remote_file(app, client_socket, file_name);
        // } else {
        // serve_local_file(client_socket, file_name);
        //}
        
        
        if(extension && strcmp(extension,"ts")==0){
            printf("HANDLING REQUEST, EXTENSION IS: %s\n",extension);
            proxy_remote_file(app, client_socket, request);
        }
        else{
            //get filename 
            if (strcmp(file, "/") == 0) {
                printf("FILE: /\n");
                serve_local_file(client_socket, file_name);
            }
            else {
                if (file[0] == '/') {
                    char mod_file[strlen(file)];
                    strcpy(mod_file, &file[1]);
                    printf("FILE: %s\n", mod_file);
                    serve_local_file(client_socket, mod_file);
                }
                else {
                    printf("FILE: %s\n", file);
                    serve_local_file(client_socket, file);
                }
            }
        }

        
    }

    // print statements for testing
    printf("\nThis is get: %s\n", get);
    printf("This is file: %s\n", file);
    printf("This is http: %s", http);
    printf("\n\nThis is request: %s\n\n", request);
    printf("\nThat's ONE LOOP!!!!\n**************\n\n\n");

    // TODO: Implement proxy and call the function under condition
    // specified in the spec
    // if (need_proxy(...)) {
    //    proxy_remote_file(app, client_socket, file_name);
    // } else {
    // serve_local_file(client_socket, file_name);
    //}
}

void serve_local_file(int client_socket, const char *path) {
    // TODO: Properly implement serving of local files
    // The following code returns a dummy response for all requests
    // but it should give you a rough idea about what a proper response looks like
    // What you need to do 
    // (when the requested file exists):
    // * Open the requested file
    // * Build proper response headers (see details in the spec), and send them
    // * Also send file content
    // (When the requested file does not exist):
    // * Generate a correct response

    //parse the file name
    printf("Start parsing %s\n",path);
    char *file = strtok(path, ".");
    char *extension = strtok(NULL, "\0");       //extentsion
    char *full_path;
    
    if(extension){
        full_path = malloc(strlen(file) + strlen(extension) + 2);
        strcpy(full_path, file);
        strcat(full_path, ".");
        strcat(full_path, extension);
        
    }
    else{
        full_path = malloc(strlen(file));
        strcpy(full_path,file);

    }

    //checking for special characters
    // while(strstr(full_path, "%%20") != NULL) {
    //     printf("\n--------------SPECIAL CHARACTERS--------------\n");
    //     char *first_half = strtok(full_path, "%");
    //     char *second_half = strtok(NULL, "\0");
    //     printf("FIRST HALF: %s\n", first_half);
    //     // char *edited_second_half = &(second_half)[2];
    //     //checking for spaces
    //     if (second_half[0] == '2' && second_half[1] == '0') {
    //         printf("********THIS HAS %20********\n");
    //         strcpy(full_path, first_half);
    //         strcat(full_path, " ");
    //         strcat(full_path, second_half);
    //     }
    //     //checking for % sign
    //     if (second_half[0] == '2' && second_half[1] == '5') {
    //         printf("********THIS HAS %25********\n");
    //         strcpy(full_path, first_half);
    //         strcat(full_path, "%");
    //         strcat(full_path, second_half);
    //     }
        // else {
        //     printf("********THIS IS THE ELSE STATEMENT********\n");
        //     strcpy(full_path, first_half);
        //     strcat(full_path, "%");
        //     strcat(full_path, second_half);
        // }
    //     printf("FIRST HALF: %s\n", first_half);
    //     printf("SECOND HALF: %s\n", second_half);
    //     printf("FULL PATH: %s\n", full_path);
        
    //     printf("--------------SPECIAL CHARACTERS--------------\n\n");
    // }
    while(strstr(full_path, "%20") != NULL) {
        printf("\n--------------SPECIAL CHARACTERS (SPACES)--------------\n");
        char *ret;
        char *ptr = full_path;

        ret = strstr(full_path, "%20");
        
        int index = ret-ptr;

        printf("The substring is: %c\n", full_path[index]);
        full_path[index]=' ';
        printf("The string is now %s\n", full_path);
        printf("Upper bound is %d\n",strlen(full_path));
        
        int i;
        for (i=index+1; i<strlen(full_path)-2; i++){
            full_path[i]=full_path[i+2];
            //printf("copyied: %c  @  %d",full_path[i],i);
            //printf(" With modified: %s\n", full_path);
        }
        full_path[i]='\0';
        
        printf("EDITED STRING: %s\n", full_path);
        printf("--------------SPECIAL CHARACTERS--------------\n\n");
    }
   
    while(strstr(full_path, "%25") != NULL) {
        printf("\n--------------SPECIAL CHARACTERS (%%)--------------\n");
        char *ret;
        char *ptr = full_path;

        ret = strstr(full_path, "%25");
        
        int index = ret-ptr;

        printf("The substring is: %c\n", full_path[index]);
        full_path[index]='%';
        printf("The string is now %s\n", full_path);
        printf("Upper bound is %d\n",strlen(full_path));
        
        int i;
        for (i=index+1; i<strlen(full_path)-2; i++){
            full_path[i]=full_path[i+2];
            //printf("copyied: %c  @  %d",full_path[i],i);
            //printf(" With modified: %s\n", full_path);
        }
        full_path[i]='\0';
        printf("EDITED STRING: %s\n", full_path);
        printf("--------------SPECIAL CHARACTERS--------------\n\n");
    }   
    printf("FULL PATH: %s\n", full_path);
    printf("extension: %s\n", extension);


    //TODO: retrieve file

    char *file_contents = read_file(full_path);
    if (file_contents == NULL) {
        printf("Error reading file.\n");
        char response[] = "HTTP/1.0 404 Not Found\r\n\r\n";
        send(client_socket, response, strlen(response), 0);
    }

    else{
        printf("File Contents:\n\n%s\n", file_contents);

        // get content length
        int content_length = find_length(full_path);
        // int content_length = strlen(file_contents);
        printf("Content length is: %d\n",content_length);

        //test
        // char response[] = "HTTP/1.1 200 OK\r\nContent-Type: text/html; charset=UTF-8\r\nContent-Length: ";
        // int total_length = strlen(response) + content_length + 10;
        // char buffer[total_length];
        // snprintf(buffer, total_length, "%s%d\r\n\r\n%s", response, content_length, file_contents);

        // //TODO: send response
        if(extension){
            if ((strcmp(extension, "txt") == 0)) {
                char response[] = "HTTP/1.1 200 OK\r\nContent-Type: text/plain; charset=UTF-8\r\nContent-Length: ";
                int total_length = strlen(response) + content_length + 10;
                char buffer[total_length];
                snprintf(buffer, total_length, "%s%d\r\n\r\n%s", response, content_length, file_contents);
                printf("BUFFER Looks like this:\n%s\n",buffer);
                send(client_socket, buffer, strlen(buffer), 0);
            }
            else if ((strcmp(extension, "html") == 0)) {
                char response[] = "HTTP/1.1 200 OK\r\nContent-Type: text/html; charset=UTF-8\r\nContent-Length: ";
                int total_length = strlen(response) + content_length + 10;
                char buffer[total_length];
                snprintf(buffer, total_length, "%s%d\r\n\r\n%s", response, content_length, file_contents);
                send(client_socket, buffer, strlen(buffer), 0);
                
            }
            else if((strcmp(extension, "jpg")==0)) {
                char response[] = "HTTP/1.1 200 OK\r\nContent-Type: image/jpeg; charset=UTF-8\r\nContent-Length: ";
                int total_length = strlen(response)+content_length + 10;
                char buffer[total_length];
                snprintf(buffer, total_length, "%s%d\r\n\r\n", response, content_length);
                send(client_socket, buffer, strlen(buffer), 0);
                send(client_socket,file_contents,content_length,0);
            }
        }
        else {
            //TODO: send an error
            char response[] = "HTTP/1.1 200 OK\r\nContent-Type: application/octet-stream; charset=UTF-8\r\nContent-Length: ";
            int total_length = strlen(response) + content_length + 10;
            char buffer[total_length];
            snprintf(buffer, total_length, "%s%d\r\n\r\n", response, content_length);
            send(client_socket, buffer, strlen(buffer), 0);
            send(client_socket,file_contents,content_length,0);
        }
    }
    // free memory
    free(file_contents);

    // char response[] = "HTTP/1.0 200 OK\r\n"
    //                   "Content-Type: text/plain; charset=UTF-8\r\n"
    //                   "Content-Length: 15\r\n"
    //                   "\r\n"
    //                   "Sample response";

    // send(client_socket, buffer, strlen(buffer), 0);
}

void proxy_remote_file(struct server_app *app, int client_socket, const char *request) {
    // TODO: Implement proxy request and replace the following code
    // What's needed:
    // * Connect to remote server (app->remote_server/app->remote_port)
    // * Forward the original request to the remote server
    // * Pass the response from remote server back
    // Bonus:
    // * When connection to the remote server fail, properly generate
    // HTTP 502 "Bad Gateway" response

    int c_socket;
    c_socket = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
    if(c_socket<0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    printf("SOCKET IS BUILT!\n");

    struct sockaddr_in sad;  
    memset(&sad, 0, sizeof(sad));
    sad.sin_family = AF_INET;
    sad.sin_addr.s_addr = inet_addr(app->remote_host);
    sad.sin_port = htons(app->remote_port); 
    if (connect(c_socket, (struct sockaddr *)&sad, sizeof(sad))< 0) {
        perror("connect failed");
        char response[] = "HTTP/1.0 502 Bad Gateway\r\n\r\n";
        send(client_socket, response, strlen(response), 0);
        exit(EXIT_FAILURE);
    }else{
        printf("Accepted connection from %s:%d\n", inet_ntoa(sad.sin_addr), ntohs(sad.sin_port));
    }

    printf("CONNECTION IS MADE!\n");

    //int back_socket = accept(c_socket, (struct sockaddr*)&sad, sizeof(sad));
    if(send(c_socket,request, strlen(request),0)<0){
        printf("REQUEST SENT UNSUCCESSFULLY");
    }else printf("REQUEST SENT to %d!; and client socket looks like this: %d\n",c_socket,client_socket);
    printf("Request is as follows:\n %s",request);

        char buffer[BUFFER_SIZE];
        ssize_t bytes_read;

    while(1){
        
        //printf("I AM GOING TO WAIT :<\n");
        bytes_read = recv(c_socket, buffer, sizeof(buffer) - 1, 0);
        //printf("I STOPPED WAITING :>\n");
        if (bytes_read <= 0) {
            printf("not receiving anything no more\n");
            printf("\n%s\n",buffer);
            send(client_socket, "\0", 1, 0);
            printf("RESPONSE SENT!\n");
            break;
        }
        printf("\n%s\n",buffer);
        // copy buffer to a new string
        //char *response = malloc(bytes_read + 1);
        //strcpy(response, buffer);

        

        //char response[] = "HTTP/1.0 501 Not Implemented\r\n\r\n";
        send(client_socket, buffer, bytes_read, 0);
    }
    
    
}

char *read_file(char *filename) {
    FILE *file;
    file = fopen(filename, "rb");

    //file doesn't contain anything
    if (file == NULL) {
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    int length = ftell(file);
    fseek(file, 0, SEEK_SET);

    printf("I AM %d LONG\n",length);

    //char d=fgetc(file);
    //printf("d is %02x",d);

    char *string = malloc(sizeof(char) * (length + 1));

    int i = 0;
    do
    {
        // Taking input single character at a time
        char c = fgetc(file);
 
        // Checking for end of file
        if (feof(file))
            break ;
 
        string[i]=c;
        i++;
    }  while(1);
    printf("I have itereated %d times\n",i);

    string[i] = '\0';
    fclose(file);

    return string;
}

int find_length(char *filename) {
    FILE *file;
    file = fopen(filename, "rb");

    //file doesn't contain anything
    if (file == NULL) {
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    int length = ftell(file);
    return length;
}