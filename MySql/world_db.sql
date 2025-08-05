CREATE DATABASE pl_world;
use pl_world;
SET SQL_SAFE_UPDATES = 0;
SET SQL_SAFE_UPDATES = 1;

-- 
drop table if exists `characters`;
create table `characters` (
    `user_id` bigint unsigned comment "auth.accounts id",
	`id` bigint unsigned comment "char_id or cid. auth.prepare_char_name에서 발급",
    `name` varchar(255) not null comment "character_name or char_name",
    
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
--delete from `characters`;
--select * from `characters`;

drop table if exists `items`;
create table `items` (
	`item_sn` bigint unsigned not null comment "item 고유키. 전서버 고유키",
    `item_type` int not null comment "",
    `item_count` int not null comment "스택 안되는 아이템 == 1",
    
    `owner_char_id` bigint unsigned not null comment "world.characters.id", 
    `bind` smallint comment "캐릭터귀속: 1, 계정귀속: 2",

    `loc_type` smallint not null comment "1: 장비, 2: 인벤토리, 3: 창고, 4: 우편첨부, 99: 압류됨",
    `loc_index` smallint not null default 0 comment "장착위치, 슬롯위치",

	constraint item_pk_item_sn primary key (`item_sn`),
    foreign key (`owner_char_id`) references `characters`(`id`) on delete cascade
);
-- desc `items`;
-- select * from `items`;