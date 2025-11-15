/*  - Initialization DB script improved with more data and constraints
 *
 *  This script should be run only once to initialize and define the
 * database structures including tables and constraints.
 *
 * Be careful when running this script to not mess up an existing database 
 * By Victor Correa */

-- DB does not exist, create it with the following command: 

CREATE DATABASE database1;

\c database1;

/* If the tables exist, delete (drop) */

DROP TABLE IF EXISTS transactions CASCADE;
DROP TABLE IF EXISTS accounts CASCADE;
DROP TABLE IF EXISTS customers CASCADE;
DROP TABLE IF EXISTS audit_logs CASCADE;

-- Creating the base tables with references

CREATE TABLE customers (
    customer_id     SERIAL PRIMARY KEY,
                    /* NOT NULL a constraint to enforces a column to NOT accept NULL values */
    full_name       VARCHAR(120)  NOT NULL,
    email           VARCHAR(200)  UNIQUE NOT NULL,
    phone           VARCHAR(30),
                    /* DATE format YYYY-MM-DD */
    date_of_birth   DATE,
    address         TEXT,
                    /* PostgreSQL acceps NOW() instead of CURRENT_TIMESTAMP 
                       But we will use CURRENT_TIMESTAMP for better cross-db compatibility */
    created_at      TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

CREATE TABLE accounts (
    account_id      SERIAL PRIMARY KEY,
    customer_id     INT NOT NULL REFERENCES customers(customer_id),
    account_type    VARCHAR(20) NOT NULL CHECK (account_type IN ('checking', 'savings', 'credit')),
    balance         NUMERIC(14,2) DEFAULT 0 NOT NULL,
    currency        VARCHAR(3) DEFAULT 'USD',
    created_at      TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

CREATE INDEX idx_accounts_customer
    ON accounts(customer_id);

CREATE TABLE transactions (
    transaction_id  SERIAL PRIMARY KEY,
    from_account    INT REFERENCES accounts(account_id),
    to_account      INT REFERENCES accounts(account_id),
    amount          NUMERIC(14,2) NOT NULL CHECK (amount > 0),
    description     TEXT,
    timestamp       TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    status          VARCHAR(20) DEFAULT 'completed'
);

CREATE INDEX idx_transactions_timestamp
    ON transactions(timestamp);

-- These logs are going to be useful for audit purposes

CREATE TABLE audit_logs (
    log_id          SERIAL PRIMARY KEY,
    log_timestamp   TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    operation       VARCHAR(100),
    details         TEXT
);

/* Some sql stored procedures to be used includes

transfer funds from one account to another
sample data which will populate the database
some triggers to add extra security layer */

\echo 'Loading sample data...'
\i database/procedures/sample.sql

\echo 'Loading transfer procedure...'
\i database/procedures/transferMoney.sql

\echo 'Loading triggers...'
\i database/procedures/triggers.sql

-- Reporting tools

CREATE OR REPLACE VIEW daily_transactions_view AS
SELECT
    transaction_id,
    from_account,
    to_account,
    amount,
    description,
    timestamp::date AS day
FROM transactions;

CREATE OR REPLACE VIEW customer_account_summary AS
SELECT
    c.customer_id,
    c.full_name,
    COUNT(a.account_id) AS num_accounts,
    SUM(a.balance) AS total_balance
FROM customers c
LEFT JOIN accounts a ON c.customer_id = a.customer_id
GROUP BY c.customer_id, c.full_name;
