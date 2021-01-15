USE alter_1;

CREATE TABLE customer(
	id INT NOT NULL,
	name VARCHAR NOT NULL,
	gender VARCHAR NOT NULL,
	PRIMARY KEY (id)
);
DESC customer;
ALTER TABLE customer RENAME TO cs;
DESC cs;
ALTER TABLE cs CHANGE name name_id INT DEFAULT 0;
DESC cs;
INSERT INTO cs VALUES (0, 0, 'b');
SELECT * FROM cs WHERE name_id = 0;