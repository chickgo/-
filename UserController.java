package com.klpbbs.controller;

import com.klpbbs.model.User;
import com.klpbbs.service.UserService;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.web.bind.annotation.*;

@RestController
@RequestMapping("/api/users")
public class UserController {

    @Autowired
    private UserService userService;

    @PostMapping("/register")
    public User registerUser(@RequestBody User user) {
        return userService.registerUser(user.getUsername(), user.getEmail(), user.getPassword());
    }
}