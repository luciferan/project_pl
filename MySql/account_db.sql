CREATE DATABASE pl_auth;
use pl_auth;

-- 
drop table if exists `accounts`;
create table `accounts` (
	`id` bigint unsigned auto_increment,
    `name` varchar(255) not null,
    `pass` varchar(255) not null,
    
    `inserted_at` DATETIME not null default "1970-01-01 00:00:00",
    `deleted_at` DATETIME not null default "1970-01-01 00:00:00",
    `last_update_at` DATETIME not null default "1970-01-01 00:00:00",

	constraint accounts_pk_id primary key (`id`),
    constraint accounts_uq_user_name unique (`name`)
);
-- desc `accounts`;

-- select * from `accounts`;
-- insert into accounts (`name`, `pass`) values ("a01", "p01"),("a02", "p02"),("a03", "p03"),("a04", "p04"),("a05", "p05");

-- 
drop table if exists  `prepare_char_names`;
create table `prepare_char_names` (
	`user_id` bigint unsigned not null,
    `char_id` bigint unsigned not null,
    `char_name` varchar(255) not null,
    
	constraint prepare_char_names_pk_user_id_char_id primary key (`user_id`, `char_id`),
    constraint prepare_char_names_uq_char_name unique (`char_name`),
	index prepere_char_names_index_char_id (`char_id`)
);
-- desc prepare_char_names;
