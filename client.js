class KlpbbsClient {
    constructor(apiUrl) {
        this.apiUrl = apiUrl;
        this.token = localStorage.getItem('token') || null;
        this.currentUser = null;
    }

    async register(username, email, password) {
        try {
            const response = await fetch(`${this.apiUrl}/register`, {
                method: 'POST',
                headers: {
                    'Content-Type': 'application/json'
                },
                body: JSON.stringify({
                    username,
                    email,
                    password
                })
            });

            if (!response.ok) {
                throw new Error('Registration failed');
            }

            const data = await response.json();
            this.token = data.token;
            localStorage.setItem('token', this.token);
            this.currentUser = {
                id: data.user.id,
                username: data.user.username,
                email: data.user.email,
                points: data.user.points,
                level: data.user.level
            };
            return this.currentUser;
        } catch (error) {
            console.error('Error during registration:', error);
            throw error;
        }
    }

    async login(username, password) {
        try {
            const response = await fetch(`${this.apiUrl}/login`, {
                method: 'POST',
                headers: {
                    'Content-Type': 'application/json'
                },
                body: JSON.stringify({
                    username,
                    password
                })
            });

            if (!response.ok) {
                throw new Error('Login failed');
            }

            const data = await response.json();
            this.token = data.token;
            localStorage.setItem('token', this.token);
            this.currentUser = {
                id: data.user.id,
                username: data.user.username,
                email: data.user.email,
                points: data.user.points,
                level: data.user.level
            };
            return this.currentUser;
        } catch (error) {
            console.error('Error during login:', error);
            throw error;
        }
    }

    async forgotPassword(email) {
        try {
            const response = await fetch(`${this.apiUrl}/forgot-password`, {
                method: 'POST',
                headers: {
                    'Content-Type': 'application/json'
                },
                body: JSON.stringify({
                    email
                })
            });

            if (!response.ok) {
                throw new Error('Failed to send reset email');
            }

            return await response.json();
        } catch (error) {
            console.error('Error during forgot password:', error);
            throw error;
        }
    }

    async resetPassword(token, password) {
        try {
            const response = await fetch(`${this.apiUrl}/reset-password/${token}`, {
                method: 'POST',
                headers: {
                    'Content-Type': 'application/json'
                },
                body: JSON.stringify({
                    password
                })
            });

            if (!response.ok) {
                throw new Error('Password reset failed');
            }

            return await response.json();
        } catch (error) {
            console.error('Error during password reset:', error);
            throw error;
        }
    }

    async checkIn() {
        try {
            const response = await fetch(`${this.apiUrl}/check-in`, {
                method: 'POST',
                headers: {
                    'Content-Type': 'application/json',
                    'Authorization': `Bearer ${this.token}`
                },
                body: JSON.stringify({
                    username: this.currentUser.username
                })
            });

            if (!response.ok) {
                throw new Error('Check-in failed');
            }

            const data = await response.json();
            this.currentUser.points = data.points;
            return data;
        } catch (error) {
            console.error('Error during check-in:', error);
            throw error;
        }
    }

    async upgrade(points) {
        try {
            const response = await fetch(`${this.apiUrl}/upgrade`, {
                method: 'POST',
                headers: {
                    'Content-Type': 'application/json',
                    'Authorization': `Bearer ${this.token}`
                },
                body: JSON.stringify({
                    points
                })
            });

            if (!response.ok) {
                throw new Error('Upgrade failed');
            }

            const data = await response.json();
            this.currentUser.level = data.level;
            this.currentUser.points = data.points;
            return data;
        } catch (error) {
            console.error('Error during upgrade:', error);
            throw error;
        }
    }

    async createPost(content) {
        try {
            const response = await fetch(`${this.apiUrl}/posts`, {
                method: 'POST',
                headers: {
                    'Content-Type': 'application/json',
                    'Authorization': `Bearer ${this.token}`
                },
                body: JSON.stringify({
                    content
                })
            });

            if (!response.ok) {
                throw new Error('Failed to create post');
            }

            return await response.json();
        } catch (error) {
            console.error('Error during post creation:', error);
            throw error;
        }
    }

    async getPosts() {
        try {
            const response = await fetch(`${this.apiUrl}/posts`);
            if (!response.ok) {
                throw new Error('Failed to fetch posts');
            }

            return await response.json();
        } catch (error) {
            console.error('Error during fetching posts:', error);
            throw error;
        }
    }

    async createComment(postId, content) {
        try {
            const response = await fetch(`${this.apiUrl}/comments/${postId}`, {
                method: 'POST',
                headers: {
                    'Content-Type': 'application/json',
                    'Authorization': `Bearer ${this.token}`
                },
                body: JSON.stringify({
                    content
                })
            });

            if (!response.ok) {
                throw new Error('Failed to create comment');
            }

            return await response.json();
        } catch (error) {
            console.error('Error during comment creation:', error);
            throw error;
        }
    }

    async uploadFile(file) {
        try {
            const formData = new FormData();
            formData.append('file', file);

            const response = await fetch(`${this.apiUrl}/files`, {
                method: 'POST',
                headers: {
                    'Authorization': `Bearer ${this.token}`
                },
                body: formData
            });

            if (!response.ok) {
                throw new Error('Failed to upload file');
            }

            return await response.json();
        } catch (error) {
            console.error('Error during file upload:', error);
            throw error;
        }
    }

    async getFiles() {
        try {
            const response = await fetch(`${this.apiUrl}/files`, {
                headers: {
                    'Authorization': `Bearer ${this.token}`
                }
            });

            if (!response.ok) {
                throw new Error('Failed to fetch files');
            }

            return await response.json();
        } catch (error) {
            console.error('Error during fetching files:', error);
            throw error;
        }
    }

    async getNotifications() {
        try {
            const response = await fetch(`${this.apiUrl}/notifications`, {
                headers: {
                    'Authorization': `Bearer ${this.token}`
                }
            });

            if (!response.ok) {
                throw new Error('Failed to fetch notifications');
            }

            return await response.json();
        } catch (error) {
            console.error('Error during fetching notifications:', error);
            throw error;
        }
    }

    async getGroups() {
        try {
            const response = await fetch(`${this.apiUrl}/groups`);
            if (!response.ok) {
                throw new Error('Failed to fetch groups');
            }

            return await response.json();
        } catch (error) {
            console.error('Error during fetching groups:', error);
            throw error;
        }
    }

    logout() {
        this.token = null;
        localStorage.removeItem('token');
        this.currentUser = null;
    }
}

// 示例用法
const client = new KlpbbsClient('http://localhost:5000/api');

// 注册用户
// client.register('john_doe', 'john@example.com', 'password123').then(user => {
//     console.log('Registered user:', user);
// });

// 登录用户
// client.login('john_doe', 'password123').then(user => {
//     console.log('Logged in user:', user);
// });

// 发送重置密码邮件
// client.forgotPassword('john@example.com').then(response => {
//     console.log('Password reset email sent:', response);
// });

// 重置密码
// client.resetPassword('reset_token_here', 'newpassword123').then(response => {
//     console.log('Password reset response:', response);
// });

// 签到
// client.checkIn().then(response => {
//     console.log('Check-in response:', response);
// });

// 升级
// client.upgrade(100).then(response => {
//     console.log('Upgrade response:', response);
// });

// 创建帖子
// client.createPost('Hello, world!').then(response => {
//     console.log('Post created:', response);
// });

// 获取所有帖子
// client.getPosts().then(posts => {
//     console.log('Posts:', posts);
// });

// 创建评论
// client.createComment(1, 'Great post!').then(response => {
//     console.log('Comment created:', response);
// });

// 上传文件
// const fileInput = document.createElement('input');
// fileInput.type = 'file';
// fileInput.onchange = async (e) => {
//     const file = e.target.files[0];
//     const result = await client.uploadFile(file);
//     console.log('File uploaded:', result);
// };
// fileInput.click();

// 获取用户文件
// client.getFiles().then(files => {
//     console.log('Files:', files);
// });

// 获取通知
// client.getNotifications().then(notifications => {
//     console.log('Notifications:', notifications);
// });

// 获取群组
// client.getGroups().then(groups => {
//     console.log('Groups:', groups);
// });