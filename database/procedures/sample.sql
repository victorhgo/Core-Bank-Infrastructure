/* Seeds the database with random generated data 
 * by Victor Correa */

\echo 'Seeding customers...'

INSERT INTO customers (full_name, email, phone, date_of_birth, address)
VALUES
('Alice Johnson',     'alice.j@bank1.com',     '555-1010', '1985-04-10', '123 Maple St'),
('Benjamin Carter',   'ben.c@bank1.com',       '555-2020', '1990-06-25', '42 Oak Road'),
('Clara Mendes',      'clara.m@bank1.com',     '555-3030', '1994-09-12', '77 Pine Avenue'),
('Daniel Thompson',   'dan.t@bank1.com',       '555-4040', '1982-01-18', '88 Elm Street'),
('Eva Martins',       'eva.m@bank1.com',       '555-5050', '1997-03-07', '12 Cedar Blvd'),
('Frank Liu',         'frank.l@bank1.com',     '555-6060', '1988-11-30', '703 Birch Lane'),
('Gloria Smith',      'gloria.s@bank1.com',    '555-7070', '1983-12-06', '901 Cherry Drive'),
('Henry Ford',        'henry.f@bank1.com',     '555-8080', '1991-08-15', '13 Aspen Blvd'),
('Isabella Costa',    'isa.c@bank1.com',       '555-9090', '1996-02-22', '44 Cypress Ave'),
('Jake Williams',     'jake.w@bank1.com',      '555-1111', '1987-07-19', '102 Palm Street'),
('Karen Davis',       'karen.d@bank1.com',     '555-1212', '1984-10-03', '11 Willow Road'),
('Leonardo Pereira',  'leo.p@bank1.com',       '555-1313', '1992-09-28', '800 Redwood St'),
('Maria Lopez',       'maria.l@bank1.com',     '555-1414', '1995-05-17', '9 Sycamore Plaza'),
('Nathan Rogers',     'nate.r@bank1.com',      '555-1515', '1989-03-29', '23 Magnolia Ct'),
('Olivia Turner',     'olivia.t@bank1.com',    '555-1616', '1993-12-11', '76 Poplar Drive'),
('Patrick Kim',       'patrick.k@bank1.com',   '555-1717', '1986-07-08', '55 Hemlock Way'),
('Queenie Zhao',      'queenie.z@bank1.com',   '555-1818', '1998-04-27', '333 Sorrel Lane'),
('Robert Black',      'rob.b@bank1.com',       '555-1919', '1991-11-04', '177 Ironwood Blvd'),
('Sara Fernandez',    'sara.f@bank1.com',      '555-2021', '1984-08-22', '18 Alder Crescent'),
('Thomas Lee',        'thomas.l@bank1.com',    '555-2121', '1990-01-30', '49 Beech Court');

\echo 'Seeding accounts...'

INSERT INTO accounts (customer_id, account_type, balance, currency)
VALUES
-- Alice
(1, 'checking', 1500.00, 'USD'),
(1, 'savings',  4200.25, 'USD'),
-- Benjamin
(2, 'checking', 820.12, 'USD'),
(2, 'savings',  6400.11, 'USD'),
-- Clara
(3, 'checking', 220.89, 'USD'),
(3, 'credit',   -150.34, 'USD'),
-- Daniel
(4, 'checking', 199.99, 'USD'),
(4, 'savings',  5250.44, 'USD'),
-- Eva
(5, 'checking', 3300.00, 'USD'),
(5, 'savings',  900.55, 'USD'),
-- Frank
(6, 'checking', 710.10, 'USD'),
(6, 'credit',   -90.00, 'USD'),
-- Gloria
(7, 'checking', 5400.01, 'USD'),
(7, 'savings',  320.50, 'USD'),
-- Henry
(8, 'checking', 180.65, 'USD'),
-- Isabella
(9, 'checking', 440.75, 'USD'),
(9, 'savings',  1000.00, 'USD'),
-- Jake
(10, 'checking', 670.93, 'USD'),
(10, 'savings',  50.00, 'USD'),

-- Karen
(11, 'checking', 1020.31, 'USD'),
(11, 'savings',  4000.00, 'USD'),
-- Leonardo
(12, 'checking', 600.44, 'USD'),
(12, 'savings',  150.00, 'USD'),
-- Maria
(13, 'checking', 220.90, 'USD'),
(13, 'credit',   -120.00, 'USD'),
-- Nathan
(14, 'checking', 900.00, 'USD'),
-- Olivia
(15, 'checking', 50.00, 'USD'),
(15, 'savings',  780.10, 'USD'),
-- Patrick
(16, 'checking', 5040.99, 'USD'),
-- Queenie
(17, 'checking', 803.33, 'USD'),
(17, 'savings',  204.87, 'USD'),
-- Robert
(18, 'checking', 440.49, 'USD'),
-- Sara
(19, 'checking', 150.00, 'USD'),
-- Thomas
(20, 'checking', 9990.20, 'USD'),
(20, 'savings',  300.13, 'USD');

\echo 'Seeding transactions...'

INSERT INTO transactions (from_account, to_account, amount, description)
VALUES
-- Random but "realist" transfers with description
(1, 3, 50.00, 'Dinner split'),
(2, 1, 120.00, 'Loan repayment'),
(5, 8, 200.00, 'Gift'),
(7, 1, 400.00, 'Refund'),
(10, 12, 60.00, 'Payment'),
(11, 9, 150.00, 'Transfer'),
(13, 6, 30.00, 'Donation'),
(14, 17, 90.00, 'Transfer'),
(18, 19, 45.22, 'Food delivery'),
(20, 8, 500.00, 'Invoice settlement'),

-- Add 110 misc random transactions
(3, 4, 20.00, 'Misc'),
(4, 2, 18.50, 'Misc'),
(6, 7, 11.11, 'Misc'),
(8, 5, 10.00, 'Misc'),
(17, 16, 22.00, 'Misc'),
(12, 3, 7.70, 'Misc'),
(9, 14, 33.90, 'Misc'),
(15, 20, 25.00, 'Misc'),
(10, 13, 14.44, 'Misc'),
(19, 11, 55.00, 'Misc');

\echo 'Sample data loaded successfully.';
