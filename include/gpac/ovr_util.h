#ifndef OVR_UTIL_H
#define OVR_UTIL_H

#include "math.h"		// for cosf(), sinf(), tanf()
#include "string.h"		// for memset()

#include <OVR_CAPI.h>

static inline ovrMatrix4f ovrMatrix4f_Transpose(const ovrMatrix4f * a)
{
	ovrMatrix4f out;
	out.M[0][0] = a->M[0][0]; out.M[0][1] = a->M[1][0]; out.M[0][2] = a->M[2][0]; out.M[0][3] = a->M[3][0];
	out.M[1][0] = a->M[0][1]; out.M[1][1] = a->M[1][1]; out.M[1][2] = a->M[2][1]; out.M[1][3] = a->M[3][1];
	out.M[2][0] = a->M[0][2]; out.M[2][1] = a->M[1][2]; out.M[2][2] = a->M[2][2]; out.M[2][3] = a->M[3][2];
	out.M[3][0] = a->M[0][3]; out.M[3][1] = a->M[1][3]; out.M[3][2] = a->M[2][3]; out.M[3][3] = a->M[3][3];
	return out;
}

// use left-multiplication to accumulate transformations
static inline ovrMatrix4f ovrMatrix4f_Multiply(const ovrMatrix4f * a, const ovrMatrix4f * b)
{
	ovrMatrix4f out;
	out.M[0][0] = a->M[0][0] * b->M[0][0] + a->M[0][1] * b->M[1][0] + a->M[0][2] * b->M[2][0] + a->M[0][3] * b->M[3][0];
	out.M[1][0] = a->M[1][0] * b->M[0][0] + a->M[1][1] * b->M[1][0] + a->M[1][2] * b->M[2][0] + a->M[1][3] * b->M[3][0];
	out.M[2][0] = a->M[2][0] * b->M[0][0] + a->M[2][1] * b->M[1][0] + a->M[2][2] * b->M[2][0] + a->M[2][3] * b->M[3][0];
	out.M[3][0] = a->M[3][0] * b->M[0][0] + a->M[3][1] * b->M[1][0] + a->M[3][2] * b->M[2][0] + a->M[3][3] * b->M[3][0];

	out.M[0][1] = a->M[0][0] * b->M[0][1] + a->M[0][1] * b->M[1][1] + a->M[0][2] * b->M[2][1] + a->M[0][3] * b->M[3][1];
	out.M[1][1] = a->M[1][0] * b->M[0][1] + a->M[1][1] * b->M[1][1] + a->M[1][2] * b->M[2][1] + a->M[1][3] * b->M[3][1];
	out.M[2][1] = a->M[2][0] * b->M[0][1] + a->M[2][1] * b->M[1][1] + a->M[2][2] * b->M[2][1] + a->M[2][3] * b->M[3][1];
	out.M[3][1] = a->M[3][0] * b->M[0][1] + a->M[3][1] * b->M[1][1] + a->M[3][2] * b->M[2][1] + a->M[3][3] * b->M[3][1];

	out.M[0][2] = a->M[0][0] * b->M[0][2] + a->M[0][1] * b->M[1][2] + a->M[0][2] * b->M[2][2] + a->M[0][3] * b->M[3][2];
	out.M[1][2] = a->M[1][0] * b->M[0][2] + a->M[1][1] * b->M[1][2] + a->M[1][2] * b->M[2][2] + a->M[1][3] * b->M[3][2];
	out.M[2][2] = a->M[2][0] * b->M[0][2] + a->M[2][1] * b->M[1][2] + a->M[2][2] * b->M[2][2] + a->M[2][3] * b->M[3][2];
	out.M[3][2] = a->M[3][0] * b->M[0][2] + a->M[3][1] * b->M[1][2] + a->M[3][2] * b->M[2][2] + a->M[3][3] * b->M[3][2];

	out.M[0][3] = a->M[0][0] * b->M[0][3] + a->M[0][1] * b->M[1][3] + a->M[0][2] * b->M[2][3] + a->M[0][3] * b->M[3][3];
	out.M[1][3] = a->M[1][0] * b->M[0][3] + a->M[1][1] * b->M[1][3] + a->M[1][2] * b->M[2][3] + a->M[1][3] * b->M[3][3];
	out.M[2][3] = a->M[2][0] * b->M[0][3] + a->M[2][1] * b->M[1][3] + a->M[2][2] * b->M[2][3] + a->M[2][3] * b->M[3][3];
	out.M[3][3] = a->M[3][0] * b->M[0][3] + a->M[3][1] * b->M[1][3] + a->M[3][2] * b->M[2][3] + a->M[3][3] * b->M[3][3];
	return out;
}


static inline ovrMatrix4f ovrMatrix4f_CreateIdentity()
{
	ovrMatrix4f out;
	out.M[0][0] = 1.0f; out.M[0][1] = 0.0f; out.M[0][2] = 0.0f; out.M[0][3] = 0.0f;
	out.M[1][0] = 0.0f; out.M[1][1] = 1.0f; out.M[1][2] = 0.0f; out.M[1][3] = 0.0f;
	out.M[2][0] = 0.0f; out.M[2][1] = 0.0f; out.M[2][2] = 1.0f; out.M[2][3] = 0.0f;
	out.M[3][0] = 0.0f; out.M[3][1] = 0.0f; out.M[3][2] = 0.0f; out.M[3][3] = 1.0f;
	return out;
}


static inline float ovrMatrix4f_Minor(const ovrMatrix4f * m, int r0, int r1, int r2, int c0, int c1, int c2)
{
	return	m->M[r0][c0] * (m->M[r1][c1] * m->M[r2][c2] - m->M[r2][c1] * m->M[r1][c2]) -
		m->M[r0][c1] * (m->M[r1][c0] * m->M[r2][c2] - m->M[r2][c0] * m->M[r1][c2]) +
		m->M[r0][c2] * (m->M[r1][c0] * m->M[r2][c1] - m->M[r2][c0] * m->M[r1][c1]);
}


static inline ovrMatrix4f ovrMatrix4f_Inverse(const ovrMatrix4f * m)
{
	const float rcpDet = 1.0f / (m->M[0][0] * ovrMatrix4f_Minor(m, 1, 2, 3, 1, 2, 3) -
		m->M[0][1] * ovrMatrix4f_Minor(m, 1, 2, 3, 0, 2, 3) +
		m->M[0][2] * ovrMatrix4f_Minor(m, 1, 2, 3, 0, 1, 3) -
		m->M[0][3] * ovrMatrix4f_Minor(m, 1, 2, 3, 0, 1, 2));
	ovrMatrix4f out;
	out.M[0][0] = ovrMatrix4f_Minor(m, 1, 2, 3, 1, 2, 3) * rcpDet;
	out.M[0][1] = -ovrMatrix4f_Minor(m, 0, 2, 3, 1, 2, 3) * rcpDet;
	out.M[0][2] = ovrMatrix4f_Minor(m, 0, 1, 3, 1, 2, 3) * rcpDet;
	out.M[0][3] = -ovrMatrix4f_Minor(m, 0, 1, 2, 1, 2, 3) * rcpDet;
	out.M[1][0] = -ovrMatrix4f_Minor(m, 1, 2, 3, 0, 2, 3) * rcpDet;
	out.M[1][1] = ovrMatrix4f_Minor(m, 0, 2, 3, 0, 2, 3) * rcpDet;
	out.M[1][2] = -ovrMatrix4f_Minor(m, 0, 1, 3, 0, 2, 3) * rcpDet;
	out.M[1][3] = ovrMatrix4f_Minor(m, 0, 1, 2, 0, 2, 3) * rcpDet;
	out.M[2][0] = ovrMatrix4f_Minor(m, 1, 2, 3, 0, 1, 3) * rcpDet;
	out.M[2][1] = -ovrMatrix4f_Minor(m, 0, 2, 3, 0, 1, 3) * rcpDet;
	out.M[2][2] = ovrMatrix4f_Minor(m, 0, 1, 3, 0, 1, 3) * rcpDet;
	out.M[2][3] = -ovrMatrix4f_Minor(m, 0, 1, 2, 0, 1, 3) * rcpDet;
	out.M[3][0] = -ovrMatrix4f_Minor(m, 1, 2, 3, 0, 1, 2) * rcpDet;
	out.M[3][1] = ovrMatrix4f_Minor(m, 0, 2, 3, 0, 1, 2) * rcpDet;
	out.M[3][2] = -ovrMatrix4f_Minor(m, 0, 1, 3, 0, 1, 2) * rcpDet;
	out.M[3][3] = ovrMatrix4f_Minor(m, 0, 1, 2, 0, 1, 2) * rcpDet;
	return out;
}


// returns the 4x4 rotation matrix for the given quaternion
static inline ovrMatrix4f ovrMatrix4f_CreateFromQuaternion(const ovrQuatf * q)
{
	const float ww = q->w * q->w;
	const float xx = q->x * q->x;
	const float yy = q->y * q->y;
	const float zz = q->z * q->z;

	ovrMatrix4f out;
	out.M[0][0] = ww + xx - yy - zz;
	out.M[0][1] = 2 * (q->x * q->y - q->w * q->z);
	out.M[0][2] = 2 * (q->x * q->z + q->w * q->y);
	out.M[0][3] = 0;

	out.M[1][0] = 2 * (q->x * q->y + q->w * q->z);
	out.M[1][1] = ww - xx + yy - zz;
	out.M[1][2] = 2 * (q->y * q->z - q->w * q->x);
	out.M[1][3] = 0;

	out.M[2][0] = 2 * (q->x * q->z - q->w * q->y);
	out.M[2][1] = 2 * (q->y * q->z + q->w * q->x);
	out.M[2][2] = ww - xx - yy + zz;
	out.M[2][3] = 0;

	out.M[3][0] = 0;
	out.M[3][1] = 0;
	out.M[3][2] = 0;
	out.M[3][3] = 1;
	return out;
}

static inline ovrMatrix4f ovrMatrix4f_CreateTranslation(const float x, const float y, const float z)
{
	ovrMatrix4f out;
	out.M[0][0] = 1.0f; out.M[0][1] = 0.0f; out.M[0][2] = 0.0f; out.M[0][3] = x;
	out.M[1][0] = 0.0f; out.M[1][1] = 1.0f; out.M[1][2] = 0.0f; out.M[1][3] = y;
	out.M[2][0] = 0.0f; out.M[2][1] = 0.0f; out.M[2][2] = 1.0f; out.M[2][3] = z;
	out.M[3][0] = 0.0f; out.M[3][1] = 0.0f; out.M[3][2] = 0.0f; out.M[3][3] = 1.0f;
	return out;
}


#endif
