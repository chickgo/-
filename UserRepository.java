package com.klpbbs.repository;

import com.klpbbs.entity.User;
import org.springframework.data.jpa.repository.JpaRepository;
import org.springframework.data.jpa.repository.JpaSpecificationExecutor;
import org.springframework.data.jpa.repository.Modifying;
import org.springframework.data.jpa.repository.Query;
import org.springframework.data.repository.query.Param;
import org.springframework.stereotype.Repository;

import java.util.Optional;
import java.util.Set;

@Repository
public interface UserRepository extends JpaRepository<User, Long>, JpaSpecificationExecutor<User> {

    Optional<User> findByUsername(String username);

    Optional<User> findByEmail(String email);

    Optional<User> findByPhone(String phone);

    Boolean existsByUsername(String username);

    Boolean existsByEmail(String email);

    Boolean existsByPhone(String phone);

    @Query("SELECT u FROM User u WHERE u.id = :userId")
    Optional<User> findUserWithRolesAndPermissionsById(@Param("userId") Long userId);

    @Modifying
    @Query("UPDATE User u SET u.password = :password WHERE u.id = :userId")
    int updatePassword(@Param("userId") Long userId, @Param("password") String password);

    @Modifying
    @Query("UPDATE User u SET u.status = :status WHERE u.id = :userId")
    int updateUserStatus(@Param("userId") Long userId, @Param("status") User.UserStatus status);

    @Query("SELECT u.id FROM User u WHERE u.id IN :userIds AND u.status = 'ACTIVE'")
    Set<Long> findActiveUserIds(@Param("userIds") Set<Long> userIds);

    @Query("SELECT u.id FROM User u WHERE u.username LIKE %:keyword% OR u.nickname LIKE %:keyword% OR u.email LIKE %:keyword% OR u.phone LIKE %:keyword%")
    Set<Long> searchUserIdsByKeyword(@Param("keyword") String keyword);

    @Query("SELECT u.id FROM User u WHERE u.id IN :userIds ORDER BY u.level DESC, u.points DESC, u.reputation DESC")
    Set<Long> sortUsersByLevelAndPointsAndReputation(@Param("userIds") Set<Long> userIds);

    @Query("SELECT u.id FROM User u WHERE u.id IN :userIds ORDER BY u.postsCount DESC, u.commentsCount DESC, u.likesCount DESC")
    Set<Long> sortUsersByActivity(@Param("userIds") Set<Long> userIds);

    @Query("SELECT u.id FROM User u WHERE u.id IN :userIds ORDER BY u.followersCount DESC, u.followingsCount ASC")
    Set<Long> sortUsersBySocialInfluence(@Param("userIds") Set<Long> userIds);

    @Query("SELECT u.id FROM User u WHERE u.id IN :userIds ORDER BY u.createTime DESC")
    Set<Long> sortUsersByRegistrationTime(@Param("userIds") Set<Long> userIds);

    @Query("SELECT u.id FROM User u WHERE u.id IN :userIds ORDER BY u.lastLoginTime DESC")
    Set<Long> sortUsersByLastLoginTime(@Param("userIds") Set<Long> userIds);

    @Query("SELECT u.id FROM User u WHERE u.id IN :userIds ORDER BY u.updateTime DESC")
    Set<Long> sortUsersByUpdateTime(@Param("userIds") Set<Long> userIds);

    @Query("SELECT COUNT(u.id) FROM User u WHERE u.status = 'ACTIVE'")
    long countActiveUsers();
}