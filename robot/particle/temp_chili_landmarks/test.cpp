#include "chili_landmarks.h"
#include <thread>

static chili_landmarks chili;

void update_chili()
{
	chili.update();
}

int main()
{
	std::thread detect(update_chili);

	while (1)
	{
		for (int i = 0; i<1024; i++)
		{
			if (chili.tags[i][0] != 0)
			{
				printf("id: %d\tx: %0.2f\ty: %0.2f\n", i, chili.tags[i][1], chili.tags[i][2]);
			}
		}
	}
}