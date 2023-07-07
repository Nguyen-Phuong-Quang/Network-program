create table users (
	user_id serial primary key,
	username varchar(50),
	password varchar(50),
	signed_in int default 0
);

create table direct_msg (
	id serial primary key,
	sender_id int,
	receiver_id int,
	content varchar(255),
	created_time timestamptz DEFAULT current_timestamp
);

create table groups (
	group_id serial primary key,
	owner_id int,
	group_name varchar(50),
	foreign key (owner_id)
		references users(user_id)
);

create table group_participants (
	participant_id serial primary key,
	group_id int,
	user_id int,
	foreign key (group_id)
		references groups(group_id),
	foreign key (user_id)
		references users(user_id)
);

create table group_msg (
	group_msg_id serial primary key,
	participant_id int,
	content varchar(255),
	created_time timestamptz DEFAULT current_timestamp,
	foreign key (participant_id)
		references group_participants(participant_id)
);

create table join_group_requests (
	request_id serial primary key,
	user_id int,
	group_id int,
	foreign key (group_id)
		references groups(group_id),
	foreign key (user_id)
		references users(user_id)
);

CREATE OR REPLACE FUNCTION insert_group_participants_function()
RETURNS TRIGGER AS $$
BEGIN
    INSERT INTO group_participants (group_id, user_id)
    VALUES (NEW.group_id, NEW.owner_id);
    RETURN NEW;
END;
$$ LANGUAGE plpgsql;

CREATE TRIGGER insert_group_participants_trigger
AFTER INSERT ON groups
FOR EACH ROW
EXECUTE FUNCTION insert_group_participants_function();


