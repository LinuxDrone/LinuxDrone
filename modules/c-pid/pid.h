//--------------------------------------------------------------------
// This file was created as a part of the LinuxDrone project:
//                http://www.linuxdrone.org
//
// Distributed under the Creative Commons Attribution-ShareAlike 4.0
// International License (see accompanying License.txt file or a copy
// at http://creativecommons.org/licenses/by-sa/4.0/legalcode)
//
// The human-readable summary of (and not a substitute for) the
// license: http://creativecommons.org/licenses/by-sa/4.0/
//--------------------------------------------------------------------

#pragma once

#define DR_PI 3.14159274101257f

typedef struct pidParams {
    double Kp;
    double Ki;
    double Kd;
    double iLimit;
    double dT;
    double iSum;
    double lastError;
} pidParams_t;


double calcPID(pidParams_t *p, double error, double dT);

void setPID(pidParams_t *p, double Kp, double Ki, double Kd, double iLimit);

void zeroPID(pidParams_t *p);

/*
void setKp(double Kp);
void setKi(double Ki);
void setKd(double Kd);
void setiLimit(double iLimit);

double getKp();
double getKi();
double getKd();
double getiLimit();
*/
