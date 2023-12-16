#ifndef PERSISTENCE_H
#define PERSISTENCE_H

#define FUSE_USE_VERSION 30
#define MAX_CONTENIDO 100
#include <fuse.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#include "structs.h"

int save_fs ();

int load_fs ();

void write_metadata (FILE *fp, int i, int j);

void load_metadata (char buffer[], int i, int j, int* offset);

#endif