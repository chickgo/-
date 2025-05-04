package com.klpbbs.service;

import com.klpbbs.model.User;
import com.klpbbs.repository.UserRepository;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.security.crypto.bcrypt.BCryptPasswordEncoder;
import org.springframework.stereotype.Service;

import java.time.LocalDate;
import java.util.UUID;

@Service
public class AuthService {

    @Autowired
    private UserRepository userRepository;

    @Autowired
    private BCryptPasswordEncoder passwordEncoder;

    public User register(String username, String email, String password) {
        User user = new User();
        user.setUsername(username);
        user.setEmail(email);
        user.setPassword(passwordEncoder.encode(password));
        user.setPoints(0);
        user.setLevel(1);
        user.setOnline(false);
        user.setLastCheckin(null);
        user.setResetToken(null);
        user.setResetTokenExpiry(null);

        return userRepository.save(user);
    }

    public User login(String username, String password) {
        User user = userRepository.findByUsername(username);
        if (user != null && passwordEncoder.matches(password, user.getPassword())) {
            user.setOnline(true);
            return userRepository.save(user);
        }
        return null;
    }

    public User forgotPassword(String email) {
        User user = userRepository.findByEmail(email);
        if (user != null) {
            String token = UUID.randomUUID().toString();
            user.setResetToken(token);
            user.setResetTokenExpiry(LocalDate.now().plusDays(1));
            return userRepository.save(user);
        }
        return null;
    }

    public User resetPassword(String token, String newPassword) {
        User user = userRepository.findByResetToken(token);
        if (user != null && user.getResetTokenExpiry().isAfter(LocalDate.now())) {
            user.setPassword(passwordEncoder.encode(newPassword));
            user.setResetToken(null);
            user.setResetTokenExpiry(null);
            return userRepository.save(user);
        }
        return null;
    }

    public User checkIn(Long userId) {
        User user = userRepository.findById(userId).orElse(null);
        if (user != null && (user.getLastCheckin() == null || !user.getLastCheckin().equals(LocalDate.now()))) {
            user.setPoints(user.getPoints() + 10);
            user.setLastCheckin(LocalDate.now());
            return userRepository.save(user);
        }
        return user;
    }

    public User upgrade(Long userId, int points) {
        User user = userRepository.findById(userId).orElse(null);
        if (user != null && user.getPoints() >= points) {
            user.setPoints(user.getPoints() - points);
            user.setLevel(user.getLevel() + 1);
            return userRepository.save(user);
        }
        return user;
    }
}