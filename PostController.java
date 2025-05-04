package com.klpbbs.controller;

import com.klpbbs.entity.Post;
import com.klpbbs.service.PostService;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.http.ResponseEntity;
import org.springframework.web.bind.annotation.*;

import java.time.LocalDateTime;
import java.util.List;
import java.util.Set;

@RestController
@RequestMapping("/api/posts")
public class PostController {

    @Autowired
    private PostService postService;

    @PostMapping
    public ResponseEntity<?> createPost(@RequestBody Post post) {
        Post createdPost = postService.createPost(post);
        return ResponseEntity.ok(createdPost);
    }

    @PutMapping("/{id}")
    public ResponseEntity<?> updatePost(@PathVariable Long id, @RequestBody Post post) {
        post.setId(id);
        Post updatedPost = postService.updatePost(post);
        return ResponseEntity.ok(updatedPost);
    }

    @PostMapping("/{id}/publish")
    public ResponseEntity<?> publishPost(@PathVariable Long id) {
        Post publishedPost = postService.publishPost(id);
        return ResponseEntity.ok(publishedPost);
    }

    @PostMapping("/{id}/unpublish")
    public ResponseEntity<?> unpublishPost(@PathVariable Long id) {
        Post unpublishedPost = postService.unpublishPost(id);
        return ResponseEntity.ok(unpublishedPost);
    }

    @DeleteMapping("/{id}")
    public ResponseEntity<?> deletePost(@PathVariable Long id) {
        Post deletedPost = postService.deletePost(id);
        return ResponseEntity.ok(deletedPost);
    }

    @GetMapping("/{id}")
    public ResponseEntity<?> getPostById(@PathVariable Long id) {
        Post post = postService.getPostById(id);
        return ResponseEntity.ok(post);
    }

    @GetMapping("/search")
    public ResponseEntity<?> searchPosts(@RequestParam String keyword) {
        List<Post> posts = postService.searchPosts(keyword);
        return ResponseEntity.ok(posts);
    }

    @GetMapping("/category/{category}/status/{status}")
    public ResponseEntity<?> getPostsByCategoryAndStatus(@PathVariable String category, @PathVariable String status) {
        List<Post> posts = postService.getPostsByCategoryAndStatus(category, status);
        return ResponseEntity.ok(posts);
    }

    @GetMapping("/tag/{tag}")
    public ResponseEntity<?> getPostsByTag(@PathVariable String tag) {
        List<Post> posts = postService.getPostsByTag(tag);
        return ResponseEntity.ok(posts);
    }

    @GetMapping("/author/{author}")
    public ResponseEntity<?> getPostsByAuthor(@PathVariable String author) {
        List<Post> posts = postService.getPostsByAuthor(author);
        return ResponseEntity.ok(posts);
    }

    @GetMapping("/status/{status}")
    public ResponseEntity<?> getPostsByStatus(@PathVariable String status) {
        List<Post> posts = postService.getPostsByStatus(status);
        return ResponseEntity.ok(posts);
    }

    @GetMapping("/type/{type}")
    public ResponseEntity<?> getPostsByType(@PathVariable String type) {
        List<Post> posts = postService.getPostsByType(type);
        return ResponseEntity.ok(posts);
    }

    @GetMapping("/sort/views")
    public ResponseEntity<?> sortPostsByViews(@RequestParam Set<Long> postIds) {
        Set<Long> sortedPostIds = postService.sortPostsByViews(postIds);
        return ResponseEntity.ok(sortedPostIds);
    }

    @GetMapping("/sort/likes")
    public ResponseEntity<?> sortPostsByLikes(@RequestParam Set<Long> postIds) {
        Set<Long> sortedPostIds = postService.sortPostsByLikes(postIds);
        return ResponseEntity.ok(sortedPostIds);
    }

    @GetMapping("/sort/comments")
    public ResponseEntity<?> sortPostsByComments(@RequestParam Set<Long> postIds) {
        Set<Long> sortedPostIds = postService.sortPostsByComments(postIds);
        return ResponseEntity.ok(sortedPostIds);
    }

    @GetMapping("/sort/shares")
    public ResponseEntity<?> sortPostsByShares(@RequestParam Set<Long> postIds) {
        Set<Long> sortedPostIds = postService.sortPostsByShares(postIds);
        return ResponseEntity.ok(sortedPostIds);
    }

    @GetMapping("/sort/collections")
    public ResponseEntity<?> sortPostsByCollections(@RequestParam Set<Long> postIds) {
        Set<Long> sortedPostIds = postService.sortPostsByCollections(postIds);
        return ResponseEntity.ok(sortedPostIds);
    }

    @GetMapping("/sort/publish-time")
    public ResponseEntity<?> sortPostsByPublishTime(@RequestParam Set<Long> postIds) {
        Set<Long> sortedPostIds = postService.sortPostsByPublishTime(postIds);
        return ResponseEntity.ok(sortedPostIds);
    }

    @GetMapping("/sort/update-time")
    public ResponseEntity<?> sortPostsByUpdateTime(@RequestParam Set<Long> postIds) {
        Set<Long> sortedPostIds = postService.sortPostsByUpdateTime(postIds);
        return ResponseEntity.ok(sortedPostIds);
    }

    @GetMapping("/sort/create-time")
    public ResponseEntity<?> sortPostsByCreateTime(@RequestParam Set<Long> postIds) {
        Set<Long> sortedPostIds = postService.sortPostsByCreateTime(postIds);
        return ResponseEntity.ok(sortedPostIds);
    }
}