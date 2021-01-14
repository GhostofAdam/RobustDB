<<<<<<< HEAD
CREATE DATABASE test;
USE test;
DROP TABLE part;
CREATE TABLE part (
		P_PARTKEY		INT PRIMARY KEY,
		P_NAME			VARCHAR,
		P_MFGR			VARCHAR,
		P_BRAND			VARCHAR,
		P_TYPE			VARCHAR,
		P_SIZE			INT,
		P_CONTAINER		VARCHAR,
		P_RETAILPRICE	FLOAT,
		P_COMMENT		VARCHAR
	);
	
DROP TABLE region;
CREATE TABLE region (
		R_REGIONKEY	INT PRIMARY KEY,
		R_NAME		VARCHAR,
		R_COMMENT	VARCHAR
	);

DROP TABLE nation;
CREATE TABLE nation (
		N_NATIONKEY		INT PRIMARY KEY,
		N_NAME			VARCHAR,
		N_REGIONKEY		INT NOT NULL,
		N_COMMENT		VARCHAR
	);

DROP TABLE supplier;
CREATE TABLE supplier (
		S_SUPPKEY		INT PRIMARY KEY,
		S_NAME			VARCHAR,
		S_addRESS		VARCHAR,
		S_NATIONKEY		INT NOT NULL,
		S_PHONE			VARCHAR,
		S_ACCTBAL		FLOAT,
		S_COMMENT		VARCHAR
	);

DROP TABLE customer;
CREATE TABLE customer (
		C_CUSTKEY		INT PRIMARY KEY,
		C_NAME			VARCHAR,
		C_addRESS		VARCHAR,
		C_NATIONKEY		INT NOT NULL,
		C_PHONE			VARCHAR,
		C_ACCTBAL		FLOAT,
		C_MKTSEGMENT	VARCHAR,
		C_COMMENT		VARCHAR
	);

DROP TABLE partsupp;
CREATE TABLE partsupp (
		PS_PARTKEY		INT NOT NULL,
		PS_SUPPKEY		INT NOT NULL,
		PS_AVAILQTY		INT,
		PS_SUPPLYCOST	FLOAT,
		PS_COMMENT		VARCHAR
	);
ALTER TABLE partsupp ADD PRIMARY KEY (PS_PARTKEY, PS_SUPPKEY);

DROP TABLE orders;
CREATE TABLE orders (
		O_ORDERKEY		INT PRIMARY KEY,
		O_CUSTKEY		INT NOT NULL,
		O_ORDERSTATUS	VARCHAR,
		O_TOTALPRICE	FLOAT,
		O_ORDERDATE		DATE,
		O_ORDERPRIORITY	VARCHAR,
		O_CLERK			VARCHAR,
		O_SHIPPRIORITY	INT,
		O_COMMENT		VARCHAR
	);

DROP TABLE lineitem;
CREATE TABLE lineitem (
		L_ORDERKEY		INT NOT NULL,
		L_PARTKEY		INT NOT NULL,
		L_SUPPKEY		INT NOT NULL,
		L_LINENUMBER	INT,
		L_QUANTITY		FLOAT,
		L_EXTENDEDPRICE	FLOAT,
		L_DISCOUNT		FLOAT,
		L_TAX			FLOAT,
		L_RETURNFLAG	VARCHAR,
		L_LINESTATUS	VARCHAR,
		L_SHIPDATE		DATE,
		L_COMMITDATE	DATE,
		L_RECEIPTDATE	DATE,
		L_SHIPINSTRUCT	VARCHAR,
		L_SHIPMODE		VARCHAR,
		L_COMMENT		VARCHAR
	);
ALTER TABLE lineitem ADD PRIMARY KEY (L_ORDERKEY, L_LINENUMBER);

ALTER TABLE SUPPLIER ADD FOREIGN KEY (S_NATIONKEY) REFERENCES NATION(N_NATIONKEY);
ALTER TABLE PARTSUPP ADD FOREIGN KEY (PS_PARTKEY) REFERENCES PART(P_PARTKEY);
ALTER TABLE PARTSUPP ADD FOREIGN KEY (PS_SUPPKEY) REFERENCES SUPPLIER(S_SUPPKEY);
ALTER TABLE CUSTOMER ADD FOREIGN KEY (C_NATIONKEY) REFERENCES NATION(N_NATIONKEY);
ALTER TABLE ORDERS ADD FOREIGN KEY (O_CUSTKEY) REFERENCES CUSTOMER(C_CUSTKEY);
ALTER TABLE LINEITEM ADD FOREIGN KEY (L_ORDERKEY) REFERENCES ORDERS(O_ORDERKEY);
ALTER TABLE LINEITEM ADD FOREIGN KEY (L_PARTKEY,L_SUPPKEY) REFERENCES PARTSUPP(PS_PARTKEY,PS_SUPPKEY);
=======
CREATE DATABASE test;
USE test;
DROP TABLE part;
CREATE TABLE part (
		P_PARTKEY		INT PRIMARY KEY,
		P_NAME			VARCHAR,
		P_MFGR			VARCHAR,
		P_BRAND			VARCHAR,
		P_TYPE			VARCHAR,
		P_SIZE			INT,
		P_CONTAINER		VARCHAR,
		P_RETAILPRICE	FLOAT,
		P_COMMENT		VARCHAR
	);
	
DROP TABLE region;
CREATE TABLE region (
		R_REGIONKEY	INT PRIMARY KEY,
		R_NAME		VARCHAR,
		R_COMMENT	VARCHAR
	);

DROP TABLE nation;
CREATE TABLE nation (
		N_NATIONKEY		INT PRIMARY KEY,
		N_NAME			VARCHAR,
		N_REGIONKEY		INT NOT NULL,
		N_COMMENT		VARCHAR
	);

DROP TABLE supplier;
CREATE TABLE supplier (
		S_SUPPKEY		INT PRIMARY KEY,
		S_NAME			VARCHAR,
		S_addRESS		VARCHAR,
		S_NATIONKEY		INT NOT NULL,
		S_PHONE			VARCHAR,
		S_ACCTBAL		FLOAT,
		S_COMMENT		VARCHAR
	);

DROP TABLE customer;
CREATE TABLE customer (
		C_CUSTKEY		INT PRIMARY KEY,
		C_NAME			VARCHAR,
		C_addRESS		VARCHAR,
		C_NATIONKEY		INT NOT NULL,
		C_PHONE			VARCHAR,
		C_ACCTBAL		FLOAT,
		C_MKTSEGMENT	VARCHAR,
		C_COMMENT		VARCHAR
	);

DROP TABLE partsupp;
CREATE TABLE partsupp (
		PS_PARTKEY		INT NOT NULL,
		PS_SUPPKEY		INT NOT NULL,
		PS_AVAILQTY		INT,
		PS_SUPPLYCOST	FLOAT,
		PS_COMMENT		VARCHAR
	);
ALTER TABLE partsupp ADD PRIMARY KEY (PS_PARTKEY, PS_SUPPKEY);

DROP TABLE orders;
CREATE TABLE orders (
		O_ORDERKEY		INT PRIMARY KEY,
		O_CUSTKEY		INT NOT NULL,
		O_ORDERSTATUS	VARCHAR,
		O_TOTALPRICE	FLOAT,
		O_ORDERDATE		DATE,
		O_ORDERPRIORITY	VARCHAR,
		O_CLERK			VARCHAR,
		O_SHIPPRIORITY	INT,
		O_COMMENT		VARCHAR
	);

DROP TABLE lineitem;
CREATE TABLE lineitem (
		L_ORDERKEY		INT NOT NULL,
		L_PARTKEY		INT NOT NULL,
		L_SUPPKEY		INT NOT NULL,
		L_LINENUMBER	INT,
		L_QUANTITY		FLOAT,
		L_EXTENDEDPRICE	FLOAT,
		L_DISCOUNT		FLOAT,
		L_TAX			FLOAT,
		L_RETURNFLAG	VARCHAR,
		L_LINESTATUS	VARCHAR,
		L_SHIPDATE		DATE,
		L_COMMITDATE	DATE,
		L_RECEIPTDATE	DATE,
		L_SHIPINSTRUCT	VARCHAR,
		L_SHIPMODE		VARCHAR,
		L_COMMENT		VARCHAR
	);
ALTER TABLE lineitem ADD PRIMARY KEY (L_ORDERKEY, L_LINENUMBER);

ALTER TABLE SUPPLIER ADD FOREIGN KEY (S_NATIONKEY) REFERENCES NATION(N_NATIONKEY);
ALTER TABLE PARTSUPP ADD FOREIGN KEY (PS_PARTKEY) REFERENCES PART(P_PARTKEY);
ALTER TABLE PARTSUPP ADD FOREIGN KEY (PS_SUPPKEY) REFERENCES SUPPLIER(S_SUPPKEY);
ALTER TABLE CUSTOMER ADD FOREIGN KEY (C_NATIONKEY) REFERENCES NATION(N_NATIONKEY);
ALTER TABLE ORDERS ADD FOREIGN KEY (O_CUSTKEY) REFERENCES CUSTOMER(C_CUSTKEY);
ALTER TABLE LINEITEM ADD FOREIGN KEY (L_ORDERKEY) REFERENCES ORDERS(O_ORDERKEY);
ALTER TABLE LINEITEM ADD FOREIGN KEY (L_PARTKEY,L_SUPPKEY) REFERENCES PARTSUPP(PS_PARTKEY,PS_SUPPKEY);
>>>>>>> 984f64af03c43e940b6ae5c349e3390ba0adf016
ALTER TABLE NATION ADD FOREIGN KEY (N_REGIONKEY) REFERENCES REGION(R_REGIONKEY);