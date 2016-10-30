#include <algorithm>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>

using namespace std;

bool fileExists(const string&);
bool checkExit(string);
ofstream initSession();
string compile(string, string);
string exec(string);
string run(string);
void finalizeSession(string);
string strip(string&);
bool isGlobal(string);

// var& passes the file reference (reference to the location in memory where
// the file object is stored)
void initSession(string filename, ofstream& file) {
    const bool file_exists = fileExists(filename);

    if (!file_exists) {
        cout << "Initializing session..." << endl;
        ofstream init_file(filename.c_str());
        if (!init_file.is_open()) {
            cerr << "Could not initialize session." << endl;
            return;
        }
        cout << "Successfully initialized session." << endl;
    }

    file.open(filename.c_str());
}

bool fileExists(const string& filename) {
    ifstream f(filename.c_str());
    return f.is_open();
}

void finalizeSession(string filename) {
    ofstream session(filename.c_str(), ios::out | ios::trunc);
    session.close();
}

string exec(string cmdStr) {
    const char * cmd = cmdStr.c_str();
    char buffer[128];
    string result = "";
    shared_ptr<FILE> pipe(popen(cmd, "r"), pclose);
    if (!pipe) {
        throw runtime_error("popen() failed.");
    }
    while (!feof(pipe.get())) {
        if (fgets(buffer, 128, pipe.get()) != NULL) {
            result += buffer;
        }
    }
    return result;
}

bool checkExit(string sessionIn) {
    transform(sessionIn.begin(), sessionIn.end(), sessionIn.begin(), ::toupper);
    if (sessionIn == "EXIT" || 
        sessionIn == "QUIT" ||
        sessionIn == "EXIT()" ||
        sessionIn == "QUIT()") {
        return true;
    } else {
        return false;
    }
}

bool checkEmpty(string s) {
    string stripped_s;
    stripped_s = strip(s);
    if (stripped_s.empty()) {
        return true;
    } else {
        return false;
    }
}

string compile(string fileName, string binName) {
    string cmd;
    cmd = "g++ -o " + binName + " " + fileName;
    return exec(cmd);
}

string run(string binName) {
    string cmd;
    cmd = "./" + binName;
    return exec(cmd);
}

void constructSession(string filename, string globalBody, string mainBody) {
    ofstream session(filename, ios::out | ios::trunc);
    session << globalBody << endl;
    session << "int main()" << endl << "{" << endl;
    session << mainBody << endl;
    session << "return 0;" << endl << "}" << endl;
}

bool isGlobal(string s) {
    string stripped_s;
    stripped_s = strip(s);
    size_t _include = stripped_s.find("#include");
    if (_include != string::npos &&
        _include == 0) {
        return true;
    }
    size_t _using = stripped_s.find("using");
    if (_using != string::npos &&
        _using == 0) {
        return true;
    }
    return false;
}

// TODO: deal with tabs, too.
string strip(string& str) {
    size_t first = str.find_first_not_of(' ');
    size_t last = str.find_last_not_of(' ');

    // Handle empty string case.
    if (first == string::npos) {
        return "";
    }

    return str.substr(first, (last - first + 1));
}

int main() {
    const string SESSION_FILENAME = ".session.cpp";
    const string SESSION_BINARY_NAME = ".session";

    ofstream session;
    ostringstream sessionMain;
    ostringstream sessionGlobal;
    string sessionIn;
    string sessionOut;

    initSession(SESSION_FILENAME, session);

    while(true) {
        // First, fetch current session.
        if (session.is_open()) {
            // Update session.
            cout << "[In]: ";
            getline(cin, sessionIn);
            if (checkExit(sessionIn)) {
                break;
            } else if (checkEmpty(sessionIn)) {
                continue;
            }
            if (isGlobal(sessionIn)) {
                sessionGlobal << sessionIn << endl;
            } else {
                sessionMain << sessionIn << endl;
            }
            constructSession(
                SESSION_FILENAME, 
                sessionGlobal.str(), 
                sessionMain.str()
            );
            sessionOut = compile(SESSION_FILENAME, SESSION_BINARY_NAME);
            cout << sessionOut;
            sessionOut = run(SESSION_BINARY_NAME);
            cout << "[Out]: " << sessionOut << endl;
        } else {
            cout << "FAIL" << endl;
            break;
        }
    }
    // finalizeSession(SESSION_FILENAME);
    return 0;
}

