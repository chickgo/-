package com.klpbbs.service;

import com.klpbbs.model.Group;
import com.klpbbs.model.GroupMember;
import com.klpbbs.model.User;
import com.klpbbs.repository.GroupRepository;
import com.klpbbs.repository.GroupMemberRepository;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Service;
import org.springframework.transaction.annotation.Transactional;

import java.time.LocalDateTime;

@Service
@Transactional
public class GroupService {

    @Autowired
    private GroupRepository groupRepository;

    @Autowired
    private GroupMemberRepository groupMemberRepository;

    public Group createGroup(String name, String description, Long creatorId) {
        Group group = new Group();
        group.setName(name);
        group.setDescription(description);
        groupRepository.save(group);

        // 自动将创建者添加为成员
        User creator = new User(); // 这里需要从数据库获取真实的用户
        creator.setId(creatorId);

        GroupMember member = new GroupMember();
        member.setUser(creator);
        member.setGroup(group);
        member.setJoinDate(LocalDateTime.now());

        groupMemberRepository.save(member);

        return group;
    }

    public Group joinGroup(Long userId, Long groupId) {
        Group group = groupRepository.findById(groupId).orElseThrow(() -> new RuntimeException("Group not found"));
        User user = new User(); // 这里需要从数据库获取真实的用户
        user.setId(userId);

        GroupMember existingMember = groupMemberRepository.findByUserIdAndGroupId(userId, groupId);
        if (existingMember != null) {
            throw new RuntimeException("User is already a member of this group");
        }

        GroupMember member = new GroupMember();
        member.setUser(user);
        member.setGroup(group);
        member.setJoinDate(LocalDateTime.now());

        groupMemberRepository.save(member);

        return group;
    }

    public Group leaveGroup(Long userId, Long groupId) {
        Group group = groupRepository.findById(groupId).orElseThrow(() -> new RuntimeException("Group not found"));
        User user = new User(); // 这里需要从数据库获取真实的用户
        user.setId(userId);

        GroupMember member = groupMemberRepository.findByUserIdAndGroupId(userId, groupId);
        if (member == null) {
            throw new RuntimeException("User is not a member of this group");
        }

        groupMemberRepository.delete(member);

        return group;
    }

    public List<Group> getAllGroups() {
        return groupRepository.findAll();
    }

    public Group getGroupById(Long groupId) {
        return groupRepository.findById(groupId).orElseThrow(() -> new RuntimeException("Group not found"));
    }

    public List<Group> getGroupsByUser(Long userId) {
        return groupRepository.findGroupsByUserId(userId);
    }
}