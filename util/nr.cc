/*****************************************************************************
 *                                                                           *
 * The breve Simulation Environment                                          *
 * Copyright (C) 2000, 2001, 2002, 2003 Jonathan Klein                       *
 *                                                                           *
 * This program is free software; you can redistribute it and/or modify      *
 * it under the terms of the GNU General Public License as published by      *
 * the Free Software Foundation; either version 2 of the License, or         *
 * (at your option) any later version.                                       *
 *                                                                           *
 * This program is distributed in the hope that it will be useful,           *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of            *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             *
 * GNU General Public License for more details.                              *
 *                                                                           *
 * You should have received a copy of the GNU General Public License         *
 * along with this program; if not, write to the Free Software               *
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA *
 *****************************************************************************/

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "util.h"

/* 
	= nr.c, mathematical functions

	= a few functions in this file based loosly Numerical Recipes in C but 
	= i fixed all the nutty stuff they do.  like arrays starting at 1.  
	= what's that all about?  what's up wit dat, sister?

	= other functions are pure klein, baby.
*/

/* MAX_N is the maximum slMatrix size.  for use in the featherstone  */
/* library we only use it for 3/6 space vectors.				   */

#define MAX_N	6

#define SIGN(x,y)	((x)*((y)>0.0?1.0:-1.0))
#define	FMAX(x,y)	(((x)>(y))?(x):(y))
#define IMIN(x,y)	(((x)>(y))?(y):(x))
#define SWAP(a,b)	{temp=(a);(a)=(b);(b)=temp;}
#define TINY 1.0e-12

int slGaussj(double *a, int n) {
	int indxc[MAX_N], indxr[MAX_N], ipiv[MAX_N];
	int i,icol=0,irow=0,j,k,l,ll;
	double big,dum,pivinv,temp;
	int rowexp, colexp;

	int nn = n * n;
	int llexp;

	if(n > MAX_N) {
		/* WARNING WARNING DOES NOT COMPUTE */
		return -1;
	}

	for (j=0;j<n;j++) {
		ipiv[j]=0;
		indxc[j]=0;
		indxr[j]=0;
	}

	for (i=0;i<n;i++) {
		int jn = 0;

		big=0.0;

		for (j=0;j<n;j++) {
			if (ipiv[j] != 1) {
				for (k=0;k<n;k++) {
					if (ipiv[k] == 0) {
						if (fabs(a[jn + k]) >= big) {
							big=fabs(a[jn + k]);
							irow=j;
							icol=k;
						}
					} else if (ipiv[k] > 1) return -1;
				}
			}

			jn += n;
		}

		rowexp = irow * n;
		colexp = icol * n;

		++(ipiv[icol]);

		if (irow != icol) {

			for (l=0;l<n;l++) SWAP(a[rowexp + l],a[colexp + l]);
		}

		indxr[i]=irow;
		indxc[i]=icol;
		if (a[colexp + icol] == 0.0) return -1;
		pivinv=1.0/a[colexp + icol];
		a[colexp + icol]=1.0;

		for (l=0;l<n;l++) a[colexp + l] *= pivinv;

		llexp = 0;

		for (ll=0;ll<n;ll++) {
			if (ll != icol) {
				dum=a[llexp + icol];
				a[llexp + icol]=0.0;
				for (l=0;l<n;l++) a[llexp + l] -= a[colexp + l]*dum;
			}

			llexp += n;
		}
	}

	for (l=n-1;l>=0;l--) {
		if (indxr[l] != indxc[l]) {
			for (k=0;k<nn;k+=n)
				SWAP(a[k + indxr[l]],a[k + indxc[l]]);
		}
	}

	return 0;
}

void slPseudoInverse(double m[3][3], double mi[3][3]) {
	double diag[3][3], u[3][3], ut[3][3], v[3][3], tempM1[3][3], s[3];

	slMatrixCopy(m, u);

	slSvdcmp(u[0], 3, 3, s, v[0]);

	slMatrixZero(diag);

	if(s[0] > TINY) diag[0][0] = 1.0/s[0];
	if(s[1] > TINY) diag[1][1] = 1.0/s[1];
	if(s[2] > TINY) diag[2][2] = 1.0/s[2];

	slMatrixTranspose(u, ut);

	slMatrixMulMatrix(diag, ut, tempM1);
	slMatrixMulMatrix(v, tempM1, mi);
}

int slSvdcmp(double *a, int m, int n, double *w, double *v) {
	extern double slPythag();
	int flag, i, its, j, jj, k, l = 0, nm = 0;
	double anorm, c, f, g, h, s, scale, x, y, zz, rv1[MAX_N];

	if(n > MAX_N || m > MAX_N) {
		return -1;
	}

	for(i=0;i<MAX_N;i++) rv1[n] = 0.0;

	g=scale=anorm=0.0;

	for(i=0;i<n;i++) {
		l=i+1;
		rv1[i]=scale*g;
		g=s=scale=0.0;
		if (i<m) {
			for (k=i; k<m; k++) scale += fabs(a[k*m + i]);
			if (scale) {	
				for (k=i;k<m;k++) {
					a[k*m + i] /=scale;
					s += a[k*m + i]*a[k*m + i];
				}

				f=a[i*m + i];
				g = -SIGN(sqrt(s),f);
				h = f*g-s;
				a[i*m + i] = f-g;
				for (j=l; j<n; j++) {
					for (s=0.0,k=i;k<m;k++)
						s+= a[k*m + i]*a[k*m + j];
						f=s/h;
						for (k=i;k<m;k++) a[k*m + j]+=f*a[k*m + i];
				}

				for (k=i;k<m;k++) a[k*m + i] *=scale;
			}
		}

		w[i]=scale*g;
		g=s=scale=0.0;

		if (i < m && i != (n - 1)) {
			for (k=l; k<n; k++) scale += fabs(a[i*m + k]);
			if (scale) {
				for (k=l;k<n;k++) {
					a[i*m + k] /= scale;
					s+= a[i*m + k]*a[i*m + k];
				}
				f=a[i*m + l];
				g = -SIGN(sqrt(s),f);
				h=f*g-s;
				a[i*m + l]=f-g;
				for (k=l;k<n;k++) rv1[k]=a[i*m + k]/h;
				for (j=l;j<m;j++) {
					for (s=0.0,k=l;k<n;k++) s += a[j*m + k]*a[i*m + k];
					for (k=l;k<n;k++) a[j*m + k] += s*rv1[k];
				}

				for (k=l;k<n;k++) a[i*m + k] *= scale;
			}
		}

		anorm=FMAX(anorm,(fabs(w[i])+fabs(rv1[i])));
	}

	for (i=n-1;i>=0;i--) {
		if (i < n) {
			if (g) {
				for (j=l;j<n;j++) v[j*m + i]=(a[i*m + j]/a[i*m + l])/g;
				for (j=l;j<n;j++) {
					for (s=0.0,k=l;k<n;k++) s+= a[i*m + k]*v[k*m + j];
					for (k=l;k<n;k++) v[k*m + j] += s*v[k*m + i];
				}
			}
			for (j=l;j<n;j++) v[i*m + j]=v[j*m + i]=0.0;
		}
		v[i*m + i]=1.0;
		g=rv1[i];
		l=i;
	}

	for (i=IMIN(m,n) - 1;i>=0;i--) {
		l=i+1;
		g=w[i];
		for (j=l;j<n;j++) a[i*m + j]=0.0;
		if (g) {
			g=1.0/g;
			for (j=l;j<n;j++) {
				for (s=0.0,k=l;k<m;k++) s += a[k*m + i]*a[k*m + j];
				f = (s/a[i*m + i])*g;
				for (k=i;k<m;k++) a[k*m + j] += f*a[k*m + i];
			}

			for (j=i;j<m;j++) a[j*m + i] *= g;
		} else for (j=i;j<m;j++) a[j*m + i]=0.0;

		++a[i*m + i];
	}

	for (k=n-1;k>=0;k--) {
		for (its=0;its<50;its++) {
			flag=1;
			for (l=k;l>=0;l--) {
				nm=l-1;
				if ((double)(fabs(rv1[l])+anorm)==anorm){
					flag=0;
					break;
				}

				if((double)(fabs(w[nm])+anorm)==anorm) break;
			}

			if (flag) {
				c=0.0;
				s=1.0;
				for (i=l;i<k;i++) {
					f=s*rv1[i];
					rv1[i]=c*rv1[i];
					if((double)(fabs(f)+anorm)==anorm) break;
					g=w[i];
					h=slPythag(f,g);
					w[i]=h;
					h=1.0/h;
					c=g*h;
					s = -f*h;
					for (j=0;j<m;j++) {
						y=a[j*m + nm];
						zz=a[j*m + i];
						a[j*m + nm]=y*c+zz*s;
						a[j*m + i]=zz*c-y*s;
					}
				}
			}

			zz=w[k];
			if (l==k) {
				if (zz<0.0) {
					w[k] = -zz;
					for (j=0;j<n;j++) v[j*m + k] = -v[j*m + k];
				}

				break;
			}

			if (its==49) {
				fprintf(stderr,"no conv in 50 svdcmp iterations\n");
				return -1;
			}

			x=w[l];
			nm=k-1;
			y=w[nm];
			g=rv1[nm];
			h=rv1[k];
			f=((y-zz)*(y+zz)+(g-h)*(g+h))/(2.0*h*y);
			g=slPythag(f,1.0);
			f=((x-zz)*(x+zz)+h*((y/(f+SIGN(g,f)))-h))/x;
			c=s=1.0;
			for (j=l;j<=nm;j++) {
				i=j+1;
				g=rv1[i];
				y=w[i];
				h=s*g;
				g=c*g;
				zz=slPythag(f,h);
				rv1[j]=zz;
				c=f/zz;
				s=h/zz;
				f=x*c+g*s;
				g=g*c-x*s;
				h=y*s;
				y*=c;
				for (jj=0;jj<n;jj++) {
					x=v[jj*m + j];
					zz=v[jj*m + i];
					v[jj*m + j]=x*c+zz*s;
					v[jj*m + i]=zz*c-x*s;
				}

				zz=slPythag(f,h);
				w[j]=zz;
				if (zz) {
					zz=1.0/zz;
					c=f*zz;
					s=h*zz;
				}

				f=c*g+s*y;
				x=c*y-s*g;

				for (jj=0;jj<m;jj++) {
					y=a[jj*m + j];
					zz=a[jj*m + i];
					a[jj*m + j]=y*c+zz*s;
					a[jj*m + i]=zz*c-y*s;
				}
			}
			rv1[l]=0.0;
			rv1[k]=f;
			w[k]=x;
		}	   
	}

	return 0;
}

int slQuadratic(double a, double b, double c, double *s1, double *s2) {
	double subExp, sqrtSubExp;
		
	*s1 = *s2 = 0;

	if(a == 0) {
		*s1 = -c / b;
		*s2 = -c / b;

		return 0;
	} 

	subExp = (b*b) - (4*a*c);

	if(subExp < 0.0) return -1;

	a *= 2;

	sqrtSubExp = sqrt(subExp);

	*s1 = (-b + sqrtSubExp) / a;
	*s2 = (-b - sqrtSubExp) / a;

	return 0;
}

double slPythag(double a, double b) {
	double absa,absb;
	absa=fabs(a);
	absb=fabs(b);
	if (absa > absb) return absa*sqrt(1.0+pow(absb/absa, 2.0));
	else return ((absb == 0.0) ? 0.0 : absb*sqrt(1.0+pow(absa/absb, 2.0)));
} 

int slIsinf(double d) {
	return (d == 1.0/0.0);
}
