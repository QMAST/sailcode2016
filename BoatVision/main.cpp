#include <opencv2/core/core.hpp>
#include<opencv2/objdetect/objdetect.hpp>
#include<opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include<iostream>

using namespace cv;
using namespace std;

void videoCap()
{
    VideoCapture camera;
    namedWindow("CamerOP",CV_WINDOW_AUTOSIZE);
    bool b =0;

    camera.open(0);
    if(!camera.isOpened())
    {
        std::cout<<"Camera did not open";
        exit(1);
    }

    camera.set(CV_CAP_PROP_FRAME_WIDTH, 640);
    camera.set(CV_CAP_PROP_FRAME_HEIGHT, 480);
    int k;
    while (k != 'ESC')
    {
        // Grab the next camera frame.
        Mat cframe;
        b=camera.read(cframe);
        if (!b)
        {
            std::cerr << "ERROR: Couldn't grab a camera frame." <<std::endl;
            exit(1);
        }

        imshow("CamerOP",cframe);

        k=waitKey(0);
    }
}

// Input = source image
// Output = binary image with detected circles in white, background in black
Mat detectCircles(Mat src)
{
    Mat gray;

    // Convert to grayscale
    cvtColor( src, gray, CV_BGR2GRAY );

    // Blur to reduce noise
    GaussianBlur( gray, gray, Size(9, 9), 2, 2 );

    vector<Vec3f> circles;

    // Apply the Hough Transform to find the circles
    // Set dp=1 so the accumulator has the same resolution as the input image
    // minDistance = Minimum distance between the centers of detected circles
    // param1 = higher threshold of the two passed to the Canny()edge detector (the lower one is twice smaller).
    // param2 = accumulator threshold for the circle centers at the detection stage (smaller it is, the more false circles may be detected)
    // min/max radius are min and max circle radius (set max to 0 for no max)
    double dp = 1;
    double minDistance = 10;
    double param1 = 110;
    double param2 = 35;
    int minRadius = 0;
    int maxRadius = 0;
    HoughCircles( gray, circles, CV_HOUGH_GRADIENT, dp, minDistance, param1, param2, minRadius, maxRadius);

    //Create a grayscale image the size of the source image
    Mat binaryImage = Mat::zeros(Size(gray.cols, gray.rows), 0);

    // Draw the circles detected onto the binary image
    // Detected circles = white, background = black
    for(int i = 0; i < circles.size(); i++ )
    {
        Point center(cvRound(circles[i][0]), cvRound(circles[i][1]));
        int radius = cvRound(circles[i][2]);
        circle(binaryImage, center, radius, Scalar(255,255,255), -1, 8, 0 );

    }

    return binaryImage;
}

// Input = source image
// Output = Binary image where white = detected red blobs, black = background
Mat detectColour(Mat src)
{
    // Blur original image to reduce noise
    medianBlur(src, src, 5);

    // Convert to HSV
    Mat hsvImage;
    cvtColor(src, hsvImage, CV_BGR2HSV);

    // Threshold image by lower and upper red hue ranges
    Mat lowerRedHueRange, upperRedHueRange;
    inRange(hsvImage, Scalar(0, 100, 100), Scalar(10, 255, 255), lowerRedHueRange);
    inRange(hsvImage, Scalar(160, 100, 100), Scalar(179, 255, 255), upperRedHueRange);

    //Uncomment to show lower and upper range images
    //imshow("Lower Range", lowerRedHueRange);
    //imshow("Upper Range", upperRedHueRange);

    // Get weighted result
    // Alpha = weight of first array
    // Beta = weight of second array
    // Gamma = scalar added to each sum
    Mat weightedResult;
    double alpha = 1.0;
    double beta = 1.0;
    double gamma = 0;
    addWeighted(lowerRedHueRange, alpha, upperRedHueRange, beta,gamma, weightedResult);

    return weightedResult;
}

int main()
{
    Mat src = imread( "buoy.png", 1 );
    resize(src, src, Size(src.cols*0.5,src.rows*0.5), 2, 2, INTER_CUBIC);
    Mat circles = detectCircles(src);
    Mat red = detectColour(src);

    imshow("Original Image", src);
    imshow("Detected Circles", circles);
    imshow("Detected Colour", red);

    // Get union of the detected circle and colour images
    Mat unionResult;
    bitwise_and(circles, red, unionResult);
    imshow("Union", unionResult);

    // Set any non-detected objects in src image to black
    Mat dst = src.clone();
    for(int i = 0; i < unionResult.rows; i++)
    {
        for(int j = 0; j < unionResult.cols; j++)
        {
            if(unionResult.at<unsigned char>(i,j) == 0)
            {
                dst.at<Vec3b>(i,j) = Vec3b(0,0,0);
            }
        }
    }

    imshow("Final Result", dst);

    waitKey();

/*
    VideoCapture camera;
    namedWindow("CamerOP",CV_WINDOW_AUTOSIZE);
    bool b =0;

    camera.open(0);
    if(!camera.isOpened())
    {
        std::cout<<"Camera did not open";
        exit(1);
    }

    //camera.set(CV_CAP_PROP_FRAME_WIDTH, 640);
    //camera.set(CV_CAP_PROP_FRAME_HEIGHT, 480);
    namedWindow("CamerOP");

    while (true)
    {
        // Grab the next camera frame.
        Mat cframe;
        b=camera.read(cframe);
        if (!b)
        {
            std::cerr << "ERROR: Couldn't grab a camera frame." <<std::endl;
            exit(1);
        }

        imshow("CamerOP",cframe);

        Mat src = cframe.clone();
    Mat circles = detectCircles(src);
    Mat red = detectColour(src);

    //imshow("Original Image", src);
    //imshow("Detected Circles", circles);
    //imshow("Detected Colour", red);

    // Get union of the detected circle and colour images
    Mat unionResult;
    bitwise_and(circles, red, unionResult);
    imshow("Union", unionResult);

    // Set any non-detected objects in src image to black
    Mat dst = src.clone();
    for(int i = 0; i < unionResult.rows; i++)
    {
        for(int j = 0; j < unionResult.cols; j++)
        {
            if(unionResult.at<unsigned char>(i,j) == 0)
            {
                dst.at<Vec3b>(i,j) = Vec3b(0,0,0);
            }
        }
    }

    imshow("Final Result", dst);
    }
*/
    return 1;
}
