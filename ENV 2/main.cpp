#include <iostream>
#include "complex.h"
#include "sort.h"

int main() {
	int size = 5;
	Complex numbers[size] = {
		Complex(3.0, 4.0),
		Complex(1.0, 1.0),
		Complex(0.0, 5.0),
		Complex(-2.0, -2.0),
		Complex(1.0, 0.0)
	
	};

	std::cout << "Original array" << std::endl;
	printArray(numbers, size);

	bubbleSort(numbers, size);
	std::cout << "Sorted array" << std::endl;
	printArray(numbers, size);

	Complex a(2.0, 3.0);
	Complex b(1.0, -1.0);
	Complex sum = a + b;
	Complex diff = a - b;
	Complex mult = a * 2.0;
	
	std::cout << "b = (" << b.getReal() << "+" << b.getImagin() << "i)"
		<< std::endl;

	std::cout << "a = (" << a.getReal() << "+" << a.getImagin() << "i)" 
	       << std::endl;	
	
	std::cout << "a + b = (" << sum.getReal() << "+" << sum.getImagin()
	       << "i)" << std::endl;

	std::cout << "a - b = (" << diff.getReal() << "+" << diff.getImagin() 
	<< "i)" << std::endl;
	
	std::cout << "a * 2 = (" << mult.getReal() << "+" << mult.getImagin() 
	<< "i)" << std::endl;
	std::cout << "absolute a = " << a.absolute() << std::endl;

	return 0;	


}
