#ifndef COMPLEX_H
#define COMPLEX_H


class Complex {

private:
	double real;
	double imagin;

public:
	Complex();
	Complex(double real, double imagin);

	double getReal() const;
	double getImagin() const;

	Complex operator+(const Complex& other) const;
	Complex operator-(const Complex& other) const;
	Complex operator*(double scalar) const;

	double absolute() const;
	bool operator<(const Complex& other) const;
	bool operator==(const Complex& other) const;
	bool operator>(const Complex& other) const;

};

#endif
