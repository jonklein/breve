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

inline void slSumVectors(int l, double *ov, double *iv, double *dv, double weight);

int slEuler(slWorld *w, slLink *r, double *deltaT, int skipFirst);
int slRK4(slWorld *w, slLink *r, double *deltaT, int skipFirst);

int slRKCKRun(slWorld *w, slLink *r, double *deltaT, int skipFirst);
int slRKCK(slWorld *w, slLink *m, double *x, double acc, double stepSize, double *nextSize);
int slRKCKS(slWorld *w, slLink *m, double *sv, double *osv, double *error, double stepSize);

int slIntRKF(slWorld *w, slLink *m, double *deltaT, int skipFirst);

void slFreeIntegrationVectors(slWorld *w);
void slAllocIntegrationVectors(slWorld *w);

inline int slCalculateDerivs(slLink *r, double *sv, double *dv, slWorld *w);

void slLinkComputeVelocities(slLink *r);
