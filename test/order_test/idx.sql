USE db_idx;

DROP TABLE customer;
CREATE TABLE customer(
	id INT NOT NULL,
	name VARCHAR NOT NULL,
	gender VARCHAR NOT NULL,
	PRIMARY KEY (id)
);

DESC customer;
CREATE INDEX IDX1 ON customer (name);
DESC customer;
ALTER TABLE customer ADD INDEX IDX2 (name);
DESC customer;
ALTER TABLE customer DROP INDEX IDX1;
DESC customer;
ALTER TABLE customer DROP INDEX IDX2;
DESC customer;