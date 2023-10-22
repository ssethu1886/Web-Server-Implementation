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

#include <time.h>
#define MB 10485760 // 1 MB = 10485760 bytes

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

// The following function is implemented for you and doesn't need to be changed
void parse_args(int argc, char *argv[], struct server_app *app);

// The following functions need to be updated
void handle_request(struct server_app *app, int client_socket);
void serve_local_file(int client_socket, const char *path,char *general_header);
void proxy_remote_file(struct server_app *app, int client_socket, const char *path);

// Helper functions
char *getfilename(char* GETLine);
char *getContentType(char *filename);
char *buildMessage(char *filename, char *contentType, char *content);
char *getDate();
void numOf10MBsections(const char* filename, size_t* sectionCount, size_t* remainingSize);


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

    // TODO: Parse the header and extract essential fields, e.g. file name 
    // Hint: if the requested path is "/" (root), default to index.html 


    //Build the response 

    //Status
    char *status = "HTTP/1.1 200 OK\r\n";
    //Connection
    char *connection = "Conncection: keep-alive\r\n";
    //Server
    char *server_name = "Server: A&S Server/1.0\r\n";
    
    //Extract filename
    char *req_token = strtok(request,"\r\n"); //first line
    printf("\033[31m%s\n",req_token);
    char *filename = getfilename(req_token);//filename function
    printf("\t\033[0mFilename Extracted: %s\n",filename);
    //req_token=strtok(NULL,"\r\n"); 

    //Extract content type
    char *content_type = getContentType(filename);

    //Date
    char *date_curr = getDate();
    
    //Header
    size_t header_length = strlen(status) + strlen(connection) + strlen(server_name) + strlen(content_type) + strlen(date_curr) + 1;
    char *general_header = (char *)malloc(header_length);
    strcpy(general_header, status);
    strcat(general_header, connection);
    strcat(general_header, server_name);
    strcat(general_header,content_type);
    strcat(general_header,date_curr);
    printf("\033[36mHeader to send:\n%s\n",general_header);
    printf("\033[0m");

    // TODO: Implement proxy and call the function under condition
    // specified in the spec
    // if (need_proxy(...)) {
    //    proxy_remote_file(app, client_socket, file_name);
    // } else {
    serve_local_file(client_socket, filename,general_header);
    //}
    free(filename);
    free(date_curr);
}

void serve_local_file(int client_socket, const char *path, char *general_header) {
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

    char response[] = "HTTP/1.0 200 OK\r\n"
                      "Content-Type: text/plain; charset=UTF-8\r\n"
                      "Content-Length: 15\r\n"
                      "\r\n";

    //char *response_msg = buildMessage();

    const char* filename = path;
    FILE* file = fopen(filename, "rb");
    if (!file) {
        perror("Failed to open file");
        return;
    }
    size_t sectionCount;//num of 10MB section
    size_t remainingSize;//remaining
    numOf10MBsections(filename, &sectionCount, &remainingSize);

    printf("Number of 10 MB sections: %zu\n", sectionCount);
    printf("Size of remaining data: %zu bytes\n", remainingSize);

    // for sections - 1 
        // send msg
    //send last section w remaining size

    send(client_socket, response, strlen(response), 0);
    //free(sections);//from chunking function
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

    char response[] = "HTTP/1.0 501 Not Implemented\r\n\r\n";
    send(client_socket, response, strlen(response), 0);
}


// Function to extract the filename from an input string and return it as a dynamically allocated string
char *getfilename( char *inputString) {
    const char *start = strstr(inputString, " /");
    const char *end = strstr(inputString, " HTTP");

    if (start != NULL && end != NULL && start < end) {
        start += 2; // Move two characters ahead to begin after the space and slash

        // Calculate the length of the filename
        size_t filenameLength = end - start;

        if (filenameLength > 0) {
            // Allocate memory for the filename
            char *filename = (char *)malloc(filenameLength + 1);
            if (filename != NULL) {
                // Copy the filename to the allocated memory
                strncpy(filename, start, filenameLength);
                filename[filenameLength] = '\0'; // Null-terminate the string
                return filename;
            }
        }
    }

    // If the filename is not found or invalid, return "index.html"
    // Prof. said we can assume the file path always starts with a '/' (piazza-71)
    char *filename = (char *)malloc(strlen("index.html") + 1);
    if (filename != NULL) {
        strcpy(filename, "index.html");
        return filename;
    }

    // Err - mem allocation err (?)
    return NULL;
}

// Extract Filetype
char *getContentType(char *filename) {
    const char *ext = strrchr(filename, '.');// Find where extension starts
    if (ext != NULL) {
        ext++; // Move past the dot
        if (strcmp(ext, "html") == 0) {
            return "Content-Type: text/html\r\n";
        } else if (strcmp(ext, "txt") == 0) {
            return "Content-Type: text/plain; charset=UTF-8\r\n";
        } else if (strcmp(ext, "jpg") == 0 || strcmp(ext, "jpeg") == 0) {
            return "Content-Type: image/jpeg\r\n";
        } else if (strcmp(ext, "png") == 0) {
            return "Content-Type: image/png\r\n";
        //add more img types ??
        } else if (strcmp(ext, "pdf") == 0) {
            return "Content-Type: application/pdf\r\n";
        } else if (strcmp(ext, "json") == 0) {
            return "Content-Type: application/json\r\n";
        }
    }

    // Default for unknown - send binary data
    return "application/octet-stream\r\n";
}

//Function to build the HTTP response message from previously extracted components
char *buildMessage(char *filename, char *contentType, char *content){
    //TO DO build msg with each section of return data
}

//Function to get the current date
char *getDate() {
    // Allocate memory for the formatted date and time string
    char* formattedDateTime = (char*)malloc(27 * sizeof(char)); // 6 chars for "Date: " 20 characters for the date and time, plus one for the null terminator
    if (formattedDateTime == NULL) {
        perror("Memory allocation failed");
        return NULL;
    }

    time_t rawTime;
    struct tm* timeInfo;

    time(&rawTime);
    timeInfo = localtime(&rawTime);

    // Format the date and time
    strftime(formattedDateTime, 27, "Date: %Y-%m-%d %H:%M:%S", timeInfo);

    return formattedDateTime;
}

void numOf10MBsections(const char* filename, size_t* sectionCount, size_t* remainingSize) {
    FILE* file = fopen(filename, "rb");
    if (!file) {
        perror("Failed to open file: File doesnt exist");
        return;
    }

    // Get the file size
    fseek(file, 0, SEEK_END);
    long long file_size = ftell(file);
    rewind(file);

    // Calculate the number of 10 MB sections
    *sectionCount = file_size / MB;

    // Calculate the size of the remaining data
    *remainingSize = file_size % MB;

    fclose(file);
}
