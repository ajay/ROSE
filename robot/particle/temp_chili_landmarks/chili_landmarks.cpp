// Chilitags
#include <chilitags/chilitags.hpp>
#include "chili_landmarks.h"

// OpenCV
// #include <opencv2/core/core.hpp>
// #include <opencv2/core/core_c.h>
#include <opencv2/highgui/highgui.hpp>

// General
// #include <cmath>
// #include <cstring>
// #include <fstream>
// #include <iostream>
// #include <stdio.h>
// #include <string>
// #include <time.h>

chili_landmarks::chili_landmarks()
{
}

chili_landmarks::~chili_landmarks()
{
}

void chili_landmarks::update()
{
	// Paramaters for image aquisition
	int xRes = 640;
	int yRes = 480;
	int cameraIndex = 0;

	// Get camera feed
	cv::VideoCapture capture(cameraIndex);
	if (!capture.isOpened())
	{
		perror("Unable to initialise video capture.");
		exit(0);
	}

	// Set camera feed resolution
	capture.set(CV_CAP_PROP_FRAME_WIDTH, xRes);
	capture.set(CV_CAP_PROP_FRAME_HEIGHT, yRes);

	// Initialize variables to be used in loop
	cv::Mat inputImage;
	chilitags::Chilitags chilitags;
	// char tagInfo[256];

	// Window to display tracked chilitags
	// cv::namedWindow("DisplayChilitags");
	// static const cv::Scalar COLOR(255, 0, 255);
	// static const int SHIFT = 16;
	// static const float PRECISION = 1<<SHIFT;

	// Main loop, exiting when 'q is pressed'
	for (int framecount = 0; 'q' != (char) cv::waitKey(1); framecount++)
	{
		// Read input image and parse for chilitags
		capture.read(inputImage);

		// Start time for processing
		// int64 startTime = cv::getTickCount();

		// Process chilitags
		chilitags::TagCornerMap detected_tags = chilitags.find(inputImage);

		// Measure the processing time needed for the detection
		// int64 endTime = cv::getTickCount();
		// float processingTime = 1000.0f*((float) endTime - startTime)/cv::getTickFrequency();

		// Reset tagInfo string
	    // std::memset(tagInfo, 0, 256);

	    // Clone input image to label chilitags on & display
		// cv::Mat outputImage = inputImage.clone();

		for (int i=0; i<1024; i++)
		{
			this->tags[i][0] = 0;
		}

		for (const std::pair<int, chilitags::Quad> & tag : detected_tags)
		{
			// Get ID and corner points of chilitag
			int id = tag.first;
			const cv::Mat_<cv::Point2f> corners(tag.second);

			// Draw outline around detected chilitag
			// for (size_t i = 0; i < 4; ++i)
			// {
				// cv::line(outputImage, PRECISION*corners(i), PRECISION*corners((i+1)%4), COLOR, 1, CV_AA, SHIFT);
			// }

			// Calculate center based on two corner points
			cv::Point2f center = 0.5f*(corners(0) + corners(2));

			// Calculate x & y in local reference frame
			// Tag size = 9 inches (W)
			// Distance = 24 inches (D)
			// Pixel diagonal width = 200 px (P)
			// Focal length = F = (P x D) / W = 533.3333

			double delta_x = center.x - (xRes/2);

			double x_squared = pow(abs(corners(0).x - corners(2).x), 2);
			double y_squared = pow(abs(corners(0).y - corners(2).y), 2);
			double diagonal_size = sqrt(x_squared + y_squared);

			double distance_inches = 9 * 533.33 / diagonal_size;
			double x_inches = -delta_x * distance_inches / 533.33;
			double y_inches = sqrt(pow(distance_inches, 2) - pow(x_inches, 2));

			this->tags[id][0] = 1;
			this->tags[id][1] = x_inches;
			this->tags[id][2] = y_inches;

			// printf("id: %d\t", id);
			// printf("x: %0.3f\t\t", tags[id][1]);
			// printf("y: %0.3f\n", tags[id][2]);

			// Write tagInfo to center of tag in displayed window
			// sprintf(tagInfo, "id: %d x:%2.2f y:%2.2f", id, x_inches, y_inches);
			// cv::putText(outputImage, tagInfo, center, cv::FONT_HERSHEY_SIMPLEX, 0.5, CV_RGB(0,255,0), 2.0);
		}

		// Write stats on the current frame (resolution and processing time) to display window
        // cv::putText(outputImage, cv::format("%dx%d %4.0f ms (press q to quit)", outputImage.cols, outputImage.rows, processingTime), cv::Point(32,32), cv::FONT_HERSHEY_SIMPLEX, 0.5f, COLOR);

        // cv::circle(outputImage, cv::Point(320, 240), 15, (0, 0, 0));

        // Display chilitags window
        // cv::imshow("DisplayChilitags", outputImage);
	}

	// Release all used resources
	capture.release();
	// cv::destroyWindow("DisplayChilitags");
}

// int main()
// {
//     // std::thread detect(update);
//     // detect.join();
// }
