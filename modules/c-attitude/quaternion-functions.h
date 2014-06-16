
// ****** find rotation matrix from rotation vector
void Rv2Rot(float Rv[3], float R[3][3]);

// ****** find roll, pitch, yaw from quaternion ********
void Quaternion2RPY(const float q[4], float rpy[3]);

// ****** find quaternion from roll, pitch, yaw ********
void RPY2Quaternion(const float rpy[3], float q[4]);

// ** Find Rbe, that rotates a vector from earth fixed to body frame, from quaternion **
void Quaternion2R(float q[4], float Rbe[3][3]);

// ****** convert Rotation Matrix to Quaternion ********
// ****** if R converts from e to b, q is rotation from e to b ****
void R2Quaternion(float R[3][3], float q[4]);

// ****** Rotation Matrix from Two Vector Directions ********
// ****** given two vector directions (v1 and v2) known in two frames (b and e) find Rbe ***
// ****** solution is approximate if can't be exact ***
uint8_t RotFrom2Vectors(const float v1b[3], const float v1e[3], const float v2b[3], const float v2e[3], float Rbe[3][3]);

// ****** Vector Cross Product ********
void CrossProduct(const float v1[3], const float v2[3], float result[3]);

// ****** Vector Magnitude ********
float VectorMagnitude(const float v[3]);

void quat_inverse(float q[4]);
void quat_copy(const float q[4], float qnew[4]);
void quat_mult(const float q1[4], const float q2[4], float qout[4]);
void rot_mult(float R[3][3], const float vec[3], float vec_out[3]);

