/* Function to safely transfers money between two accounts.
 * transfer_funds(from_account_id, to_account_id, amount, description) */

CREATE OR REPLACE FUNCTION transfer_funds(
    transf_from_account INT,
    transf_to_account   INT,
    transf_amount       NUMERIC(12,2),
    transf_description  TEXT DEFAULT 'Transfer'
)

/* Start working on the function definition */