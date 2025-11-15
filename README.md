# Core Bank Infrastructure

by Victor Correa

This is an educational project developed with the main goal of providing a practical environment for learning and applying key concepts related to relational database design such as implementation, maintenance within the context of a simplified banking infrastructure system. Its targeted to be a complete, reproducible study environment where different layers of a financial application can be implemented and tested.

- [Motivation](#motivation)
- [Database](#database)

    - [Installing and Initializing PostgreSQL on ArchLinux](#installing-and-initializing-postgresql-on-archlinux)
    - [Running PostgreSQL for the first time](#running-postgresql-for-the-first-time)
    - [Creating a new database and defining tables on SQL](#creating-a-new-database-and-defining-tables-on-sql)
    - [Populating the database tables with data](#populating-the-database-tables-with-data)
    - [Defining Stored Procedures for database](#defining-stored-procedures-for-database)
    - [Unit Tests](#unit-tests)
    - [SQL Triggers](#sql-triggers)

- [Data Access Layer](#data-access-layer-dal)

- [References and Tutorials](#references-and-tutorials)

## Motivation

Since the beginning of my career as a System Administrator, I have been deeply interested in understanding how large and complex infrastructures such as a core bank operates "under the hood". But working on these environments with high security and operational restrictions we have a very limited or no visibility at all into production databases, also a restricted access to stored procedure such as maintenance routine and reporting scripts, and the opportunity to modify or analyze systems at a foundational level is almost non-existant.

Specially when it comes to banks, many critical components such as scripts, integration points, or business rules are already well established, and modifying them is an almost impossible process. This makes it quite challenging to clearly understand why things were build the way they were, how individual components interact with each other (for instance, how the Finance Team gets a report from all the transactions executed on a day from the database perspective?) or what the complete data flow looks like (what happens when a customer sends money to another one? What scripts and routines are involved in this process?). The architecture looks really "blurry" from the perspective of a beginning system administrator, leaving important details and questions burried.

So I came up with this project idea, where I can build up a simple but working core infrastructure from scratch, in a controlled and unrestricted environment. Where I will be able to explore the database architecture with no restrictions, experiment some ideas and see what happens. I believe this will help me obtain a deeper understanding of how these applications work under the hood, integrating multiple languages and methods on a "realistic scenario".

Therefore, this project is a learning platform and a personal “laboratory” for developing practical knowledge in system architecture, database development and backend service design.

## Database

For this topic we'll need to learn relational modeling and SQL fundamentals. Also it's important to learn how to design a initial database schema and understand overall system architecture of a bank.

### Installing and Initializing PostgreSQL on ArchLinux

Here are the steps I've used to install and initialize PostgreSQL on Arch Linux:

- Install the latest PostgreSQL package

`sudo pacman -S postgresql`

- After installation, version can be checked with

`postgres --version`

- To confirm psql is not running (it should not be running after installation)

`sudo systemctl status postgresql`

- Login as the postgres user (always do this when doing admin tasks on psql)

`sudo su - postgres`

- Initializing data directory (default db: /var/lib/postgres/data)

`sudo -u postgres initdb --locale en_GB.UTF-8 -D /var/lib/postgres/data`

- Logout of **postgres** user

`exit`

- Confirm if psql is still not running

`sudo systemctl status postgresql`

- Start psql

`sudo systemctl start postgresql`

- Confirm psql is running (now it should be running)
`sudo systemctl status postgresql`

To create a new user on PostgreSQL:

- Login into postgres

`sudo su - postgres`

- Create a new user (**Note:** user can be called anything however if you create a PostgreSQL user with the same name as your Linux username, it allows you to access the PostgreSQL database shell without having to specify a user to login (which makes it quite convenient).)

`createuser --interactive`

- Enter name of role to add: `MY_LINUX_USERNAME`

    - Shall the new role be a superuser?: `y`

- Logout of **postgres** user

`exit`

- Restart psql

`sudo systemctl restart postgresql`

- Confirm psql is running (can check that it was restarted by looking at the timestamp on the ***Active*** field)

`sudo systemctl status postgresql`

> Default db for psql is /var/lib/postgres/data
> `sudo -u postgres initdb --locale en_GB.UTF-8 -D /var/lib/postgres/data`


### Running PostgreSQL for the first time

```bash
➜  ~ sudo -u postgres initdb --locale en_GB.UTF-8 -D /var/lib/postgres/data
The files belonging to this database system will be owned by user "postgres".
This user must also own the server process.

The database cluster will be initialized with locale "en_GB.UTF-8".
The default database encoding has accordingly been set to "UTF8".
The default text search configuration will be set to "english".

Data page checksums are enabled.

fixing permissions on existing directory /var/lib/postgres/data ... ok
creating subdirectories ... ok
selecting dynamic shared memory implementation ... posix
selecting default "max_connections" ... 100
selecting default "shared_buffers" ... 128MB
selecting default time zone ... Europe/**********
creating configuration files ... ok
running bootstrap script ... ok
performing post-bootstrap initialization ... ok
syncing data to disk ... ok

initdb: warning: enabling "trust" authentication for local connections
initdb: hint: You can change this by editing pg_hba.conf or using the option -A, or --auth-local and --auth-host, the next time you run initdb.

Success. You can now start the database server using:

    pg_ctl -D /var/lib/postgres/data -l logfile start
# Start PostgreSQL
➜  ~ sudo systemctl start postgresql
# Enable PostgreSQL
➜  ~ sudo systemctl enable postgresql
Created symlink '/etc/systemd/system/multi-user.target.wants/postgresql.service' → '/usr/lib/systemd/system/postgresql.service'.
```

### Creating a new database and defining tables on SQL

First we being with creating the database cluster with `CREATE DATABASE` SQL command, after the database is successfully created, we can begin populating the information on it. First we need to create tables which will store variables related to it. For instance, let's create a table `customers` where we store crucial data regarding the customers such as full name (first and last on separated columns), email, address, birthdate and etc. With a single SQL command we can do all of it:

```sql
CREATE TABLE customers (
    customer_id SERIAL PRIMARY KEY,
    name VARCHAR(100),
    email VARCHAR(120),
    date_of_birth DATE,
    address TEXT
);
```

We should also define a table that stores bank `accounts` info, which will relate an account_number to a `customer_id`. We want our accounts table to store information such as account namber, type, balance, associate it with an owner (customer id) referenced to the `customer` tables. This can be achieved with `REFERENCES` command in the following way:

```sql
CREATE TABLE accounts (
    account_id SERIAL PRIMARY KEY,
    customer_id INTEGER REFERENCES customers(customer_id),
    account_type VARCHAR(20),
    balance NUMERIC(12,2),
    created_at TIMESTAMP DEFAULT NOW()
);
```

For the transactions, we want to reference the account where the money left, to the account it went to. We also want to store information about the transaction itself, such as `transfer_id` which can be referenced to collect information about a specific transaction. The amount and timestamp will store the amount of money and the time the transaction happened.

```sql
CREATE TABLE transactions (
    transaction_id SERIAL PRIMARY KEY,
    from_account INTEGER REFERENCES accounts(account_id),
    to_account INTEGER REFERENCES accounts(account_id),
    amount NUMERIC(10,2),
    timestamp TIMESTAMP DEFAULT NOW()
);
```

### Populating the database tables with data

Now the database is populated with tables, but it's empty. We can begin populating its tables with the `INSERT` command. In this example, let's add a customer called `John Doe` and relate him to the account number `0001`, which will have an initial deposit of `1500$`

```sql
INSERT INTO customers (customer_id, name, email, date_of_birth, address)
VALUES (1, 'John Doe', 'jond@mail.com', '1985-06-12', 'Sample Street 42');

INSERT INTO accounts (account_id, customer_id, account_type, balance)
VALUES (0001, 1, 'checking', 1500.00);
```

To run this SQL script and create the database, tables and insert `John Doe`, we simply run:

```bash
$ psql -U postgres -f database/initdb.sql

# The ouput

CREATE DATABASE
You are now connected to database "StadtBankDB" as user "postgres".
CREATE TABLE
CREATE TABLE
CREATE TABLE
INSERT 0 1
INSERT 0 1
```

Now we can enter our database and check how it looks: the structure, the data we just added in and do some queries on the tables. First we enter the database with `postgres` user:

```bash
$ psql -d StadtBankDB
```

A display `\d` will show all the tables on our database, in which we can do a simple query on the customers table to check the entered information with `SELECT * FROM customers` and `accounts`:

```bash
StadtBankDB=# \d
                       List of relations
 Schema |              Name               |   Type   |  Owner
--------+---------------------------------+----------+----------
 public | accounts                        | table    | postgres
 public | accounts_account_id_seq         | sequence | postgres
 public | customers                       | table    | postgres
 public | customers_customer_id_seq       | sequence | postgres
 public | transactions                    | table    | postgres
 public | transactions_transaction_id_seq | sequence | postgres
(6 rows)

StadtBankDB=# SELECT * FROM customers;
 customer_id |   name   |     email     | date_of_birth |     address
-------------+----------+---------------+---------------+------------------
           1 | John Doe | jond@mail.com | 1985-06-12    | Sample Street 42
(1 row)

StadtBankDB=# SELECT * FROM accounts;
 account_id | customer_id | account_type | balance |         created_at
------------+-------------+--------------+---------+----------------------------
          1 |           1 | checking     | 1500.00 | 2025-11-14 10:25:29.161791
(1 row)

# From here we can see the transaction table is empty, because no transaction has been made yet:

StadtBankDB=# SELECT * FROM transactions;
 transaction_id | from_account | to_account | amount | timestamp
----------------+--------------+------------+--------+-----------
(0 rows)
```

But as the project grows up, we'll need to have more data on each table, and also find a more efficient way of adding up customers on the database. For now, I will rely on [this seed](/database/procedures/sample.sql) to populate my database with some customer data. To run this stored procedure:

```sh
$ psql -U postgres -d database1 -f database/procedures/sample.sql

Seeding customers...
INSERT 0 20
Seeding accounts...
INSERT 0 35
Seeding transactions...
INSERT 0 20
Sample data loaded successfully.;
```

### Defining Stored Procedures for database

For dealing with transactions (transfer money), adding new customers we can rely on Stored Procedures, which are prepared SQL code that can be saved. The code can be reused over and over again. So instead of having an SQL query that we must write over and over again, we can simply save it as a stored procedure, then just call it to be executed when needed.

Let's create a procedure called `moneyTransfer.sql` to safely transfer money between accounts. A function that transfers money between two accounts, in simplest form possible, needs the source account (where the money comes from) and a target account (where the money goes to), it also needs the amount of money being transfered and an optional description for the receiving account. We can begin creating a function with `CREATE FUNCTION function_name` SQL command (**Note:** Adding the `OR REPLACE` command to either CREATE or REPLACE an existing function with the same name). For more information about creating functions in PostgreSQL [click here](https://www.postgresql.org/docs/current/sql-createfunction.html). This function receives the following as parameters:

```sql
CREATE OR REPLACE FUNCTION transferMoney(
    /* from account - source account */
    transf_from_account INT,
    /* destination account */
    transf_to_account   INT,
    /* Amount to be transfered */
    transf_amount       NUMERIC(12,2),
    /* Optional Details on the transfer */
    transf_description  TEXT DEFAULT 'Transfer'
)
```

We know this function will work in the following way:

1. First it must check if the transfer amount entered is valid (greater than zero). A simple check can be done by:

```sql
-- Validate the amount set up by transaction
IF transf_amount <= 0 THEN
    RAISE EXCEPTION 'Transfer amount must be positive';
END IF;
```

2. It must lock both accounts in a way that no other transaction can modify these accounts while the transfer is running, because it could led to duplicated transfers, or the money being received for the destination account but never leaving the source one, and vice versa.

```sql
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
```

3. It must check if the accounts exist, otherwise it can led to unpredictable behaviors such as the money leaving the source account and arriving nowhere, or vice versa.

```sql
IF v_from_currency IS NULL THEN
    RAISE EXCEPTION 'Source account % does not exist', p_from_account;
END IF;
```

4. Checks if the amount is valid, if the source account has sufficent funds for transfer.

```sql
IF ver_from_balance < transf_amount THEN
    RAISE EXCEPTION
        'Insufficient funds in account %, balance: %, attempted: %',
        transf_from_account, ver_from_balance, transf_amount;
END IF;
```

5. Update the balances for each account simultaneously to ensure that if an error happens after the first update, PostgreSQL will automatically roll back all changes, preventing partial transfers for instance.

```sql
UPDATE accounts
SET balance = balance - transf_amount
WHERE account_id = transf_from_account;

UPDATE accounts
SET balance = balance + transf_amount
WHERE account_id = transf_to_account;
```

6. Record the transaction for audit records such as: tracing the money, financial reports and etc.

```sql
INSERT INTO transactions (from_account, to_account, amount, description)
VALUES (transf_from_account, transf_to_account, transf_amount, transf_description);
```

The function is deployed at [`transferMoney)`](/database/procedures/transferMoney.sql). Now that we wrote our SQL script to transfer money, we can add this stored procedure for the database with

```bash
psql -U postgres -d database1 -f database/procedures/transferMoney.sql
```

And calling it inside the database to transfer money from one account that has insufficient funds:

```bash
database1=# SELECT transferMoney(6, 12, 30.00, 'Fridge repair service');

ERROR:  Insufficient funds in account 6, balance: 9.66, attempted: 30.00
CONTEXT:  PL/pgSQL function transfermoney(integer,integer,numeric,text) line 45 at RAISE
```

But how can we be sure this function is working as expected before we can safely deploy it to the Production Environment?

### Unit Tests

We can perform unit tests to ensure that a function is working as expected, testing every possible case with different scenarios and inputs to see its behaviour and validate if it's ready to be deployed on production or not.

**Note that** unit testing is an important subject that won't be developed further in this project documentation. But a small one will be deployed for this function using pgTAP (Test Anything Protocol), which is the standard framework for PostgreSQL unit tests. [Click here](https://pgtap.org/documentation.html) for the documentation.

I'll be following the same principles learned from MIT 6.0001 Introduction to Computer Science on these tests, testing every scenario for my function.

We need to enable pgTAP extension on our database with `CREATE EXTENSION pgtap;`.

After adding the test to [test_transferMoney.sql](/database/tests/test_transferMoney.sql) we can run this test on our database with the following command:

```sh
$ psql -U postgres -d database1 -f tests/test_transferMoney.sql

# --- Results of running test on DB ---
    ok 1 - Transfer between accounts 1 and 2 should succeed
    ok 2 - Account 1 should have 70.00
    ok 3 - Account 2 should have 80.00
    ok 4 - Should throw on insufficient balance
    ok 5 - Account 2 balance unchanged after failed transfer
    ok 6 - Should throw on negative transfer amount
    ok 7 - Should throw when accounts have different currencies
    ok 8 - Transaction should be recorded
    ok 9 - Should throw on non-existing source account
    ok 10 - Should throw on non-existing destination account
```

In the simplest way possible, these test's results indicate that our function is ready to be deployed on a production database. Of course on a real scenario the tests are not as simple as the one we ran, but it's a good way to see how a unit test might look like on a system.

### SQL Triggers

SQL triggers are stored procedures that automatically execute in response to certain events (that can trigger it) in a database. As they add an extra layer of protection to our database, they can be used to enforce business rules, security on the database by preventing illegal changes (preventing direct changes on a user balance for instance).

We want to implement triggers for the bank that ensures only stored procedures can change data on the database (changing money, changing user definitions), also noo one can rewrite transaction history to keep the data integrity.

The syntax rule for defining triggers is pretty straightforward ([click here](https://www.postgresql.org/docs/current/sql-createtrigger.html) for more details on writting them), let's implement a trigger to prevent updates of account balance, only `transferMoney()` function is allowed to do it.

```sql
CREATE OR REPLACE FUNCTION prevent_direct_balance_update()
RETURNS trigger AS $$
BEGIN
    -- Reject if balance changes outside allowed paths
    IF TG_OP = 'UPDATE' AND OLD.balance <> NEW.balance THEN
        RAISE EXCEPTION
            'Direct balance updates are not allowed. Use transferMoney() instead.';
    END IF;

    RETURN NEW;
END;
$$ LANGUAGE plpgsql;

CREATE TRIGGER trg_prevent_direct_balance_update
BEFORE UPDATE OF balance ON accounts
FOR EACH ROW
EXECUTE FUNCTION prevent_direct_balance_update();
```

After loading the `triggers.sql` on our database and trying to directly modify an account value:

```bash
database1=# UPDATE accounts
database1-# SET balance = 50.00
database1-# WHERE account_id = 5;
ERROR:  Direct balance updates are not allowed. Use transferMoney() instead.
CONTEXT:  PL/pgSQL function prevent_direct_balance_update() line 5 at RAISE
```

**Triggers for protecting transactions of being changed**

Another very important trigger that needs to be set up is to protect transaction table, they must be immutable. Only new transactions can be added up, but existing ones cannot be `UPDATE` or `DELETE`. We can achieve this by setting up a trigger on both commands such as:


```sql
CREATE OR REPLACE FUNCTION prevent_transaction_modifications()
RETURNS trigger AS $$
BEGIN
    RAISE EXCEPTION
        'Transactions cannot be modified or deleted!';
END;
$$ LANGUAGE plpgsql;

CREATE TRIGGER trg_no_update_transactions
BEFORE UPDATE ON transactions
FOR EACH ROW
EXECUTE FUNCTION prevent_transaction_modifications();

CREATE TRIGGER trg_no_delete_transactions
BEFORE DELETE ON transactions
FOR EACH ROW
EXECUTE FUNCTION prevent_transaction_modifications();
```

When trying to delete or modify the `transaction` table, an error is raised:

```sh
database1=# DELETE FROM transactions WHERE transaction_id = 23;
ERROR:  Transactions cannot be modified or deleted. They are immutable.
CONTEXT:  PL/pgSQL function prevent_transaction_modifications() line 3 at RAISE
```

Some other triggers can be set up to deal with deletion of accounts that have pending transactions for instance, which will prevent an account to be deleted when unfinished transaction is associated to it, or deleting an account in which the balance is different of zero.

Now that our database is functional from a study perspective, we can step up a layer from the low level data storage and start thinking about designing a way for users to perform CRUD (**C**reate, **R**ead, **U**pdate and **D**elete) operations on the database. On the next topic we can start thinking about the Data Access Layer.

## Data Access Layer (DAL)

Data Access Layer (or DAL) is a layer of a computer program which provides simplified access to database. So instead of using commands such as `INSERT`, `DELETE`, and `UPDATE` directly into PostgreSQL, we can set up a class and a few stored procedures into the database. These procedures will be called from a method inside the class, and this class will return an object containing the requested values. This will allow the client modules (or user) to be created with a higher level of abstraction.

And of course, we are talking about classes, which basically means `Oriented Object Programming`. Here most enterprises would decide to implement their Data Access Layer in Java due to its platform independence and extensive ecosystem of open-source frameworks (which are a set of libraries and/or tools that provide a structure for building applications) that simplify database interactions. 

## References and Tutorials

All the materials consulted for building up this project include documentations, posts, foruns, blogs and videos. Some of them are:

**PostgreSQL manual, references and tutorials**

- [PostgreSQL main page](https://www.postgresql.org/)

- [Basic SQL syntax and other fundamental operations](https://neon.com/postgresql/tutorial)

- [`initdb` - Creating a new PostgreSQL database cluster](https://www.postgresql.org/docs/current/app-initdb.html)

- [Stored Procedures](https://www.geeksforgeeks.org/postgresql/postgresql-introduction-to-stored-procedures/)

- [pgTAB - PostgreSQL unit testing framework](https://pgtap.org/documentation.html)

- [SQL Triggers](https://www.datacamp.com/tutorial/sql-triggers)

**Data Access Layer**

- [What is Data Access Layer](https://en.wikipedia.org/wiki/Data_access_layer)

- A helpful article on writting Data Access Layer on Medium:

- [Part 1](https://medium.com/swlh/designing-a-data-access-layer-part-1-f10068408e60)

- [Part 2](https://medium.com/swlh/designing-a-data-access-layer-part-2-3c9fa905f1ed)

- [Part 3](https://medium.com/swlh/designing-data-access-layer-part-3-ffe0f17198e6)

