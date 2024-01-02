#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <sqlite3.h>
#include <sql_bindings.h>
#include <stdexcept>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <cctype>
#include <vector>

namespace rn = std::ranges;

std::vector<sqlite3_stmt*> load_sql_script(sqlite3 *db, const char *path);
void execute_stmt(sqlite3 *db, sqlite3_stmt *stmt);

int main(int argc, char **argv) {

    int rc;
    char *errmsg;
    const char *db_name = ":memory:";
    const char *script_path;

    if(argc <= 1) {
        fprintf(stderr, "Usage: %s <sql file>\n", *argv);
        return EXIT_FAILURE;
    }
    script_path = argv[1];
    
    sqlite3 *db;
    rc = sqlite3_open_v2(db_name, &db, SQLITE_OPEN_READWRITE|SQLITE_OPEN_CREATE, nullptr);
    if(rc != 0) throw std::runtime_error("Failed to open database");

    sqhell::init_sql_bindings(db);

    auto script = load_sql_script(db, script_path);

    while(true)
    for(auto stmt : script)
    execute_stmt(db, stmt);
}

void execute_stmt(sqlite3 *db, sqlite3_stmt *stmt) {
    //fprintf(stderr, "EXECUTING: %s\n", sqlite3_sql(stmt));
    while(true) {
        int rc = sqlite3_step(stmt);
        if(rc == SQLITE_ROW) continue;
        if(rc == SQLITE_DONE) {
            sqlite3_reset(stmt);
            break;
        }
        fprintf(stderr, "ERROR: %d %s\n", rc, sqlite3_errmsg(db));
        exit(EXIT_FAILURE);
    }
}

std::vector<sqlite3_stmt*> load_sql_script(sqlite3 *db, const char *path) {
    
    std::ifstream fin(path);

    fin.seekg(0, std::ios::end);
    auto length = fin.tellg();
    char *script = new char[((size_t)length)+1];

    fin.seekg(0, std::ios::beg);
    fin.read(script, length);
    script[length] = '\0';

    fin.close();

    const char *sql = script;
    std::vector<sqlite3_stmt*> stmts;
    while(sql < script+length) {
        sqlite3_stmt *stmt;
        int rc = sqlite3_prepare_v3(db, sql, -1, SQLITE_PREPARE_PERSISTENT, &stmt, &sql);
        if(sql == script+length) break; //skip last (empty) statement
        if(rc == SQLITE_OK) {
            // we need to execute each statement before compiling the next one
            // otherwise SQLite will error due to missing tables
            execute_stmt(db, stmt);
            stmts.push_back(stmt);
        } else {
            fprintf(stderr, "ERROR COMPILING SQL: %d %s", rc, sqlite3_errmsg(db));
        }
    }

    delete[] script;
    return stmts;
}