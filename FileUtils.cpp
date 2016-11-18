#include "FileUtils.h"

bool FileUtils::fileExists(const string& filename) {
    ifstream file(filename);
    return file.is_open();
}

int FileUtils::fileSize(const string& filename) {
    ifstream in(filename, ios::binary | ios::ate);
    return in.tellg();
}

void FileUtils::touchFile(const string& filename) {
    ofstream file(filename);
}

void FileUtils::printFileContents(const string& filename) {
    string line;
    ifstream file(filename);
    if (file.is_open()) {
        while(!file.eof()) {
            getline(file, line);
            cout << line << endl;
        }
    }
}

void FileUtils::clearFile(const string& filename) { 
    fstream file;
    file.open(filename, fstream::out | fstream::trunc);
    file.close();
}

