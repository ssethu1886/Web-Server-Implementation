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
#define MB 1048576 // 1 MB 

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
char *getDate();
void numOf1MBsections(const char* filename, size_t* sectionCount, size_t* remainingSize);
char *chunkHeader(const char *fn, size_t dataLength, char *general_header);
char *readInData(size_t data_len, char *chunk_header);


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

    //Build the response 

    //Status
    char *status = "HTTP/1.1 200 OK\r\n";
    //Connection
    char *connection = "Connection: close\r\n";
    //Server
    char *server_name = "Server: A&S Server/1.0\r\n";
    //Filename
    char *req_token = strtok(request,"\r\n"); //first line
    printf("\033[31m%s\n",req_token);
    char *filename = getfilename(req_token);//filename function
    printf("\t\033[0mFilename Extracted: %s\n",filename);
    //Content Type
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
    printf("\033[36mGeneral Header:\n%s\n",general_header);
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
    free(general_header);
    free(request);
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

    //TODO: Check if file exist
    const char* filename = path;

    //Size of file
    size_t sectionCount;//num of 1 MB sections
    size_t remainingSize;//remaining bytes
    numOf1MBsections(filename, &sectionCount, &remainingSize);//get num of 1 MB sections plus extra
    printf("Number of 1 MB sections: %zu\n", sectionCount);
    printf("Size of remaining data: %zu bytes\n", remainingSize);

    //Send chunks (first the 1 MB sections)
    for (size_t i = 0; i < sectionCount; i++) {
        char *chunk_header = chunkHeader( filename, MB, general_header);//build chunk header based off content length
        
        //char *response;//full response msg
        //readInData(response,MB,chunk_header);//TODO
        
        //Send the chunk
        send(client_socket, chunk_header, strlen(chunk_header), 0);//change to add data later
        //send(client_socket, response, strlen(response), 0);//change to add data later
        printf("\033[36mSent chunk #%zu\n",i);
        printf("\033[0m%s",chunk_header);

        free(chunk_header);
        //free(response);
    }
    //Send trailing bytes (non-multiple of a MB)
    char *chunk_header = chunkHeader( filename,remainingSize, general_header);//build chunk header based off content length
    printf("\033[31mChunk Header:\n%s",chunk_header);
    
    //char *response;
    //printf("plz2");
    //response = readInData(remainingSize,chunk_header);
    
    send(client_socket, chunk_header, strlen(chunk_header), 0);//change to add data later
    //send(client_socket, response, strlen(response), 0);//change to add data later
    printf("\033[36mSent chunk #%zu\n",sectionCount+1);
    
    //free(response);    
    free(chunk_header);
    printf("\033[0mResponses Sent!\n");
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
    return "Content-Type: application/octet-stream\r\n";
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

void numOf1MBsections(const char* filename, size_t* sectionCount, size_t* remainingSize) {
    FILE* file = fopen(filename, "rb");
    if (!file) {
        perror("Failed to open file: File doesnt exist");
        return;
    }

    // Get the file size
    fseek(file, 0, SEEK_END);
    long long file_size = ftell(file);
    rewind(file);

    // Calculate the number of 1 MB sections
    *sectionCount = file_size / MB;

    // Calculate the size of the remaining data
    *remainingSize = file_size % MB;

    fclose(file);
}

/*
char *chunkHeader(const char *fn,size_t dataLength, char *general_header){
    int intSize = snprintf(NULL, 0, "%zu", dataLength);
    char content_length[64]; // Buffer for "Content-Length" line
    snprintf(content_length, sizeof(content_length), "Content-Length: %zu", dataLength);

    // Calculate the total length, including the null terminator
    size_t totalLength = strlen(general_header) + strlen(content_length) + 4; // 4 accounts for "\r\n\r\n"

    char *chunk_header = (char *)malloc(totalLength + dataLength);
    //Open file and read into a buffer
    char *test_buffer = (char *)malloc(dataLength);
     // error checking
    if (!test_buffer) {
        perror("Failed to allocate test_buffer");
        exit(1);
    }//error checking

    FILE *file = fopen(fn, "r");
    if(!file){
        perror("Failed to open file");
        free(test_buffer);
        exit(1);
    }//error checking

    // read into buffer 
    printf("Read %zu bytes\n",dataLength);
    size_t i = fread(test_buffer, 1, dataLength, file);
    printf("read %zu bytes\n",i);
    printf("%s",test_buffer);
    fclose(file);

    if (chunk_header) {
        strcpy(chunk_header, general_header);
        strcat(chunk_header, "\r\n");
        strcat(chunk_header, content_length);
        strcat(chunk_header, "\r\n\r\n");//Data after
        strcat(chunk_header, test_buffer);
        printf("\nBuilt CH: %s\n",chunk_header);
        
        free(test_buffer);
        return chunk_header;
    } else {
        printf("Chunk header memory allocation failed");
        return NULL;
    }
}
*/

char *chunkHeader(const char *fn, size_t dataLength, char *general_header) {
    int intSize = snprintf(NULL, 0, "%zu", dataLength);
    char content_length[64]; // Buffer for "Content-Length" line
    snprintf(content_length, sizeof(content_length), "Content-Length: %zu", dataLength);

    // Calculate the total length, including the null terminator
    size_t general_header_length = strlen(general_header);
    size_t content_length_length = strlen(content_length);
    size_t totalLength = general_header_length + content_length_length + 4; // 4 accounts for "\r\n\r\n"

    char *chunk_header = (char *)malloc(totalLength + dataLength);
    
    // Error checking
    if (!chunk_header) {
        perror("Failed to allocate chunk_header");
        exit(1);
    }

    //Open file and read into a buffer
    char *test_buffer = (char *)malloc(dataLength);

    // Error checking
    if (!test_buffer) {
        perror("Failed to allocate test_buffer");
        free(chunk_header);
        exit(1);
    }

    FILE *file = fopen(fn, "r");
    if (!file) {
        perror("Failed to open file");
        free(chunk_header);
        free(test_buffer);
        exit(1);
    }

    // read into buffer
    printf("Read %zu bytes\n", dataLength);
    size_t i = fread(test_buffer, 1, dataLength, file);
    printf("Read %zu bytes\n", i);
    fclose(file);

    if (chunk_header) {
        // Copy general_header to chunk_header
        memcpy(chunk_header, general_header, general_header_length);
        char *current_ptr = chunk_header + general_header_length;

        // Concatenate "\r\n" to chunk_header
        memcpy(current_ptr, "\r\n", 2);
        current_ptr += 2;

        // Copy content_length to chunk_header
        memcpy(current_ptr, content_length, content_length_length);
        current_ptr += content_length_length;

        // Concatenate "\r\n\r\n" to chunk_header
        memcpy(current_ptr, "\r\n\r\n", 4);
        current_ptr += 4;

        // Copy test_buffer to chunk_header
        memcpy(current_ptr, test_buffer, dataLength);
        
        //printf("\nBuilt CH: %s\n",chunk_header);

        free(test_buffer);
        return chunk_header;
    } else {
        printf("Chunk header memory allocation failed");
        return NULL;
    }
}



//TODO add function to check whether a file exists.
//return page that says file doesnt exist ? (set filename to it)

//TODO fix error when file doesnt exist it just keeps going w weird behavior