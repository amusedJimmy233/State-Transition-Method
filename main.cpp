#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <map>

#define ERR -1
#define RESERVED 1
#define SEPARATOR 2
#define OPERATOR 3
#define CONSTANT 4
#define VARIABLE 5

using namespace std;

map<string, string> reserved_word = {{"if",   "1"},
                                     {"then", "2"},
                                     {"else", "3"},
                                     {"int",  "4"},
                                     {"char", "5"},
                                     {"for",  "6"}};
map<string, string> operator_word = {{"=",  "1"},
                                     {">=", "2"},
                                     {"==", "3"},
                                     {"+",  "4"},
                                     {"/",  "5"},
                                     {"%",  "6"},
                                     {"++", "7"}};
map<string, string> separator_word = {{"\"", "1"},
                                      {";",  "2"}};
map<string, string> state;
vector<string> constant_word;
vector<string> variable_word;
int constant_num = 1;
int variable_num = 1;

//从文件读入到string里
string read_file_to_string(const char *filename) {
    ifstream ifile(filename);
    //将文件读入到ostringstream对象buf中
    ostringstream buf;
    char ch;
    while (buf && ifile.get(ch)) {
        buf.put(ch);
    }

    //返回与流对象buf关联的字符串
    return buf.str();
}

string delete_comments(string target) {
    //单行注释
    size_t start = target.find("//");
    while (start != std::string::npos) {
        int cnt = 0;
        size_t i = start;
        while (target[i] != '\n') {
            i++;
            cnt++;
        }
        target.erase(start, cnt);
        start = target.find("//");
    }

    //多行注释
    start = target.find("/*");
    while (start != std::string::npos) {
        size_t end = target.find("*/");
        target.erase(start, end - start + 2);
        start = target.find("/*");
    }

    return target;
}

string delete_elements(string file1) {
    //删除注释
    file1 = delete_comments(file1);

    return file1;
}

string modify_file(string original_file) {
    //去除注释
    string file1 = delete_elements(std::move(original_file));

    istringstream iss(file1);
    string modified_file;
    string word;
    while (iss >> word) {
        modified_file += word;
        modified_file += " ";
    }

    return modified_file;
}

bool isalpha(char word) {
    if ((word >= 'a' && word <= 'z') || (word >= 'A' && word <= 'Z'))
        return true;
    return false;
}

bool isdigit(char word) {
    if (word >= '0' && word <= '9')
        return true;
    return false;
}

void add_state(const string &character, int state_num) {
    if (state_num == ERR) {
        state[character] = "err";
        return;
    }
    if (character == "over") return;

    string category = to_string(state_num);
    if (state.count(character) == 0) {
        switch (state_num) {
            case RESERVED:
                category += reserved_word[character];
                break;
            case SEPARATOR:
                category += separator_word[character];
                break;
            case OPERATOR:
                category += operator_word[character];
                break;
            case CONSTANT:
                constant_word.push_back(character);
                category += to_string(constant_num);
                constant_num++;
                break;
            case VARIABLE:
                variable_word.push_back(character);
                category += to_string(variable_num);
                variable_num++;
                break;
            default:
                break;
        }
        state[character] = category;
    }
}

void print_state(const string &character) {
    if (character == "over") cout << "over" << endl;
    else {
        cout << character << "\t\t(" << character << "," << state[character] << ")" << endl;
    }
}

void print_information() {
    cout << "constant: ";
    for (int i = 0; i < constant_word.size(); i++) {
        if (i) cout << " ";
        cout << constant_word[i];
    }
    cout << endl << "variable: ";
    for (int i = 0; i < variable_word.size(); i++) {
        if (i) cout << " ";
        cout << variable_word[i];
    }
}

void state_transition(string modified_file) {
    for (int i = 0; i < modified_file.size(); i++) {
        string character;
        int category = 0;
        //提取数字
        if (isdigit(modified_file[i])) {
            //数字第一位
            character.push_back(modified_file[i]);
            for (int j = i + 1;; j++) {
                if (isdigit(modified_file[j])) {
                    character.push_back(modified_file[j]);
                } else {
                    i = j - 1;
                    break;
                }
            }
            add_state(character, CONSTANT); //常量
        }
            // 提取变量/保留字
        else if (isalpha(modified_file[i])) {
            character.push_back(modified_file[i]);
            int cnt = 0;
            int flag = 0;
            for (int j = i + 1; cnt < 10; j++, cnt++) {
                //不是字母或数字，即符号
                if (isalpha(modified_file[j]) || isdigit(modified_file[j])) {
                    if (isdigit(modified_file[j])) {
                        flag = 1; //标记变量中有数字
                    }
                    character.push_back(modified_file[j]);
                } else {
                    i = j - 1;
                    break;
                }
            }
            //变量中有数字，非法
            if (flag) {
                add_state(character, ERR);
            }
            string re;
            for (auto i: character) {
                re.push_back(tolower(i));//i = tolower(i);
            }
            if (reserved_word.count(re)) {
                character = re;
                add_state(character, RESERVED);     //保留字
            } else {
                add_state(character, VARIABLE);     //变量
            }
        } else if (modified_file[i] == ' ') continue;
            //符号
        else {
            character.push_back(modified_file[i]);
            if (modified_file[i] == '>' || modified_file[i] == '=' || modified_file[i] == '<') {
                if (modified_file[i + 1] == '=') {
                    character.push_back(modified_file[i + 1]);
                    i++;
                }
            } else if (modified_file[i] == '+' && modified_file[i + 1] == '+') {
                character.push_back(modified_file[i + 1]);
                i++;
            } else if (modified_file[i] == '\\' && modified_file[i + 1] == 'n') {
                character.push_back(modified_file[i + 1]);
                i++;
            }
            if (operator_word.count(character)) {
                add_state(character, OPERATOR);     //运算符
            } else if (separator_word.count(character)) {
                add_state(character, SEPARATOR);    //分隔符
            } else {
                add_state(character, ERR);          //err符号
            }
        }
        print_state(character);
    }
}


int main() {
    const char *f = "../procedure4.txt";
    string original_file = read_file_to_string(f);
    string modified_file = modify_file(original_file);
    cout << modified_file << endl;
    state_transition(modified_file);
    print_information();

    return 0;
}
