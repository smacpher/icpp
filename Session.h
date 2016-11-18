#ifndef Session_h
#define Session_h

#include <iostream>
#include <sstream>
#include <string>

using namespace std;

class Session {
    private:
        ofstream session;
        ofstringstream sessionMain;
        ofstringstream sessionGlobal;
        ostringstream prevSessionMain;
        ofstringstream prevSessionGlobal;

    public:
        // Public static members.
        static const string SESSION_FILENAME; // .session.cpp 
        static const string SESSION_BINARY_FILENAME; // .session
        static const string STDERR_FILENAME; // .stderr
        static const string STDOUT_FILENAME; // .stdout
        
        // Constructor(s) / Destructor.
        Session();
        ~Session();

        // Getters.
        ofstream& getSession() const;
        ofstream& getSessionMain() const;
        ofstream& getSessionGlobal() const;
        ofstream& getPrevSessionMain() const;
        ofstream& getPrevSessionGlobal() const;

        // Setters.
        bool setSession(ofstream);
        bool setSessionMain(ofstream);
        bool setSessionGlobal(ofstream);
        bool setPrevSessionMain(ofstream);
        bool setPrevSessionGlobal(ofstream);

        // Build / teardown session.
        void initSession(const string&, ofstream&);
        void constructSession(const string&, const string&, const string&);
        void finalizeSession(const string&);

        // Compile / execute session.
        string exec(const string&);
        void compile(const string&, const string&);
        string run(const string&);

        // Error handling.
        void rollbackIfError(const string&);

}

#endif /* Session_h */
