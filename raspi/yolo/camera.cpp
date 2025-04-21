#include <lccv.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/dnn.hpp>
#include <chrono>
#include <iostream>
#include <cmath>
#include <string>

void initializeCamera(lccv::PiCamera& cam) {
    cam.options->photo_width = 608;
    cam.options->photo_height = 608;
    cam.options->framerate = 10;
    cam.startVideo();
}

int main() {
    lccv::PiCamera cam;
    
    initializeCamera(cam);

    cv::namedWindow("FENIX Camera", cv::WINDOW_NORMAL);

    cv::dnn::Net net = cv::dnn::readNet("assets/cone/yolov4-tiny-cone_final.weights", "assets/cone/yolov4-tiny-cone.cfg");
    net.setPreferableBackend(cv::dnn::DNN_BACKEND_OPENCV);
    net.setPreferableTarget(cv::dnn::DNN_TARGET_CPU);

    std::vector<std::string> classNames;
    std::ifstream ifs("assets/cone/obj.names");
    std::string line;
    while (getline(ifs, line)) classNames.push_back(line);

    // Timer para calcular FPS
    auto last_time = std::chrono::high_resolution_clock::now();
    float fps = 0.0;

    cv::Mat frame;
    while (true) {
        if (!cam.getVideoFrame(frame, 1000)) {
            std::cerr << "Erro ao capturar o frame!" << std::endl;
            continue;
        }

        // Calcula o FPS
        auto now = std::chrono::high_resolution_clock::now();
        std::chrono::duration<float> duration = now - last_time;
        last_time = now;
        fps = 1.0f / duration.count();

        // Preprocessamento para YOLO
        cv::Mat blob;
        cv::dnn::blobFromImage(frame, blob, 1 / 255.0, cv::Size(306, 306), cv::Scalar(), true, false);,
        net.setInput(blob);

        std::vector<cv::Mat> outputs;
        net.forward(outputs, net.getUnconnectedOutLayersNames());

        float confThreshold = 0.5;
        for (auto &output : outputs) {
            for (int i = 0; i < output.rows; ++i) {
                cv::Mat row = output.row(i);
                float confidence = row.at<float>(4);

                if (confidence >= confThreshold) {
                    float* scores = row.ptr<float>() + 5;
                    cv::Point classIdPoint;
                    double maxClassScore;
                    cv::minMaxLoc(cv::Mat(1, classNames.size(), CV_32F, scores), 0, &maxClassScore, 0, &classIdPoint);
                    int classId = classIdPoint.x;

                    if (maxClassScore > confThreshold) {
                        int centerX = (int)(row.at<float>(0) * frame.cols);
                        int centerY = (int)(row.at<float>(1) * frame.rows);
                        int width = (int)(row.at<float>(2) * frame.cols);
                        int height = (int)(row.at<float>(3) * frame.rows);
                        int left = centerX - width / 2;
                        int top = centerY - height / 2;

                        rectangle(frame, cv::Rect(left, top, width, height), cv::Scalar(255, 0, 0), 1.5);
                        putText(frame, classNames[classId], cv::Point(left, top - 5), cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(255, 0, 0), 1.8);
                        putText(frame, std::to_string(100 * confidence), cv::Point(left, top + 30), cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(255, 0, 0), 1.8);

                    }
                }
            }
        }

        // Escreve FPS na imagem
        std::string fpsText = "FPS: " + std::to_string((int)fps);
        putText(frame, fpsText, cv::Point(10, 30), cv::FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(255, 0, 0), 2);

        imshow("FENIX Camera", frame);
        
        if (cv::waitKey(1) == 27) break; // ESC para sair
    }

    cam.stopVideo();
    cv::destroyAllWindows();

    return 0;
}
