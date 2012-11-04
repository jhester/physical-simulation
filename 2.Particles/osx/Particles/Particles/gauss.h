/*
 gauss.h - header file for Gaussian Random Number Generator
 
 Donald H. House     	July 1, 1982
 conversion to C -- Nov. 30, 1989
 tweaks to make C++ compatible -- Aug. 26, 2005
 
 This function takes as parameters real valued mean and standard-deviation,
 and an integer valued seed.  It returns a real number which may be
 interpreted as a sample of a normally distributed (Gaussian) random
 variable with the specified mean and standard deviation.
 After the first call to gauss, the seed parameter is ignored.
 
 Your program must #include <cmath>, and link -lm
*/
#ifndef _GAUSS_H
#define _GAUSS_H

double gauss(double mean, double std, int seed);

#endif