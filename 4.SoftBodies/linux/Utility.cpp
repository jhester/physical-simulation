//----------------------------------------------
//    Implementation of general utility routines
//----------------------------------------------
//
//  Programmer: Donald House
//  Date: March 8, 1999
//
//  Copyright (C) - Donald H. House. 2005
//

using namespace std;

#include "Utility.h"

/*
  computes sqrt(a^2 + b^2) without destructive underflow or overflow
*/
double pythag(double a, double b)
{
  double absa, absb;

  absa = Abs(a);
  absb = Abs(b);

  if(absa > absb)
    return absa * sqrt(1.0 + Sqr(absb / absa));
  else if(absb > 0)
    return absb * sqrt(1.0 + Sqr(absa / absb));
  else
    return 0;
}

/*
  Utility message routines
*/
void prompt(char *s)
{
  cout << s << " ";
}

void message(char *s1, char *s2, char *s3)
{
  cout << s1;
  if(s2 != NULL && strlen(s2) > 0)
    cout << " " << s2;
  if(s3 != NULL && strlen(s3) > 0)
    cout << " " << s3;
  cout << endl;
}

void status(char *s1, char *s2, char *s3)
{
  cout << "Status: ";
  message(s1, s2, s3);
}

void error(char *s1, char *s2, char *s3)
{
  cerr << "Error: ";
  cerr << s1;
  if(s2 != NULL && strlen(s2) > 0)
    cerr << " " << s2;
  if(s3 != NULL && strlen(s3) > 0)
    cerr << " " << s3;
  cerr << endl;
}

void abort(char *s1, char *s2, char *s3)
{
  error(s1, s2, s3);
  exit(1);
}
