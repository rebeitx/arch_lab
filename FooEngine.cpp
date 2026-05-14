#include <iostream>
#include <string>
#include <windows.h>
using namespace std;

class AEngine {
public:
    void forward(int time_ms);
    void right(int time_ms);
    void left(int time_ms);
    void stop(int time_ms);
};

class FooEngine : public AEngine {
public:

    void stop() {
        cout << "Left engine stop" << endl;
        cout << "Right engine stop" << endl;
        cout << endl;
    }
    void forward(int time_ms) {
        cout << "Left engine forward" << endl;
        cout << "Right engine forward" << endl;
        cout << endl;
        Sleep(time_ms);
        stop();
    }

    void right(int time_ms) {
        cout << "Left engine forward" << endl;
        cout << "Right engine back" << endl;
        cout << endl;
        Sleep(time_ms);
        stop();
    }

    void left(int time_ms) {
        cout << "Left engine back" << endl;
        cout << "Right engine forward" << endl;
        cout << endl;
        Sleep(time_ms);
        stop();
    }
};

class ACmdReceiver {
public:
    string receive();
};

class FooCmdReceiver : public ACmdReceiver {
public:
    string receive() {
        string command;
        cin >> command;
        return command;
    }
};

int main(){
    FooEngine engine;
    FooCmdReceiver receiver;
    int time_ms;
    
    string command = receiver.receive();
    cin >> time_ms;
    
    if(command == "forward") {
        engine.forward(time_ms);
    }
    else if(command == "right") {
        engine.right(time_ms);
    }
    else if(command == "left") {
        engine.left(time_ms);
    }
    
    return 0;
}
