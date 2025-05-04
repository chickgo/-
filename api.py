from flask import Blueprint, request, jsonify
from models import User, Post, Comment, File, Notification, Group, GroupMember
from auth import login_required, generate_token
from utils import filter_sensitive_words

api = Blueprint('api', __name__)

@api.route('/posts', methods=['POST'])
@login_required
def create_post():
    content = request.json.get('content')
    if not content:
        return jsonify({'error': 'Content is required'}), 400

    content = filter_sensitive_words(content)

    post = Post(content=content, user_id=request.user.id)
    post.save()

    return jsonify({'post': post.to_dict()}), 201

@api.route('/posts', methods=['GET'])
def get_posts():
    posts = Post.query.order_by(Post.date_posted.desc()).all()
    return jsonify({'posts': [post.to_dict() for post in posts]})

@api.route('/posts/<int:post_id>/comments', methods=['POST'])
@login_required
def create_comment(post_id):
    content = request.json.get('content')
    if not content:
        return jsonify({'error': 'Content is required'}), 400

    content = filter_sensitive_words(content)

    post = Post.query.get(post_id)
    if not post:
        return jsonify({'error': 'Post not found'}), 404

    comment = Comment(content=content, user_id=request.user.id, post_id=post_id)
    comment.save()

    return jsonify({'comment': comment.to_dict()}), 201

@api.route('/comments/<int:comment_id>', methods=['DELETE'])
@login_required
def delete_comment(comment_id):
    comment = Comment.query.get(comment_id)
    if not comment:
        return jsonify({'error': 'Comment not found'}), 404

    if comment.user_id != request.user.id and request.user.role != 'admin':
        return jsonify({'error': 'Permission denied'}), 403

    comment.delete()
    return jsonify({'message': 'Comment deleted successfully'})

@api.route('/files', methods=['POST'])
@login_required
def upload_file():
    if 'file' not in request.files:
        return jsonify({'error': 'File is required'}), 400

    file = request.files['file']
    if file.filename == '':
        return jsonify({'error': 'File is required'}), 400

    filename = secure_filename(file.filename)
    file_path = os.path.join(current_app.config['UPLOAD_FOLDER'], filename)
    file.save(file_path)

    file_record = File(filename=filename, path=file_path, user_id=request.user.id)
    file_record.save()

    return jsonify({'file': file_record.to_dict()}), 201

@api.route('/files/<int:file_id>', methods=['GET'])
def get_file(file_id):
    file_record = File.query.get(file_id)
    if not file_record:
        return jsonify({'error': 'File not found'}), 404

    return send_file(file_record.path)

@api.route('/notifications', methods=['GET'])
@login_required
def get_notifications():
    notifications = Notification.query.filter_by(user_id=request.user.id).all()
    return jsonify({'notifications': [notification.to_dict() for notification in notifications]})

@api.route('/groups', methods=['GET'])
@login_required
def get_groups():
    groups = Group.query.all()
    return jsonify({'groups': [group.to_dict() for group in groups]})

@api.route('/groups/<int:group_id>/join', methods=['POST'])
@login_required
def join_group(group_id):
    group = Group.query.get(group_id)
    if not group:
        return jsonify({'error': 'Group not found'}), 404

    existing_member = GroupMember.query.filter_by(user_id=request.user.id, group_id=group_id).first()
    if existing_member:
        return jsonify({'error': 'Already a member of this group'}), 400

    member = GroupMember(user_id=request.user.id, group_id=group_id)
    member.save()

    return jsonify({'message': 'Joined group successfully'})

@api.route('/groups/<int:group_id>/leave', methods=['POST'])
@login_required
def leave_group(group_id):
    group = Group.query.get(group_id)
    if not group:
        return jsonify({'error': 'Group not found'}), 404

    member = GroupMember.query.filter_by(user_id=request.user.id, group_id=group_id).first()
    if not member:
        return jsonify({'error': 'Not a member of this group'}), 400

    member.delete()
    return jsonify({'message': 'Left group successfully'})

@api.route('/search', methods=['GET'])
def search():
    query = request.args.get('q')
    if not query:
        return jsonify({'error': 'Query is required'}), 400

    users = User.query.filter(
        (User.username.contains(query)) |
        (User.bio.contains(query)) |
        (User.location.contains(query))
    ).all()

    posts = Post.query.filter(Post.content.contains(query)).all()

    return jsonify({
        'users': [user.to_dict() for user in users],
        'posts': [post.to_dict() for post in posts]
    })

@api.route('/messages', methods=['POST'])
@login_required
def send_message():
    receiver_id = request.json.get('receiver_id')
    content = request.json.get('content')

    if not receiver_id or not content:
        return jsonify({'error': 'Receiver and content are required'}), 400

    message = Message(sender_id=request.user.id, receiver_id=receiver_id, content=content)
    message.save()

    # 发送通知
    notification = Notification(
        content=f'You have a new message from {request.user.username}',
        user_id=receiver_id
    )
    notification.save()

    return jsonify({'message': message.to_dict()})

@api.route('/messages', methods=['GET'])
@login_required
def get_messages():
    messages = Message.query.filter(
        (Message.sender_id == request.user.id) |
        (Message.receiver_id == request.user.id)
    ).order_by(Message.send_time.desc()).all()

    return jsonify({'messages': [message.to_dict() for message in messages]})

@api.route('/messages/<int:message_id>/read', methods=['POST'])
@login_required
def mark_message_as_read(message_id):
    message = Message.query.get(message_id)
    if not message:
        return jsonify({'error': 'Message not found'}), 404

    if message.receiver_id != request.user.id:
        return jsonify({'error': 'Permission denied'}), 403

    message.is_read = True
    message.save()

    return jsonify({'message': 'Message marked as read'})