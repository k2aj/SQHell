#include <fstream>

namespace sqhell {

char *read_text(const char *path) {
    std::ifstream fin(path);

    fin.seekg(0, std::ios::end);
    auto length = fin.tellg();
    char *text = (char*) calloc(1, 1+(size_t)length);

    fin.seekg(0, std::ios::beg);
    fin.read(text, length);

    return text;
}

}