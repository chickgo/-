#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <ctime>
#include "data.h"
#include "security.h"
#include "network.h"
#include "utils.h"

using namespace std;

class User {
public:
    long id;
    string username;
    string email;
    string password;
    bool is_online;
    time_t last_checkin;
    int points;
    int level;
    string reset_token;
    time_t reset_token_expiry;

    User(long id, string username, string email, string password)
        : id(id), username(username), email(email), password(password),
          is_online(false), last_checkin(0), points(0), level(1),
          reset_token(""), reset_token_expiry(0) {}

    void checkIn() {
        if (!last_checkin || difftime(time(nullptr), last_checkin) > 86400) { // 24 hours
            points += 10;
            last_checkin = time(nullptr);
            cout << "Checked in successfully. Points: " << points << endl;
        } else {
            cout << "Already checked in today." << endl;
        }
    }

    void upgrade(int points_cost) {
        if (points >= points_cost) {
            points -= points_cost;
            level += 1;
            cout << "Upgraded to level " << level << ". Points remaining: " << points << endl;
        } else {
            cout << "Not enough points." << endl;
        }
    }

    void setPassword(string new_password) {
        password = new_password;
    }

    string generateResetToken() {
        reset_token = Security::generateToken(36);
        reset_token_expiry = time(nullptr) + 3600; // 1 hour expiry
        return reset_token;
    }

    bool validateResetToken(string token) {
        return reset_token == token && difftime(reset_token_expiry, time(nullptr)) > 0;
    }
};

class Post {
public:
    long id;
    string content;
    time_t date_posted;
    long user_id;

    Post(long id, string content, long user_id)
        : id(id), content(content), user_id(user_id) {
        date_posted = time(nullptr);
    }
};

class Comment {
public:
    long id;
    string content;
    time_t date_posted;
    long user_id;
    long post_id;

    Comment(long id, string content, long user_id, long post_id)
        : id(id), content(content), user_id(user_id), post_id(post_id) {
        date_posted = time(nullptr);
    }
};

class File {
public:
    long id;
    string filename;
    string path;
    time_t upload_date;
    long user_id;

    File(long id, string filename, string path, long user_id)
        : id(id), filename(filename), path(path), user_id(user_id) {
        upload_date = time(nullptr);
    }
};

class Notification {
public:
    long id;
    string content;
    time_t date_sent;
    bool is_read;
    long user_id;

    Notification(long id, string content, long user_id)
        : id(id), content(content), user_id(user_id) {
        date_sent = time(nullptr);
        is_read = false;
    }
};

class Group {
public:
    long id;
    string name;
    string description;
    time_t created_at;

    Group(long id, string name, string description)
        : id(id), name(name), description(description) {
        created_at = time(nullptr);
    }
};

class GroupMember {
public:
    long id;
    long user_id;
    long group_id;
    time_t joined_at;

    GroupMember(long id, long user_id, long group_id)
        : id(id), user_id(user_id), group_id(group_id) {
        joined_at = time(nullptr);
    }
};

class AuthService {
public:
    static User* registerUser(string username, string email, string password) {
        // Check if user already exists (simplified)
        static vector<User> users;
        for (auto& user : users) {
            if (user.username == username || user.email == email) {
                cout << "Username or email already exists." << endl;
                return nullptr;
            }
        }

        static long userId = 1;
        User* user = new User(userId++, username, email, Security::hashPassword(password));
        users.push_back(*user);
        return user;
    }

    static User* login(string username, string password) {
        static vector<User> users;
        for (auto& user : users) {
            if (user.username == username && Security::verifyPassword(password, user.password)) {
                user.is_online = true;
                cout << "Login successful." << endl;
                return &user;
            }
        }
        cout << "Invalid username or password." << endl;
        return nullptr;
    }

    static string forgotPassword(string email) {
        static vector<User> users;
        for (auto& user : users) {
            if (user.email == email) {
                string token = user.generateResetToken();
                cout << "Password reset token generated: " << token << endl;
                // Send email with token (simplified)
                return token;
            }
        }
        cout << "Email not found." << endl;
        return "";
    }

    static bool resetPassword(string token, string newPassword) {
        static vector<User> users;
        for (auto& user : users) {
            if (user.validateResetToken(token)) {
                user.setPassword(Security::hashPassword(newPassword));
                user.reset_token = "";
                user.reset_token_expiry = 0;
                cout << "Password reset successfully." << endl;
                return true;
            }
        }
        cout << "Invalid or expired token." << endl;
        return false;
    }
};

class FileService {
public:
    static File* uploadFile(string filename, string path, long userId) {
        static long fileId = 1;
        File* file = new File(fileId++, filename, path, userId);
        cout << "File uploaded successfully. ID: " << file->id << endl;
        return file;
    }

    static vector<File> getFilesByUser(long userId) {
        static vector<File> files = {
            File(1, "file1.txt", "/uploads/file1.txt", 1),
            File(2, "file2.jpg", "/uploads/file2.jpg", 1),
            File(3, "file3.pdf", "/uploads/file3.pdf", 2)
        };

        vector<File> userFiles;
        for (auto& file : files) {
            if (file.user_id == userId) {
                userFiles.push_back(file);
            }
        }
        return userFiles;
    }
};

int main() {
    // Example usage
    User* user = AuthService::registerUser("john_doe", "john@example.com", "password123");
    if (user) {
        AuthService::login("john_doe", "password123");
        user->checkIn();
        user->upgrade(100);

        File* file = FileService::uploadFile("example.txt", "/uploads/example.txt", user->id);
        auto files = FileService::getFilesByUser(user->id);
        cout << "Files count: " << files.size() << endl;

        string token = AuthService::forgotPassword("john@example.com");
        if (!token.empty()) {
            AuthService::resetPassword(token, "newpassword123");
        }
    }

    return 0;
}