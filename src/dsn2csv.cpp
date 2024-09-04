// DSNTree.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>

#include <dsn2csv.hpp>

int main(int argc, char* argv[])
{
    DSNTreeApp app(false);

    auto ret = app.Arguments(argc, argv);
	if (ret.length() != 0 && ret != "?")
		return -1;
	if (ret == "?")		// help was requested and usage has been printed
		return 0;
	try {
		app.Run(); }
	catch (const std::exception& e) {
		std::cout << "Execution error " << e.what() << std::endl;
		return -1; }
	return 0;
}
