#include "sort.h"
#include <iostream>

void bubbleSort(Complex* arr, int size) {
	for(int i = 0; i < size - 1; i++){
		for(int j = 0; j < size - i - 1; j++){
			if(arr[j] > arr[j + 1]) {
				Complex temp = arr[j];
				arr[j] = arr[j + 1];
				arr[j + 1]  = temp;
			
			}
		}
	}

}

void printArray(Complex* arr, int size) {
	for(int i = 0; i < size; i++) {
		std::cout << "(" << arr[i].getReal() << "+" 
		      << arr[i].getImagin() << "i)";

		if(i < size - 1){
			std::cout << ", ";
		}	
	}

	std::cout << std::endl;
}
