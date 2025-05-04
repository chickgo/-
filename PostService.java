package com.klpbbs.service;

import com.klpbbs.entity.Post;
import com.klpbbs.repository.PostRepository;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Service;
import org.springframework.transaction.annotation.Transactional;

import java.time.LocalDateTime;
import java.util.List;
import java.util.Optional;
import java.util.Set;

@Service
@Transactional
public class PostService {

    @Autowired
    private PostRepository postRepository;

    public Post createPost(Post post) {
        // 设置默认值
        post.setStatus("DRAFT");
        post.setCreateTime(LocalDateTime.now());
        post.setUpdateTime(LocalDateTime.now());
        post.setViews(0);
        post.setLikes(0);
        post.setComments(0);
        post.setShares(0);
        post.setCollections(0);

        return postRepository.save(post);
    }

    public Post updatePost(Post post) {
        Optional<Post> existingPost = postRepository.findById(post.getId());
        if (!existingPost.isPresent()) {
            throw new RuntimeException("Post not found");
        }

        Post updatedPost = existingPost.get();
        updatedPost.setTitle(post.getTitle());
        updatedPost.setSummary(post.getSummary());
        updatedPost.setStatus(post.getStatus());
        updatedPost.setType(post.getType());
        updatedPost.setCategory(post.getCategory());
        updatedPost.setTags(post.getTags());
        updatedPost.setKeywords(post.getKeywords());
        updatedPost.setCoverImage(post.getCoverImage());
        updatedPost.setAuthor(post.getAuthor());
        updatedPost.setUpdateTime(LocalDateTime.now());

        return postRepository.save(updatedPost);
    }

    public Post publishPost(Long postId) {
        Optional<Post> existingPost = postRepository.findById(postId);
        if (!existingPost.isPresent()) {
            throw new RuntimeException("Post not found");
        }

        Post post = existingPost.get();
        post.setStatus("PUBLISHED");
        post.setPublishTime(LocalDateTime.now());
        post.setUpdateTime(LocalDateTime.now());
        return postRepository.save(post);
    }

    public Post unpublishPost(Long postId) {
        Optional<Post> existingPost = postRepository.findById(postId);
        if (!existingPost.isPresent()) {
            throw new RuntimeException("Post not found");
        }

        Post post = existingPost.get();
        post.setStatus("UNPUBLISHED");
        post.setUpdateTime(LocalDateTime.now());
        return postRepository.save(post);
    }

    public Post deletePost(Long postId) {
        Optional<Post> existingPost = postRepository.findById(postId);
        if (!existingPost.isPresent()) {
            throw new RuntimeException("Post not found");
        }

        Post post = existingPost.get();
        post.setStatus("DELETED");
        post.setUpdateTime(LocalDateTime.now());
        return postRepository.save(post);
    }

    public Post getPostById(Long postId) {
        Optional<Post> existingPost = postRepository.findById(postId);
        if (!existingPost.isPresent()) {
            throw new RuntimeException("Post not found");
        }

        postRepository.incrementViews(postId);
        return existingPost.get();
    }

    public List<Post> searchPosts(String keyword) {
        return postRepository.findByTitleContainingOrSummaryContainingOrKeywordsContaining(keyword, keyword, keyword);
    }

    public List<Post> getPostsByCategoryAndStatus(String category, String status) {
        return postRepository.findByCategoryAndStatus(category, status);
    }

    public List<Post> getPostsByTag(String tag) {
        return postRepository.findByTagsContaining(tag);
    }

    public List<Post> getPostsByAuthor(String author) {
        return postRepository.findByAuthor(author);
    }

    public List<Post> getPostsByStatus(String status) {
        return postRepository.findByStatus(status);
    }

    public List<Post> getPostsByType(String type) {
        return postRepository.findByType(type);
    }

    public Set<Long> sortPostsByViews(Set<Long> postIds) {
        return postRepository.sortPostsByViews(postIds);
    }

    public Set<Long> sortPostsByLikes(Set<Long> postIds) {
        return postRepository.sortPostsByLikes(postIds);
    }

    public Set<Long> sortPostsByComments(Set<Long> postIds) {
        return postRepository.sortPostsByComments(postIds);
    }

    public Set<Long> sortPostsByShares(Set<Long> postIds) {
        return postRepository.sortPostsByShares(postIds);
    }

    public Set<Long> sortPostsByCollections(Set<Long> postIds) {
        return postRepository.sortPostsByCollections(postIds);
    }

    public Set<Long> sortPostsByPublishTime(Set<Long> postIds) {
        return postRepository.sortPostsByPublishTime(postIds);
    }

    public Set<Long> sortPostsByUpdateTime(Set<Long> postIds) {
        return postRepository.sortPostsByUpdateTime(postIds);
    }

    public Set<Long> sortPostsByCreateTime(Set<Long> postIds) {
        return postRepository.sortPostsByCreateTime(postIds);
    }
}