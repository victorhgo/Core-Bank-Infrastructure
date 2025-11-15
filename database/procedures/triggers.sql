/* Here we can define some triggers to add extra layer of 
 * protection to our database */

/* Trigger to prevent direct UPDATE of account balances
 * Only transferMoney() procedure is allowed to modify balances. */

CREATE OR REPLACE FUNCTION prevent_direct_balance_update()
RETURNS trigger AS $$
BEGIN
    -- Reject if balance changes not using transferMoney
    IF TG_OP = 'UPDATE' AND OLD.balance <> NEW.balance THEN
        RAISE EXCEPTION
            'Direct balance updates are not allowed. Use transferMoney() instead.';
    END IF;

    RETURN NEW;
END;
$$ LANGUAGE plpgsql;

CREATE OR REPLACE TRIGGER trg_prevent_direct_balance_update
BEFORE UPDATE OF balance ON accounts
FOR EACH ROW
EXECUTE FUNCTION prevent_direct_balance_update();

/* Transactions immutable (no UPDATE, no DELETE)
 * transactions can be only added on the table, but not updated or modified */

CREATE OR REPLACE FUNCTION prevent_transaction_modifications()
RETURNS trigger AS $$
BEGIN
    RAISE EXCEPTION
        'Transactions cannot be modified or deleted.';
END;
$$ LANGUAGE plpgsql;

CREATE OR REPLACE TRIGGER trg_no_update_transactions
BEFORE UPDATE ON transactions
FOR EACH ROW
EXECUTE FUNCTION prevent_transaction_modifications();

CREATE OR REPLACE TRIGGER trg_no_delete_transactions
BEFORE DELETE ON transactions
FOR EACH ROW
EXECUTE FUNCTION prevent_transaction_modifications();

/* Prevent deletion of an account that still has funds (or debts)
 * An account can only be deleted if its balance is exactly zero */

CREATE OR REPLACE FUNCTION prevent_delete_nonzero_balance()
RETURNS trigger AS $$
BEGIN
    IF OLD.balance <> 0 THEN
        RAISE EXCEPTION
            'Cannot delete account %. Balance must be zero. Current balance: %',
            OLD.account_id, OLD.balance;
    END IF;

    RETURN OLD;
END;
$$ LANGUAGE plpgsql;

CREATE OR REPLACE TRIGGER trg_prevent_delete_nonzero_balance
BEFORE DELETE ON accounts
FOR EACH ROW
EXECUTE FUNCTION prevent_delete_nonzero_balance();