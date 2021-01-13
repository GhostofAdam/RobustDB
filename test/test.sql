CREATE DATABASE orderDB;

USE orderDB;

CREATE TABLE customer(
	id INT NOT NULL,
	name VARCHAR NOT NULL,
	gender VARCHAR NOT NULL,
	PRIMARY KEY (id)
);

CREATE TABLE book (
  id INT NOT NULL,
  title VARCHAR NOT NULL,
  authors VARCHAR,
  publisher VARCHAR,
  copies INT,
  PRIMARY KEY (id)
);

CREATE TABLE website(
	id INT NOT NULL,
	name VARCHAR NOT NULL,
	url VARCHAR,
	PRIMARY KEY (id)
);

CREATE TABLE price(
	website_id INT NOT NULL,
	book_id INT NOT NULL,
	price FLOAT NOT NULL,
	PRIMARY KEY (website_id,book_id),
	FOREIGN KEY (website_id) REFERENCES website(id),
	FOREIGN KEY (book_id) REFERENCES book(id)
);

CREATE TABLE orders(
	id INT NOT NULL,
	website_id INT NOT NULL,
	customer_id INT NOT NULL,
	book_id INT NOT NULL,
	order_date DATE,
	quantity INT,
	PRIMARY KEY (id),
	FOREIGN KEY (website_id) REFERENCES website(id),
	FOREIGN KEY (customer_id) REFERENCES customer(id),
	FOREIGN KEY (book_id) REFERENCES book(id)
);
