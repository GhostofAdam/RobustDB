USE rename_1;

CREATE TABLE customer(
	id INT NOT NULL,
	name VARCHAR NOT NULL,
	gender VARCHAR NOT NULL,
	PRIMARY KEY (id)
);
SHOW TABLES;
DESC customer;
ALTER TABLE customer RENAME to fuck;
EXIT;