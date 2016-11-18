#include "Session.h"
#include "FileUtils.h"

// Constructor(s) / destructor.
Session::Session() {
    initSession();
}

Session::~Session() {
    finalizeSession();
}

// Getters.
ofstream& Session::getSession() const {
    return session;
}

ofstream& Session::getSessionMain() const {
    return sessionMain;
}

ofstream& Session::getSessionGlobal() const {
    return sessionGlobal;
}

ofstream& Session::getPrevSessionMain() const {
    return prevSessionMain;
}

ofstream& Session::getPrevSessionGlobal() const {
    return prevSessionGlobal;
}

// Setters.

// Build / teardown session.
void Session::initSession() {
    const bool exists = fileExists(SESSION_FILENAME);
    if (!exists) {
        cout << "Initializing session..." << endl;
        ofstream init_file(SESSION_FILENAME.c_str());
        if (!init_file.is_open()) {
            cerr << "Could not initialize session." << endl;
            exit(-1);
        }
        cout << "Successfully initialized session." << endl;
    }
    file.open(SESSION_FILENAME.c_str());
}

void Session::finalizeSession() {
    ofstream session(SESSION_FILENAME.c_str(), ios::out | ios::trunc);
    session.close();
}
