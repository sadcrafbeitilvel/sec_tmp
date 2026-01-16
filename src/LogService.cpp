#include "LogService.h"
#include <dirent.h>
#include <vector>
#include <string>
#include <iostream>
#include <cstdlib>

using namespace std;

vector<string> listDir(const string& path) {
    vector<string> result;
    DIR* dir = opendir(path.c_str());
    if (dir == nullptr) {
      return result;
    }
    dirent* entry;
    while ((entry = readdir(dir)) != nullptr) {
        string name = entry->d_name;
        if (name != "." && name != "..") {
          result.push_back(name);
        }
    }
    closedir(dir);
    return result;
}

Result LogService::listDirectory(string path) {
    vector<string> contents = listDir(path);
    string names = "";
    for (const string& name : contents) {
        //cout << name << endl;
        names = names+ "#" +name;
      }
    if (names.empty()) {
        return { 201, "未查询到该路径下内容" };
    }else{
        return { 200, names };
    }  
}

Result LogService::listDirHtml(string path) {
    vector<string> contents = listDir(path);
    string html = "<h1>" + path + "</h1>\n";
    for (const string& name : contents) {
        html += "<p>" + name + "</p>\n";
    }
    if (contents.empty()) {
        return { 201, "未查询到该路径下内容" };
    } else {
        return { 200, html };
    }
}

Result LogService::archiveLogs(string para) {
    string command = "/itstec/arch " + para;
    int result = system(command.c_str());
    if (result != 0) {
        return { 201, "归档失败" + command };
    }
    return { 200, "归档完成" };
}

Result LogService::quickArchiveLogs() {
    string command = "quickArch";
    int result = system(command.c_str());
    if (result != 0) {
        return { 201, "快速归档失败" };
    }
    return { 200, "快速归档完成" };
}


