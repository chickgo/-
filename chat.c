#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

#define PORT 8080
#define BUFFER_SIZE 4096
#define MAX_CLIENTS 100

typedef struct {
    int socket;
    struct sockaddr_in address;
    SSL *ssl;
    pthread_mutex_t mutex;
} Client;

typedef struct {
    Client **clients;
    int count;
    pthread_mutex_t mutex;
} ClientManager;

typedef struct {
    char *sender;
    char *content;
    time_t timestamp;
} Message;

typedef struct {
    Message **messages;
    int count;
    pthread_mutex_t mutex;
} MessageQueue;

ClientManager client_manager;
MessageQueue message_queue;

SSL_CTX *create_context() {
    const SSL_METHOD *method;
    SSL_CTX *ctx;

    method = SSLv23_server_method();
    ctx = SSL_CTX_new(method);
    if (!ctx) {
        perror("Unable to create SSL context");
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }

    return ctx;
}

void configure_context(SSL_CTX *ctx) {
    SSL_CTX_set_ecdh_auto(ctx, 1);

    if (SSL_CTX_use_certificate_file(ctx, "server.crt", SSL_FILETYPE_PEM) <= 0) {
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }

    if (SSL_CTX_use_PrivateKey_file(ctx, "server.key", SSL_FILETYPE_PEM) <= 0) {
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }
}

void init_client_manager(ClientManager *manager) {
    manager->clients = (Client **)malloc(MAX_CLIENTS * sizeof(Client *));
    manager->count = 0;
    pthread_mutex_init(&manager->mutex, NULL);
}

void add_client(ClientManager *manager, Client *client) {
    pthread_mutex_lock(&manager->mutex);
    if (manager->count < MAX_CLIENTS) {
        manager->clients[manager->count++] = client;
    }
    pthread_mutex_unlock(&manager->mutex);
}

void remove_client(ClientManager *manager, int socket) {
    pthread_mutex_lock(&manager->mutex);
    for (int i = 0; i < manager->count; i++) {
        if (manager->clients[i]->socket == socket) {
            SSL_free(manager->clients[i]->ssl);
            close(manager->clients[i]->socket);
            free(manager->clients[i]);
            for (int j = i; j < manager->count - 1; j++) {
                manager->clients[j] = manager->clients[j + 1];
            }
            manager->count--;
            break;
        }
    }
    pthread_mutex_unlock(&manager->mutex);
}

void init_message_queue(MessageQueue *queue) {
    queue->messages = (Message **)malloc(100 * sizeof(Message *));
    queue->count = 0;
    pthread_mutex_init(&queue->mutex, NULL);
}

void add_message(MessageQueue *queue, Message *message) {
    pthread_mutex_lock(&queue->mutex);
    queue->messages[queue->count++] = message;
    pthread_mutex_unlock(&queue->mutex);
}

void broadcast_message(ClientManager *manager, Message *message) {
    pthread_mutex_lock(&manager->mutex);
    for (int i = 0; i < manager->count; i++) {
        Client *client = manager->clients[i];
        pthread_mutex_lock(&client->mutex);
        SSL_write(client->ssl, message->content, strlen(message->content));
        pthread_mutex_unlock(&client->mutex);
    }
    pthread_mutex_unlock(&manager->mutex);
}

void *handle_client(void *arg) {
    Client *client = (Client *)arg;
    char buffer[BUFFER_SIZE];
    int bytes_received;

    while (1) {
        pthread_mutex_lock(&client->mutex);
        bytes_received = SSL_read(client->ssl, buffer, BUFFER_SIZE - 1);
        pthread_mutex_unlock(&client->mutex);

        if (bytes_received <= 0) {
            break;
        }

        buffer[bytes_received] = '\0';
        printf("Received: %s\n", buffer);

        Message *message = (Message *)malloc(sizeof(Message));
        message->sender = "Client";
        message->content = (char *)malloc(strlen(buffer) + 1);
        strcpy(message->content, buffer);
        message->timestamp = time(NULL);

        add_message(&message_queue, message);
        broadcast_message(&client_manager, message);
    }

    remove_client(&client_manager, client->socket);
    free(client);
    pthread_detach(pthread_self());
    return NULL;
}

int main(int argc, char *argv[]) {
    SSL_library_init();
    SSL_load_error_strings();
    OpenSSL_add_all_algorithms();

    SSL_CTX *ctx = create_context();
    configure_context(ctx);

    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, MAX_CLIENTS) < 0) {
        perror("listen failed");
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d...\n", PORT);

    init_client_manager(&client_manager);
    init_message_queue(&message_queue);

    while (1) {
        struct sockaddr_in client_address;
        socklen_t client_address_len = sizeof(client_address);
        int client_socket = accept(server_fd, (struct sockaddr *)&client_address, &client_address_len);

        SSL *ssl = SSL_new(ctx);
        SSL_set_fd(ssl, client_socket);

        if (SSL_accept(ssl) <= 0) {
            ERR_print_errors_fp(stderr);
            close(client_socket);
            continue;
        }

        Client *client = (Client *)malloc(sizeof(Client));
        client->socket = client_socket;
        client->address = client_address;
        client->ssl = ssl;
        pthread_mutex_init(&client->mutex, NULL);

        pthread_t thread;
        pthread_create(&thread, NULL, handle_client, client);

        add_client(&client_manager, client);
    }

    close(server_fd);
    SSL_CTX_free(ctx);
    return 0;
}