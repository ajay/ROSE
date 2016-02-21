#ifndef CHILI_LANDMARKS_H
#define CHILI_LANDMARKS_H

class chili_landmarks
{
	public:
		chili_landmarks();
		~chili_landmarks();
		void update();

		double tags [1024][3];
};

#endif