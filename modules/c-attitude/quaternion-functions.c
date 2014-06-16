//--------------------------------------------------------------------
// This file was created as a part of the LinuxDrone project:
//                http://www.linuxdrone.org
//
// This work is based on the original code from the OpenPilot project
// which is licensed under CC-BY-SA terms. See http://www.openpilot.org
//
// Distributed under the Creative Commons Attribution-ShareAlike 4.0
// International License (see accompanying License.txt file or a copy
// at http://creativecommons.org/licenses/by-sa/4.0/legalcode)
//
// The human-readable summary of (and not a substitute for) the
// license: http://creativecommons.org/licenses/by-sa/4.0/
//--------------------------------------------------------------------

#include <math.h>
#include <stdint.h>
#include "quaternion-functions.h"

#define MIN_ALLOWABLE_MAGNITUDE 1e-30f

#define M_PI_F       3.14159265358979323846264338328f      /* pi */
#define M_PI_D       3.14159265358979323846264338328d      /* pi */
// Conversion macro
#define RAD2DEG(rad)            ((rad) * (180.0f / M_PI_F))
#define DEG2RAD(deg)            ((deg) * (M_PI_F / 180.0f))

#define RAD2DEG_D(rad)          ((rad) * (180.0d / M_PI_D))
#define DEG2RAD_D(deg)          ((deg) * (M_PI_D / 180.0d))


// ****** find roll, pitch, yaw from quaternion ********
void Quaternion2RPY(const float q[4], float rpy[3])
{
    float R13, R11, R12, R23, R33;
    float q0s = q[0] * q[0];
    float q1s = q[1] * q[1];
    float q2s = q[2] * q[2];
    float q3s = q[3] * q[3];

    R13    = 2.0f * (q[1] * q[3] - q[0] * q[2]);
    R11    = q0s + q1s - q2s - q3s;
    R12    = 2.0f * (q[1] * q[2] + q[0] * q[3]);
    R23    = 2.0f * (q[2] * q[3] + q[0] * q[1]);
    R33    = q0s - q1s - q2s + q3s;

    rpy[1] = RAD2DEG(asinf(-R13)); // pitch always between -pi/2 to pi/2
    rpy[2] = RAD2DEG(atan2f(R12, R11));
    rpy[0] = RAD2DEG(atan2f(R23, R33));

    // TODO: consider the cases where |R13| ~= 1, |pitch| ~= pi/2
}

// ****** find quaternion from roll, pitch, yaw ********
void RPY2Quaternion(const float rpy[3], float q[4])
{
    float phi, theta, psi;
    float cphi, sphi, ctheta, stheta, cpsi, spsi;

    phi    = DEG2RAD(rpy[0] / 2);
    theta  = DEG2RAD(rpy[1] / 2);
    psi    = DEG2RAD(rpy[2] / 2);
    cphi   = cosf(phi);
    sphi   = sinf(phi);
    ctheta = cosf(theta);
    stheta = sinf(theta);
    cpsi   = cosf(psi);
    spsi   = sinf(psi);

    q[0]   = cphi * ctheta * cpsi + sphi * stheta * spsi;
    q[1]   = sphi * ctheta * cpsi - cphi * stheta * spsi;
    q[2]   = cphi * stheta * cpsi + sphi * ctheta * spsi;
    q[3]   = cphi * ctheta * spsi - sphi * stheta * cpsi;

    if (q[0] < 0) { // q0 always positive for uniqueness
        q[0] = -q[0];
        q[1] = -q[1];
        q[2] = -q[2];
        q[3] = -q[3];
    }
}

// ** Find Rbe, that rotates a vector from earth fixed to body frame, from quaternion **
void Quaternion2R(float q[4], float Rbe[3][3])
{
    float q0s = q[0] * q[0], q1s = q[1] * q[1], q2s = q[2] * q[2], q3s = q[3] * q[3];

    Rbe[0][0] = q0s + q1s - q2s - q3s;
    Rbe[0][1] = 2 * (q[1] * q[2] + q[0] * q[3]);
    Rbe[0][2] = 2 * (q[1] * q[3] - q[0] * q[2]);
    Rbe[1][0] = 2 * (q[1] * q[2] - q[0] * q[3]);
    Rbe[1][1] = q0s - q1s + q2s - q3s;
    Rbe[1][2] = 2 * (q[2] * q[3] + q[0] * q[1]);
    Rbe[2][0] = 2 * (q[1] * q[3] + q[0] * q[2]);
    Rbe[2][1] = 2 * (q[2] * q[3] - q[0] * q[1]);
    Rbe[2][2] = q0s - q1s - q2s + q3s;
}

// ****** convert Rotation Matrix to Quaternion ********
// ****** if R converts from e to b, q is rotation from e to b ****
void R2Quaternion(float R[3][3], float q[4])
{
    float m[4], mag;
    uint8_t index, i;

    m[0]  = 1 + R[0][0] + R[1][1] + R[2][2];
    m[1]  = 1 + R[0][0] - R[1][1] - R[2][2];
    m[2]  = 1 - R[0][0] + R[1][1] - R[2][2];
    m[3]  = 1 - R[0][0] - R[1][1] + R[2][2];

    // find maximum divisor
    index = 0;
    mag   = m[0];
    for (i = 1; i < 4; i++) {
        if (m[i] > mag) {
            mag   = m[i];
            index = i;
        }
    }
    mag = 2 * sqrtf(mag);

    if (index == 0) {
        q[0] = mag / 4;
        q[1] = (R[1][2] - R[2][1]) / mag;
        q[2] = (R[2][0] - R[0][2]) / mag;
        q[3] = (R[0][1] - R[1][0]) / mag;
    } else if (index == 1) {
        q[1] = mag / 4;
        q[0] = (R[1][2] - R[2][1]) / mag;
        q[2] = (R[0][1] + R[1][0]) / mag;
        q[3] = (R[0][2] + R[2][0]) / mag;
    } else if (index == 2) {
        q[2] = mag / 4;
        q[0] = (R[2][0] - R[0][2]) / mag;
        q[1] = (R[0][1] + R[1][0]) / mag;
        q[3] = (R[1][2] + R[2][1]) / mag;
    } else {
        q[3] = mag / 4;
        q[0] = (R[0][1] - R[1][0]) / mag;
        q[1] = (R[0][2] + R[2][0]) / mag;
        q[2] = (R[1][2] + R[2][1]) / mag;
    }

    // q0 positive, i.e. angle between pi and -pi
    if (q[0] < 0) {
        q[0] = -q[0];
        q[1] = -q[1];
        q[2] = -q[2];
        q[3] = -q[3];
    }
}

// ****** Rotation Matrix from Two Vector Directions ********
// ****** given two vector directions (v1 and v2) known in two frames (b and e) find Rbe ***
// ****** solution is approximate if can't be exact ***
uint8_t RotFrom2Vectors(const float v1b[3], const float v1e[3], const float v2b[3], const float v2e[3], float Rbe[3][3])
{
    float Rib[3][3], Rie[3][3];
    float mag;
    uint8_t i, j, k;

    // identity rotation in case of error
    for (i = 0; i < 3; i++) {
        for (j = 0; j < 3; j++) {
            Rbe[i][j] = 0;
        }
        Rbe[i][i] = 1;
    }

    // The first rows of rot matrices chosen in direction of v1
    mag = VectorMagnitude(v1b);
    if (fabsf(mag) < MIN_ALLOWABLE_MAGNITUDE) {
        return -1;
    }
    for (i = 0; i < 3; i++) {
        Rib[0][i] = v1b[i] / mag;
    }

    mag = VectorMagnitude(v1e);
    if (fabsf(mag) < MIN_ALLOWABLE_MAGNITUDE) {
        return -1;
    }
    for (i = 0; i < 3; i++) {
        Rie[0][i] = v1e[i] / mag;
    }

    // The second rows of rot matrices chosen in direction of v1xv2
    CrossProduct(v1b, v2b, &Rib[1][0]);
    mag = VectorMagnitude(&Rib[1][0]);
    if (fabsf(mag) < MIN_ALLOWABLE_MAGNITUDE) {
        return -1;
    }
    for (i = 0; i < 3; i++) {
        Rib[1][i] = Rib[1][i] / mag;
    }

    CrossProduct(v1e, v2e, &Rie[1][0]);
    mag = VectorMagnitude(&Rie[1][0]);
    if (fabsf(mag) < MIN_ALLOWABLE_MAGNITUDE) {
        return -1;
    }
    for (i = 0; i < 3; i++) {
        Rie[1][i] = Rie[1][i] / mag;
    }

    // The third rows of rot matrices are XxY (Row1xRow2)
    CrossProduct(&Rib[0][0], &Rib[1][0], &Rib[2][0]);
    CrossProduct(&Rie[0][0], &Rie[1][0], &Rie[2][0]);

    // Rbe = Rbi*Rie = Rib'*Rie
    for (i = 0; i < 3; i++) {
        for (j = 0; j < 3; j++) {
            Rbe[i][j] = 0;
            for (k = 0; k < 3; k++) {
                Rbe[i][j] += Rib[k][i] * Rie[k][j];
            }
        }
    }

    return 1;
}

void Rv2Rot(float Rv[3], float R[3][3])
{
    // Compute rotation matrix from a rotation vector
    // To save .text space, uses Quaternion2R()
    float q[4];

    float angle = VectorMagnitude(Rv);

    if (angle <= 0.00048828125f) {
        // angle < sqrt(2*machine_epsilon(float)), so flush cos(x) to 1.0f
        q[0] = 1.0f;

        // and flush sin(x/2)/x to 0.5
        q[1] = 0.5f * Rv[0];
        q[2] = 0.5f * Rv[1];
        q[3] = 0.5f * Rv[2];
        // This prevents division by zero, while retaining full accuracy
    } else {
        q[0] = cosf(angle * 0.5f);
        float scale = sinf(angle * 0.5f) / angle;
        q[1] = scale * Rv[0];
        q[2] = scale * Rv[1];
        q[3] = scale * Rv[2];
    }

    Quaternion2R(q, R);
}

// ****** Vector Cross Product ********
void CrossProduct(const float v1[3], const float v2[3], float result[3])
{
    result[0] = v1[1] * v2[2] - v2[1] * v1[2];
    result[1] = v2[0] * v1[2] - v1[0] * v2[2];
    result[2] = v1[0] * v2[1] - v2[0] * v1[1];
}

// ****** Vector Magnitude ********
float VectorMagnitude(const float v[3])
{
    return sqrtf(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
}

/**
 * @brief Compute the inverse of a quaternion
 * @param [in][out] q The matrix to invert
 */
void quat_inverse(float q[4])
{
    q[1] = -q[1];
    q[2] = -q[2];
    q[3] = -q[3];
}

/**
 * @brief Duplicate a quaternion
 * @param[in] q quaternion in
 * @param[out] qnew quaternion to copy to
 */
void quat_copy(const float q[4], float qnew[4])
{
    qnew[0] = q[0];
    qnew[1] = q[1];
    qnew[2] = q[2];
    qnew[3] = q[3];
}

/**
 * @brief Multiply two quaternions into a third
 * @param[in] q1 First quaternion
 * @param[in] q2 Second quaternion
 * @param[out] qout Output quaternion
 */
void quat_mult(const float q1[4], const float q2[4], float qout[4])
{
    qout[0] = q1[0] * q2[0] - q1[1] * q2[1] - q1[2] * q2[2] - q1[3] * q2[3];
    qout[1] = q1[0] * q2[1] + q1[1] * q2[0] + q1[2] * q2[3] - q1[3] * q2[2];
    qout[2] = q1[0] * q2[2] - q1[1] * q2[3] + q1[2] * q2[0] + q1[3] * q2[1];
    qout[3] = q1[0] * q2[3] + q1[1] * q2[2] - q1[2] * q2[1] + q1[3] * q2[0];
}

/**
 * @brief Rotate a vector by a rotation matrix
 * @param[in] R a three by three rotation matrix (first index is row)
 * @param[in] vec the source vector
 * @param[out] vec_out the output vector
 */
void rot_mult(float R[3][3], const float vec[3], float vec_out[3])
{
    vec_out[0] = R[0][0] * vec[0] + R[0][1] * vec[1] + R[0][2] * vec[2];
    vec_out[1] = R[1][0] * vec[0] + R[1][1] * vec[1] + R[1][2] * vec[2];
    vec_out[2] = R[2][0] * vec[0] + R[2][1] * vec[1] + R[2][2] * vec[2];
}
