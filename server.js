const express = require('express');
const bodyParser = require('body-parser');
const path = require('path');
const cors = require('cors');
const helmet = require('helmet');
const rateLimit = require('express-rate-limit');
const mongoose = require('mongoose');
const authRoutes = require('./routes/auth');
const postRoutes = require('./routes/posts');
const commentRoutes = require('./routes/comments');
const fileRoutes = require('./routes/files');
const notificationRoutes = require('./routes/notifications');
const groupRoutes = require('./routes/groups');
const userRoutes = require('./routes/users');
const security = require('./middleware/security');
const utils = require('./utils');

// Initialize app
const app = express();
const PORT = process.env.PORT || 5000;

// Middleware
app.use(cors());
app.use(helmet());
app.use(rateLimit({
    windowMs: 15 * 60 * 1000, // 15 minutes
    max: 100 // Limit each IP to 100 requests per windowMs
}));
app.use(bodyParser.json());
app.use(bodyParser.urlencoded({ extended: true }));
app.use(express.static(path.join(__dirname, 'public')));

// Database connection
mongoose.connect(process.env.MONGODB_URI || 'mongodb://localhost:27017/klpbbs', {
    useNewUrlParser: true,
    useUnifiedTopology: true,
    useCreateIndex: true,
    useFindAndModify: false
})
.then(() => console.log('MongoDB connected'))
.catch(err => console.error('MongoDB connection error:', err));

// Routes
app.use('/api/auth', authRoutes);
app.use('/api/posts', security.authenticateToken, postRoutes);
app.use('/api/comments', security.authenticateToken, commentRoutes);
app.use('/api/files', security.authenticateToken, fileRoutes);
app.use('/api/notifications', security.authenticateToken, notificationRoutes);
app.use('/api/groups', security.authenticateToken, groupRoutes);
app.use('/api/users', security.authenticateToken, userRoutes);

// Error handling middleware
app.use((err, req, res, next) => {
    console.error(err.stack);
    res.status(500).send('Something went wrong!');
});

// Start server
app.listen(PORT, () => {
    console.log(`Server running on port ${PORT}`);
});