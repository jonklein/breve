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

typedef struct slCamera slCamera;
typedef struct slBillboardEntry slBillboardEntry;
typedef struct slLight slLight;
typedef struct slCameraText slCameraText;
typedef struct slContactList slContactList;
typedef struct slContact slContact;
typedef struct slWorldFileHeader slWorldFileHeader;
typedef struct slWorldObjectFileHeader slWorldObjectFileHeader;
typedef struct slLinkHeader slLinkHeader;
typedef struct slSavedStationary slSavedStationary;
typedef struct slShapeHeader slShapeHeader;
typedef struct slFaceHeader slFaceHeader;
typedef struct slTerrain slTerrain;
typedef struct slSerializedTerrain slSerializedTerrain;
typedef struct slLink slLink;
typedef struct slJoint slJoint;
typedef struct slLinkIntegrationPosition slLinkIntegrationPosition;
typedef struct slMultibody slMultibody;
typedef struct slFeature slFeature;
typedef struct slPoint slPoint;
typedef struct slEdge slEdge;
typedef struct slFace slFace;
typedef struct slPlane slPlane;
typedef struct slPosition slPosition;
typedef struct slShape slShape;
typedef struct slCollisionHeap slCollisionHeap;
typedef struct slBoundSort slBoundSort;
typedef struct slPairEntry slPairEntry;
typedef struct slCollision slCollision;
typedef struct slVclipData slVclipData;
typedef struct slMPFace slMPFace;
typedef struct slMPPolyhedron slMPPolyhedron;
typedef struct slWorld slWorld;
typedef struct slWorldObject slWorldObject;
typedef struct slStationary slStationary;
typedef struct slObjectConnection slObjectConnection;
typedef struct slObjectLine slObjectLine;

typedef struct slSpring slSpring;

typedef struct slNetsimData slNetsimData;
typedef struct slNetsimServerData slNetsimServerData;
typedef struct slNetsimClientData slNetsimClientData;
typedef struct slNetsimBoundsMessage slNetsimBoundsMessage;
typedef struct slNetsimSyncMessage slNetsimSyncMessage;
typedef struct slNetsimRemoteHostData slNetsimRemoteHostData;
