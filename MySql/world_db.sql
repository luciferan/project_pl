CREATE DATABASE pl_world;
use pl_world;

-- 
drop table if exists `characters`;
create table `characters` (
    `user_id` bigint unsigned,
	`id` bigint unsigned auto_increment,
    `name` varchar(255) not null,
    
    `zone` int not null default 0,
    `pos_x` int not null default 0,
    `pos_y` int not null default 0,
    `pos_z` int not null default 0,

    `inserted_at` DATETIME not null default "1970-01-01 00:00:00",
    `deleted_at` DATETIME not null default "1970-01-01 00:00:00",
    `last_update_at` DATETIME not null default "1970-01-01 00:00:00",

	constraint character_pk_id primary key (`id`),
    constraint character_uq_name unique (`name`)
);
-- desc `characters`;
-- select * from `characters`;

-- 
drop table if exists `items`;
create table `items` (
	`item_sn` bigint unsigned not null,
    `item_type` int not null,
    `item_count` int not null,
    
    `owner_char_id` bigint unsigned not null,
    `soul_bind` tinyint,

    `loc_type` tinyint not null,
    `loc_index` smallint not null,

	constraint item_pk_item_sn primary key (`item_sn`),
    foreign key (`owner_char_id`) references `characters`(`id`) on delete cascade
);
-- desc `items`;
-- select * from `items`;