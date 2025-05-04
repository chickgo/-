package com.klpbbs.controller.admin;

import com.klpbbs.entity.User;
import com.klpbbs.entity.Post;
import com.klpbbs.service.UserService;
import com.klpbbs.service.PostService;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.http.ResponseEntity;
import org.springframework.web.bind.annotation.*;

import java.time.LocalDateTime;
import java.util.List;
import java.util.Set;

@RestController
@RequestMapping("/api/admin")
public class AdminController {

    @Autowired
    private UserService userService;

    @Autowired
    private PostService postService;

    @GetMapping("/users")
    public ResponseEntity<?> getAllUsers() {
        return ResponseEntity.ok(userService.findAllUsers());
    }

    @GetMapping("/users/search")
    public ResponseEntity<?> searchUsers(@RequestParam String keyword) {
        Set<Long> userIds = userService.searchUsersByKeyword(keyword);
        return ResponseEntity.ok(userIds);
    }

    @GetMapping("/users/active")
    public ResponseEntity<?> getActiveUsers() {
        return ResponseEntity.ok(userService.getActiveUsers());
    }

    @GetMapping("/users/sort/level-points-reputation")
    public ResponseEntity<?> sortUsersByLevelAndPointsAndReputation() {
        return ResponseEntity.ok(userService.sortUsersByLevelAndPointsAndReputation());
    }

    @GetMapping("/users/sort/activity")
    public ResponseEntity<?> sortUsersByActivity() {
        return ResponseEntity.ok(userService.sortUsersByActivity());
    }

    @GetMapping("/users/sort/social-influence")
    public ResponseEntity<?> sortUsersBySocialInfluence() {
        return ResponseEntity.ok(userService.sortUsersBySocialInfluence());
    }

    @GetMapping("/users/sort/registration-time")
    public ResponseEntity<?> sortUsersByRegistrationTime() {
        return ResponseEntity.ok(userService.sortUsersByRegistrationTime());
    }

    @GetMapping("/users/sort/last-login-time")
    public ResponseEntity<?> sortUsersByLastLoginTime() {
        return ResponseEntity.ok(userService.sortUsersByLastLoginTime());
    }

    @GetMapping("/users/sort/update-time")
    public ResponseEntity<?> sortUsersByUpdateTime() {
        return ResponseEntity.ok(userService.sortUsersByUpdateTime());
    }

    @GetMapping("/users/count/active")
    public ResponseEntity<?> countActiveUsers() {
        return ResponseEntity.ok(userService.countActiveUsers());
    }

    @GetMapping("/posts")
    public ResponseEntity<?> getAllPosts() {
        return ResponseEntity.ok(postService.findAllPosts());
    }

    @GetMapping("/posts/search")
    public ResponseEntity<?> searchPosts(@RequestParam String keyword) {
        List<Post> posts = postService.searchPosts(keyword);
        return ResponseEntity.ok(posts);
    }

    @GetMapping("/posts/category/{category}/status/{status}")
    public ResponseEntity<?> getPostsByCategoryAndStatus(@PathVariable String category, @PathVariable String status) {
        List<Post> posts = postService.getPostsByCategoryAndStatus(category, status);
        return ResponseEntity.ok(posts);
    }

    @GetMapping("/posts/tag/{tag}")
    public ResponseEntity<?> getPostsByTag(@PathVariable String tag) {
        List<Post> posts = postService.getPostsByTag(tag);
        return ResponseEntity.ok(posts);
    }

    @GetMapping("/posts/author/{author}")
    public ResponseEntity<?> getPostsByAuthor(@PathVariable String author) {
        List<Post> posts = postService.getPostsByAuthor(author);
        return ResponseEntity.ok(posts);
    }

    @GetMapping("/posts/status/{status}")
    public ResponseEntity<?> getPostsByStatus(@PathVariable String status) {
        List<Post> posts = postService.getPostsByStatus(status);
        return ResponseEntity.ok(posts);
    }

    @GetMapping("/posts/type/{type}")
    public ResponseEntity<?> getPostsByType(@PathVariable String type) {
        List<Post> posts = postService.getPostsByType(type);
        return ResponseEntity.ok(posts);
    }

    @GetMapping("/posts/sort/views")
    public ResponseEntity<?> sortPostsByViews() {
        return ResponseEntity.ok(postService.sortPostsByViews());
    }

    @GetMapping("/posts/sort/likes")
    public ResponseEntity<?> sortPostsByLikes() {
        return ResponseEntity.ok(postService.sortPostsByLikes());
    }

    @GetMapping("/posts/sort/comments")
    public ResponseEntity<?> sortPostsByComments() {
        return ResponseEntity.ok(postService.sortPostsByComments());
    }

    @GetMapping("/posts/sort/shares")
    public ResponseEntity<?> sortPostsByShares() {
        return ResponseEntity.ok(postService.sortPostsByShares());
    }

    @GetMapping("/posts/sort/collections")
    public ResponseEntity<?> sortPostsByCollections() {
        return ResponseEntity.ok(postService.sortPostsByCollections());
    }

    @GetMapping("/posts/sort/publish-time")
    public ResponseEntity<?> sortPostsByPublishTime() {
        return ResponseEntity.ok(postService.sortPostsByPublishTime());
    }

    @GetMapping("/posts/sort/update-time")
    public ResponseEntity<?> sortPostsByUpdateTime() {
        return ResponseEntity.ok(postService.sortPostsByUpdateTime());
    }

    @GetMapping("/posts/sort/create-time")
    public ResponseEntity<?> sortPostsByCreateTime() {
        return ResponseEntity.ok(postService.sortPostsByCreateTime());
    }
}