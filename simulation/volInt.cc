#include "simulation.h"

/*
	Revision history

	26 Jan 1996	Program creation.

	 3 Aug 1996	Corrected bug arising when polyhedron density
			is not 1.0.  Changes confined to function main().
			Thanks to Zoran Popovic for catching this one.

	27 May 1997     Corrected sign error in translation of inertia
	                product terms to center of mass frame.  Changes 
			confined to function main().  Thanks to 
			Chris Hecker.
*/

/*
   ============================================================================
   globals
   ============================================================================
*/

#define X 0
#define Y 1
#define Z 2

static int A;   /* alpha */
static int B;   /* beta */
static int C;   /* gamma */

/* projection integrals */
static double P1, Pa, Pb, Paa, Pab, Pbb, Paaa, Paab, Pabb, Pbbb;

/* face integrals */
static double Fa, Fb, Fc, Faa, Fbb, Fcc, Faaa, Fbbb, Fccc, Faab, Fbbc, Fcca;

/* volume integrals */
static double T0, T1[3], T2[3], TP[3];


/*
   ============================================================================
   read in a polyhedron
   ============================================================================
*/

int slConvertShapeToPolyhedron(slShape *s, slMPPolyhedron *p) {
    int edge, pointNumber;
    slPoint *thePoint;
    slFace *theFace;
    slEdge *theEdge;
	std::vector<slPoint*>::iterator pi;
	std::vector<slFace*>::iterator fi;

    /* first get all the points... */

    p->numVerts = 0;
    p->numFaces = 0;

	for(pi = s->points.begin(); pi != s->points.end(); pi++ ) {
		thePoint = *pi;

		p->verts[p->numVerts][X] = thePoint->vertex.x;
		p->verts[p->numVerts][Y] = thePoint->vertex.y;
		p->verts[p->numVerts][Z] = thePoint->vertex.z;
		p->numVerts++;

		if(p->numVerts >= MAX_VERTS) return -1;
    }

	for(fi = s->faces.begin(); fi != s->faces.end(); fi++ ) {
		theFace = *fi;

		p->faces[p->numFaces].poly = p;

		p->faces[p->numFaces].norm[X] = theFace->plane.normal.x;
		p->faces[p->numFaces].norm[Y] = theFace->plane.normal.y;
		p->faces[p->numFaces].norm[Z] = theFace->plane.normal.z;

		theEdge = theFace->neighbors[0];
		thePoint = theEdge->points[0];

		p->faces[p->numFaces].w = -slVectorDot(&theFace->plane.normal, &thePoint->vertex);

		p->faces[p->numFaces].numVerts = 0;

		for(edge=0;edge<theFace->edgeCount;edge++) {
			slPoint *thePoint = theFace->points[edge];

			pointNumber = slFindPointNumber(s, thePoint);

			if(pointNumber == -1) return -1;

			/* add the vertex, increment the vertex counter */

			p->faces[p->numFaces].verts[p->faces[p->numFaces].numVerts++] = pointNumber;

			if(p->faces[p->numFaces].numVerts >= MAX_POLYGON_SZ) return -1;
		}            

		p->numFaces++;

		if(p->numFaces >= MAX_FACES) return -1;
	}

    return 0;
}

int slFindPointNumber(slShape *s, slPoint *p) {
    unsigned int n;
    int number = 0;

    for(n=0;n<s->features.size();n++) {
        if(s->features[n] == p) return number;
        if(s->features[n]->type == FT_POINT) number++;
    }

    return -1;
}

/*
   ============================================================================
   compute mass properties
   ============================================================================
*/


/* compute various integrations over projection of face */
void compProjectionIntegrals(slMPFace *f)
{
  double a0, a1, da;
  double b0, b1, db;
  double a0_2, a0_3, a0_4, b0_2, b0_3, b0_4;
  double a1_2, a1_3, b1_2, b1_3;
  double C1, Ca, Caa, Caaa, Cb, Cbb, Cbbb;
  double Cab, Kab, Caab, Kaab, Cabb, Kabb;
  int i;

  P1 = Pa = Pb = Paa = Pab = Pbb = Paaa = Paab = Pabb = Pbbb = 0.0;

  for (i = 0; i < f->numVerts; i++) {
    a0 = f->poly->verts[f->verts[i]][A];
    b0 = f->poly->verts[f->verts[i]][B];
    a1 = f->poly->verts[f->verts[(i+1) % f->numVerts]][A];
    b1 = f->poly->verts[f->verts[(i+1) % f->numVerts]][B];
    da = a1 - a0;
    db = b1 - b0;
    a0_2 = a0 * a0; a0_3 = a0_2 * a0; a0_4 = a0_3 * a0;
    b0_2 = b0 * b0; b0_3 = b0_2 * b0; b0_4 = b0_3 * b0;
    a1_2 = a1 * a1; a1_3 = a1_2 * a1; 
    b1_2 = b1 * b1; b1_3 = b1_2 * b1;

    C1 = a1 + a0;
    Ca = a1*C1 + a0_2; Caa = a1*Ca + a0_3; Caaa = a1*Caa + a0_4;
    Cb = b1*(b1 + b0) + b0_2; Cbb = b1*Cb + b0_3; Cbbb = b1*Cbb + b0_4;
    Cab = 3*a1_2 + 2*a1*a0 + a0_2; Kab = a1_2 + 2*a1*a0 + 3*a0_2;
    Caab = a0*Cab + 4*a1_3; Kaab = a1*Kab + 4*a0_3;
    Cabb = 4*b1_3 + 3*b1_2*b0 + 2*b1*b0_2 + b0_3;
    Kabb = b1_3 + 2*b1_2*b0 + 3*b1*b0_2 + 4*b0_3;

    P1 += db*C1;
    Pa += db*Ca;
    Paa += db*Caa;
    Paaa += db*Caaa;
    Pb += da*Cb;
    Pbb += da*Cbb;
    Pbbb += da*Cbbb;
    Pab += db*(b1*Cab + b0*Kab);
    Paab += db*(b1*Caab + b0*Kaab);
    Pabb += da*(a1*Cabb + a0*Kabb);
  }

  P1 /= 2.0;
  Pa /= 6.0;
  Paa /= 12.0;
  Paaa /= 20.0;
  Pb /= -6.0;
  Pbb /= -12.0;
  Pbbb /= -20.0;
  Pab /= 24.0;
  Paab /= 60.0;
  Pabb /= -60.0;
}

void compFaceIntegrals(slMPFace *f)
{
  double *n, w;
  double k1, k2, k3, k4;

  compProjectionIntegrals(f);

  w = f->w;
  n = f->norm;
  k1 = 1 / n[C]; k2 = k1 * k1; k3 = k2 * k1; k4 = k3 * k1;

  Fa = k1 * Pa;
  Fb = k1 * Pb;
  Fc = -k2 * (n[A]*Pa + n[B]*Pb + w*P1);

  Faa = k1 * Paa;
  Fbb = k1 * Pbb;
  Fcc = k3 * (SQR(n[A])*Paa + 2*n[A]*n[B]*Pab + SQR(n[B])*Pbb
	 + w*(2*(n[A]*Pa + n[B]*Pb) + w*P1));

  Faaa = k1 * Paaa;
  Fbbb = k1 * Pbbb;
  Fccc = -k4 * (CUBE(n[A])*Paaa + 3*SQR(n[A])*n[B]*Paab 
	   + 3*n[A]*SQR(n[B])*Pabb + CUBE(n[B])*Pbbb
	   + 3*w*(SQR(n[A])*Paa + 2*n[A]*n[B]*Pab + SQR(n[B])*Pbb)
	   + w*w*(3*(n[A]*Pa + n[B]*Pb) + w*P1));

  Faab = k1 * Paab;
  Fbbc = -k2 * (n[A]*Pabb + n[B]*Pbbb + w*Pbb);
  Fcca = k3 * (SQR(n[A])*Paaa + 2*n[A]*n[B]*Paab + SQR(n[B])*Pabb
	 + w*(2*(n[A]*Paa + n[B]*Pab) + w*Pa));
}

void compVolumeIntegrals(slMPPolyhedron *p)
{
  slMPFace *f;
  double nx, ny, nz;
  int i;

  T0 = T1[X] = T1[Y] = T1[Z] 
     = T2[X] = T2[Y] = T2[Z] 
     = TP[X] = TP[Y] = TP[Z] = 0;

  for (i = 0; i < p->numFaces; i++) {

    f = &p->faces[i];

    nx = fabs(f->norm[X]);
    ny = fabs(f->norm[Y]);
    nz = fabs(f->norm[Z]);
    if (nx > ny && nx > nz) C = X;
    else C = (ny > nz) ? Y : Z;
    A = (C + 1) % 3;
    B = (A + 1) % 3;

    compFaceIntegrals(f);

    T0 += f->norm[X] * ((A == X) ? Fa : ((B == X) ? Fb : Fc));

    T1[A] += f->norm[A] * Faa;
    T1[B] += f->norm[B] * Fbb;
    T1[C] += f->norm[C] * Fcc;
    T2[A] += f->norm[A] * Faaa;
    T2[B] += f->norm[B] * Fbbb;
    T2[C] += f->norm[C] * Fccc;
    TP[A] += f->norm[A] * Faab;
    TP[B] += f->norm[B] * Fbbc;
    TP[C] += f->norm[C] * Fcca;
  }

  T1[X] /= 2; T1[Y] /= 2; T1[Z] /= 2;
  T2[X] /= 3; T2[Y] /= 3; T2[Z] /= 3;
  TP[X] /= 2; TP[Y] /= 2; TP[Z] /= 2;
}

int slSetMassProperties(slShape *s, double density) {
  slMPPolyhedron p;
  double r[3];            /* center of mass */

  memset(&p, 0, sizeof(slMPPolyhedron));

  if(slConvertShapeToPolyhedron(s, &p)) return -1;
 
  compVolumeIntegrals(&p);

  s->mass = density * T0;

  /* compute center of mass */
  r[X] = T1[X] / T0;
  r[Y] = T1[Y] / T0;
  r[Z] = T1[Z] / T0;

  // printf("%f, %f, %f: %f\n", r[X], r[Y], r[Z], s->mass);

  /* compute inertia tensor */
  s->inertia[X][X] = density * (T2[Y] + T2[Z]);
  s->inertia[Y][Y] = density * (T2[Z] + T2[X]);
  s->inertia[Z][Z] = density * (T2[X] + T2[Y]);
  s->inertia[X][Y] = s->inertia[Y][X] = - density * TP[X];
  s->inertia[Y][Z] = s->inertia[Z][Y] = - density * TP[Y];
  s->inertia[Z][X] = s->inertia[X][Z] = - density * TP[Z];

  /* translate inertia tensor to center of mass */
  s->inertia[X][X] -= s->mass * (r[Y]*r[Y] + r[Z]*r[Z]);
  s->inertia[Y][Y] -= s->mass * (r[Z]*r[Z] + r[X]*r[X]);
  s->inertia[Z][Z] -= s->mass * (r[X]*r[X] + r[Y]*r[Y]);
  s->inertia[X][Y] = s->inertia[Y][X] += s->mass * r[X] * r[Y]; 
  s->inertia[Y][Z] = s->inertia[Z][Y] += s->mass * r[Y] * r[Z]; 
  s->inertia[Z][X] = s->inertia[X][Z] += s->mass * r[Z] * r[X]; 

  return 0;
}
