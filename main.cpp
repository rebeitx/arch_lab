#include <opencv2/opencv.hpp>
#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <thread>
#include <mutex>


using namespace cv;

Point pered;
Point zad;
Point centre;
Point user;
Point station;

double angle;
double distance;
bool direction;//0-влево, 1-вправо
double omega = 10;
double velocity = 52;

const double min_angle = 5;
const double min_dist = 10;

enum Color {red, green, blue, purple};

std::mutex mtx;

void mute() {
    std::lock_guard<std::mutex> lock(mtx);
}

void calculate_centre(){
    centre.x = (pered.x+zad.x)/2;
    centre.y = (pered.y+zad.y)/2;
}

void calculate(){
    calculate_centre();
    std::vector<int> a = {(pered.x - centre.x), (pered.y - centre.y)};
    std::vector<int> b = {(station.x - centre.x), (station.y - centre.y)};
    double b_lengh = sqrt(b[0]*b[0]+b[1]*b[1]);
    double a_lengh = sqrt(a[0]*a[0]+a[1]*a[1]);
    if(!a_lengh || !b_lengh){
        return;
    }
    angle = acos((a[0]*b[0]+a[1]*b[1])/(a_lengh*b_lengh))*180/CV_PI;
    distance = b_lengh;
    double perpendikular = a[0]*b[1]-a[1]*b[0];
    if(perpendikular>0){
        direction = 1;
    } else direction = 0;
    std::cout<<perpendikular << std::endl;
    std::cout << angle << " " << b_lengh << std::endl;
}

void draw(Mat& frame){
    line(frame, pered, zad, Scalar(255,0,0), 1);
    line(frame, centre, station, Scalar(255, 0, 0), 1);
}

void decodeQRCode(Mat qrcode, const std::vector<Point2f> &src_points) { //Удалил Mat Frame ибо нехуй ему тут делать
    QRCodeDetector detector;
    std::string data;

    data = detector.detectAndDecode(qrcode);
    std::cout << data << std::endl;
    if (!data.empty()) {
        if(data == "1"){
            pered.x = (src_points[0].x+src_points[1].x+src_points[2].x+src_points[3].x)/4;
            pered.y = (src_points[0].y+src_points[1].y+src_points[2].y+src_points[3].y)/4;
        }
        if(data == "2"){
            zad.x = (src_points[0].x+src_points[1].x+src_points[2].x+src_points[3].x)/4;
            zad.y = (src_points[0].y+src_points[1].y+src_points[2].y+src_points[3].y)/4;
        }
        if(data == "3"){
            user.x = (src_points[0].x+src_points[1].x+src_points[2].x+src_points[3].x)/4;
            user.y = (src_points[0].y+src_points[1].y+src_points[2].y+src_points[3].y)/4;
        }
        if(data == "4"){
            station.x = (src_points[0].x+src_points[1].x+src_points[2].x+src_points[3].x)/4;
            station.y = (src_points[0].y+src_points[1].y+src_points[2].y+src_points[3].y)/4;
        }
        calculate();
        // switch(data){
        // case "1":
        //     pered.x = (src_points[0].x+src_points[1].x+src_points[2].x+src_points[3].x)/4;
        //     pered.y = (src_points[0].y+src_points[1].y+src_points[2].y+src_points[3].y)/4;
        // case "2":
        //     zad.x = (src_points[0].x+src_points[1].x+src_points[2].x+src_points[3].x)/4;
        //     zad.y = (src_points[0].y+src_points[1].y+src_points[2].y+src_points[3].y)/4;
        // case "3":
        //     user.x = (src_points[0].x+src_points[1].x+src_points[2].x+src_points[3].x)/4;
        //     user.y = (src_points[0].y+src_points[1].y+src_points[2].y+src_points[3].y)/4;
        // case "4":
        //     station.x = (src_points[0].x+src_points[1].x+src_points[2].x+src_points[3].x)/4;
        //     station.y = (src_points[0].y+src_points[1].y+src_points[2].y+src_points[3].y)/4;
        // } Альтернативка, я думал красивее будет, но что-то хуйня, можно через словари попробовать типа
        // 1:pered, 2:zad, 3:user, 4:station, ну или че-нибудь подобное со структурой придумать

    }
}

std::vector<std::vector<Point2f>> findColor(cv::Mat image, Color color_enum)  {
    std::vector<std::vector<Point2f>> output;
    cv::Mat image_hsv;
    std::vector< std::vector<cv::Point> > contours;
    cv::cvtColor(image, image_hsv, cv::COLOR_BGR2HSV );
    cv::Mat tmp_img1(image.size(),CV_8U), tmp_img2(image.size(),CV_8U), tmp_img(image.size(),CV_8U);
    Scalar color;
    if(color_enum == red){
        color = {175, 133, 150}; // Так красивее
        // color[0] = 0;
        // color[1] = 190;
        // color[2] = 175;
        cv::inRange(image_hsv, cv::Scalar(0,color[1]-50,color[2]-55), cv::Scalar(5, color[1]+50,color[2]+55), tmp_img1);
        cv::inRange(image_hsv, cv::Scalar(165, color[1]-50,color[2]-55), cv::Scalar(180, color[1]+50,color[2]+55), tmp_img2);
        bitwise_or(tmp_img1, tmp_img2, tmp_img);
    } else if(color_enum == green){
        color = {57, 84, 100}; // Так красивее
        // color[0] = 85;
        // color[1] = 191;
        // color[2] = 107;
        cv::inRange(image_hsv, cv::Scalar(color[0]-10,color[1]-50,color[2]-55), cv::Scalar(color[0]+10,color[1]+50,color[2]+55), tmp_img);
    } /*else if(color_enum == blue){
        color[0] = 107;
        color[1] = 163;
        color[2] = 97;
        cv::inRange(image_hsv, cv::Scalar(color[0]-10,color[1]-50,color[2]-55), cv::Scalar(color[0]+10,color[1]+50,color[2]+55), tmp_img);
    } else if(color_enum == purple){
        color[0] = 18;
        color[1] = 176;
        color[2] = 163;
        cv::inRange(image_hsv, cv::Scalar(color[0]-10,color[1]-50,color[2]-55), cv::Scalar(color[0]+10,color[1]+50,color[2]+55), tmp_img);
    }*/
    // Ибо оно нам нахуй не надо, мы с 2 цветами сексимся

    cv::dilate(tmp_img, tmp_img,cv::Mat(), cv::Point(-1,-1),5);
    cv::erode(tmp_img, tmp_img,cv::Mat(), cv::Point(-1,-1),2);

    cv::findContours(tmp_img,contours, RETR_EXTERNAL, CHAIN_APPROX_NONE);
    for (uint i = 0; i<contours.size(); i++) {
        if(contours[i].size() < 3) continue;
        cv::RotatedRect rect = minAreaRect(contours[i]);
        cv::Point2f pts[4];
        rect.points(pts);
        std::vector<cv::Point2f> perenos;
        for (int j = 0; j < 4; j++){
            perenos.push_back(pts[j]);
        }
        output.push_back(perenos);
    }
    return output;
}

void correct_perspective(Mat& img, const std::vector<Point2f>& src_points) {
    float w = cv::norm(src_points[0] - src_points[1]);
    float h = cv::norm(src_points[1] - src_points[2]);
    Size size(w*10, h*10);
    std::vector<Point2f> dst_points = {{0, 0}, {w*10, 0}, {w*10, h*10}, {0, h*10}};
    Mat M = getPerspectiveTransform(src_points, dst_points);
    Mat warped;
    warpPerspective(img, warped, M, size);
    decodeQRCode(warped, src_points);
    imshow("QR", warped);
}

void create_json(std::string command, int time){
    std::ofstream out("message.json");
    out << "{\n";
    out << "\"command\": \"" << command << "\",\n";
    out << "\"time\": " << time << "\n}";
    out.close();
}

void calculate_direction(){
    int time = angle/omega*200;
    if(direction){
        create_json("right", time);
    } else create_json("left", time);
}

void calculate_distance(){
    int time = distance/velocity*200;
    create_json("forward", time);
}

std::string read_json(){
    mute();
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

int main() {
    VideoCapture cap(1);
    if(!cap.isOpened()) return -1;

    namedWindow("QR Scanner", WINDOW_AUTOSIZE);
    int iterations = 0;
    int fake_iterations = 0;
    std::vector<std::vector<Point2f>> stickers;
    std::thread([]{system("python client_get.py");}).detach();
    while(true) {
        Mat frame;
        cap >> frame;
        Mat fake_frame = frame.clone();
        iterations++;
        std::string result = read_json();
        if(iterations == 30) {
            stickers = findColor(frame, green);
            std::vector<std::vector<Point2f>> red_stickers = findColor(frame, red);
            stickers.reserve(stickers.size() + red_stickers.size());
            stickers.insert(stickers.end(), red_stickers.begin(), red_stickers.end());
            for(int i = 0; i < stickers.size(); i++){
                correct_perspective(frame, stickers[i]);
            }
            iterations = 0;
        }
        for(int i = 0; i < stickers.size(); i++){
            line(fake_frame, stickers[i][0], stickers[i][1], Scalar(255, 0, 0), 1);
            line(fake_frame, stickers[i][1], stickers[i][2], Scalar(255, 0, 0), 1);
            line(fake_frame, stickers[i][2], stickers[i][3], Scalar(255, 0, 0), 1);
            line(fake_frame, stickers[i][3], stickers[i][0], Scalar(255, 0, 0), 1);
        }
        draw(fake_frame);
        imshow("QR Scanner", fake_frame);
        if(waitKey(1) >= 0) break;
        fake_iterations++;
        if (result == "not received" || result == "done" || result == ""){
            if(fake_iterations > 300){
                if(angle > min_angle){
                    calculate_direction();
                    system("python client_post.py");
                }else if(distance > min_dist){
                    calculate_distance();
                    system("python client_post.py");
                }
                fake_iterations = 0;
            }
        }
        
    }

    cap.release();
    destroyAllWindows();
    return 0;
}
