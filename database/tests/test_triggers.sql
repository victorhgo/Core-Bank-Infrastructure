/* Unit tests for triggers
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
\i database/procedures/triggers.sql

-- Seed some sample data to be tested
INSERT INTO accounts (balance, currency) VALUES
    /* Account 1: deletable, account 2: not deletable */
    (100.00, 'USD'),
    (0.00,   'USD'); 

INSERT INTO transactions (from_account, to_account, amount, description)
VALUES (1, 2, 25.00, 'Test TX');

/* Test definitions */

-- Test 1: tests trg_prevent_direct_balance_update

SELECT throws_ok(
    $$ UPDATE accounts SET balance = 999.00 WHERE account_id = 1; $$,
    'Direct balance updates are not allowed. Use transferMoney() instead.',
    'Direct balance update should be blocked'
);

-- Test 2: Only non related to balance fields can be updated

SELECT lives_ok(
    $$ UPDATE accounts SET currency = 'USD' WHERE account_id = 1; $$,
    'Updating non-balance fields should be allowed'
);
 
-- Test 3: trg_no_update_transactions: must block updates

SELECT throws_ok(
    $$ UPDATE transactions SET amount = 999.00 WHERE id = 1; $$,
    'Transactions cannot be modified or deleted.',
    'Updating transaction should be blocked'
);
 
-- Test 4: test trg_no_delete_transactions, no transactions can be deleted

SELECT throws_ok(
    $$ DELETE FROM transactions WHERE id = 1; $$,
    'Transactions cannot be modified or deleted.',
    'Deleting transaction should be blocked'
);

-- Test 5: Trying to change a transaction
SELECT is(
    (SELECT amount FROM transactions WHERE id = 1),
    25.00::numeric,
    'Transactions cannot be modified'
);

-- Test 6: tests trg_prevent_delete_nonzero_balance

SELECT throws_ok(
    $$ DELETE FROM accounts WHERE account_id = 1; $$,
    'Cannot delete account 1. Balance must be zero. Current balance: 100.00',
    'Should not allow deleting account with nonzero balance'
);

-- Test 7: Deleting a zero-balance account is allowed

SELECT lives_ok(
    $$ DELETE FROM accounts WHERE account_id = 2; $$,
    'Deleting zero-balance account should succeed'
);

-- Test 8: Account 2 is actually deleted
 
SELECT is(
    (SELECT COUNT(*) FROM accounts WHERE account_id = 2),
    0::bigint,
    'Zero-balance account should be successfully deleted'
);

-- Test 9: verify balance unchanged after blocked delete
 
SELECT is(
    (SELECT balance FROM accounts WHERE account_id = 1),
    100.00::numeric,
    'Balance remains unchanged after blocked delete operation'
);
 
-- Test 10: ensure delete protection does not block valid ops

SELECT lives_ok(
    $$ UPDATE accounts SET currency = 'EUR' WHERE account_id = 1; $$,
    'Safe updates still allowed when triggers exist'
);

/* Finish test */
SELECT * FROM finish();

/* Rolls back all changes made during test */
ROLLBACK;