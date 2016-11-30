#ifndef DEBUG_H
#define DEBUG_H

#include <bits/stdc++.h>
using namespace std;

class MyDebugger
{
    private:
        bool m_debug;

    public:
        MyDebugger();
        void setDebug(bool debug);
        void debug(const char* message);
};

#endif