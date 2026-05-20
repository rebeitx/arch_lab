#include <opencv2/opencv.hpp>
#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <thread>
#include <mutex>

using namespace cv;
using namespace std;

Point pered;
Point zad;
Point centre;
Point user;
Point station;

double angle;
double length;
string direction; //0-влево, 1-вправо
double omega = 10;
double velocity = 52;

const double min_angle = 5;
const double min_length = 10;

enum Color {red, green, blue, purple};

std::mutex mtx;

void calculating_centre(){
    centre.x = (pered.x+zad.x)/2;
    centre.y = (pered.y+zad.y)/2;
}

void calculating_angle(){
    lock_guard<std::mutex> lock(mtx);
    vector<int> a = {(pered.x - centre.x), (pered.y - centre.y)};
    vector<int> b = {(station.x - centre.x), (station.y - centre.y)};
    double b_lengh = sqrt(b[0]*b[0]+b[1]*b[1]);
    double a_lengh = sqrt(a[0]*a[0]+a[1]*a[1]);
    if(!a_lengh || !b_lengh){
        return;
    }
    angle = acos((a[0]*b[0]+a[1]*b[1])/(a_lengh*b_lengh))*180/CV_PI;
    double perpendikular = a[0]*b[1]-a[1]*b[0];
    if(perpendikular>0){
        direction = "RIGHT";
    } else direction = "LEFT";
}
void calculating_length(){
    lock_guard<std::mutex> lock(mtx);
    std::vector<int> a = {(pered.x - centre.x), (pered.y - centre.y)};
    std::vector<int> b = {(station.x - centre.x), (station.y - centre.y)};
    double b_lengh = sqrt(b[0]*b[0]+b[1]*b[1]);
    length = b_lengh;
}

void create_json(std::string command, int time){
    std::ofstream out("message.json");
    out << "{\n";
    out << "\"command\": \"" << command << "\",\n";
    out << "\"time\": " << time << "\n}";
    out.close();
}

void rotate(){
    int time = angle/omega*200;
    create_json(direction, time);
}

void move(){
    int time = length/velocity*200;
    create_json("forward", time);
}

struct ZNAK_SVERHU {};
struct ANGLE_CALCULATED {};
struct ROTATED {};
struct DISTANCE_CALCULATED {};
struct MOVED {};

class Connection {
    enum class State : char { WAITING, CALCULATING_ANGLE, ROTATING, CALCULATING_DISTANCE, MOVING} state{State::WAITING};
public:
    constexpr void process_event(ZNAK_SVERHU const&) {
        std::cout << "ZNAK_SVERHU" << std::endl;
        switch(state) {
        default: break;
        case State::WAITING:
            calculating_angle();
            state = State::CALCULATING_ANGLE;
            process_event(ANGLE_CALCULATED{});
        }
    }
    constexpr void process_event(ANGLE_CALCULATED const&) {
        std::cout << "ANGLE_CALCULATED" << std::endl;
        switch(state) {
        default: break;
        case State::CALCULATING_ANGLE:
            if(angle > min_angle){
                rotate();
                system("python client_post.py");//мб на плюсах
                state = State::ROTATING;
                process_event(ROTATED{});
            }else{
                calculating_length();
                state = State::CALCULATING_DISTANCE;
                process_event(DISTANCE_CALCULATED{});
            }
        }
    }
    constexpr void process_event(ROTATED const&) {
        std::cout << "ROTATED" << std::endl;
        if (state == State::ROTATING) {
            calculating_angle();
            state = State::CALCULATING_ANGLE;
            process_event(ANGLE_CALCULATED{});
        }
    }
    constexpr void process_event(DISTANCE_CALCULATED const&) {
        std::cout << "DISTANCE_CALCULATED" << std::endl;
        if (state == State::CALCULATING_DISTANCE) {
            if(length < min_length){
                state = State::WAITING;
            }else{
                move();
                system("python client_post.py");
                state = State::MOVING;
                process_event(MOVED{});
            }
        }
    }
    constexpr void process_event(MOVED const&) {
        std::cout << "MOVED" << std::endl;
        if (state == State::MOVING) {
            calculating_length();
            if(length < min_length){
                state = State::WAITING;
            }else{
                calculating_angle();
                state = State::CALCULATING_ANGLE;
                process_event(ANGLE_CALCULATED{});
            }
        }
    }
};


void draw(Mat& frame){
    line(frame, pered, zad, Scalar(255,0,0), 1);
    line(frame, centre, station, Scalar(255, 0, 0), 1);
}

void decodeQRCode(Mat &frame) {
    lock_guard<std::mutex> lock(mtx);
    QRCodeDetector detector;
    vector<string> data;
    vector<Point> points;

    detector.detectAndDecodeMulti(frame, data, points);
    if (!data.empty()) {
        for (size_t j = 0; j < data.size(); j++) {
            if (!data[j].empty()) {
                if(data[j] == "1"){
                    pered.x = (points[j*4].x+points[j*4+1].x+points[j*4+2].x+points[j*4+3].x)/4;
                    pered.y = (points[j*4].y+points[j*4+1].y+points[j*4+2].y+points[j*4+3].y)/4;
                }
                if(data[j] == "2"){
                    zad.x = (points[j*4].x+points[j*4+1].x+points[j*4+2].x+points[j*4+3].x)/4;
                    zad.y = (points[j*4].y+points[j*4+1].y+points[j*4+2].y+points[j*4+3].y)/4;
                }
                if(data[j] == "3"){
                    user.x = (points[j*4].x+points[j*4+1].x+points[j*4+2].x+points[j*4+3].x)/4;
                    user.y = (points[j*4].y+points[j*4+1].y+points[j*4+2].y+points[j*4+3].y)/4;
                }
                if(data[j] == "4"){
                    station.x = (points[j*4].x+points[j*4+1].x+points[j*4+2].x+points[j*4+3].x)/4;
                    station.y = (points[j*4].y+points[j*4+1].y+points[j*4+2].y+points[j*4+3].y)/4;
                }
            }
        }
        calculating_centre();
    }
}




std::string read_json(){
    std::ifstream in("answer.json");
    std::string line;
    std::getline(in, line);
    if(line == "{\"answer\": \"received\"}"){
        in.close();
        return "received";
    }else if(line == "{\"answer\": \"not received\"}"){
        in.close();
        return "not received";
    }else if(line == "{\"answer\": \"done\"}"){
        in.close();
        return "done";
    }
    return "";
}

void cam(){
    VideoCapture cap(1);
    if(!cap.isOpened()) return;
    namedWindow("QR Scanner", WINDOW_AUTOSIZE);
    while(true) {
        Mat frame;
        cap >> frame;
        decodeQRCode(frame);
        draw(frame);
        imshow("QR Scanner", frame);

        if (waitKey(1) == 'q'){
            break;
        }
    }
    cap.release();
    destroyAllWindows();
}

int main() {
    Connection connection{};
    thread t1(cam);
    t1.detach();
    while(true){
        if (waitKey(1) == 'r'){
            cout << "sdfsdf" << std::endl;
            if(pered.x != 0 && pered.y != 0 && zad.x != 0 && zad.y != 0 && station.x != 0 && station.y != 0){
                cout << "ZNAK_SVERHU" << endl;
                connection.process_event(ZNAK_SVERHU{});
            }
        }
    }
    return 0;
}
