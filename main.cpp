#include <opencv2/opencv.hpp>
#include <iostream>
#include <vector>
#include <string>

using namespace cv;

Point pered;
Point zad;
Point centre;
Point user;
Point station;

void recogniseStickersByThreshold(cv::Mat image)  { 
    cv::Mat image_hsv; 
    std::vector< std::vector<cv::Point> > contours;
    cv::cvtColor(image, image_hsv, cv::COLOR_BGR2HSV );  // Преобразуем в hsv
    cv::Mat tmp_img1(image.size(),CV_8U);
    cv::Mat tmp_img2(image.size(),CV_8U); 
    cv::Mat tmp_img(image.size(),CV_8U); 
    // Выделение подходящих по цвету областей. Цвет задается константой :) 
    cv::inRange(image_hsv, cv::Scalar(0,140,120), cv::Scalar(10, 240, 230), tmp_img1); 
    cv::inRange(image_hsv, cv::Scalar(170,140,120), cv::Scalar(180, 240, 230), tmp_img2);
    bitwise_or(tmp_img1, tmp_img2, tmp_img);
    // "Замазать" огрехи в при выделении по цвету 
    cv::dilate(tmp_img,tmp_img,cv::Mat(),cv::Point(-1,-1),10);   
    cv::erode(tmp_img,tmp_img,cv::Mat(),cv::Point(-1,-1),10); 
    //Выделение непрерывных областей 
    cv::findContours(tmp_img,contours, RETR_EXTERNAL, CHAIN_APPROX_NONE); 
    for (uint i = 0; i<contours.size(); i++) {
        cv::Mat sticker; 
        //Для каждой области определяем ограничивающий прямоугольник 
        cv::RotatedRect rect= minAreaRect(contours[i]); 
        cv::Point2f pts[4];
        rect.points(pts);
        for (int j = 0; j < 4; j++){
            cv::line(image, pts[j], pts[(j+1)%4], cv::Scalar(0,255,0), 2);
        } 
    } 
} 


void draw(Mat& frame){
    line(frame, pered, zad, Scalar(0,255,0), 3);
    line(frame, centre, station, Scalar(0, 255, 0), 3);
}

void calculate_centre(){
    centre.x = (pered.x+zad.x)/2;
    centre.y = (pered.y+zad.y)/2;
}

void calculate(){
    std::vector<int> a = {(pered.x - centre.x), (pered.y - centre.y)};
    std::vector<int> b = {(station.x - centre.x), (station.y - centre.y)};
    double b_lengh = sqrt(b[0]*b[0]+b[1]*b[1]);
    double angle = acos((a[0]*b[0]+a[1]*b[1])/(sqrt(a[0]*a[0]+a[1]*a[1])*sqrt(b[0]*b[0]+b[1]*b[1])))*180/CV_PI;
    std::cout << angle << " " << b_lengh << std::endl;
}   

void decodeQRCodeCV2(Mat& frame) {
    QRCodeDetector detector;
    std::vector<std::string> data;
    std::vector<Point> bbox;

    bool found_detect = detector.detectMulti(frame, bbox);
    if(found_detect){
        bool found_decode = detector.decodeMulti(frame, bbox, data);
        if (found_decode){
            for (size_t j = 0; j < data.size(); j++) {
                if (!data[j].empty()) {
                    if(data[j] == "1"){
                        pered.x = (bbox[j*4].x+bbox[j*4+1].x+bbox[j*4+2].x+bbox[j*4+3].x)/4;
                        pered.y = (bbox[j*4].y+bbox[j*4+1].y+bbox[j*4+2].y+bbox[j*4+3].y)/4;
                    }
                    if(data[j] == "2"){
                        zad.x = (bbox[j*4].x+bbox[j*4+1].x+bbox[j*4+2].x+bbox[j*4+3].x)/4;
                        zad.y = (bbox[j*4].y+bbox[j*4+1].y+bbox[j*4+2].y+bbox[j*4+3].y)/4;
                    }
                    if(data[j] == "3"){
                        user.x = (bbox[j*4].x+bbox[j*4+1].x+bbox[j*4+2].x+bbox[j*4+3].x)/4;
                        user.y = (bbox[j*4].y+bbox[j*4+1].y+bbox[j*4+2].y+bbox[j*4+3].y)/4;
                    }
                    if(data[j] == "4"){
                        station.x = (bbox[j*4].x+bbox[j*4+1].x+bbox[j*4+2].x+bbox[j*4+3].x)/4;
                        station.y = (bbox[j*4].y+bbox[j*4+1].y+bbox[j*4+2].y+bbox[j*4+3].y)/4;
                    }

                }
            }
        }
    }
}




int main() {
    VideoCapture cap(0);
    if(!cap.isOpened()) return -1;

    namedWindow("QR Scanner", WINDOW_AUTOSIZE);

    while(true) {
        Mat frame;
        cap >> frame;
        if(frame.empty()) break;
        decodeQRCodeCV2(frame);
        calculate_centre();
        calculate();
        draw(frame);
        recogniseStickersByThreshold(frame);
        imshow("QR Scanner", frame);
        if(waitKey(1) >= 0) break;
    }

    cap.release();
    destroyAllWindows();
    return 0;
}