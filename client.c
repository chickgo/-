#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include <jwt.h>
#include <unistd.h>

#define API_URL "http://localhost:5000/api"
#define TOKEN_FILE ".token"

typedef struct {
    char *token;
    char username[50];
    char email[50];
    int points;
    int level;
} User;

char *read_file(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) return NULL;

    fseek(file, 0, SEEK_END);
    long length = ftell(file);
    fseek(file, 0, SEEK_SET);

    char *buffer = malloc(length + 1);
    if (!buffer) {
        fclose(file);
        return NULL;
    }

    fread(buffer, 1, length, file);
    buffer[length] = '\0';

    fclose(file);
    return buffer;
}

void write_file(const char *filename, const char *content) {
    FILE *file = fopen(filename, "w");
    if (!file) return;

    fputs(content, file);
    fclose(file);
}

char *load_token() {
    return read_file(TOKEN_FILE);
}

void save_token(const char *token) {
    write_file(TOKEN_FILE, token);
}

void free_response_content(char *content) {
    if (content) {
        free(content);
    }
}

static size_t write_callback(void *contents, size_t size, size_t nmemb, void *userp) {
    size_t realsize = size * nmemb;
    char **response = (char **)userp;

    *response = realloc(*response, realsize + 1);
    if (*response == NULL) {
        printf("Memory allocation failed\n");
        return 0;
    }

    memcpy(*response, contents, realsize);
    (*response)[realsize] = 0;

    return realsize;
}

char *perform_request(const char *url, const char *method, const char *json_data, const char *token) {
    CURL *curl;
    CURLcode res;
    char *response = NULL;

    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();

    if (curl) {
        struct curl_slist *headers = NULL;
        headers = curl_slist_append(headers, "Content-Type: application/json");
        if (token) {
            char auth_header[200];
            snprintf(auth_header, sizeof(auth_header), "Authorization: Bearer %s", token);
            headers = curl_slist_append(headers, auth_header);
        }

        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

        if (strcmp(method, "POST") == 0) {
            curl_easy_setopt(curl, CURLOPT_POST, 1L);
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json_data);
        } else if (strcmp(method, "GET") == 0) {
            curl_easy_setopt(curl, CURLOPT_HTTPGET, 1L);
        }

        res = curl_easy_perform(curl);

        if (res != CURLE_OK) {
            fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
            free(response);
            response = NULL;
        }

        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
    }

    curl_global_cleanup();
    return response;
}

void register_user(const char *username, const char *email, const char *password) {
    char url[200];
    snprintf(url, sizeof(url), "%s/register", API_URL);

    char json[500];
    snprintf(json, sizeof(json), "{\"username\": \"%s\", \"email\": \"%s\", \"password\": \"%s\"}", username, email, password);

    char *response = perform_request(url, "POST", json, NULL);

    if (response) {
        printf("Registration response: %s\n", response);
        free(response);
    }
}

void login_user(const char *username, const char *password) {
    char url[200];
    snprintf(url, sizeof(url), "%s/login", API_URL);

    char json[500];
    snprintf(json, sizeof(json), "{\"username\": \"%s\", \"password\": \"%s\"}", username, password);

    char *response = perform_request(url, "POST", json, NULL);

    if (response) {
        printf("Login response: %s\n", response);
        // Parse token from response and save it
        char *token_start = strstr(response, "\"token\":\"");
        if (token_start) {
            token_start += 8;
            char *token_end = strstr(token_start, "\"");
            if (token_end) {
                int token_length = token_end - token_start;
                char *token = malloc(token_length + 1);
                strncpy(token, token_start, token_length);
                token[token_length] = '\0';

                save_token(token);
                printf("Token saved: %s\n", token);
                free(token);
            }
        }
        free(response);
    }
}

void forgot_password(const char *email) {
    char url[200];
    snprintf(url, sizeof(url), "%s/forgot-password", API_URL);

    char json[200];
    snprintf(json, sizeof(json), "{\"email\": \"%s\"}", email);

    char *response = perform_request(url, "POST", json, NULL);

    if (response) {
        printf("Forgot password response: %s\n", response);
        free(response);
    }
}

void reset_password(const char *token, const char *password) {
    char url[300];
    snprintf(url, sizeof(url), "%s/reset-password/%s", API_URL, token);

    char json[200];
    snprintf(json, sizeof(json), "{\"password\": \"%s\"}", password);

    char *response = perform_request(url, "POST", json, NULL);

    if (response) {
        printf("Reset password response: %s\n", response);
        free(response);
    }
}

void check_in() {
    char *token = load_token();
    if (!token) {
        printf("No token found. Please login first.\n");
        return;
    }

    char url[200];
    snprintf(url, sizeof(url), "%s/check-in", API_URL);

    char json[200];
    // Username is extracted from the token in the actual implementation
    snprintf(json, sizeof(json), "{\"username\": \"john_doe\"}");

    char *response = perform_request(url, "POST", json, token);

    if (response) {
        printf("Check-in response: %s\n", response);
        free(response);
    }

    free(token);
}

void upgrade(int points) {
    char *token = load_token();
    if (!token) {
        printf("No token found. Please login first.\n");
        return;
    }

    char url[200];
    snprintf(url, sizeof(url), "%s/upgrade", API_URL);

    char json[200];
    snprintf(json, sizeof(json), "{\"points\": %d}", points);

    char *response = perform_request(url, "POST", json, token);

    if (response) {
        printf("Upgrade response: %s\n", response);
        free(response);
    }

    free(token);
}

void create_post(const char *content) {
    char *token = load_token();
    if (!token) {
        printf("No token found. Please login first.\n");
        return;
    }

    char url[200];
    snprintf(url, sizeof(url), "%s/posts", API_URL);

    char json[300];
    snprintf(json, sizeof(json), "{\"content\": \"%s\"}", content);

    char *response = perform_request(url, "POST", json, token);

    if (response) {
        printf("Create post response: %s\n", response);
        free(response);
    }

    free(token);
}

void get_posts() {
    char url[200];
    snprintf(url, sizeof(url), "%s/posts", API_URL);

    char *response = perform_request(url, "GET", NULL, NULL);

    if (response) {
        printf("Get posts response: %s\n", response);
        free(response);
    }
}

void create_comment(int post_id, const char *content) {
    char *token = load_token();
    if (!token) {
        printf("No token found. Please login first.\n");
        return;
    }

    char url[300];
    snprintf(url, sizeof(url), "%s/comments/%d", API_URL, post_id);

    char json[300];
    snprintf(json, sizeof(json), "{\"content\": \"%s\"}", content);

    char *response = perform_request(url, "POST", json, token);

    if (response) {
        printf("Create comment response: %s\n", response);
        free(response);
    }

    free(token);
}

void upload_file(const char *file_path) {
    char *token = load_token();
    if (!token) {
        printf("No token found. Please login first.\n");
        return;
    }

    CURL *curl;
    CURLcode res;
    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();

    if (curl) {
        struct curl_slist *headers = NULL;
        char auth_header[200];
        snprintf(auth_header, sizeof(auth_header), "Authorization: Bearer %s", token);
        headers = curl_slist_append(headers, auth_header);

        char url[200];
        snprintf(url, sizeof(url), "%s/files", API_URL);

        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_POST, 1L);

        curl_mime *mime;
        curl_mimepart *part;
        mime = curl_mime_init(curl);
        part = curl_mime_addpart(mime);
        curl_mime_name(part, "file");
        curl_mime_filedata(part, file_path);

        curl_easy_setopt(curl, CURLOPT_MIMEPOST, mime);

        res = curl_easy_perform(curl);

        if (res != CURLE_OK) {
            fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
        } else {
            printf("File uploaded successfully\n");
        }

        curl_mime_free(mime);
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
    }

    curl_global_cleanup();
    free(token);
}

void get_files() {
    char *token = load_token();
    if (!token) {
        printf("No token found. Please login first.\n");
        return;
    }

    char url[200];
    snprintf(url, sizeof(url), "%s/files", API_URL);

    char *response = perform_request(url, "GET", NULL, token);

    if (response) {
        printf("Get files response: %s\n", response);
        free(response);
    }

    free(token);
}

void get_notifications() {
    char *token = load_token();
    if (!token) {
        printf("No token found. Please login first.\n");
        return;
    }

    char url[200];
    snprintf(url, sizeof(url), "%s/notifications", API_URL);

    char *response = perform_request(url, "GET", NULL, token);

    if (response) {
        printf("Get notifications response: %s\n", response);
        free(response);
    }

    free(token);
}

void get_groups() {
    char url[200];
    snprintf(url, sizeof(url), "%s/groups", API_URL);

    char *response = perform_request(url, "GET", NULL, NULL);

    if (response) {
        printf("Get groups response: %s\n", response);
        free(response);
    }
}

void logout() {
    unlink(TOKEN_FILE);
    printf("Logged out successfully\n");
}

int main() {
    // 注册用户
    // register_user("john_doe", "john@example.com", "password123");

    // 登录用户
    // login_user("john_doe", "password123");

    // 发送重置密码邮件
    // forgot_password("john@example.com");

    // 重置密码
    // reset_password("reset_token_here", "newpassword123");

    // 签到
    // check_in();

    // 升级
    // upgrade(100);

    // 创建帖子
    // create_post("Hello, world!");

    // 获取所有帖子
    // get_posts();

    // 创建评论
    // create_comment(1, "Great post!");

    // 上传文件
    // upload_file("example.txt");

    // 获取用户文件
    // get_files();

    // 获取通知
    // get_notifications();

    // 获取群组
    // get_groups();

    // 登出
    // logout();

    return 0;
}