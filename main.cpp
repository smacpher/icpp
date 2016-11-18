// TODO
/*
 * - Handle multiline inputs (functions, paranthesis, etc).
 * - Only output stdout from newly added code.
 * - Once memory is allocated, leave it on stack / heap; don't reallocate.
 * - With the above ^, we could avoid using .session files / eliminate fileio overhead.
 * - Add indexes to [In]'s and [Out]'s.
 * - Handle all global cases (as of now, only have headers and namespaces).
 *
 *
 *
 */

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

// Colors.
const string RED_START = "\033[1;31m";
const string RED_END = "\033[0m";
const string GREEN_START = "\033[1;36m";
const string GREEN_END = "\033[0m";

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
string exec(string);
string getUserInput();
string run(string);
string strip(string&);
void compile(string, string);
void finalizeSession(string);
void handleKeyboardInterrupt(int);
void output(string);
void printFileContents(string);
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
    file.close();
}

void printFileContents(string filename) {
    string line;
    ifstream file(filename.c_str());
    if (file.is_open()) {
        while (! file.eof()) {
            getline(file, line);
            cout << line << endl;
        }
    }
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

void compile(string fileName, string binName) {
    string cmd;
    cmd = "g++ -o " + binName + " " + fileName + " 2>" + STDERR_FILENAME;
    exec(cmd);
}

string run(string binName) {
    string cmd;
    cmd = "./" + binName + " 2>" + STDERR_FILENAME;
    return exec(cmd);
}

string getUserInput() {
    string userInput;
    cout << GREEN_START << REPL_IN << GREEN_END;
    getline(cin, userInput);
    return userInput;
}

void output(string out) {
    double kbytes = (double) filesize(SESSION_BINARY_NAME) / 1000;
    cout << RED_START << kbytes << "KB " << REPL_OUT << RED_END
        << endl << out << endl;
}

int main() {
    // Register signals.
    signal(SIGINT, handleKeyboardInterrupt);

    ofstream session;
    ostringstream sessionMain;
    ostringstream sessionGlobal;
    ostringstream prevSessionMain;
    ostringstream prevSessionGlobal;
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

        // Build session file.
        constructSession(
            SESSION_FILENAME,
            sessionGlobal.str(),
            sessionMain.str()
        );

        // Compile session file.
        compile(SESSION_FILENAME, SESSION_BINARY_NAME);
        // Handle compilation error.
        if (filesize(STDERR_FILENAME) > 0) {
            // First, clear contents of session.
            sessionGlobal.str("");
            sessionGlobal.clear();
            sessionMain.str("");
            sessionMain.clear();
            // Second, revert back to before error.
            sessionGlobal << prevSessionGlobal.str();
            sessionMain << prevSessionMain.str();
            // Third, output error.
            printFileContents(STDERR_FILENAME);
            // Lastly, truncate stderr file.
            clearFile(STDERR_FILENAME);
        }

        // Run session file.
        sessionOut = run(SESSION_BINARY_NAME);
        // Handle runtime error.
        if (filesize(STDERR_FILENAME) > 0) {
            // First, clear contents of session.
            sessionGlobal.str("");
            sessionGlobal.clear();
            sessionMain.str("");
            sessionMain.clear();
            // Second, revert back to before error.
            sessionGlobal << prevSessionGlobal.str();
            sessionMain << prevSessionMain.str();
            // Third, output error.
            printFileContents(STDERR_FILENAME);
            // Lastly, truncate stderr file.
            clearFile(STDERR_FILENAME);
        }

        // Output run.
        output(sessionOut);
        // Store successful session for next iteration to fall back on.
        prevSessionGlobal.str("");
        prevSessionGlobal.clear();
        prevSessionMain.str("");
        prevSessionMain.clear();
        prevSessionGlobal << sessionGlobal.str();
        prevSessionMain << sessionMain.str();
    }
    // Clean up.
    finalizeSession(SESSION_FILENAME);
    return 0;
}
