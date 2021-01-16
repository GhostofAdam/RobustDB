USE db_pk;

CREATE TABLE customer(
	id INT NOT NULL,
	name VARCHAR NOT NULL,
	gender VARCHAR NOT NULL,
	PRIMARY KEY (id)
);

DESC customer;
INSERT INTO customer VALUES (0,'a','b'),(1,'a','b'),(2,'a','b'),(3,'a','b');
INSERT INTO customer VALUES (0,'c','d');
SELECT * FROM customer WHERE name = 'A';

ALTER TABLE customer DROP PRIMARY KEY;
DESC customer;

ALTER TABLE customer ADD CONSTRAINT pk1 PRIMARY KEY (RID, id);
DESC customer;

ALTER TABLE customer DROP PRIMARY KEY pk2;
DESC customer;
ALTER TABLE customer DROP PRIMARY KEY pk1;
DESC customer;