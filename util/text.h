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

char *slStrdup(char *o);
char *slStrdupAndFree(char *str);

char *slSplit(char *start, char *substr, int n);

int slUtilGzread(char *ptr, int count, gzFile stream);
int slUtilGzwrite(void *ptr, size_t size, size_t nmemb, gzFile stream);

int slUtilFread(void *ptr, size_t size, size_t nmemb, FILE *stream);
int slUtilFwrite(void *ptr, size_t size, size_t nmemb, FILE *stream);

int slUtilRead(int socket, void *buffer, size_t size);
int slUtilRecv(int socket, void *buffer, size_t size);
int slUtilWrite(int socket, void *buffer, size_t size);

char *slUtilReadFile(char *path);
int slUtilWriteFile(char *path, char *text);

char *slUtilReadStream(FILE *stream);

char *slDequote(char *q);

char *slStrcatM(char *start, char *end);

char *slFgets(char *str, int size, FILE *stream);

