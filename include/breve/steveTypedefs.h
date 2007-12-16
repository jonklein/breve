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

/* yeeeeeeeeeeeeeeeeeeeeeehaw! look at 'em all! */

typedef struct stExp stExp;
typedef struct stKeyword stKeyword;
typedef struct stStringExp stStringExp;
typedef struct stSubstringExp stSubstringExp;
typedef struct stBinaryExp stBinaryExp;
typedef struct stCodeArrayExp stCodeArrayExp;
typedef struct stUnaryExp stUnaryExp;
typedef struct stVectorExp stVectorExp;
typedef struct stMatrixExp stMatrixExp;
typedef struct stPrintExp stPrintExp;
typedef struct stAllExp stAllExp;
typedef struct stCopyListExp stCopyListExp;
typedef struct stFreeExp stFreeExp;
typedef struct stVectorElementExp stVectorElementExp;
typedef struct stVectorElementAssignExp stVectorElementAssignExp;
typedef struct stMethodExp stMethodExp;
typedef struct stAssignExp stAssignExp;
typedef struct stLoadExp stLoadExp;
typedef struct stRandomExp stRandomExp;
typedef struct stEvalExp stEvalExp;
typedef struct stArrayIndexExp stArrayIndexExp;
typedef struct stArrayIndexAssignExp stArrayIndexAssignExp;
typedef struct stCCallExp stCCallExp;
typedef struct stCCall stCCall;
typedef struct stListInsertExp stListInsertExp;
typedef struct stListRemoveExp stListRemoveExp;
typedef struct stSortExp stSortExp;
typedef struct stListIndexExp stListIndexExp;
typedef struct stListIndexAssignExp stListIndexAssignExp;
typedef struct stHashIndexExp stHashIndexExp;
typedef struct stHashIndexAssignExp stHashIndexAssignExp;
typedef struct stWhileExp stWhileExp;
typedef struct stIsaExp stIsaExp;
typedef struct stListExp stListExp;
typedef struct stReturnExp stReturnExp;
typedef struct stForeachExp stForeachExp;
typedef struct stForExp stForExp;
typedef struct stIfExp stIfExp;
typedef struct stInstanceExp stInstanceExp;
typedef struct stObject stObject;
typedef struct stMethod stMethod;
typedef struct stInstance stInstance;
typedef struct stRunInstance stRunInstance;
typedef struct stKeywordEntry stKeywordEntry;
typedef struct stVarType stVarType;
typedef struct stVar stVar;

typedef struct stVersionRequirement stVersionRequirement;

typedef struct stSteveData stSteveData;

typedef struct stGCRecord stGCRecord;
