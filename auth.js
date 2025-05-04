const express = require('express');
const router = express.Router();
const bcrypt = require('bcryptjs');
const jwt = require('jsonwebtoken');
const User = require('../models/User');
const security = require('../middleware/security');
const utils = require('../utils');

// Register
router.post('/register', async (req, res) => {
    try {
        const { username, email, password } = req.body;

        // Check if user already exists
        let user = await User.findOne({ $or: [{ username }, { email }] });
        if (user) {
            return res.status(400).json({ msg: 'User already exists' });
        }

        // Create new user
        user = new User({
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

        // Hash password
        const salt = await bcrypt.genSalt(10);
        user.password = await bcrypt.hash(password, salt);

        await user.save();

        // Generate JWT token
        const payload = {
            user: {
                id: user.id
            }
        };
        jwt.sign(payload, process.env.JWT_SECRET || 'your_jwt_secret', { expiresIn: '7d' }, (err, token) => {
            if (err) throw err;
            res.json({ token });
        });

    } catch (err) {
        console.error(err);
        res.status(500).json({ msg: 'Server error' });
    }
});

// Login
router.post('/login', async (req, res) => {
    try {
        const { username, password } = req.body;

        // Check if user exists
        const user = await User.findOne({ username });
        if (!user) {
            return res.status(400).json({ msg: 'Invalid credentials' });
        }

        // Validate password
        const isMatch = await bcrypt.compare(password, user.password);
        if (!isMatch) {
            return res.status(400).json({ msg: 'Invalid credentials' });
        }

        // Update online status
        user.isOnline = true;
        await user.save();

        // Generate JWT token
        const payload = {
            user: {
                id: user.id
            }
        };
        jwt.sign(payload, process.env.JWT_SECRET || 'your_jwt_secret', { expiresIn: '7d' }, (err, token) => {
            if (err) throw err;
            res.json({ token });
        });

    } catch (err) {
        console.error(err);
        res.status(500).json({ msg: 'Server error' });
    }
});

// Forgot password
router.post('/forgot-password', async (req, res) => {
    try {
        const { email } = req.body;

        // Check if user exists
        const user = await User.findOne({ email });
        if (!user) {
            return res.status(400).json({ msg: 'Email not found' });
        }

        // Generate reset token
        const resetToken = utils.generateResetToken();
        user.resetToken = resetToken;
        user.resetTokenExpiry = Date.now() + 3600000; // 1 hour expiry
        await user.save();

        // Send email with reset token
        utils.sendResetEmail(user.email, resetToken);

        res.json({ msg: 'Password reset email sent' });

    } catch (err) {
        console.error(err);
        res.status(500).json({ msg: 'Server error' });
    }
});

// Reset password
router.post('/reset-password/:token', async (req, res) => {
    try {
        const { token } = req.params;
        const { password } = req.body;

        // Check if token is valid
        const user = await User.findOne({ resetToken: token });
        if (!user || user.resetTokenExpiry < Date.now()) {
            return res.status(400).json({ msg: 'Invalid or expired token' });
        }

        // Hash new password
        const salt = await bcrypt.genSalt(10);
        user.password = await bcrypt.hash(password, salt);
        user.resetToken = null;
        user.resetTokenExpiry = null;
        await user.save();

        res.json({ msg: 'Password reset successfully' });

    } catch (err) {
        console.error(err);
        res.status(500).json({ msg: 'Server error' });
    }
});

// Check-in for points
router.post('/check-in', security.authenticateToken, async (req, res) => {
    try {
        const user = await User.findById(req.user.id);
        if (!user) {
            return res.status(404).json({ msg: 'User not found' });
        }

        // Check if already checked in today
        if (!user.lastCheckin || new Date(user.lastCheckin).toDateString() !== new Date().toDateString()) {
            user.points += 10;
            user.lastCheckin = Date.now();
            await user.save();
            res.json({ msg: 'Checked in successfully', points: user.points });
        } else {
            res.status(400).json({ msg: 'Already checked in today' });
        }

    } catch (err) {
        console.error(err);
        res.status(500).json({ msg: 'Server error' });
    }
});

// Upgrade user level
router.post('/upgrade', security.authenticateToken, async (req, res) => {
    try {
        const { points } = req.body;
        const user = await User.findById(req.user.id);
        if (!user) {
            return res.status(404).json({ msg: 'User not found' });
        }

        if (user.points >= points) {
            user.points -= points;
            user.level += 1;
            await user.save();
            res.json({ msg: 'Upgraded successfully', level: user.level, points: user.points });
        } else {
            res.status(400).json({ msg: 'Not enough points' });
        }

    } catch (err) {
        console.error(err);
        res.status(500).json({ msg: 'Server error' });
    }
});

module.exports = router;