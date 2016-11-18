#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

using namespace std;

class FileUtils {
    public:
        // File helpers.
        static bool fileExists(const string&);
        static int fileSize(const string&);
        static void touchFile(const string&);
        static void printFileContents(const string&);
        static void clearFile(const string&);
};

