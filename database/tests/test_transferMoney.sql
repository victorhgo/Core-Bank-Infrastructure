/* Unit tests for transferMoney() function
 * by Victor Correa */

-- Load pgTAP if not already loaded on our database
CREATE EXTENSION IF NOT EXISTS pgtap;

-- Start the test set
BEGIN;

SELECT plan(10);

-- Create test schema test envirnoment
CREATE SCHEMA IF NOT EXISTS test_env;

SET search_path TO test_env, public;

-- Testdatabase table definitions
CREATE TABLE accounts (
    account_id SERIAL PRIMARY KEY,
    balance NUMERIC(12,2) NOT NULL,
    currency TEXT NOT NULL
);

CREATE TABLE transactions (
    id SERIAL PRIMARY KEY,
    from_account INT,
    to_account INT,
    amount NUMERIC(12,2),
    description TEXT,
    created_at TIMESTAMP DEFAULT NOW()
);

-- Load the function transferMoney under test
\i database/procedures/transferMoney.sql

-- Seed some sample data to be tested
INSERT INTO accounts (balance, currency) VALUES
/* Account ID 1, 2 and 3 and 3 set with a different currency */
    (100.00, 'USD'),
    (50.00,  'USD'),
    (10.00,  'EUR');

/* Test definitions */

-- Test 1: Test a Successful transfer

SELECT lives_ok(
    $$ SELECT transferMoney(1, 2, 30.00, 'Test transfer'); $$,
    'Transfer between accounts 1 and 2 should succeed'
);
 
-- Tests 2 and 3: Assert correct balances
 
SELECT is(
    (SELECT balance FROM accounts WHERE account_id = 1),
    70.00::numeric,
    'Account 1 should have 70.00'
);

SELECT is(
    (SELECT balance FROM accounts WHERE account_id = 2),
    80.00::numeric,
    'Account 2 should have 80.00'
);
 
-- Test 4: Check if Insufficient funds error is working properly
 
SELECT throws_ok(
    $$ SELECT transferMoney(2, 1, 999.00, 'Too big'); $$,
    'Insufficient funds in account 2, balance: 80.00, attempted: 999.00',
    'Should throw on insufficient balance'
);

-- Test 5: Check if balance remains unchanged after a fail transaction
 
SELECT is(
    (SELECT balance FROM accounts WHERE account_id = 2),
    80.00::numeric,
    'Account 2 balance unchanged after failed transfer'
);
 
-- Test 6: Try to make a negative transfer and check the error
 
SELECT throws_ok(
    $$ SELECT transferMoney(1, 2, -5.00, 'Bad'); $$,
    'Transfer amount must be positive',
    'Should throw on negative transfer amount'
);
 
-- Test 7: Different currency, just to assure it (but won't be used)
 
SELECT throws_ok(
    $$ SELECT transferMoney(1, 3, 10.00, 'Wrong currency'); $$,
    'Currency mismatch: USD vs EUR',
    'Should throw when accounts have different currencies'
);

-- Test 8: If the transaction already exists
 
SELECT isnt_empty(
    $$ SELECT * FROM transactions WHERE amount = 30 AND description = 'Test transfer' $$,
    'Transaction should be recorded'
);

-- Test 9: Check if the source account exists
 
SELECT throws_ok(
    $$ SELECT transferMoney(999, 1, 10, 'Invalid'); $$,
    'Source account 999 does not exist',
    'Should throw on non-existing source account'
);

-- Test 10: Checks if the destination account exists
 
SELECT throws_ok(
    $$ SELECT transferMoney(1, 999, 10, 'Invalid'); $$,
    'Destination account 999 does not exist',
    'Should throw on non-existing destination account'
);

/* Finish test */
SELECT * FROM finish();

/* Rolls back all changes made during test */
ROLLBACK;
