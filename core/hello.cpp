#include <iostream>
#include <pqxx/pqxx>
 
int main() {
    try {
        // Connect to the database.  In practice we may have to pass some
        // arguments to say where the database server is, and so on.
        // The constructor parses options exactly like libpq's
        // PQconnectdb/PQconnect, see:
        // https://www.postgresql.org/docs/10/static/libpq-connect.html
        pqxx::connection cx;
    
        // Start a transaction.  In libpqxx, you always work in one.
        pqxx::work tx(cx);
    
        // work::exec1() executes a query returning a single row of data.
        // We'll just ask the database to return the number 1 to us.

        /* The following is deprecated:

        pqxx::row r = tx.exec1("SELECT 1");
        
            warning: ‘pqxx::row pqxx::transaction_base::exec1(pqxx::zview)’ 
            is deprecated: Use exec() instead, and call one_row() on the result.
            
            To fix it: */
        
        pqxx::result res = tx.exec("SELECT 1");
        pqxx::row r = res.one_row();

        //pqxx::row res = tx.exec1("SELECT 1");
        //pqxx::row r = res.one_row();   // throws if not exactly 1 row
    
        // Commit your transaction.  If an exception occurred before this
        // point, execution will have left the block, and the transaction will
        // have been destroyed along the way.  In that case, the failed
        // transaction would implicitly abort instead of getting to this point.
        tx.commit();
    
        // Look at the first and only field in the row, parse it as an integer,
        // and print it.
        //
        // "r[0]" returns the first field, which has an "as<...>()" member
        // function template to convert its contents from their string format
        // to a type of your choice.
        std::cout << r[0].as<int>() << " - Hello PostgreSQL :)" << std::endl;
    }
    catch (std::exception const &e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }
}