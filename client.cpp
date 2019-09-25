
#include <stdio.h>
#include <iostream>
#include <fstream> //  checks if image exists
#include <grpcpp/grpcpp.h>
#include <string.h>
#include "compiled/image.grpc.pb.h"

#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include "opencv2/opencv.hpp"
// #include "/usr/local/include/opencv4/opencv2/imgcodecs.hpp"

typedef unsigned char byte;

using std::cout;
using std::cin;
using std::endl;

// serializes the Mat image type into an NLImage
NLImage deconstructImage(cv::Mat image, int is_color_image) {
    int height = image.rows;
    int width = image.cols;

    // calculate total number of elements needed for data array
    unsigned long length = image.total()*image.elemSize()/sizeof(byte);
    char *imgBytes = (char*)malloc(length);
    memcpy(imgBytes, image.data, length);

    std::string NLstring(imgBytes);

    if (length > NLstring.max_size()) {
        // will probably crash before this check
        cout << "Image too large for gRPC." << endl;
    }

    NLImage reply;

    reply.set_height(height);
    reply.set_width(width);
    reply.set_color(image.type());
    reply.set_data(NLstring);

    free(imgBytes);
    // free(image.data); // malloc in reconstructImage imgData

    return reply;
}

// deserializes NLImage into a Mat image type
cv::Mat reconstructImage(NLImage image_message) {
    int height = image_message.height();
    int width = image_message.width();
    int type = image_message.color();

    // convert string from Protobuf into a char array for cv::Mat
    byte *imgData = (byte*)malloc(sizeof(image_message.data()[0])*image_message.data().length()); //(byte*)image_message.data().c_str();
    memcpy(imgData, image_message.data().c_str(), image_message.data().length());

    cv::Mat frame(height, width, type, imgData);

    return frame;
}

class ImageClientImpl {
public:
    ImageClientImpl(std::shared_ptr<grpc::Channel> channel) : stub_(NLImageService::NewStub(channel)) {}

    void RotateImage(char* fileName, int is_color_image, NLImageRotateRequest::Rotation degree_rotation) {

        // read in the image frame
        cv::Mat frame;
        frame = cv::imread(fileName, is_color_image);

        // check for invalid input
        if (!frame.data) {
            printf("Error with data; invalid input\n");
            return;
        }

        // Data we are sending to the server.
        NLImageRotateRequest *request = new NLImageRotateRequest; // has mutable_image()
        request->set_rotation(degree_rotation);
        NLImage tmp = deconstructImage(frame, is_color_image);
        // request->set_allocated_image(*tmp);
        request->mutable_image()->set_height(tmp.height());
        request->mutable_image()->set_width(tmp.width());
        request->mutable_image()->set_color(tmp.color());
        request->mutable_image()->set_data(tmp.data());

        // Container for the data we expect from the server.
        NLImage reply;

        // Context for the client. Required but not used
        grpc::ClientContext context;

        // testing image writing and stuff
        cv::Mat test = reconstructImage(request->image());

        // The actual RPC function call
        grpc::Status status = stub_->RotateImage(&context, *request, &reply);

        // rebuild the reply into a Mat image type
        cv::Mat final = reconstructImage(reply);

        // Act upon its status.
        if (status.ok()) {
            // save image
            cv::imwrite("rotated_image.png", final);
            cout << "Saved image successfully!" << endl;
        } else {
            std::cout << status.error_code() << ": " << status.error_message()
                    << std::endl;
        }

    }

    void medianBlurFilter(char* fileName, int is_color_image, int kernel) {

        // image we're going to process
        cv::Mat frame;
        frame = cv::imread(fileName, is_color_image);

        // check for invalid input
        if (!frame.data) {
            printf("Error with data; invalid input\n");
            return;
        }

        // Data we are sending to the server.
        NLBlurImageEndpointRequest *request = new NLBlurImageEndpointRequest; // has mutable_image()
        NLImage tmp = deconstructImage(frame, is_color_image);
        // request->set_allocated_image(*tmp);
        request->mutable_image()->set_height(tmp.height());
        request->mutable_image()->set_width(tmp.width());
        request->mutable_image()->set_color(tmp.color());
        request->mutable_image()->set_data(tmp.data());

        // set kernel
        request->set_kernel(kernel);

        // Container for the data we expect from the server.
        NLImage reply;

        // Context for the client. Required but not used
        grpc::ClientContext context;

        // The actual RPC.
        grpc::Status status = stub_->MedianBlurFilter(&context, *request, &reply);

        // rebuild the reply into a Mat image type
        cv::Mat final = reconstructImage(reply);

        // Act upon its status.
        if (status.ok()) {
            // save image
            cv::imwrite("median_blur_image.png", final);
            cout << "Saved image successfully!" << endl;
        } else {
            cout << status.error_code() << ": " << status.error_message()
                    << endl;
        }

    }

    void CustomImageEndpoint(char* fileName, int is_color_image) {

        // image we're going to process
        cv::Mat frame;
        frame = cv::imread(fileName, is_color_image);

        // check for invalid input
        if (!frame.data) {
            printf("Error with data; invalid input\n");
            return;
        }

        // Data we are sending to the server.
        NLCustomImageEndpointRequest *request = new NLCustomImageEndpointRequest; // has mutable_image()
        NLImage tmp = deconstructImage(frame, is_color_image);
        // request->set_allocated_image(*tmp);
        request->mutable_image()->set_height(tmp.height());
        request->mutable_image()->set_width(tmp.width());
        request->mutable_image()->set_color(tmp.color());
        request->mutable_image()->set_data(tmp.data());

        // Container for the data we expect from the server.
        NLCustomImageEndpointResponse reply;

        // Context for the client. Required but not used
        grpc::ClientContext context;

        // The actual RPC.
        grpc::Status status = stub_->CustomImageEndpoint(&context, *request, &reply);

        // Act upon its status.
        if (status.ok()) {
            // print out image
            cout << reply.image().data() << endl;
        } else {
            cout << status.error_code() << ": " << status.error_message()
                    << endl;
        }

    }

private:
    std::unique_ptr<NLImageService::Stub> stub_;

};

int main() {
    char name[50];
    int is_color_image;
    char is_yes;
    int degree_rotation;

    cout << "Please enter the file name " << endl;
    cin  >> name;

    std::ifstream imgCheck(name);
    if (imgCheck.fail()) {
        cout << "Error: could not open image " << name << "; exiting." << endl;
        return -1;
    }
    imgCheck.close();

    cout << "Would you like to use the graystyle color space? (Y/n)" << endl;
    cin >> is_yes;
    if (is_yes == 'y' || is_yes == 'Y') {
        is_color_image = cv::IMREAD_GRAYSCALE;
    } else {
        is_color_image = cv::IMREAD_UNCHANGED;
    }

    // need to create the channel
    // auto channel = CreateChannel("localhost:50051", InsecureChannelCredentials());
    ImageClientImpl imageChannel(grpc::CreateChannel("localhost:50051", grpc::InsecureChannelCredentials()));

    cout << "Would you like to rotate this image? (Y/n)" << endl;
    cin >> is_yes;
    if (is_yes == 'y' || is_yes == 'Y') {
        do {
            cout << "How many degrees? Enter one of the following: [0, 90, 180, 270]" << endl;
            cin >> degree_rotation;
        } while (degree_rotation % 90 != 0);

        while (degree_rotation < 0) {
            degree_rotation += 360;
        }

        NLImageRotateRequest::Rotation rotation;
        if (degree_rotation % 270 == 0) {
            rotation = NLImageRotateRequest::TWO_SEVENTY_DEG;
        } else if (degree_rotation % 180 == 0) {
            rotation = NLImageRotateRequest::ONE_EIGHTY_DEG;
        } else if (degree_rotation % 90 == 0) {
            rotation = NLImageRotateRequest::NINETY_DEG;
        } else {
            rotation = NLImageRotateRequest::NONE;
        }

        imageChannel.RotateImage(name, is_color_image, rotation);
    }

    cout << "Would you like to run a median blur filter over the image? (Y/n)" << endl;
    cin >> is_yes;
    if (is_yes == 'y' || is_yes == 'Y') {
        int kernel = 5;

        do {
            cout << "What kernel size would you like to use? (Recommend 5, must be an odd number)" << endl;
            cin >> kernel;
        } while (kernel < 0 || kernel % 2 == 0);

        imageChannel.medianBlurFilter(name, is_color_image, kernel);
    }

    cout << "Would you like to build ascii art? (Y/n)" << endl;
    cin >> is_yes;
    if (is_yes == 'y' || is_yes == 'Y') {
        imageChannel.CustomImageEndpoint(name, CV_8U);
    }
}
