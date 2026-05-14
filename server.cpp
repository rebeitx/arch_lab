#include <iostream>
#include "httplib.h"
#include "nlohmann/json.hpp" // nlohmann/json.hpp "

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

FooEngine engine;

using json = nlohmann::json;

void send_receive(const httplib::Request &req, httplib::Response &res){
    json response_json;
    response_json["answer"] = "received";
    res.set_content(response_json.dump(), "application/json");
    res.status = 200; // OK
    cout << "received_going";
}


void f(const httplib::Request& req, httplib::Response& res) {
        std::string request_body = req.body;
        std::cout << "Body: " << request_body << std::endl;
        if (req.get_header_value("Content-Type").find("application/json") == std::string::npos) {
            res.status = 415; // Unsupported Media Type
            res.set_content("Unsupported Content-Type. Expected application/json", "text/plain");
            return;
        }
        try {
            json request_json = json::parse(request_body);
            // Access data from the JSON object
            std::string command = request_json.value("command", "none");
            std::cout << command << std::endl;
            int time = request_json.value("time", 0);
            std::cout << time << std::endl;
            // Create a JSON response object
            if(command == "forward") {
                // send_receive(req, res);
                json response_json;
                response_json["answer"] = "received";
                res.set_content(response_json.dump(), "application/json");
                res.status = 200; // OK
                engine.forward(time);   
            }
            else if(command == "right") {
                // send_receive(req, res);
                json response_json;
                response_json["answer"] = "received";
                res.set_content(response_json.dump(), "application/json");
                res.status = 200; // OK
                engine.right(time);
            }
            else if(command == "left") {
                // send_receive(req, res);
                json response_json;
                response_json["answer"] = "received";
                res.set_content(response_json.dump(), "application/json");
                res.status = 200; // OK
                engine.left(time);
            } 
            json response_json;
            response_json["answer"] = "done";
            res.set_content(response_json.dump(), "application/json");
            res.status = 200; // OK
        } catch (const json::parse_error& e) {
            json response_json;
            response_json["answer"] = "not recieved";
            res.set_content(response_json.dump(), "application/json");
            res.status = 200; // OK
        }
    }
int main() {
    httplib::Server svr;
    svr.Post("/commands", f);
    std::cout << "Server listening on http://localhost:8081" << std::endl;    
    svr.listen("0.0.0.0", 8081);
    return 0;
}
