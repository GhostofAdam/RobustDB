USE orderDB_1;

CREATE TABLE customer(
	id INT NOT NULL,
	name VARCHAR NOT NULL,
	gender VARCHAR NOT NULL,
	PRIMARY KEY (id)
);
SHOW TABLES;
DESC customer;
ALTER TABLE customer ADD FUCK INT;
DESC customer;
ALTER TABLE customer DROP FUCK;
INSERT INTO customer VALUES (0,'a','b'),(1,'a','b'),(2,'a','b'),(3,'a','b');
SELECT * FROM customer WHERE name = 'a';
EXIT;