#include <iostream>
#include <fstream>
#include <cstring>


int main(int argc, char* argv[]) {
	
	if(argc != 3) {
		std::cerr << "Error expected 2 arguments but got"<< argc-1  << std::endl;
		return 1;
	}
	
	std::string sourcePath = argv[1];
	std::string destPath = argv[2];

	std::ifstream sourceFile(sourcePath, std::ios::binary);
	if(!sourceFile.is_open()) {
		std::cerr << "Cannot open source file" << std::endl;
		return 1;
	
	}

	std::ofstream destFile(destPath, std::ios::binary | std::ios::trunc);
	if(!destFile.is_open()) {
		std::cerr << "error cannot create dest file " << std::endl;
		sourceFile.close();
		return 1;
	}

	destFile << sourceFile.rdbuf();

	if(!destFile.good()){
		std::cerr << "Error failed to write in dest file" << std::endl;
		sourceFile.close();
		destFile.close();
		return 1;
	
	}

	sourceFile.close();
	destFile.close();

	return 0;

}
