CREATE DATABASE orderDB_1;
USE orderDB_1;

CREATE TABLE customer(
	id INT NOT NULL,
	name VARCHAR NOT NULL,
	gender VARCHAR NOT NULL
);
SHOW TABLES;
INSERT INTO customer VALUES (0,'a','b'),(1,'a','b'),(2,'a','b'),(3,'a','b');
EXIT;
