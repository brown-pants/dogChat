CREATE DATABASE dogChat CHARACTER SET utf8mb4 COLLATE utf8mb4_bin;

USE dogChat;

CREATE TABLE users
(
    `name`        VARCHAR(255) NOT NULL PRIMARY KEY,
    `password`    VARCHAR(255) NOT NULL,
    `profile_url` VARCHAR(255) NOT NULL
);

CREATE TABLE friends
(
    `userA`     VARCHAR(255) NOT NULL,
    `userB`     VARCHAR(255) NOT NULL,
    PRIMARY KEY (`userA`, `userB`),
    FOREIGN KEY (`userA`) REFERENCES `users` (`name`),
    FOREIGN KEY (`userB`) REFERENCES `users` (`name`)
);

CREATE TABLE friend_apply
(
    `id`             INT AUTO_INCREMENT PRIMARY KEY,
    `user_apply`     VARCHAR(255) NOT NULL,
    `user_accept`    VARCHAR(255) NOT NULL,
    `leave_msg`      VARCHAR(255),
    `apply_state`    VARCHAR(255) DEFAULT 'wait', 
    FOREIGN KEY (`user_apply`) REFERENCES `users` (`name`),
    FOREIGN KEY (`user_accept`) REFERENCES `users` (`name`)
);

DELIMITER $$

-- 注册用户
CREATE PROCEDURE `reg_user`(
    IN `new_name` VARCHAR(255), 
    IN `new_pwd` VARCHAR(255), 
    IN `new_profile_url` VARCHAR(255), 
    OUT `result` INT)
BEGIN
    -- 如果在执行过程中遇到任何错误，则回滚事务
    DECLARE EXIT HANDLER FOR SQLEXCEPTION
    BEGIN
        -- 回滚事务
        ROLLBACK;
        -- 设置返回值为-1，表示错误
        SET result = -1;
    END;
    -- 开始事务
    START TRANSACTION;
    -- 检查用户名是否已存在
    IF EXISTS (SELECT 1 FROM `users` WHERE `name` = new_name) THEN
        SET result = 0; -- 用户名已存在
        COMMIT;
    ELSE
        -- 在users表中插入新记录
        INSERT INTO `users` (`name`, `password`, `profile_url`) VALUES (new_name, new_pwd, new_profile_url);
        SET result = 1; -- 成功
        COMMIT;
    END IF;
END$$

-- 添加好友
CREATE PROCEDURE `add_friend`(
    IN `_userA` VARCHAR(255),
    IN `_userB` VARCHAR(255),
    OUT `result` INT)
BEGIN
    -- 如果在执行过程中遇到任何错误，则回滚事务
    DECLARE EXIT HANDLER FOR SQLEXCEPTION
    BEGIN
        -- 回滚事务
        ROLLBACK;
        -- 设置返回值为-1，表示错误
        SET result = -1;
    END;
    -- 开始事务
    START TRANSACTION;
    -- 检查用户名是否存在
    IF NOT EXISTS (SELECT 1 FROM `users` WHERE `name` = _userA) OR NOT EXISTS (SELECT 1 FROM `users` WHERE `name` = _userB) THEN
        SET result = 2;
        COMMIT;
    -- 检查好友是否存在
    ELSEIF EXISTS (SELECT 1 FROM `friends` WHERE `userA` = _userA AND "userB" = _userB) THEN
        SET result = 3;
        COMMIT;
    ELSE
        -- 在friends表中插入新记录
        INSERT INTO `friends` (`userA`, `userB`) VALUES (_userA, _userB);
        SET result = 1; -- 成功
        COMMIT;
    END IF;
END$$

-- 删除好友
CREATE PROCEDURE `remove_friend`(
    IN `_userA` VARCHAR(255),
    IN `_userB` VARCHAR(255),
    OUT `result` INT)
BEGIN
    -- 如果在执行过程中遇到任何错误，则回滚事务
    DECLARE EXIT HANDLER FOR SQLEXCEPTION
    BEGIN
        -- 回滚事务
        ROLLBACK;
        -- 设置返回值为-1，表示错误
        SET result = -1;
    END;
    -- 开始事务
    START TRANSACTION;
    -- 检查用户名是否存在
    IF NOT EXISTS (SELECT 1 FROM `users` WHERE `name` = _userA) OR NOT EXISTS (SELECT 1 FROM `users` WHERE `name` = _userB) THEN
        SET result = 2;
        COMMIT;
    -- 检查好友是否存在
    ELSEIF NOT EXISTS (SELECT 1 FROM `friends` WHERE `userA` = _userA AND `userB` = _userB) THEN
        SET result = 3;
        COMMIT;
    ELSE
        -- 在friends表中删除记录
        DELETE FROM `friends` WHERE `userA` = _userA AND `userB` = _userB;
        SET result = 1; -- 成功
        COMMIT;
    END IF;
END$$

-- 好友申请
CREATE PROCEDURE `apply_friend`(
    IN `new_user_apply` VARCHAR(255),
    IN `new_user_accept` VARCHAR(255),
    IN `new_leave_msg` VARCHAR(255),
    OUT `result` INT
)
BEGIN
    -- 如果在执行过程中遇到任何错误，则回滚事务
    DECLARE EXIT HANDLER FOR SQLEXCEPTION
    BEGIN
        -- 回滚事务
        ROLLBACK;
        -- 设置返回值为-1，表示错误
        SET result = -1;
    END;
    -- 开始事务
    START TRANSACTION;
    -- 检查用户名是否存在
    IF NOT EXISTS (SELECT 1 FROM `users` WHERE `name` = new_user_apply) OR NOT EXISTS (SELECT 1 FROM `users` WHERE `name` = new_user_accept) THEN
        SET result = 2;
        COMMIT;
    -- 检查是否已经申请过
    ELSEIF EXISTS (SELECT 1 FROM `friend_apply` WHERE `user_apply` = new_user_apply AND `user_accept` = new_user_accept) THEN
        SET result = 3;
        COMMIT;
    ELSE
        -- 在friend_apply表中插入新记录
        INSERT INTO `friend_apply` (`user_apply`, `user_accept`, `leave_msg`) VALUES (new_user_apply, new_user_accept, new_leave_msg);
        SET result = 1; -- 成功
        COMMIT;
    END IF;
END$$

-- 好友申请状态修改
CREATE PROCEDURE `change_friend_apply_state`(
    IN `_user_apply` VARCHAR(255),
    IN `_user_accept` VARCHAR(255),
    IN `_apply_state` VARCHAR(255),
    OUT `result` INT
)
BEGIN
    -- 如果在执行过程中遇到任何错误，则回滚事务
    DECLARE EXIT HANDLER FOR SQLEXCEPTION
    BEGIN
        -- 回滚事务
        ROLLBACK;
        -- 设置返回值为-1，表示错误
        SET result = -1;
    END;
    -- 开始事务
    START TRANSACTION;
    -- 检查用户名是否存在
    IF NOT EXISTS (SELECT 1 FROM `users` WHERE `name` = _user_apply) OR NOT EXISTS (SELECT 1 FROM `users` WHERE `name` = _user_accept) THEN
        SET result = 2;
        COMMIT;
    -- 检查是否有申请记录
    ELSEIF NOT EXISTS (SELECT 1 FROM `friend_apply` WHERE `user_apply` = _user_apply AND `user_accept` = _user_accept) THEN
        SET result = 3;
        COMMIT;
    ELSE
        -- 在friend_apply表中插入新记录
        UPDATE `friend_apply` SET `apply_state` = _apply_state WHERE `user_apply` = _user_apply AND `user_accept` = _user_accept;
        SET result = 1; -- 成功
        COMMIT;
    END IF;
END$$

-- 好友申请记录删除
CREATE PROCEDURE `remove_friend_apply`(
    IN `_user_apply` VARCHAR(255),
    IN `_user_accept` VARCHAR(255),
    OUT `result` INT
)
BEGIN
    -- 如果在执行过程中遇到任何错误，则回滚事务
    DECLARE EXIT HANDLER FOR SQLEXCEPTION
    BEGIN
        -- 回滚事务
        ROLLBACK;
        -- 设置返回值为-1，表示错误
        SET result = -1;
    END;
    -- 开始事务
    START TRANSACTION;
    -- 检查用户名是否存在
    IF NOT EXISTS (SELECT 1 FROM `users` WHERE `name` = _user_apply) OR NOT EXISTS (SELECT 1 FROM `users` WHERE `name` = _user_accept) THEN
        SET result = 2;
        COMMIT;
    -- 检查是否有申请记录
    ELSEIF NOT EXISTS (SELECT 1 FROM `friend_apply` WHERE `user_apply` = _user_apply AND `user_accept` = _user_accept) THEN
        SET result = 3;
        COMMIT;
    ELSE
        -- 在friend_apply表中删除记录
        DELETE FROM `friend_apply` WHERE `user_apply` = _user_apply AND `user_accept` = _user_accept;
        SET result = 1; -- 成功
        COMMIT;
    END IF;
END$$
DELIMITER ;