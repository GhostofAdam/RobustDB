USE orderDB_1;

CREATE TABLE customer(
	id INT NOT NULL,
	name VARCHAR NOT NULL,
	gender VARCHAR NOT NULL,
	PRIMARY KEY (id)
);
SHOW TABLES;
DESC customer;
CREATE INDEX index_1 on customer (name, gender);
DESC customer;
DROP INDEX index_1 on customer;
DESC customer;
EXIT;