#pragma once

struct sqlite3;

namespace sqhell {

void init_sql_bindings(sqlite3 *db);

}