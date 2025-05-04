import express, { Request, Response } from 'express';
import bodyParser from 'body-parser';
import mongoose from 'mongoose';
import bcrypt from 'bcryptjs';
import jwt from 'jsonwebtoken';
import multer from 'multer';
import path from 'path';

// Initialize app
const app = express();
const PORT = process.env.PORT || 5000;
const upload = multer({ dest: 'uploads/' });

// Middleware
app.use(bodyParser.json());
app.use(bodyParser.urlencoded({ extended: true }));
app.use(express.static(path.join(__dirname, 'public')));

// Database connection
mongoose.connect('mongodb://localhost:27017/klpbbs', {
  useNewUrlParser: true,
  useUnifiedTopology: true,
  useCreateIndex: true,
  useFindAndModify: false
})
.then(() => console.log('MongoDB connected'))
.catch(err => console.error('MongoDB connection error:', err));

// Models
interface UserDocument extends mongoose.Document {
  username: string;
  email: string;
  password: string;
  isOnline: boolean;
  lastCheckin: Date;
  points: number;
  level: number;
  resetToken: string;
  resetTokenExpiry: Date;
  posts: mongoose.Schema.Types.ObjectId[];
  comments: mongoose.Schema.Types.ObjectId[];
  files: mongoose.Schema.Types.ObjectId[];
  notifications: mongoose.Schema.Types.ObjectId[];
  groups: mongoose.Schema.Types.ObjectId[];
}

const userSchema = new mongoose.Schema({
  username: { type: String, required: true, unique: true },
  email: { type: String, required: true, unique: true },
  password: { type: String, required: true },
  isOnline: { type: Boolean, default: false },
  lastCheckin: Date,
  points: { type: Number, default: 0 },
  level: { type: Number, default: 1 },
  resetToken: String,
  resetTokenExpiry: Date,
  posts: [{ type: mongoose.Schema.Types.ObjectId, ref: 'Post' }],
  comments: [{ type: mongoose.Schema.Types.ObjectId, ref: 'Comment' }],
  files: [{ type: mongoose.Schema.Types.ObjectId, ref: 'File' }],
  notifications: [{ type: mongoose.Schema.Types.ObjectId, ref: 'Notification' }],
  groups: [{ type: mongoose.Schema.Types.ObjectId, ref: 'Group' }]
});

userSchema.pre('save', async function (next) {
  if (!this.isModified('password')) return next();
  const salt = await bcrypt.genSalt(10);
  this.password = await bcrypt.hash(this.password, salt);
  next();
});

const User = mongoose.model<UserDocument>('User', userSchema);

interface PostDocument extends mongoose.Document {
  content: string;
  datePosted: Date;
  userId: mongoose.Schema.Types.ObjectId;
  comments: mongoose.Schema.Types.ObjectId[];
}

const postSchema = new mongoose.Schema({
  content: { type: String, required: true },
  datePosted: { type: Date, default: Date.now },
  userId: { type: mongoose.Schema.Types.ObjectId, ref: 'User', required: true },
  comments: [{ type: mongoose.Schema.Types.ObjectId, ref: 'Comment' }]
});

const Post = mongoose.model<PostDocument>('Post', postSchema);

interface CommentDocument extends mongoose.Document {
  content: string;
  datePosted: Date;
  userId: mongoose.Schema.Types.ObjectId;
  postId: mongoose.Schema.Types.ObjectId;
}

const commentSchema = new mongoose.Schema({
  content: { type: String, required: true },
  datePosted: { type: Date, default: Date.now },
  userId: { type: mongoose.Schema.Types.ObjectId, ref: 'User', required: true },
  postId: { type: mongoose.Schema.Types.ObjectId, ref: 'Post', required: true }
});

const Comment = mongoose.model<CommentDocument>('Comment', commentSchema);

interface FileDocument extends mongoose.Document {
  filename: string;
  path: string;
  uploadDate: Date;
  userId: mongoose.Schema.Types.ObjectId;
}

const fileSchema = new mongoose.Schema({
  filename: { type: String, required: true },
  path: { type: String, required: true },
  uploadDate: { type: Date, default: Date.now },
  userId: { type: mongoose.Schema.Types.ObjectId, ref: 'User', required: true }
});

const File = mongoose.model<FileDocument>('File', fileSchema);

interface NotificationDocument extends mongoose.Document {
  content: string;
  dateSent: Date;
  isRead: boolean;
  userId: mongoose.Schema.Types.ObjectId;
}

const notificationSchema = new mongoose.Schema({
  content: { type: String, required: true },
  dateSent: { type: Date, default: Date.now },
  isRead: { type: Boolean, default: false },
  userId: { type: mongoose.Schema.Types.ObjectId, ref: 'User', required: true }
});

const Notification = mongoose.model<NotificationDocument>('Notification', notificationSchema);

interface GroupDocument extends mongoose.Document {
  name: string;
  description: string;
  createdAt: Date;
  members: mongoose.Schema.Types.ObjectId[];
}

const groupSchema = new mongoose.Schema({
  name: { type: String, required: true },
  description: String,
  createdAt: { type: Date, default: Date.now },
  members: [{ type: mongoose.Schema.Types.ObjectId, ref: 'User' }]
});

const Group = mongoose.model<GroupDocument>('Group', groupSchema);

// Routes
app.post('/register', async (req: Request, res: Response) => {
  try {
    const { username, email, password } = req.body;

    // Check if user already exists
    const existingUser = await User.findOne({ $or: [{ username }, { email }] });
    if (existingUser) {
      return res.status(400).json({ message: 'User already exists' });
    }

    // Create new user
    const user = new User({
      username,
      email,
      password,
      points: 0,
      level: 1,
      isOnline: false,
      lastCheckin: null,
      resetToken: null,
      resetTokenExpiry: null
    });

    await user.save();

    // Generate JWT token
    const token = jwt.sign({ userId: user._id }, 'your_jwt_secret', { expiresIn: '7d' });

    res.status(201).json({
      message: 'User registered successfully',
      token,
      user: {
        id: user._id,
        username: user.username,
        email: user.email,
        points: user.points,
        level: user.level
      }
    });

  } catch (err) {
    console.error(err);
    res.status(500).json({ message: 'Server error' });
  }
});

app.post('/login', async (req: Request, res: Response) => {
  try {
    const { username, password } = req.body;

    // Check if user exists
    const user = await User.findOne({ username });
    if (!user) {
      return res.status(400).json({ message: 'Invalid credentials' });
    }

    // Validate password
    const isMatch = await bcrypt.compare(password, user.password);
    if (!isMatch) {
      return res.status(400).json({ message: 'Invalid credentials' });
    }

    // Update online status
    user.isOnline = true;
    await user.save();

    // Generate JWT token
    const token = jwt.sign({ userId: user._id }, 'your_jwt_secret', { expiresIn: '7d' });

    res.json({
      message: 'Login successful',
      token,
      user: {
        id: user._id,
        username: user.username,
        email: user.email,
        points: user.points,
        level: user.level
      }
    });

  } catch (err) {
    console.error(err);
    res.status(500).json({ message: 'Server error' });
  }
});

app.post('/forgot-password', async (req: Request, res: Response) => {
  try {
    const { email } = req.body;

    // Check if user exists
    const user = await User.findOne({ email });
    if (!user) {
      return res.status(400).json({ message: 'Email not found' });
    }

    // Generate reset token
    const resetToken = crypto.randomBytes(32).toString('hex');
    user.resetToken = resetToken;
    user.resetTokenExpiry = new Date(Date.now() + 3600000); // 1 hour expiry
    await user.save();

    // Send email with reset token
    console.log(`Password reset token sent to ${email}: ${resetToken}`);

    res.json({ message: 'Password reset email sent' });

  } catch (err) {
    console.error(err);
    res.status(500).json({ message: 'Server error' });
  }
});

app.post('/reset-password/:token', async (req: Request, res: Response) => {
  try {
    const { token } = req.params;
    const { password } = req.body;

    // Check if token is valid
    const user = await User.findOne({ resetToken: token });
    if (!user || user.resetTokenExpiry < new Date()) {
      return res.status(400).json({ message: 'Invalid or expired token' });
    }

    // Hash new password
    const salt = await bcrypt.genSalt(10);
    user.password = await bcrypt.hash(password, salt);
    user.resetToken = null;
    user.resetTokenExpiry = null;
    await user.save();

    res.json({ message: 'Password reset successfully' });

  } catch (err) {
    console.error(err);
    res.status(500).json({ message: 'Server error' });
  }
});

app.post('/check-in', authenticateToken, async (req: Request, res: Response) => {
  try {
    const user = await User.findById(req.user.id);
    if (!user) {
      return res.status(404).json({ message: 'User not found' });
    }

    // Check if already checked in today
    if (!user.lastCheckin || new Date(user.lastCheckin).toDateString() !== new Date().toDateString()) {
      user.points += 10;
      user.lastCheckin = new Date();
      await user.save();
      res.json({ message: 'Checked in successfully', points: user.points });
    } else {
      res.status(400).json({ message: 'Already checked in today' });
    }

  } catch (err) {
    console.error(err);
    res.status(500).json({ message: 'Server error' });
  }
});

app.post('/upgrade', authenticateToken, async (req: Request, res: Response) => {
  try {
    const { points } = req.body;
    const user = await User.findById(req.user.id);
    if (!user) {
      return res.status(404).json({ message: 'User not found' });
    }

    if (user.points >= points) {
      user.points -= points;
      user.level += 1;
      await user.save();
      res.json({ message: 'Upgraded successfully', level: user.level, points: user.points });
    } else {
      res.status(400).json({ message: 'Not enough points' });
    }

  } catch (err) {
    console.error(err);
    res.status(500).json({ message: 'Server error' });
  }
});

app.post('/posts', authenticateToken, async (req: Request, res: Response) => {
  try {
    const { content } = req.body;
    const post = new Post({
      content,
      userId: req.user.id
    });
    await post.save();
    res.status(201).json({
      message: 'Post created',
      post: {
        id: post._id,
        content: post.content,
        datePosted: post.datePosted,
        userId: post.userId
      }
    });
  } catch (err) {
    console.error(err);
    res.status(500).json({ message: 'Server error' });
  }
});

app.get('/posts', async (req: Request, res: Response) => {
  try {
    const posts = await Post.find().populate('userId', 'username').populate('comments');
    res.json(posts);
  } catch (err) {
    console.error(err);
    res.status(500).json({ message: 'Server error' });
  }
});

app.post('/comments/:postId', authenticateToken, async (req: Request, res: Response) => {
  try {
    const { content } = req.body;
    const postId = req.params.postId;
    const comment = new Comment({
      content,
      userId: req.user.id,
      postId
    });
    await comment.save();
    res.status(201).json({
      message: 'Comment created',
      comment: {
        id: comment._id,
        content: comment.content,
        datePosted: comment.datePosted,
        userId: comment.userId,
        postId: comment.postId
      }
    });
  } catch (err) {
    console.error(err);
    res.status(500).json({ message: 'Server error' });
  }
});

app.post('/files', authenticateToken, upload.single('file'), async (req: Request, res: Response) => {
  try {
    if (!req.file) {
      return res.status(400).json({ message: 'No file uploaded' });
    }

    const file = new File({
      filename: req.file.originalname,
      path: req.file.path,
      userId: req.user.id
    });
    await file.save();
    res.status(201).json({
      message: 'File uploaded',
      file: {
        id: file._id,
        filename: file.filename,
        path: file.path,
        uploadDate: file.uploadDate,
        userId: file.userId
      }
    });
  } catch (err) {
    console.error(err);
    res.status(500).json({ message: 'Server error' });
  }
});

app.get('/files', authenticateToken, async (req: Request, res: Response) => {
  try {
    const files = await File.find({ userId: req.user.id });
    res.json(files);
  } catch (err) {
    console.error(err);
    res.status(500).json({ message: 'Server error' });
  }
});

app.get('/notifications', authenticateToken, async (req: Request, res: Response) => {
  try {
    const notifications = await Notification.find({ userId: req.user.id });
    res.json(notifications);
  } catch (err) {
    console.error(err);
    res.status(500).json({ message: 'Server error' });
  }
});

app.get('/groups', authenticateToken, async (req: Request, res: Response) => {
  try {
    const groups = await Group.find().populate('members');
    res.json(groups);
  } catch (err) {
    console.error(err);
    res.status(500).json({ message: 'Server error' });
  }
});

// Middleware to authenticate token
function authenticateToken(req: Request, res: Response, next: Function) {
  const token = req.header('Authorization')?.replace('Bearer ', '');
  if (!token) {
    return res.status(401).json({ message: 'Access denied' });
  }

  try {
    const decoded = jwt.verify(token, 'your_jwt_secret') as { userId: string };
    req.user = decoded;
    next();
  } catch (err) {
    res.status(400).json({ message: 'Invalid token' });
  }
}

// Start server
app.listen(PORT, () => {
  console.log(`Server running on port ${PORT}`);
});