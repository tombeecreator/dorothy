#include <iostream>
#include <fstream>
#include "../../include/lexer.hpp"
using namespace std;
using namespace Dorothy;

int
main(int argc, char** argv) {
    int tokenLength;
    Lexer lexer;
    auto tokens = lexer.lex(argv[1]);
    cout << "target triple = \"arm64-apple-macosx13.0.0\"" << endl;
    cout << "define i32 @main()" << endl;
    cout << "{" << endl;
    cout << "ret i32 " << tokens[0].getInteger() << endl;
    cout << "}" << endl;
    return 0;
}
