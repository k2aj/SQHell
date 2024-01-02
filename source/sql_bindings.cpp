#include <util.h>
#include <sqlite3.h>
#include <stdexcept>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <cassert>
#include <vector>
#include <cstring>

namespace sqhell {

std::vector<float> floatStack;

void sql_print(sqlite3_context *ctx, int argc, sqlite3_value **argv) {
    for(int i=0; i<argc; ++i)
        printf("%s", sqlite3_value_text(argv[i]));
}
void sql_println(sqlite3_context *ctx, int argc, sqlite3_value **argv) {
    sql_print(ctx, argc, argv);
    puts("");
}
void sql_exit(sqlite3_context *ctx, int argc, sqlite3_value **argv) {
    assert(argc <= 1);
    int exit_code = argc == 0 ? 0 : sqlite3_value_int(argv[0]);
    std::exit(exit_code);
}

void sql_read_file_text(sqlite3_context *ctx, int argc, sqlite3_value **argv) {
    assert(argc == 1);
    auto path = (const char*) sqlite3_value_text(argv[0]);
    sqlite3_result_text(ctx, read_text(path), -1, free);
}

void sql_push_floats(sqlite3_context *ctx, int argc, sqlite3_value **argv) {
    for(int i=0; i<argc; ++i)
        floatStack.push_back(sqlite3_value_double(argv[i]));
}

void sql_clear_floats(sqlite3_context *ctx, int argc, sqlite3_value **argv) {
    assert(argc == 0);
    floatStack.clear();
}

void sql_get_floats(sqlite3_context *ctx, int argc, sqlite3_value **argv) {
    assert(argc == 0);
    sqlite3_result_int64(ctx, (int64_t) floatStack.data());
}

void sql_glfwInit(sqlite3_context *ctx, int argc, sqlite3_value **argv) {
    assert(argc == 0);
    sqlite3_result_int(ctx, glfwInit());
}

void sql_glfwTerminate(sqlite3_context *ctx, int argc, sqlite3_value **argv) {
    assert(argc == 0);
    glfwTerminate();
}

void sql_glfwCreateWindow(sqlite3_context *ctx, int argc, sqlite3_value **argv) {
    assert(argc == 3);
    int width = sqlite3_value_int(argv[0]);
    int height = sqlite3_value_int(argv[1]);
    const unsigned char *title = sqlite3_value_text(argv[2]);
    sqlite3_result_int64(ctx, (int64_t) glfwCreateWindow(width, height, (const char*)title, nullptr, nullptr));
}

void sql_glfwPollEvents(sqlite3_context *ctx, int argc, sqlite3_value **argv) {
    assert(argc == 0);
    glfwPollEvents();
}

void sql_glfwGetKey(sqlite3_context *ctx, int argc, sqlite3_value **argv) {
    assert(argc == 2);
    GLFWwindow *window = (GLFWwindow*) sqlite3_value_int64(argv[0]);
    int type = sqlite3_value_type(argv[1]);
    int key;
    if(type == SQLITE_INTEGER) key = sqlite3_value_int(argv[1]);
    else key = sqlite3_value_text(argv[1])[0];
    sqlite3_result_int(ctx, glfwGetKey(window, key));
}

void sql_glfwSwapBuffers(sqlite3_context *ctx, int argc, sqlite3_value **argv) {
    assert(argc == 1);
    glfwSwapBuffers((GLFWwindow*) sqlite3_value_int64(argv[0]));
}

void sql_glfwDestroyWindow(sqlite3_context *ctx, int argc, sqlite3_value **argv) {
    assert(argc == 1);
    glfwDestroyWindow((GLFWwindow*)sqlite3_value_int64(argv[0]));
}

void sql_glfwMakeContextCurrent(sqlite3_context *ctx, int argc, sqlite3_value **argv) {
    assert(argc == 1);
    glfwMakeContextCurrent((GLFWwindow*)sqlite3_value_int64(argv[0]));
}

void sql_glfwWindowShouldClose(sqlite3_context *ctx, int argc, sqlite3_value **argv) {
    assert(argc == 1);
    int result = glfwWindowShouldClose((GLFWwindow*)sqlite3_value_int64(argv[0]));
    sqlite3_result_int(ctx, result);
}

void sql_glfwGetTime(sqlite3_context *ctx, int argc, sqlite3_value **argv) {
    assert(argc == 0);
    sqlite3_result_double(ctx, glfwGetTime());
}

void sql_gladLoadGL(sqlite3_context *ctx, int argc, sqlite3_value **argv) {
    assert(argc == 0);
    sqlite3_result_int(ctx, gladLoadGL());
}

void sql_glClearColor(sqlite3_context *ctx, int argc, sqlite3_value **argv) {
    assert(argc == 3 || argc == 4);
    glClearColor(
        sqlite3_value_double(argv[0]),
        sqlite3_value_double(argv[1]),
        sqlite3_value_double(argv[2]),
        argc == 3 ? 1.0 : sqlite3_value_double(argv[3])
    );
}

void sql_glClear(sqlite3_context *ctx, int argc, sqlite3_value **argv) {
    assert(argc == 1);
    glClear(sqlite3_value_int(argv[0]));
}

void sql_glCreateShader(sqlite3_context *ctx, int argc, sqlite3_value **argv) {
    assert(argc == 1);
    sqlite3_result_int(ctx, glCreateShader(sqlite3_value_int(argv[0])));
}

void sql_glShaderSource(sqlite3_context *ctx, int argc, sqlite3_value **argv) {
    assert(argc == 2);
    GLuint shader = sqlite3_value_int(argv[0]);
    const char *src = (const char*) sqlite3_value_text(argv[1]);
    int len = strlen(src);
    glShaderSource(shader, 1, &src, &len);
}

void sql_glCompileShader(sqlite3_context *ctx, int argc, sqlite3_value **argv) {
    assert(argc == 1);
    glCompileShader(sqlite3_value_int(argv[0]));
}

void sql_glCreateProgram(sqlite3_context *ctx, int argc, sqlite3_value **argv) {
    assert(argc == 0);
    sqlite3_result_int(ctx, glCreateProgram());
}

void sql_glAttachShader(sqlite3_context *ctx, int argc, sqlite3_value **argv) {
    assert(argc == 2);
    glAttachShader(sqlite3_value_int(argv[0]), sqlite3_value_int(argv[1]));
}

void sql_glLinkProgram(sqlite3_context *ctx, int argc, sqlite3_value **argv) {
    assert(argc == 1);
    glLinkProgram(sqlite3_value_int(argv[0]));
}

void sql_glUseProgram(sqlite3_context *ctx, int argc, sqlite3_value **argv) {
    assert(argc == 1);
    glUseProgram(sqlite3_value_int(argv[0]));
}

void sql_glCreateBuffer(sqlite3_context *ctx, int argc, sqlite3_value **argv) {
    assert(argc == 0);
    GLuint buffer;
    glCreateBuffers(1, &buffer);
    sqlite3_result_int(ctx, buffer);
}

void sql_glBindBuffer(sqlite3_context *ctx, int argc, sqlite3_value **argv) {
    assert(argc == 2);
    GLenum target = sqlite3_value_int(argv[0]);
    GLuint buffer = sqlite3_value_int(argv[1]);
    glBindBuffer(target, buffer);
}

void sql_glNamedBufferData(sqlite3_context *ctx, int argc, sqlite3_value **argv) {
    assert(argc == 4);
    GLenum target = sqlite3_value_int(argv[0]);
    GLuint buffer = sqlite3_value_int(argv[1]);
    glNamedBufferData(
        sqlite3_value_int(argv[0]),
        sqlite3_value_int(argv[1]),
        (const void*) sqlite3_value_int64(argv[2]),
        sqlite3_value_int(argv[3])
    );
}


void sql_glCreateVertexArray(sqlite3_context *ctx, int argc, sqlite3_value **argv) {
    assert(argc == 0);
    GLuint vao;
    glCreateVertexArrays(1, &vao);
    sqlite3_result_int(ctx, vao);
}

void sql_glBindVertexArray(sqlite3_context *ctx, int argc, sqlite3_value **argv) {
    assert(argc == 1);
    glBindVertexArray(sqlite3_value_int(argv[0]));
}

void sql_glEnableVertexAttribArray(sqlite3_context *ctx, int argc, sqlite3_value **argv) {
    assert(argc == 1);
    glEnableVertexAttribArray(sqlite3_value_int(argv[0]));
}

void sql_glVertexAttribPointer(sqlite3_context *ctx, int argc, sqlite3_value **argv) {
    assert(argc == 6);
    glVertexAttribPointer(
        sqlite3_value_int(argv[0]),
        sqlite3_value_int(argv[1]),
        sqlite3_value_int(argv[2]),
        sqlite3_value_int(argv[3]),
        sqlite3_value_int(argv[4]),
        (const void*) sqlite3_value_int(argv[5])
    );
}

void sql_glDrawArrays(sqlite3_context *ctx, int argc, sqlite3_value **argv) {
    assert(argc == 3);
    glDrawArrays(
        sqlite3_value_int(argv[0]),
        sqlite3_value_int(argv[1]),
        sqlite3_value_int(argv[2])
    );
}

void create_scalar_function(sqlite3 *db, const char *function_name, int narg, void (*ptr)(sqlite3_context*, int, sqlite3_value**)) {
    int rc = sqlite3_create_function(db, function_name, narg, SQLITE_UTF8, nullptr, ptr, nullptr, nullptr);
    if(rc != SQLITE_OK) throw std::runtime_error("failed to create function");
}

template<int64_t value>
void sql_int_constant(sqlite3_context *ctx, int argc, sqlite3_value **argv) {
    sqlite3_result_int64(ctx, value);
}

template<int64_t value>
void create_int_constant(sqlite3 *db, const char *name) {
    create_scalar_function(db, name, 0, sql_int_constant<value>);
}

void init_sql_bindings(sqlite3 *db) {
    create_scalar_function(db, "print",                    -1, sql_print);
    create_scalar_function(db, "println",                  -1, sql_println);
    create_scalar_function(db, "exit",                      0, sql_exit);
    create_scalar_function(db, "exit",                      1, sql_exit);
    create_scalar_function(db, "readFileText",              1, sql_read_file_text);
    create_scalar_function(db, "pushFloats",               -1, sql_push_floats);
    create_scalar_function(db, "clearFloats",               0, sql_clear_floats);
    create_scalar_function(db, "getFloats",                 0, sql_get_floats);
    create_scalar_function(db, "glfwInit",                  0, sql_glfwInit);
    create_scalar_function(db, "glfwTerminate",             0, sql_glfwTerminate);
    create_scalar_function(db, "glfwCreateWindow",          3, sql_glfwCreateWindow);
    create_scalar_function(db, "glfwDestroyWindow",         1, sql_glfwDestroyWindow);
    create_scalar_function(db, "glfwSwapBuffers",           1, sql_glfwSwapBuffers);
    create_scalar_function(db, "glfwPollEvents",            0, sql_glfwPollEvents);
    create_scalar_function(db, "glfwGetKey",                2, sql_glfwGetKey);
    create_int_constant<GLFW_PRESS>(db, "GLFW_PRESS");
    create_scalar_function(db, "glfwMakeContextCurrent",    1, sql_glfwMakeContextCurrent);
    create_scalar_function(db, "glfwWindowShouldClose",     1, sql_glfwWindowShouldClose);
    create_scalar_function(db, "glfwGetTime",               0, sql_glfwGetTime);
    create_scalar_function(db, "gladLoadGL",                0, sql_gladLoadGL);
    create_scalar_function(db, "glClearColor",              3, sql_glClearColor);
    create_scalar_function(db, "glClearColor",              4, sql_glClearColor);
    create_scalar_function(db, "glClear",                   1, sql_glClear);
    create_int_constant<GL_COLOR_BUFFER_BIT>(db, "GL_COLOR_BUFFER_BIT");
    create_scalar_function(db, "glCreateShader",            1, sql_glCreateShader);
    create_int_constant<GL_VERTEX_SHADER>(db, "GL_VERTEX_SHADER");
    create_int_constant<GL_FRAGMENT_SHADER>(db, "GL_FRAGMENT_SHADER");
    create_scalar_function(db, "glShaderSource",            2, sql_glShaderSource);
    create_scalar_function(db, "glCompileShader",           1, sql_glCompileShader);
    create_scalar_function(db, "glCreateProgram",           0, sql_glCreateProgram);
    create_scalar_function(db, "glAttachShader",            2, sql_glAttachShader);
    create_scalar_function(db, "glLinkProgram",             1, sql_glLinkProgram);
    create_scalar_function(db, "glUseProgram",              1, sql_glUseProgram);
    create_scalar_function(db, "glCreateBuffer",            0, sql_glCreateBuffer);
    create_scalar_function(db, "glBindBuffer",              2, sql_glBindBuffer);
    create_int_constant<GL_ARRAY_BUFFER>(db, "GL_ARRAY_BUFFER");
    create_int_constant<GL_ELEMENT_ARRAY_BUFFER>(db, "GL_ELEMENT_ARRAY_BUFFER");
    create_scalar_function(db, "glNamedBufferData",         4, sql_glNamedBufferData);
    create_int_constant<GL_STREAM_DRAW>(db, "GL_STREAM_DRAW");
    create_scalar_function(db, "glCreateVertexArray",       0, sql_glCreateVertexArray);
    create_scalar_function(db, "glBindVertexArray",         1, sql_glBindVertexArray);
    create_scalar_function(db, "glEnableVertexAttribArray", 1, sql_glEnableVertexAttribArray);
    create_scalar_function(db, "glVertexAttribPointer",     6, sql_glVertexAttribPointer);
    create_int_constant<GL_FLOAT>(db, "GL_FLOAT");
    create_scalar_function(db, "glDrawArrays",              3, sql_glDrawArrays);
    create_int_constant<GL_TRIANGLES>(db, "GL_TRIANGLES");
}

}