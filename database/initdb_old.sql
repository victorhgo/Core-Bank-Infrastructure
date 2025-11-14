/* Sample to create and initialize a DB with some data
 * by Victor Correa
 */

-- Create and switch to DB
CREATE DATABASE "Bank1";

\c Bank1;

-- Creating the tables

CREATE TABLE customers (
    customer_id SERIAL PRIMARY KEY,
    name VARCHAR(100),
    email VARCHAR(120),
    date_of_birth DATE,
    address TEXT
);

CREATE TABLE accounts (
    account_id SERIAL PRIMARY KEY,
    customer_id INTEGER REFERENCES customers(customer_id),
    account_type VARCHAR(20),
    balance NUMERIC(12,2),
    created_at TIMESTAMP DEFAULT NOW()
);

CREATE TABLE transactions (
    transaction_id SERIAL PRIMARY KEY,
    from_account INTEGER REFERENCES accounts(account_id),
    to_account INTEGER REFERENCES accounts(account_id),
    amount NUMERIC(10,2),
    timestamp TIMESTAMP DEFAULT NOW()
);

-- Inserting information on the tables
INSERT INTO customers (customer_id, name, email, date_of_birth, address)
VALUES (1, 'John Doe', 'jond@mail.com', '1985-06-12', 'Sample Street 42');

INSERT INTO accounts (account_id, customer_id, account_type, balance)
VALUES (0001, 1, 'checking', 1500.00);
