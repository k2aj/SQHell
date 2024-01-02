#include <util.h>
#include <sqlite3.h>
#include <stdexcept>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <imgui/imgui.h>
#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_opengl3.h>
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

void sql_ImGuiCreateContext(sqlite3_context *ctx, int argc, sqlite3_value **argv) {
    assert(argc == 0);
    sqlite3_result_int64(ctx, (int64_t) ImGui::CreateContext());
}

void sql_ImGui_ImplGlfw_InitForOpenGL(sqlite3_context *ctx, int argc, sqlite3_value **argv) {
    assert(argc == 2);
    auto window = (GLFWwindow*) sqlite3_value_int64(argv[0]);
    auto installCallbacks = sqlite3_value_int(argv[1]) != 0;
    bool ret = ImGui_ImplGlfw_InitForOpenGL(window, installCallbacks);
    sqlite3_result_int(ctx, ret);
}

void sql_ImGui_ImplOpenGL3_Init(sqlite3_context *ctx, int argc, sqlite3_value **argv) {
    assert(argc == 0 || argc == 1);
    const char *glslVersion = nullptr;
    if(argc == 1) glslVersion = (const char*) sqlite3_value_text(argv[0]);
    bool ret = ImGui_ImplOpenGL3_Init(glslVersion);
    sqlite3_result_int(ctx, ret);
}

void sql_ImGui_ImplOpenGL3_NewFrame(sqlite3_context *ctx, int argc, sqlite3_value **argv) {
    assert(argc == 0);
    ImGui_ImplOpenGL3_NewFrame();
}

void sql_ImGui_ImplGlfw_NewFrame(sqlite3_context *ctx, int argc, sqlite3_value **argv) {
    assert(argc == 0);
    ImGui_ImplGlfw_NewFrame();
}

void sql_ImGuiNewFrame(sqlite3_context *ctx, int argc, sqlite3_value **argv) {
    assert(argc == 0);
    ImGui::NewFrame();
}

void sql_ImGuiRender(sqlite3_context *ctx, int argc, sqlite3_value **argv) {
    assert(argc == 0);
    ImGui::Render();
}

void sql_ImGuiGetDrawData(sqlite3_context *ctx, int argc, sqlite3_value **argv) {
    assert(argc == 0);
    sqlite3_result_int64(ctx, (int64_t)ImGui::GetDrawData());
}

void sql_ImGui_ImplOpenGL3_RenderDrawData(sqlite3_context *ctx, int argc, sqlite3_value **argv) {
    assert(argc == 1);
    ImGui_ImplOpenGL3_RenderDrawData((ImDrawData*)sqlite3_value_int64(*argv));
}

void sql_ImGuiBegin(sqlite3_context *ctx, int argc, sqlite3_value **argv) {
    assert(argc == 1 || argc == 2);
    int flags = 0;
    if(argc == 2) flags = sqlite3_value_int(argv[1]); 
    bool ret = ImGui::Begin((const char*)sqlite3_value_text(*argv), nullptr, flags);
    sqlite3_result_int(ctx, ret);
}

void sql_ImGuiEnd(sqlite3_context *ctx, int argc, sqlite3_value **argv) {
    assert(argc == 0);
    ImGui::End();
}

void sql_ImGuiLabel(sqlite3_context *ctx, int argc, sqlite3_value **argv) {
    assert(argc == 2);
    char *name = strdup((const char*)sqlite3_value_text(argv[0]));
    const char *value = (const char*) sqlite3_value_text(argv[1]);
    ImGui::LabelText(name, "%s", value);
    free(name);
}

void sql_ImGuiButton(sqlite3_context *ctx, int argc, sqlite3_value **argv) {
    assert(argc == 1 || argc == 2 || argc == 3);
    auto text = (const char*) sqlite3_value_text(argv[0]);
    float sx = argc < 2 ? 0.0 : sqlite3_value_double(argv[1]);
    float sy = argc < 3 ? 0.0 : sqlite3_value_double(argv[2]);
    bool ret = ImGui::Button(text, ImVec2(sx,sy));
    sqlite3_result_int(ctx, ret);
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

    create_scalar_function(db, "ImGuiCreateContext",        0, sql_ImGuiCreateContext);
    create_scalar_function(db, "ImGui_ImplGlfw_InitForOpenGL", 2, sql_ImGui_ImplGlfw_InitForOpenGL);
    create_scalar_function(db, "ImGui_ImplOpenGL3_Init",    0, sql_ImGui_ImplOpenGL3_Init);
    create_scalar_function(db, "ImGui_ImplOpenGL3_Init",    1, sql_ImGui_ImplOpenGL3_Init);
    create_scalar_function(db, "ImGui_ImplOpenGL3_NewFrame",0, sql_ImGui_ImplOpenGL3_NewFrame);
    create_scalar_function(db, "ImGui_ImplGlfw_NewFrame",   0, sql_ImGui_ImplGlfw_NewFrame);
    create_scalar_function(db, "ImGuiNewFrame",             0, sql_ImGuiNewFrame);
    create_scalar_function(db, "ImGuiRender",               0, sql_ImGuiRender);
    create_scalar_function(db, "ImGuiGetDrawData",          0, sql_ImGuiGetDrawData);
    create_scalar_function(db, "ImGui_ImplOpenGL3_RenderDrawData", 1, sql_ImGui_ImplOpenGL3_RenderDrawData);
    create_scalar_function(db, "ImGuiBegin",                1, sql_ImGuiBegin);
    create_scalar_function(db, "ImGuiBegin",                2, sql_ImGuiBegin);
    create_scalar_function(db, "ImGuiEnd",                  0, sql_ImGuiEnd);
    create_scalar_function(db, "ImGuiLabel",                2, sql_ImGuiLabel);
    create_scalar_function(db, "ImGuiButton",               1, sql_ImGuiButton);
    create_scalar_function(db, "ImGuiButton",               2, sql_ImGuiButton);
    create_scalar_function(db, "ImGuiButton",               3, sql_ImGuiButton);
}

}