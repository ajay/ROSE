#include <cstdint>
#include <fstream>
#include <iostream>
#include <njson/json.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <signal.h>
#include <thread>

#include "arm.h"
#include "defs.h"

#define KEYID(k) ((k)-'a')

using namespace arma;
using namespace std;
using namespace cv;
using json = nlohmann::json;
static bool stopsig;
Arm arm;
thread ui;
static int iValue[6];
static vec sense;

double deg2rad(double deg)
{
	return deg * M_PI / 180.0;
}

double rad2deg(double rad)
{
	return rad * 180.0 / M_PI;
}

// These are the callbacks
void joint0callback(int iValue, void *userData)
{
	sense(0) = iValue - 90;
	arm.set_joint(sense(0), 0);
}

void joint1callback(int iValue, void *userData)
{
	sense(1) = iValue - 90;
	arm.set_joint(sense(1), 1);
}

void joint2callback(int iValue, void *userData)
{
	sense(2) = iValue;
	arm.set_joint(sense(2), 2);
}

void joint3callback(int iValue, void *userData)
{
	sense(3) = iValue - 90;
	arm.set_joint(sense(3), 3);
}

void joint4callback(int iValue, void *userData)
{
	sense(4) = iValue - 90;
	arm.set_joint(sense(4), 4);
}

void joint5callback(int iValue, void *userData)
{
	sense(5) = iValue;
	arm.set_joint(sense(5), 5);
}

void stopsignal(int)
{
	stopsig = true;
}

void run_ui(void)
{
	string winname = "Press 'q' to quit";
	namedWindow(winname);
	createTrackbar("Joint0", winname, &iValue[0], 180, joint0callback, NULL);
	createTrackbar("Joint1", winname, &iValue[1], 180, joint1callback, NULL);
	createTrackbar("Joint2", winname, &iValue[2], 180, joint2callback, NULL);
	createTrackbar("Joint3", winname, &iValue[3], 180, joint3callback, NULL);
	createTrackbar("Joint4", winname, &iValue[4], 180, joint4callback, NULL);
	createTrackbar("Claw", winname, &iValue[5], 180, joint5callback, NULL);
	cv::Mat bufimage(480, 720, CV_8UC3);
	char numbuf[64];
	for (;;)
	{
		bufimage = Scalar(255, 255, 255);

		vec values = sense + vec({ 90, 90, 0, 90, 90, 0 });

		// Display the value bars
		for (int i = 0; i < (int)values.n_elem; i++)
		{
			Point topleft(80, 20 + i * 80);
			Point btmright(values(i) + 80, 60 + i * 80);
			rectangle(bufimage, topleft, btmright, Scalar(255, 32, 10));
			sprintf(numbuf, "%0.2lf", sense(i));
			putText(bufimage, string(numbuf), Point(20, 60 + i * 80), FONT_HERSHEY_PLAIN, 1, Scalar(255, 32, 10), 1, 8, false);
		}

		// Display the forward kinematics
		arm.arm_read = sense;
		mat positions(3, 8, fill::zeros);
		for (int i = 0; i <= 6; i++)
		{
			vec eepos = arm.get_end_effector_pos(i);
			positions.col(i+1) = eepos;
			sprintf(numbuf, "[%0.2lf %0.2lf %0.2lf]", eepos(0), eepos(1), eepos(2));
			putText(bufimage, string(numbuf), Point(280, 60 + i * 60), FONT_HERSHEY_PLAIN, 1, Scalar(255, 32, 10), 1, 8, false);
		}

		// Draw shit
		vector<Scalar> colors = { Scalar(0,0,0), Scalar(255,0,128), Scalar(255,0,0), Scalar(255,128,0), Scalar(0,128,0), Scalar(0,128,255), Scalar(0,0,255) };
		for (int i = 0; i <= 6; i++)
		{
			vec pos1 = positions.col(i);
			pos1 = vec({ pos1(1), -pos1(2) });
			pos1 = 4 * pos1 + vec({ 560, 150 });
			vec pos2 = positions.col(i+1);
			pos2 = vec({ pos2(1), -pos2(2) });
			pos2 = 4 * pos2 + vec({ 560, 150 });
			line(bufimage, Point(pos1(0), pos1(1)), Point(pos2(0), pos2(1)), colors[i], 2);
		}

		imshow(winname, bufimage);
		int k = waitKey(30);
		if ((k & 0x7f) == 'q')
		{
			break;
		}
	}
}

void start_arm()
{
	arm.connect();
	if (!arm.connected())
	{
		printf("[ARM TEST] Not connected to anything, disconnecting...\n");
		arm.disconnect();
		exit(1);
	}

	arm.load_calibration_params("calib_params.json");
	if (!arm.calibrated())
	{
		arm.disconnect();
		exit(1);
	}
}

void stop_arm()
{
	arm.disconnect();
}

int main(int argc, char *argv[])
{
	signal(SIGINT, stopsignal);

	// Connect to the arm
	start_arm();
	sleep(1);
	vec values = arm.sense();
	arm.set_pose(values(0), values(1), values(2), values(3), values(4), values(5));
	sense = values;
	values += vec({ 90, 90, 0, 90, 90, 0 });
	iValue[0] = (values(0));
	iValue[1] = (values(1));
	iValue[2] = (values(2));
	iValue[3] = (values(3));
	iValue[4] = (values(4));
	iValue[5] = (values(5));

	// Run loop
	thread ui_thread(run_ui);
	ui_thread.join(); // wait until the ui is finished

	for (int i = 0; i < 100; i++)
	{
		arm.set_pose(0, 0, 0, 0, 0, 0, false);
	}

	stop_arm();
	return 0;
}
