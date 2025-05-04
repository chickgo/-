package com.klpbbs.service;

import com.klpbbs.model.Role;
import com.klpbbs.model.User;
import com.klpbbs.repository.RoleRepository;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Service;

import java.util.Optional;

@Service
public class RoleService {

    @Autowired
    private RoleRepository roleRepository;

    public void assignRoleToUser(Long userId, Long roleId) {
        Optional<User> user = roleRepository.findById(userId);
        Optional<Role> role = roleRepository.findById(roleId);

        if (user.isPresent() && role.isPresent()) {
            user.get().getRoles().add(role.get());
            roleRepository.save(user.get());
        }
    }
}