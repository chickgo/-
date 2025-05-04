require 'sinatra'
require 'json'
require 'bcrypt'
require 'securerandom'
require 'jwt'
require 'active_record'
require 'sqlite3'

# Database configuration
ActiveRecord::Base.establish_connection(
  adapter: 'sqlite3',
  database: 'app.db'
)

# Models
class User < ActiveRecord::Base
  has_many :posts
  has_many :comments
  has_many :files
  has_many :notifications
  has_many :group_members
  has_many :groups, through: :group_members

  before_create :generate_api_key

  def self.find_or_create_by(username:, email:, password:)
    user = find_by(username: username) || find_by(email: email)
    return user if user

    user = new(username: username, email: email, password: password)
    user.save
    user
  end

  def generate_reset_token
    self.reset_token = SecureRandom.hex(16)
    self.reset_token_expiry = Time.now + 1.hour
    save
  end

  def check_in
    return false if last_checkin && last_checkin > Time.now - 1.day

    self.points += 10
    self.last_checkin = Time.now
    save
  end

  def upgrade(points)
    return false if self.points < points

    self.points -= points
    self.level += 1
    save
  end

  def valid_reset_token?(token)
    self.reset_token == token && self.reset_token_expiry > Time.now
  end

  private

  def generate_api_key
    self.api_key = SecureRandom.hex(16)
  end
end

class Post < ActiveRecord::Base
  belongs_to :user
  has_many :comments
end

class Comment < ActiveRecord::Base
  belongs_to :user
  belongs_to :post
end

class File < ActiveRecord::Base
  belongs_to :user
end

class Notification < ActiveRecord::Base
  belongs_to :user
end

class Group < ActiveRecord::Base
  has_many :group_members
  has_many :users, through: :group_members
end

class GroupMember < ActiveRecord::Base
  belongs_to :group
  belongs_to :user
end

# Routes
before do
  content_type :json
end

helpers do
  def authenticate_user
    token = request.env['HTTP_AUTHORIZATION']&.split(' ')&.last
    if token
      begin
        decoded_token = JWT.decode(token, 'your_jwt_secret', true, { algorithm: 'HS256' })
        @current_user = User.find_by(id: decoded_token[0]['user_id'])
      rescue JWT::DecodeError
        halt 401, { message: 'Invalid token' }.to_json
      end
    else
      halt 401, { message: 'Token missing' }.to_json
    end
  end
end

post '/register' do
  user = User.new(params)
  user.password = BCrypt::Password.create(params[:password])
  user.points = 0
  user.level = 1
  user.is_online = false

  if user.save
    { message: 'User registered successfully', user: user }.to_json
  else
    { message: 'Error registering user', errors: user.errors.full_messages }.to_json
  end
end

post '/login' do
  user = User.find_by(username: params[:username])
  if user && BCrypt::Password.new(user.password) == params[:password]
    user.is_online = true
    user.save

    token = JWT.encode({ user_id: user.id }, 'your_jwt_secret', 'HS256')
    { message: 'Login successful', token: token, user: user }.to_json
  else
    { message: 'Invalid credentials' }.to_json
  end
end

post '/forgot-password' do
  user = User.find_by(email: params[:email])
  if user
    user.generate_reset_token
    # Send email with reset token
    { message: 'Password reset email sent' }.to_json
  else
    { message: 'Email not found' }.to_json
  end
end

post '/reset-password/:token' do |token|
  user = User.find_by(reset_token: token)
  if user && user.valid_reset_token?(token)
    user.password = BCrypt::Password.create(params[:password])
    user.reset_token = nil
    user.reset_token_expiry = nil
    user.save
    { message: 'Password reset successfully' }.to_json
  else
    { message: 'Invalid or expired token' }.to_json
  end
end

post '/check-in' do
  authenticate_user
  if @current_user.check_in
    { message: 'Checked in successfully', points: @current_user.points }.to_json
  else
    { message: 'Already checked in today' }.to_json
  end
end

post '/upgrade' do
  authenticate_user
  points = params[:points].to_i
  if @current_user.upgrade(points)
    { message: 'Upgraded successfully', level: @current_user.level, points: @current_user.points }.to_json
  else
    { message: 'Not enough points' }.to_json
  end
end

post '/posts' do
  authenticate_user
  post = Post.new(content: params[:content], user_id: @current_user.id)
  if post.save
    { message: 'Post created successfully', post: post }.to_json
  else
    { message: 'Error creating post', errors: post.errors.full_messages }.to_json
  end
end

get '/posts' do
  posts = Post.all.includes(:comments)
  posts.to_json(include: :comments)
end

post '/comments/:post_id' do
  authenticate_user
  post = Post.find_by(id: params[:post_id])
  if post
    comment = Comment.new(content: params[:content], user_id: @current_user.id, post_id: post.id)
    if comment.save
      { message: 'Comment created successfully', comment: comment }.to_json
    else
      { message: 'Error creating comment', errors: comment.errors.full_messages }.to_json
    end
  else
    { message: 'Post not found' }.to_json
  end
end

post '/files' do
  authenticate_user
  file = File.new(
    filename: params[:file][:filename],
    path: "uploads/#{params[:file][:filename]}",
    user_id: @current_user.id
  )
  if file.save
    FileUtils.cp(params[:file][:tempfile], file.path)
    { message: 'File uploaded successfully', file: file }.to_json
  else
    { message: 'Error uploading file', errors: file.errors.full_messages }.to_json
  end
end

get '/files' do
  authenticate_user
  files = File.where(user_id: @current_user.id)
  files.to_json
end

get '/notifications' do
  authenticate_user
  notifications = Notification.where(user_id: @current_user.id)
  notifications.to_json
end

get '/groups' do
  groups = Group.all.includes(:users)
  groups.to_json(include: :users)
end

# Initialize database
unless File.exist?('app.db')
  User.create_table
  Post.create_table
  Comment.create_table
  File.create_table
  Notification.create_table
  Group.create_table
  GroupMember.create_table
end

# Start server
run! if app_file == $0