#include "complex.h"
#include <cmath>

Complex::Complex(): real(0.0), imagin(0.0) {}
Complex::Complex(double real, double imagin): real(real), imagin(imagin) {}

double Complex::getReal() const {
	return real;
}

double Complex::getImagin() const {
	return imagin;
}

Complex Complex::operator+(const Complex& other) const {
	return Complex(real + other.real, imagin + other.imagin);
}

Complex Complex::operator-(const Complex& other) const {
	return Complex(real - other.real, imagin - other.imagin);
}

Complex Complex::operator*(double scalar) const {
	return Complex(real * scalar, imagin * scalar);
}

double Complex::absolute() const {
	return std::sqrt(real * real + imagin * imagin);
}

bool Complex::operator<(const Complex& other) const {
	return this->absolute() < other.absolute();
}

bool Complex::operator>(const Complex& other) const {
	return this->absolute() > other.absolute();
}

bool Complex::operator==(const Complex& other) const {
	return (real == other.real) && (imagin == other.imagin);
}
