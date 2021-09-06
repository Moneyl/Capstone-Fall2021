#include <iostream>
#include "Application.h"

int main(int argc, char* argv[])
{
	Application app;
	bool result = app.Run();
	return result ? EXIT_SUCCESS : EXIT_FAILURE;
}
