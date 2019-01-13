#include "AppManager.h"

int main(int argc, char **argv)
{
	AppManager *iWar = new AppManager();

	if (iWar->Init())
	{
		iWar->Run();
		iWar->Destroy();
	}

	return 0;
}