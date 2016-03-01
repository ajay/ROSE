#ifndef planner_h
#define planner_h

class planner {
public:
	planner(Rose *robot, chili_landmarks *landmarks);

	void start_planning(void);

	void stop_planning(void);

	Rose *robot;
	chili_landmarks *landmarks;
	std::thread planning_thread;
};

#endif