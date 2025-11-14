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

- [References and Tutorials](#references-and-tutorials)

    - [PostgreSQL manual, references and tutorials](#postgresql-manual-references-and-tutorials)

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

Let's create a procedure called `moneyTransfer.sql` to safely transfer money between accounts. A function that transfers money between two accounts, in simplest form possible, needs the source account (where the money comes from) and a target account (where the money goes to), it also needs the amount of money being transfered and an optional description for the receiving account. We can begin creating a function with `CREATE FUNCTION function_name` SQL command (**Note:** Adding the `OR REPLACE` command to either CREATE or REPLACE an existing function with the same name)

```sql
CREATE OR REPLACE FUNCTION transfer_funds(
    transf_from_account INT,
    transf_to_account   INT,
    transf_amount       NUMERIC(12,2),
    transf_description  TEXT DEFAULT 'Transfer'
)
```

## References and Tutorials

All the materials consulted for building up this project include documentations, posts, foruns, blogs and videos. Some of them are:

### PostgreSQL manual, references and tutorials

[PostgreSQL main page](https://www.postgresql.org/)

[Basic SQL syntax and other fundamental operations](https://neon.com/postgresql/tutorial)

[`initdb` - Creating a new PostgreSQL database cluster](https://www.postgresql.org/docs/current/app-initdb.html)

[Stored Procedures](https://www.geeksforgeeks.org/postgresql/postgresql-introduction-to-stored-procedures/)