CREATE DATABASE dogChat;

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

DELIMITER $$

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

CREATE PROCEDURE `add_friend`(
    IN `userA` VARCHAR(255),
    IN `userB` VARCHAR(255),
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
    IF NOT EXISTS (SELECT 1 FROM `users` WHERE `name` = userA) OR NOT EXISTS (SELECT 1 FROM `users` WHERE `name` = userB) THEN
        SET result = 2;
        COMMIT;
    -- 检查好友是否存在
    ELSEIF EXISTS (SELECT 1 FROM `friends` WHERE `userA` = userA AND "userB" = userB) THEN
        SET result = 3;
        COMMIT;
    ELSE
        -- 在friends表中插入新记录
        INSERT INTO `friends` (`userA`, `userB`) VALUES (userA, userB);
        SET result = 1; -- 成功
        COMMIT;
    END IF;
END$$

DELIMITER ;