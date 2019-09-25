
#include <stdio.h>
#include <grpcpp/grpcpp.h>
#include "compiled/image.grpc.pb.h"

#include <stdlib.h>
#include <string> // for conversion from bytes

#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

typedef unsigned char byte;

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;

using std::cout;
using std::cin;
using std::endl;

class ImageServiceImpl final : public NLImageService::Service {

private:
    /*
     * converts from cv::Mat type image to NLImage
     */
    void deconstructImage(cv::Mat image, NLImage *reply) {
        reply->set_height(image.rows);
        reply->set_width(image.cols);
        reply->set_color(image.type());
        // reply.set_color(image.type()); // TODO align with bool values, not just in
        // calculate total number of elements needed for data array
        unsigned long length = image.total()*image.elemSize()/sizeof(byte);
        char *imgBytes = (char*)malloc(length);
        memcpy(imgBytes, image.data, length);
        std::string img_string(imgBytes);

        reply->set_data(img_string);

        free(imgBytes);
        // free(image.data); // malloc in reconstructImage
    }

    cv::Mat reconstructImage(NLImage image_message) {
        int height = image_message.height();
        int width = image_message.width();

        byte *imgData = (byte*)malloc(sizeof(image_message.data()[0])*image_message.data().length()); //(byte*)image_message.data().c_str();
        memcpy(imgData, image_message.data().c_str(), image_message.data().size());

        cv::Mat frame(height, width, image_message.color(), imgData); // can also add length

        return frame;
    }

public:
    /*
     * Rotates an image by a factor of 90 degrees, postivie moving in the counter
     * clockwise direction.
     */
    Status RotateImage(ServerContext* context, const NLImageRotateRequest* request, NLImage* reply) {
        double angle = (*request).rotation()*90;

        cv::Mat src = reconstructImage((*request).image()); // should return the whole image class

        // get rotation matrix for rotating the image around its center in pixel coordinates
        double imageHeight = (src.rows-1)/2.0;
        double imageWidth = (src.cols-1)/2.0;

        cv::Point2f center(imageWidth, imageHeight);
        cv::Mat rot = cv::getRotationMatrix2D(center, angle, 1.0);

        // determine bounding rectangle, center not relevant
        cv::Rect2f bbox = cv::RotatedRect(cv::Point2f(), src.size(), angle).boundingRect2f();

        // adjust transformation matrix
        rot.at<double>(0,2) += bbox.width/2.0 - src.cols/2.0;
        rot.at<double>(1,2) += bbox.height/2.0 - src.rows/2.0;

        cv::Mat dst;
        cv::warpAffine(src, dst, rot, bbox.size());

        deconstructImage(dst, reply);

        return Status::OK;
    }

    /*
     * Custom endpoint fixes images that have dead pixels by applying a blur to
     * the photo
    */
    Status MedianBlurFilter(ServerContext* context, const NLBlurImageEndpointRequest* request, NLImage* reply) {
        // read in image
        cv::Mat src = reconstructImage((*request).image());

        if ((*request).kernel() % 2 == 0) {
            // kernel must be >= 3 and not divisible by 2
            printf("Kernel size %i is invalid, exiting\n", (*request).kernel());
            return Status::CANCELLED;
        }

        // src, dst, kernel
        cv::medianBlur(src, src, (*request).kernel());

        // places blurred photo into reply NLImage protobuf
        deconstructImage(src, reply);

        return Status::OK;
    }

    /*
     * Custom endpoint converts an image into ascii art, and returns the art
     * as an ascii string.
    */
    Status CustomImageEndpoint(ServerContext* context, const NLCustomImageEndpointRequest* request, NLCustomImageEndpointResponse* reply) {
        // read in image
        cv::Mat src = reconstructImage((*request).image());

        if (src.rows > 200 || src.cols > 250) {
            reply->mutable_image()->set_data("It looks like your image may be quite large (>200x250px). We recommend reducing the size of the image and trying again.\n");
            return Status::OK;
        }

        const uchar* const table = src.data;

        // makes sure it's  char and not float color values
        CV_Assert(src.depth() == CV_8U);

        int length = src.total()+src.rows;
        char *ascii_art = (char*)malloc(length);

        // setup addresses for each character
        char _B[] = {'B'};
        char _0[] = {'0'};
        char _I[] = {'I'};
        char _p[] = {'.'};
        char _s[] = {' '};
        char _n[] = {'\n'};

        const int channels = src.channels();

        switch(channels)
        {
        case 1:
            {
                int counter = 0;

                for( int i = 0; i < src.rows - 1; i += 2) {
                    for( int j = 0; j < src.cols; ++j ) {
                        // calculate intensity value for both pixels
                        int touple = (src.at<uchar>(i,j) + src.at<uchar>(i,j)) / 2;
                        // add corrisponding char to ascii art
                        if (src.at<uchar>(i,j) > 185) {
                            ascii_art[counter++] = *_B;
                        } else if (src.at<uchar>(i,j) > 150) {
                            ascii_art[counter++] = *_0;
                        } else if (src.at<uchar>(i,j) > 100) {
                            ascii_art[counter++] = *_I;
                        } else if (src.at<uchar>(i,j) > 45) {
                            ascii_art[counter++] = *_p;
                        } else {
                            ascii_art[counter++] = *_s;
                        }
                    }
                    ascii_art[counter++] = *_n;
                }
                break;
            }
        case 3:
            {
                cout << "There is no support for 3-channel images at this time" << endl;
                break;
            }
        default:
            cout << "Must be a single channel image, you have " << channels << " channels." << endl;
        }

        // cout << ascii_art << endl;

        // convert to a type that can be saved into the protobuf object
        char* ar = (char*)malloc(sizeof(char)*length);
        memcpy(ar, ascii_art, length);
        std::string str(ar);
        reply->mutable_image()->set_data(str);

        free(ascii_art);

        return Status::OK;
    }
};


void runServer() {

    printf("Runserver function\n");

    std::string server_address("0.0.0.0:50051");
    ImageServiceImpl service;

    grpc::ServerBuilder builder;
    // Listen on the given address without any authentication mechanism.
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
    // Register "service" as the instance through which we'll communicate with
    // clients. In this case it corresponds to an *synchronous* service.
    builder.RegisterService(&service);
    // Finally assemble the server.
    std::unique_ptr<Server> server(builder.BuildAndStart());
    std::cout << "Server listening on " << server_address << std::endl;

    // Wait for the server to shutdown. Note that some other thread must be
    // responsible for shutting down the server for this call to ever return.
    server->Wait();

}


int main() {
    printf("server.cpp starting.. \n");

    runServer();

    return 0;
}
