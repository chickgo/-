from flask import Flask, render_template, request, redirect, url_for, flash, jsonify
from flask_sqlalchemy import SQLAlchemy
from flask_login import LoginManager, UserMixin, login_user, login_required, logout_user, current_user
from werkzeug.security import generate_password_hash, check_password_hash
import os
from datetime import datetime
import uuid

app = Flask(__name__)
app.config['SECRET_KEY'] = 'your_secret_key'
app.config['SQLALCHEMY_DATABASE_URI'] = 'sqlite:///site.db'
app.config['UPLOAD_FOLDER'] = 'uploads'
app.config['MAX_CONTENT_LENGTH'] = 16 * 1024 * 1024  # 16MB max upload

db = SQLAlchemy(app)
login_manager = LoginManager(app)
login_manager.login_view = 'login'

# 跨平台支持
@app.route('/')
def index():
    return render_template('index.html')

# 登录
@app.route('/login', methods=['GET', 'POST'])
def login():
    if request.method == 'POST':
        username = request.form.get('username')
        password = request.form.get('password')
        user = User.query.filter_by(username=username).first()
        if user and check_password_hash(user.password, password):
            login_user(user)
            return redirect(url_for('dashboard'))
        flash('Invalid username or password')
    return render_template('login.html')

# 注册
@app.route('/register', methods=['GET', 'POST'])
def register():
    if request.method == 'POST':
        username = request.form.get('username')
        email = request.form.get('email')
        password = request.form.get('password')
        hashed_password = generate_password_hash(password)
        user = User(username=username, email=email, password=hashed_password)
        db.session.add(user)
        db.session.commit()
        flash('Account created successfully')
        return redirect(url_for('login'))
    return render_template('register.html')

# 密码找回
@app.route('/forgot-password', methods=['GET', 'POST'])
def forgot_password():
    if request.method == 'POST':
        email = request.form.get('email')
        user = User.query.filter_by(email=email).first()
        if user:
            # 生成重置链接并发送邮件
            token = str(uuid.uuid4())
            user.reset_token = token
            user.reset_token_expiry = datetime.now() + timedelta(hours=1)
            db.session.commit()
            send_reset_email(user, token)
            flash('Password reset email sent')
        else:
            flash('Email not found')
    return render_template('forgot_password.html')

# 重置密码
@app.route('/reset-password/<token>', methods=['GET', 'POST'])
def reset_password(token):
    user = User.query.filter_by(reset_token=token).first()
    if not user or user.reset_token_expiry < datetime.now():
        flash('Invalid or expired token')
        return redirect(url_for('forgot_password'))
    if request.method == 'POST':
        password = request.form.get('password')
        user.password = generate_password_hash(password)
        user.reset_token = None
        user.reset_token_expiry = None
        db.session.commit()
        flash('Password reset successfully')
        return redirect(url_for('login'))
    return render_template('reset_password.html')

# 用户等级系统
@app.route('/upgrade', methods=['POST'])
@login_required
def upgrade():
    points = int(request.form.get('points'))
    if current_user.points >= points:
        current_user.points -= points
        current_user.level += 1
        db.session.commit()
        flash('Upgraded successfully')
    else:
        flash('Not enough points')
    return redirect(url_for('dashboard'))

# 签到积分
@app.route('/check-in', methods=['POST'])
@login_required
def check_in():
    if not current_user.last_checkin or current_user.last_checkin < datetime.now().date():
        current_user.points += 10
        current_user.last_checkin = datetime.now().date()
        db.session.commit()
        flash('Checked in successfully')
    else:
        flash('Already checked in today')
    return redirect(url_for('dashboard'))

# 发帖
@app.route('/post', methods=['POST'])
@login_required
def create_post():
    content = request.form.get('content')
    post = Post(content=content, user_id=current_user.id)
    db.session.add(post)
    db.session.commit()
    flash('Post created')
    return redirect(url_for('dashboard'))

# 评论
@app.route('/comment/<int:post_id>', methods=['POST'])
@login_required
def comment(post_id):
    content = request.form.get('content')
    comment = Comment(content=content, user_id=current_user.id, post_id=post_id)
    db.session.add(comment)
    db.session.commit()
    flash('Comment added')
    return redirect(url_for('post_detail', post_id=post_id))

# 文件上传
@app.route('/upload', methods=['POST'])
@login_required
def upload_file():
    if 'file' not in request.files:
        flash('No file part')
        return redirect(request.url)
    file = request.files['file']
    if file.filename == '':
        flash('No selected file')
        return redirect(request.url)
    if file:
        filename = secure_filename(file.filename)
        file_path = os.path.join(app.config['UPLOAD_FOLDER'], filename)
        file.save(file_path)
        file_record = File(filename=filename, path=file_path, user_id=current_user.id)
        db.session.add(file_record)
        db.session.commit()
        flash('File uploaded')
        return redirect(url_for('dashboard'))

# 实时在线用户
@app.route('/online-users')
def online_users():
    users = User.query.filter_by(is_online=True).all()
    return jsonify([user.username for user in users])

# 消息通知
@app.route('/notifications')
@login_required
def notifications():
    notifications = Notification.query.filter_by(user_id=current_user.id).all()
    return render_template('notifications.html', notifications=notifications)

# 群聊功能
@app.route('/groups')
@login_required
def groups():
    groups = Group.query.all()
    return render_template('groups.html', groups=groups)

# 用户搜索
@app.route('/search', methods=['GET'])
def search():
    query = request.args.get('q')
    users = User.query.filter(User.username.contains(query)).all()
    return render_template('search_results.html', users=users)

class User(UserMixin, db.Model):
    id = db.Column(db.Integer, primary_key=True)
    username = db.Column(db.String(150), unique=True, nullable=False)
    email = db.Column(db.String(150), unique=True, nullable=False)
    password = db.Column(db.String(150), nullable=False)
    is_online = db.Column(db.Boolean, default=False)
    last_checkin = db.Column(db.Date)
    points = db.Column(db.Integer, default=0)
    level = db.Column(db.Integer, default=1)
    reset_token = db.Column(db.String(36))
    reset_token_expiry = db.Column(db.DateTime)
    posts = db.relationship('Post', backref='author', lazy=True)
    comments = db.relationship('Comment', backref='author', lazy=True)
    files = db.relationship('File', backref='uploader', lazy=True)
    notifications = db.relationship('Notification', backref='user', lazy=True)
    groups = db.relationship('GroupMember', backref='user', lazy=True)

class Post(db.Model):
    id = db.Column(db.Integer, primary_key=True)
    content = db.Column(db.Text, nullable=False)
    date_posted = db.Column(db.DateTime, default=datetime.now)
    user_id = db.Column(db.Integer, db.ForeignKey('user.id'), nullable=False)
    comments = db.relationship('Comment', backref='post', lazy=True)

class Comment(db.Model):
    id = db.Column(db.Integer, primary_key=True)
    content = db.Column(db.Text, nullable=False)
    date_posted = db.Column(db.DateTime, default=datetime.now)
    user_id = db.Column(db.Integer, db.ForeignKey('user.id'), nullable=False)
    post_id = db.Column(db.Integer, db.ForeignKey('post.id'), nullable=False)

class File(db.Model):
    id = db.Column(db.Integer, primary_key=True)
    filename = db.Column(db.String(150), nullable=False)
    path = db.Column(db.String(200), nullable=False)
    upload_date = db.Column(db.DateTime, default=datetime.now)
    user_id = db.Column(db.Integer, db.ForeignKey('user.id'), nullable=False)

class Notification(db.Model):
    id = db.Column(db.Integer, primary_key=True)
    content = db.Column(db.Text, nullable=False)
    date_sent = db.Column(db.DateTime, default=datetime.now)
    is_read = db.Column(db.Boolean, default=False)
    user_id = db.Column(db.Integer, db.ForeignKey('user.id'), nullable=False)

class Group(db.Model):
    id = db.Column(db.Integer, primary_key=True)
    name = db.Column(db.String(100), nullable=False)
    description = db.Column(db.Text)
    created_at = db.Column(db.DateTime, default=datetime.now)
    members = db.relationship('GroupMember', backref='group', lazy=True)

class GroupMember(db.Model):
    id = db.Column(db.Integer, primary_key=True)
    user_id = db.Column(db.Integer, db.ForeignKey('user.id'), nullable=False)
    group_id = db.Column(db.Integer, db.ForeignKey('group.id'), nullable=False)
    joined_at = db.Column(db.DateTime, default=datetime.now)

@login_manager.user_loader
def load_user(user_id):
    return User.query.get(int(user_id))

def send_reset_email(user, token):
    # 这里实现邮件发送逻辑
    pass

if __name__ == '__main__':
    db.create_all()
    app.run(debug=True)