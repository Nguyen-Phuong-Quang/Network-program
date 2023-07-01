#include <QSqlDatabase>
#include <QSqlQuery>
#include <QDebug>
#include <QSqlRecord>

int main()
{
    // Register the PostgreSQL driver
    QSqlDatabase db = QSqlDatabase::addDatabase("QPSQL");

    // Set the connection parameters
    db.setHostName("chat-app.c6aoubm3unwy.us-east-1.rds.amazonaws.com");
    db.setPort(5432); // Default PostgreSQL port
    db.setDatabaseName("test");
    db.setUserName("postgres");
    db.setPassword("Quang251209");

    // Attempt to open the database connection
    if (db.open()) {
        qDebug() << "Connected to PostgreSQL database!";
        // Perform database operations here
    } else {
        qDebug() << "Error connecting to PostgreSQL database" ;
    }

    QSqlQuery query;

    // Prepare the SQL query
    QString queryString = "SELECT * FROM product";
    query.prepare(queryString);

    // Execute the query
    if (query.exec()) {
        // Iterate over the retrieved records
        while (query.next()) {
            // Access individual fields of the record
            int productId = query.value("id").toInt();
            QString productName = query.value("name").toString();
            // Access other fields as needed

            // Print the retrieved record to the console or perform other operations
            qDebug() << "Product ID:" << productId << "Product Name:" << productName;
        }
    } else {
        qDebug() << "Error executing query:" << query.last();
        return 1; // Return an error code or handle the error appropriately
    }
    return 0;
}
