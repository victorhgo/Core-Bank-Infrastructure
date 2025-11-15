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
    ver_from_balance NUMERIC;
    ver_from_currency TEXT;
    ver_to_currency TEXT;
BEGIN
    -- Validate the amount set up by transaction
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

    /* Verify if they're using the same currency
    Note: This step is really not relevant for this project because we'll be working only with USD.
    But as seen on application, this is a very important step.*/

    IF ver_from_currency <> ver_to_currency THEN
        RAISE EXCEPTION 'Currency mismatch: % vs %', ver_from_currency, ver_to_currency;
    END IF;

    -- Verify if the sending account balance is sufficient for transaction
    IF ver_from_balance < transf_amount THEN
        RAISE EXCEPTION
            'Insufficient funds in account %, balance: %, attempted: %',
            transf_from_account, ver_from_balance, transf_amount;
    END IF;

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

END;
$$ LANGUAGE plpgsql;
