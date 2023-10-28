# README

## Project Description

This project is a basic HTTP server, designed to handle requests by either serving local files or proxying remote files. The starter code provides a foundation upon which you can build the necessary logic to serve requests based on their content and the presence of files locally.

## File Structure

- **server.c**: The main server code containing the logic for handling HTTP requests, parsing arguments, and serving/proxying content.
- **makefile**: The build system for compiling and cleaning the project.

## Build Instructions

To build the project, ensure you have `gcc` installed on your system. Navigate to the directory containing the `server.c` and `makefile`, then use the following commands:

- To compile the project:
    ```bash
    make
    ```
  
- To clean the build (remove the compiled output):
    ```bash
    make clean
    ```

## Running the Server

After compiling, you can run the server using the following command:

```bash
./server [-b local_port] [-r remote_host] [-p remote_port]
```

### Command-line Arguments:

- `-b local_port`: Specify the local port on which the HTTP server will run. Defaults to `8081`.
- `-r remote_host`: Specify the remote host's address for proxying. Defaults to `131.179.176.34`.
- `-p remote_port`: Specify the remote port for proxying. Defaults to `5001`.
