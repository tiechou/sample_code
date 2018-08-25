// A translator for experimental C++ extensions.  
#include <iostream>  
#include <fstream>  
#include <cctype>  
#include <cstring>  
#include <string>  

using namespace std;  

// Prototypes for the functions that handle  
// the extended keywords.  
void foreach();  
void cases();  
void repeat();  
void until();  
void typeof();  

// Prototypes for tokenizing the input file.  
bool gettoken(string &tok);  
void skipspaces();  

// Indentation padding string.  
string indent = "";  

// The input and output file streams.  
ifstream fin;  
ofstream fout;  

// Exception class for syntax errors.  
class SyntaxExc {  
    string what;  
    public:  
    SyntaxExc(char *e) { what = string(e); }  
    string geterror() { return what; }  
};  

int main(int argc, char *argv[]) {  
    string token;  

    if(argc != 3) {  
        cout << "Usage: ep <input file> <output file>\n";  
        return 1;  
    }  

    fin.open(argv[1]);  

    if(!fin) {  
        cout << "Cannot open " << argv[1] << endl;  
        return 1;  
    }  

    fout.open(argv[2]);  

    if(!fout) {  
        cout << "Cannot open " << argv[2] << endl;  
        return 1;  
    }  

    // Write header.  
    fout << "// Translated from an .exp source file.\n";  

    try {  
        // Main translation loop.  
        while(gettoken(token)) {  

            // Skip over // comments.  
            if(token == "//") {  
                do {  
                    fout << token;  
                    gettoken(token);  
                } while(token.find('\n') == string::npos);  
                fout << token;  
            }  

            // Skip over /* comments.  
            else if(token == "/*") {  
                do {  
                    fout << token;  
                    gettoken(token);  
                } while(token != "*/");  
                fout << token;  
            }  

            // Skip over quoted string.  
            else if(token == "\"") {  
                do {  
                    fout << token;  
                    gettoken(token);  
                } while(token != "\"");  
                fout << token;  
            }  

            else if(token == "foreach") foreach();  

            else if(token == "cases") cases();  

            else if(token == "repeat") repeat();  

            else if(token == "until") until();  

            else if(token == "typeof") typeof();  

            else fout << token;  
        }  
    } catch(SyntaxExc exc) {  
        cout << exc.geterror() << endl;  
        return 1;  
    }  

    return 0;  
}  

// Get the next token from the input stream.  
bool gettoken(string &tok) {  
    char ch;  
    char ch2;  
    static bool trackIndent = true;  

    tok = "";  

    ch = fin.get();  

    // Check for EOF and return false if EOF  
    // is found.  
    if(!fin) return false;  

    // Read whitespace.  
    if(isspace(ch)) {    
        while(isspace(ch)) {  
            tok += ch;  

            // Reset indent counter with each new line.  
            if(ch == '\n') {  
                indent = "";  
                trackIndent = true;  
            }  
            else if(trackIndent) indent += ch;  

            ch = fin.get();  
        }  
        fin.putback(ch);  
        return true;  
    }  

    // Stop tracking indentation after encountering  
    // first non-whitespace character on a line.  
    trackIndent = false;  

    // Read an identifier or keyword.  
    if(isalpha(ch) || ch=='_') {  
        while(isalpha(ch) || isdigit(ch) || ch=='_') {  
            tok += ch;  
            ch = fin.get();  
        }  
        fin.putback(ch);  
        return true;  
    }  

    // Read a number.  
    if(isdigit(ch)) {  
        while(isdigit(ch) || ch=='.' ||  
                tolower(ch) == 'e' ||  
                ch == '-' || ch =='+') {  
            tok += ch;  
            ch = fin.get();  
        }  
        fin.putback(ch);  
        return true;  
    }  

    // Check for \"  
    if(ch == '\\') {  
        ch2 = fin.get();  
        if(ch2 == '"') {  
            tok += ch;  
            tok += ch2;  
            ch = fin.get();  
        } else   
            fin.putback(ch2);  
    }  

    // Check for '"'  
    if(ch == '\'') {  
        ch2 = fin.get();  
        if(ch2 == '"') {  
            tok += ch;  
            tok += ch2;  
            return true;  
        } else   
            fin.putback(ch2);  
    }  

    // Check for begin comment symbols.  
    if(ch == '/') {  
        tok += ch;  
        ch = fin.get();  
        if(ch == '/' || ch == '*') {  
            tok += ch;  
        }  
        else fin.putback(ch);  
        return true;  
    }  

    // Check for end comment symbols.  
    if(ch == '*') {  
        tok += ch;  
        ch = fin.get();  
        if(ch == '/') {  
            tok += ch;  
        }  
        else fin.putback(ch);  
        return true;  
    }  

    tok += ch;  

    return true;  
}   

// Translate a foreach loop.  
void foreach() {  
    string token;  
    string varname;  
    string arrayname;  

    char forvarname[5] = "_i";  
    static char counter[2] = "a";  

    // Create loop control variable for generated  
    // for loop.  
    strcat(forvarname, counter);  
    counter[0]++;  

    // Only 26 foreach loops in a file because  
    // generated loop control variables limited to  
    // _ia to _iz. This can be changed if desired.  
    if(counter[0] > 'z')   
        throw SyntaxExc("Too many foreach loops.");  

    fout << "int " << forvarname  
        << " = 0;\n";  

    // Write beginning of generated for loop.  
    fout << indent << "for(";  

    skipspaces();  

    // Read the (  
    gettoken(token);  
    if(token[0] != '(')   
        throw SyntaxExc("( expected in foreach.");  

    skipspaces();  

    // Get the type of the foreach variable.   
    gettoken(token);  
    fout << token << " ";  

    skipspaces();  

    // Read and save the foreach variable's name.  
    gettoken(token);  
    varname = token;  

    skipspaces();  

    // Read the "in"  
    gettoken(token);  
    if(token != "in")   
        throw SyntaxExc("in expected in foreach.");  

    skipspaces();  

    // Read the array name.  
    gettoken(token);  
    arrayname = token;  

    fout << varname << " = " << arrayname << "[0];\n";  

    // Construct target value.  
    fout << indent + "    " << forvarname << " < "  
        << "((sizeof " << token << ")/"   
        << "(sizeof " << token << "[0]));\n";  

    fout << indent + "    " << forvarname << "++, "  
        << varname << " = " << arrayname << "["  
        << forvarname << "])";  

    skipspaces();  

    // Read the )  
    gettoken(token);  
    if(token[0] != ')')  
        throw SyntaxExc(") expected in foreach.");  
}  

// Translate a cases statement.  
void cases() {  
    string token;  
    int start, end;  

    skipspaces();  

    // Get starting value.  
    gettoken(token);  

    if(isdigit(token[0])) {  
        // is an int constant  
        start = atoi(token.c_str());  
    }  
    else if(token[0] == '\'') {  
        // is char constant  
        gettoken(token);  

        start = (int) token[0];  

        // discard closing '  
        gettoken(token);  
        if(token[0] != '\'')  
            throw SyntaxExc("' expected in cases.");  
    }  
    else  
        throw SyntaxExc("Constant expected in cases.");  

    skipspaces();  

    // Read and discard the "to".  
    gettoken(token);  
    if(token != "to")  
        throw SyntaxExc("to expected in cases.");  

    skipspaces();  

    // Get ending value.  
    gettoken(token);  

    if(isdigit(token[0])) {  
        // is an int constant  
        end = atoi(token.c_str());  
    }  
    else if(token[0] == '\'') {  
        // is char constant  
        gettoken(token);  

        end = (int) token[0];  

        // discard closing '  
        gettoken(token);  
        if(token[0] != '\'')  
            throw SyntaxExc("' expected in cases.");  
    }  
    else  
        throw SyntaxExc("Constant expected in cases.");  

    skipspaces();  

    // Read and discard the :  
    gettoken(token);  

    if(token != ":")   
        throw SyntaxExc(": expected in cases.");  

    // Generate stacked case statments.  
    fout << "case " << start << ":\n";  
    for(int i = start+1 ; i <= end; i++) {  
        fout << indent << "case " << i << ":";  
        if(i != end) fout << endl;  
    }  
}  

// Translate a repeat loop.  
void repeat() {  
    fout << "do";  
}    

// Translate an until.  
void until() {  
    string token;  
    int parencount = 1;  

    fout << "while";  

    skipspaces();  

    // Read and store the (  
    gettoken(token);  
    if(token != "(")  
        throw SyntaxExc("( expected in typeof.");  
    fout << "(";  

    // Begin while by reversing and  
    // parenthesizing the condition.  
    fout << "!(";  

    // Now, read the expression.  
    do {  
        if(!gettoken(token))  
            throw SyntaxExc("Unexpected EOF encountered.");  

        if(token == "(") parencount++;  
        if(token == ")") parencount--;  

        fout << token;  
    } while(parencount > 0);  
    fout << ")";  
}  

// Translate a typeof expression.   
void typeof() {  
    string token;  
    string temp;  

    fout << "typeid(";  

    skipspaces();  

    gettoken(token);  

    do {  
        temp = token;  

        if(!gettoken(token))  
            throw SyntaxExc("Unexpected EOF encountered.");  

        if(token != "same") fout << temp;  
    } while(token != "same");  

    skipspaces();  

    gettoken(token);  

    if(token != "as") throw SyntaxExc("as expected.");  

    fout << ") == typeid(";  

    skipspaces();  

    do {  
        if(!gettoken(token))  
            throw SyntaxExc("Unexpected EOF encountered.");  

        fout << token;  
    } while(token != ")");  
    fout << ")";  
}  

void skipspaces() {  
    char ch;  

    do {  
        ch = fin.get();  
    } while(isspace(ch));  
    fin.putback(ch);  
}

