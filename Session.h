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
        static const string SESSION_FILENAME; // .session.cpp 
        static const string SESSION_BINARY_FILENAME; // .session
        static const string STDERR_FILENAME; // .stderr
        static const string STDOUT_FILENAME; // .stdout
        

}
