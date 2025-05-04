#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <pthread.h>
#include "network.h"
#include "data.h"
#include "security.h"
#include "utils.h"

#define PORT 8080
#define MAX_CLIENTS 10
#define BUFFER_SIZE 1024

typedef struct {
    int socket;
    struct sockaddr_in address;
} Client;

static int server_socket;
static Client clients[MAX_CLIENTS];
static int client_count = 0;
static pthread_mutex_t client_mutex = PTHREAD_MUTEX_INITIALIZER;

void start_server() {
    // Create socket
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Configure socket
    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // Bind socket
    if (bind(server_socket, (struct sockaddr *)&address, sizeof(address)) == -1) {
        perror("Socket bind failed");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    // Listen for incoming connections
    if (listen(server_socket, MAX_CLIENTS) == -1) {
        perror("Listen failed");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d...\n", PORT);

    // Accept clients
    while (1) {
        struct sockaddr_in client_address;
        socklen_t client_len = sizeof(client_address);
        int client_socket = accept(server_socket, (struct sockaddr *)&client_address, &client_len);
        if (client_socket == -1) {
            perror("Accept failed");
            continue;
        }

        pthread_mutex_lock(&client_mutex);
        if (client_count < MAX_CLIENTS) {
            clients[client_count].socket = client_socket;
            clients[client_count].address = client_address;
            client_count++;
            printf("Client connected: %s:%d\n", inet_ntoa(client_address.sin_addr), ntohs(client_address.sin_port));
        } else {
            printf("Max clients reached. Closing connection.\n");
            close(client_socket);
        }
        pthread_mutex_unlock(&client_mutex);
    }
}

void stop_server() {
    for (int i = 0; i < client_count; i++) {
        close(clients[i].socket);
    }
    close(server_socket);
    printf("Server stopped.\n");
}

void send_message_to_all(const char *message) {
    pthread_mutex_lock(&client_mutex);
    for (int i = 0; i < client_count; i++) {
        send(clients[i].socket, message, strlen(message), 0);
    }
    pthread_mutex_unlock(&client_mutex);
}

void handle_client(int client_socket) {
    char buffer[BUFFER_SIZE];
    int bytes_read;

    while ((bytes_read = recv(client_socket, buffer, BUFFER_SIZE, 0)) > 0) {
        buffer[bytes_read] = '\0';
        printf("Received: %s\n", buffer);

        // Process command
        if (strncmp(buffer, "REGISTER", 8) == 0) {
            // REGISTER:username:email:password
            char *username = strtok(buffer + 8, ":");
            char *email = strtok(NULL, ":");
            char *password = strtok(NULL, ":");

            if (username && email && password) {
                if (register_user(username, email, password)) {
                    send(client_socket, "REGISTER_SUCCESS", 15, 0);
                } else {
                    send(client_socket, "REGISTER_FAILED", 14, 0);
                }
            } else {
                send(client_socket, "INVALID_REQUEST", 15, 0);
            }
        } else if (strncmp(buffer, "LOGIN", 5) == 0) {
            // LOGIN:username:password
            char *username = strtok(buffer + 5, ":");
            char *password = strtok(NULL, ":");

            if (username && password) {
                if (login_user(username, password)) {
                    send(client_socket, "LOGIN_SUCCESS", 13, 0);
                } else {
                    send(client_socket, "LOGIN_FAILED", 12, 0);
                }
            } else {
                send(client_socket, "INVALID_REQUEST", 15, 0);
            }
        } else if (strncmp(buffer, "CHECKIN", 7) == 0) {
            // CHECKIN:username
            char *username = strtok(buffer + 7, ":");

            if (username) {
                if (checkin_user(username)) {
                    send(client_socket, "CHECKIN_SUCCESS", 14, 0);
                } else {
                    send(client_socket, "CHECKIN_FAILED", 13, 0);
                }
            } else {
                send(client_socket, "INVALID_REQUEST", 15, 0);
            }
        } else if (strncmp(buffer, "UPGRADE", 7) == 0) {
            // UPGRADE:username:points
            char *username = strtok(buffer + 7, ":");
            char *points_str = strtok(NULL, ":");
            int points = points_str ? atoi(points_str) : 0;

            if (username && points_str) {
                if (upgrade_user(username, points)) {
                    send(client_socket, "UPGRADE_SUCCESS", 15, 0);
                } else {
                    send(client_socket, "UPGRADE_FAILED", 14, 0);
                }
            } else {
                send(client_socket, "INVALID_REQUEST", 15, 0);
            }
        } else if (strncmp(buffer, "FORGOT_PASSWORD", 14) == 0) {
            // FORGOT_PASSWORD:email
            char *email = strtok(buffer + 14, ":");

            if (email) {
                if (forgot_password(email)) {
                    send(client_socket, "RESET_LINK_SENT", 15, 0);
                } else {
                    send(client_socket, "EMAIL_NOT_FOUND", 15, 0);
                }
            } else {
                send(client_socket, "INVALID_REQUEST", 15, 0);
            }
        } else if (strncmp(buffer, "RESET_PASSWORD", 14) == 0) {
            // RESET_PASSWORD:token:new_password
            char *token = strtok(buffer + 14, ":");
            char *new_password = strtok(NULL, ":");

            if (token && new_password) {
                if (reset_password(token, new_password)) {
                    send(client_socket, "PASSWORD_RESET_SUCCESS", 20, 0);
                } else {
                    send(client_socket, "PASSWORD_RESET_FAILED", 19, 0);
                }
            } else {
                send(client_socket, "INVALID_REQUEST", 15, 0);
            }
        } else if (strncmp(buffer, "UPLOAD_FILE", 10) == 0) {
            // UPLOAD_FILE:username:filename
            char *username = strtok(buffer + 10, ":");
            char *filename = strtok(NULL, ":");

            if (username && filename) {
                if (upload_file(username, filename)) {
                    send(client_socket, "FILE_UPLOAD_SUCCESS", 17, 0);
                } else {
                    send(client_socket, "FILE_UPLOAD_FAILED", 16, 0);
                }
            } else {
                send(client_socket, "INVALID_REQUEST", 15, 0);
            }
        } else if (strncmp(buffer, "GET_FILES", 9) == 0) {
            // GET_FILES:username
            char *username = strtok(buffer + 9, ":");

            if (username) {
                char *files = get_files(username);
                send(client_socket, files, strlen(files), 0);
                free(files);
            } else {
                send(client_socket, "INVALID_REQUEST", 15, 0);
            }
        } else if (strncmp(buffer, "CREATE_POST", 11) == 0) {
            // CREATE_POST:username:content
            char *username = strtok(buffer + 11, ":");
            char *content = strtok(NULL, ":");

            if (username && content) {
                if (create_post(username, content)) {
                    send(client_socket, "POST_CREATED_SUCCESS", 18, 0);
                } else {
                    send(client_socket, "POST_CREATED_FAILED", 17, 0);
                }
            } else {
                send(client_socket, "INVALID_REQUEST", 15, 0);
            }
        } else if (strncmp(buffer, "CREATE_COMMENT", 13) == 0) {
            // CREATE_COMMENT:username:post_id:content
            char *username = strtok(buffer + 13, ":");
            char *post_id_str = strtok(NULL, ":");
            char *content = strtok(NULL, ":");
            int post_id = post_id_str ? atoi(post_id_str) : 0;

            if (username && post_id_str && content) {
                if (create_comment(username, post_id, content)) {
                    send(client_socket, "COMMENT_CREATED_SUCCESS", 20, 0);
                } else {
                    send(client_socket, "COMMENT_CREATED_FAILED", 19, 0);
                }
            } else {
                send(client_socket, "INVALID_REQUEST", 15, 0);
            }
        } else if (strncmp(buffer, "GET_NOTIFICATIONS", 15) == 0) {
            // GET_NOTIFICATIONS:username
            char *username = strtok(buffer + 15, ":");

            if (username) {
                char *notifications = get_notifications(username);
                send(client_socket, notifications, strlen(notifications), 0);
                free(notifications);
            } else {
                send(client_socket, "INVALID_REQUEST", 15, 0);
            }
        } else if (strncmp(buffer, "GET_GROUPS", 10) == 0) {
            // GET_GROUPS:username
            char *username = strtok(buffer + 10, ":");

            if (username) {
                char *groups = get_groups(username);
                send(client_socket, groups, strlen(groups), 0);
                free(groups);
            } else {
                send(client_socket, "INVALID_REQUEST", 15, 0);
            }
        } else {
            send(client_socket, "UNKNOWN_COMMAND", 15, 0);
        }
    }

    // Client disconnected
    pthread_mutex_lock(&client_mutex);
    for (int i = 0; i < client_count; i++) {
        if (clients[i].socket == client_socket) {
            for (int j = i; j < client_count - 1; j++) {
                clients[j] = clients[j + 1];
            }
            client_count--;
            break;
        }
    }
    pthread_mutex_unlock(&client_mutex);

    close(client_socket);
    printf("Client disconnected.\n");
}

int main() {
    start_server();

    // Create thread for each client
    while (1) {
        pthread_t thread;
        pthread_create(&thread, NULL, (void *(*)(void *))handle_client, (void *)&clients[client_count - 1].socket);
        pthread_detach(thread);
    }

    stop_server();
    return 0;
}