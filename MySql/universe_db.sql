CREATE DATABASE pl_universe;
use pl_universe;

-- 
drop table if exists `friends`;
create table `friends` (
    `user_id` bigint unsigned not null comment "auth.accounts.id",
    `friend_id` bigint unsigned not null  comment "auth.accounts.id",
    `restrict` tinyint not null default 0 comment "차단됨 == 1",
    `comment` varchar(64) not null,

    `inserted_at` DATETIME not null default "1970-01-01 00:00:00",
    `last_update_at` DATETIME not null default "1970-01-01 00:00:00",
    constraint friends_pk_user_id_friend_id primary key (`user_id`, `friend_id`)
);
-- desc `friends`;
-- select * from `friends`;

drop table if exists `guilds`;
create table `guilds` (
	`id` bigint unsigned auto_increment comment "guild unique id",
    `name` varchar(255) not null comment "길드명",
    
    `owner_char_id` bigint unsigned not null comment "world.characters.id",
    `level` int not null default 1 comment "",

    `inserted_at` DATETIME not null default "1970-01-01 00:00:00",
    `deleted_at` DATETIME not null default "1970-01-01 00:00:00",
    `last_update_at` DATETIME not null default "1970-01-01 00:00:00",
    
    constraint guilds_pk_id primary key (id)
);
-- desc `guilds`;
-- select * from `guilds`;

--
drop table if exists `guild_members`;
create table `guild_members` (
	`guild_id` bigint unsigned not null comment "guilds.id",
    `char_id` int not null default 0 comment "world.characters.id",

    `inserted_at` DATETIME not null default "1970-01-01 00:00:00",
    `last_update_at` DATETIME not null default "1970-01-01 00:00:00",

	constraint guild_members_pk_guild_id_char_id primary key (`guild_id`, `char_id`),
    foreign key (`guild_id`) references `guilds`(`id`) on delete cascade
);
-- desc `guild_members`;
-- select * from `guild_members`;