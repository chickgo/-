package com.klpbbs.repository;

import com.klpbbs.entity.Post;
import org.springframework.data.jpa.repository.JpaRepository;
import org.springframework.data.jpa.repository.JpaSpecificationExecutor;
import org.springframework.data.jpa.repository.Modifying;
import org.springframework.data.jpa.repository.Query;
import org.springframework.data.repository.query.Param;
import org.springframework.stereotype.Repository;

import java.util.List;
import java.util.Optional;
import java.util.Set;

@Repository
public interface PostRepository extends JpaRepository<Post, Long>, JpaSpecificationExecutor<Post> {

    List<Post> findByTitleContainingOrSummaryContainingOrKeywordsContaining(String title, String summary, String keywords);

    List<Post> findByCategoryAndStatus(String category, String status);

    List<Post> findByTagsContaining(String tags);

    List<Post> findByAuthor(String author);

    List<Post> findByStatus(String status);

    List<Post> findByType(String type);

    @Query("SELECT p FROM Post p WHERE p.id = :postId")
    Optional<Post> findPostWithUsersAndCommentsAndLikesAndCollectionsById(@Param("postId") Long postId);

    @Modifying
    @Query("UPDATE Post p SET p.views = p.views + 1 WHERE p.id = :postId")
    int incrementViews(@Param("postId") Long postId);

    @Modifying
    @Query("UPDATE Post p SET p.likes = p.likes + 1 WHERE p.id = :postId")
    int incrementLikes(@Param("postId") Long postId);

    @Modifying
    @Query("UPDATE Post p SET p.comments = p.comments + 1 WHERE p.id = :postId")
    int incrementComments(@Param("postId") Long postId);

    @Modifying
    @Query("UPDATE Post p SET p.shares = p.shares + 1 WHERE p.id = :postId")
    int incrementShares(@Param("postId") Long postId);

    @Modifying
    @Query("UPDATE Post p SET p.collections = p.collections + 1 WHERE p.id = :postId")
    int incrementCollections(@Param("postId") Long postId);

    @Query("SELECT p.id FROM Post p WHERE p.id IN :postIds ORDER BY p.views DESC")
    Set<Long> sortPostsByViews(@Param("postIds") Set<Long> postIds);

    @Query("SELECT p.id FROM Post p WHERE p.id IN :postIds ORDER BY p.likes DESC")
    Set<Long> sortPostsByLikes(@Param("postIds") Set<Long> postIds);

    @Query("SELECT p.id FROM Post p WHERE p.id IN :postIds ORDER BY p.comments DESC")
    Set<Long> sortPostsByComments(@Param("postIds") Set<Long> postIds);

    @Query("SELECT p.id FROM Post p WHERE p.id IN :postIds ORDER BY p.shares DESC")
    Set<Long> sortPostsByShares(@Param("postIds") Set<Long> postIds);

    @Query("SELECT p.id FROM Post p WHERE p.id IN :postIds ORDER BY p.collections DESC")
    Set<Long> sortPostsByCollections(@Param("postIds") Set<Long> postIds);

    @Query("SELECT p.id FROM Post p WHERE p.id IN :postIds ORDER BY p.publishTime DESC")
    Set<Long> sortPostsByPublishTime(@Param("postIds") Set<Long> postIds);

    @Query("SELECT p.id FROM Post p WHERE p.id IN :postIds ORDER BY p.updateTime DESC")
    Set<Long> sortPostsByUpdateTime(@Param("postIds") Set<Long> postIds);

    @Query("SELECT p.id FROM Post p WHERE p.id IN :postIds ORDER BY p.createTime DESC")
    Set<Long> sortPostsByCreateTime(@Param("postIds") Set<Long> postIds);
}