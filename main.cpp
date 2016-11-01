#include <algorithm>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <memory>
#include <signal.h>
#include <sstream>
#include <stdexcept>
#include <stdlib.h>
#include <string>

using namespace std;

const string STDERR_FILENAME = ".stderr";
const string STDOUT_FILENAME = ".stdout";
const string REPL_IN = "[In]: ";
const string REPL_OUT = "[Out]: ";
const string SESSION_FILENAME = ".session.cpp";
const string SESSION_BINARY_NAME = ".session";

bool checkEmpty(string);
bool checkExit(string);
bool fileExists(const string&);
bool isGlobal(string);
int filesize(string);
ofstream initSession();
string compile(string, string);
string exec(string);
string getUserInput();
string run(string);
string strip(string&);
void finalizeSession(string);
void handleKeyboardInterrupt(int);
void touchFile(string);

// Signal handlers.
void handleKeyboardInterrupt(int sig) {
    finalizeSession(SESSION_FILENAME);
    cout << endl;
    exit(sig);
}

// Utils.
bool checkEmpty(string s) {
    string stripped_s;
    stripped_s = strip(s);
    if (stripped_s.empty()) {
        return true;
    } else {
        return false;
    }
}

bool checkExit(string sessionIn) {
    transform(sessionIn.begin(), sessionIn.end(), sessionIn.begin(), ::toupper);
    if (sessionIn == "EXIT" || sessionIn == "EXIT()" ||
        sessionIn == "QUIT" || sessionIn == "QUIT()") {
        return true;
    } else {
        return false;
    }
}

bool fileExists(const string& filename) {
    ifstream f(filename.c_str());
    return f.is_open();
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

void touchFile(string filename) {
    ofstream file(filename.c_str());
}

int filesize(string filename) {
    ifstream in(filename.c_str(), ios::binary | ios::ate);
    return in.tellg();
}

void clearFile(string filename) {
    fstream file;
    file.open(filename.c_str(), fstream::out | fstream::trunc);
    f.close();
}

// TODO: Handle trailing and leading tabs, too.
string strip(string& str) {
    size_t first = str.find_first_not_of(' ');
    size_t last = str.find_last_not_of(' ');
    // Handle empty string case.
    if (first == string::npos) {
        return "";
    }
    return str.substr(first, (last - first + 1));
}

// Build/Teardown Session.
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

void constructSession(string filename, string globalBody, string mainBody) {
    ofstream session(filename, ios::out | ios::trunc);
    session << globalBody << endl;
    session << "int main()" << endl << "{" << endl;
    session << mainBody << endl;
    session << "return 0;" << endl << "}" << endl;
}

void finalizeSession(string filename) {
    ofstream session(filename.c_str(), ios::out | ios::trunc);
    session.close();
}

// Execute session.
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

string compile(string fileName, string binName) {
    string cmd;
    cmd = "g++ -o " + binName + " " + fileName + " 2>" + STDERR_FILENAME;
    return exec(cmd);
}

string run(string binName) {
    string cmd;
    cmd = "./" + binName;
    return exec(cmd);
}

string getUserInput() {
    string userInput;
    cout << REPL_IN;
    getline(cin, userInput);
    return userInput;
}

int main() {
    // Register signals.
    signal(SIGINT, handleKeyboardInterrupt);

    ofstream session;
    ostringstream sessionMain;
    ostringstream sessionGlobal;
    ostringstream sessionTempMain;
    ostringstream sessionTempGlobal;
    string sessionIn;
    string sessionOut;

    // Initialize session.
    initSession(SESSION_FILENAME, session);
    touchFile(STDERR_FILENAME);

    while(true) {
        // Ensure session is open.
        if (!session.is_open()) {
            cout << "Session failure." << endl;
            break;
        }
        // Fetch user input.
        sessionIn = getUserInput();

        // Handle cases.
        if (checkExit(sessionIn)) {
            break;
        } else if (checkEmpty(sessionIn)) {
            continue;
        } else if (isGlobal(sessionIn)) {
            sessionGlobal << sessionIn << endl;
        } else {
            sessionMain << sessionIn << endl;
        }

        // Store previous session state to fallback on if error is thrown.
        sessionTempGlobal << sessionGlobal.rdbuf();
        sessionTempMain << sessionMain.rdbuf();

        constructSession(
            SESSION_FILENAME,
            sessionGlobal.str(),
            sessionMain.str()
        );
        sessionOut = compile(SESSION_FILENAME, SESSION_BINARY_NAME);
        cout << sessionOut;
        // Error occurred.
        if filesize(STDERR_FILENAME > 0) {
            ;
        }
        sessionOut = run(SESSION_BINARY_NAME);
        cout << "[Out]: " << sessionOut << endl;
    }
    finalizeSession(SESSION_FILENAME);
    return 0;
}

