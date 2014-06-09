
#include <math.h>
//#include <stdint.h>
#include "pid.h"

double calcPID(pidParams_t *p, double error, double dT)
{
  // Сalculation of the integral unit
  p->iSum += p->Ki * error * dT;

  // Limit on the magnitude of the integral
  if (p->iSum > p->iLimit) {
     p->iSum = p->iLimit;
  } else if (p->iSum < -p->iLimit) {
      p->iSum = -p->iLimit;
  }

  // Calculation of the differential unit
  double diff = p->Kd * (error - p->lastError) / dT;
  p->lastError = error;

  return (p->Kp * error) + p->iSum + diff;
}

// Parameter setting
void setPID(pidParams_t *p, double Kp, double Ki, double Kd, double iLimit)
{
    p->Kp       = Kp;
    p->Ki       = Ki;
    p->Kd       = Kd;
    p->iLimit   = iLimit;
    p->dT       = 0.0f;
    p->iSum     = 0.0f;
    p->lastError = 0.0f;
}

void zeroPID(pidParams_t *p)
{
    p->iSum         = 0.0f;
    p->lastError    = 0.0f;
}

/*
void setKp(double Kp)
{
  m_Kp = Kp;
}

void setKi(double Ki)
{
  m_Ki = Ki;
}

void setKd(double Kd)
{
  m_Kd = Kd;
}

void setiLimit(double iLimit)
{
  m_iLimit = iLimit;
}

// Get parameter
double getKp()
{
  return m_Kp;
}

double getKi()
{
  return m_Ki;
}

double getKd()
{
  return m_Kd;
}

double getiLimit()
{
  return m_iLimit;
}
*/

