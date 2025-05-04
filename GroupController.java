package com.klpbbs.controller;

import com.klpbbs.model.Group;
import com.klpbbs.service.GroupService;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.http.ResponseEntity;
import org.springframework.web.bind.annotation.*;

import java.util.List;

@RestController
@RequestMapping("/api/groups")
public class GroupController {

    @Autowired
    private GroupService groupService;

    @PostMapping
    public ResponseEntity<?> createGroup(@RequestParam String name, @RequestParam String description, @RequestParam Long creatorId) {
        Group group = groupService.createGroup(name, description, creatorId);
        return ResponseEntity.ok(group);
    }

    @PostMapping("/{groupId}/join")
    public ResponseEntity<?> joinGroup(@PathVariable Long groupId, @RequestParam Long userId) {
        Group group = groupService.joinGroup(userId, groupId);
        return ResponseEntity.ok(group);
    }

    @PostMapping("/{groupId}/leave")
    public ResponseEntity<?> leaveGroup(@PathVariable Long groupId, @RequestParam Long userId) {
        Group group = groupService.leaveGroup(userId, groupId);
        return ResponseEntity.ok(group);
    }

    @GetMapping
    public ResponseEntity<?> getAllGroups() {
        List<Group> groups = groupService.getAllGroups();
        return ResponseEntity.ok(groups);
    }

    @GetMapping("/{groupId}")
    public ResponseEntity<?> getGroupById(@PathVariable Long groupId) {
        Group group = groupService.getGroupById(groupId);
        return ResponseEntity.ok(group);
    }

    @GetMapping("/user/{userId}")
    public ResponseEntity<?> getGroupsByUser(@PathVariable Long userId) {
        List<Group> groups = groupService.getGroupsByUser(userId);
        return ResponseEntity.ok(groups);
    }
}