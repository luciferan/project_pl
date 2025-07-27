CREATE DATABASE pl_universe;
use pl_universe;

-- 
drop table if exists `guilds`;
create table `guilds` (
	`id` bigint unsigned auto_increment,
    `name` varchar(255) not null,
    
    `owner_char_id` bigint unsigned not null,
)

drop table if exists `guild_members`;
create table `guild_members` (
	`id` bigint unsigned not null default 0,
    `char_id` int not null default 0,
    `item_count` int not null default 0,
    
    `owner_char_id` bigint unsigned not null default 0,
    `soul_bind` tinyint default 0,

    `loc_type` tinyint not null default 0,
    `loc_index` smallint not null default 0,

	constraint item_pk_item_sn primary key (item_sn),
    foreign key (owner_char_id) references `characters`(id) on delete cascade
);