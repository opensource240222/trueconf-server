

#include "std/cpplib/VS_Cpu.h"
#include <iostream>



int main(int argc, char** argv) {

	char model_name[1024];
	VS_GetCPUInternalName(model_name, sizeof(model_name));
	std::cout << "Cpu model name " << model_name << std::endl;

	unsigned phcores, lcores;

	VS_GetNumCPUCores(&phcores, &lcores);

	std::cout << "Physical cores = " << phcores << std::endl;
	std::cout << "Logical cores = " << lcores << std::endl;

	return -1;
}

