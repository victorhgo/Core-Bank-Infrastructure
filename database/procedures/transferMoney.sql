/* Function to safely transfers money between two accounts.
 * transferMoney(from_account_id, to_account_id, amount, description) */

CREATE OR REPLACE FUNCTION transferMoney(
    transf_from_account INT,
    transf_to_account   INT,
    transf_amount       NUMERIC(12,2),
    transf_description  TEXT DEFAULT 'Transfer'
)
RETURNS VOID AS $$
DECLARE
    ver_from_balance   NUMERIC;
    ver_from_currency  TEXT;
    ver_to_currency    TEXT;
BEGIN
    -- Validate the amount
    IF transf_amount <= 0 THEN
        RAISE EXCEPTION 'Transfer amount must be positive';
    END IF;

    -- Lock source account row
    SELECT balance, currency
    INTO ver_from_balance, ver_from_currency
    FROM accounts
    WHERE account_id = transf_from_account
    FOR UPDATE;

    -- Lock destination account row
    SELECT currency
    INTO ver_to_currency
    FROM accounts
    WHERE account_id = transf_to_account
    FOR UPDATE;

    -- Verify if accounts exist
    IF ver_from_currency IS NULL THEN
        RAISE EXCEPTION 'Source account % does not exist', transf_from_account;
    END IF;

    IF ver_to_currency IS NULL THEN
        RAISE EXCEPTION 'Destination account % does not exist', transf_to_account;
    END IF;

    -- Same currency check
    IF ver_from_currency <> ver_to_currency THEN
        RAISE EXCEPTION 'Currency mismatch: % vs %', ver_from_currency, ver_to_currency;
    END IF;

    -- Sufficient funds
    IF ver_from_balance < transf_amount THEN
        RAISE EXCEPTION
            'Insufficient funds in account %, balance: %, attempted: %',
            transf_from_account, ver_from_balance, transf_amount;
    END IF;

    BEGIN
        -- Allow balance updates for this function
        PERFORM set_config('database1.allow_balance_update', 'on', true);

        -- Apply balance updates
        UPDATE accounts
        SET balance = balance - transf_amount
        WHERE account_id = transf_from_account;

        UPDATE accounts
        SET balance = balance + transf_amount
        WHERE account_id = transf_to_account;

        -- Record transaction
        INSERT INTO transactions (from_account, to_account, amount, description)
        VALUES (transf_from_account, transf_to_account, transf_amount, transf_description);

        -- Optionally turn the flag off again
        PERFORM set_config('database1.allow_balance_update', 'off', true);
    EXCEPTION WHEN OTHERS THEN
        -- Make sure we clear the flag even if something fails
        PERFORM set_config('database1.allow_balance_update', 'off', true);
        RAISE;
    END;
END;
$$ LANGUAGE plpgsql;