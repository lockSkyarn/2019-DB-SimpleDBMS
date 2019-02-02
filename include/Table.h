#ifndef TABLE_H
#define TABLE_H
#include <stdlib.h>
#include <stdio.h>
#include "User.h"

#define MAX_TABLE_SIZE 10000

typedef struct Table {
    size_t capacity;
    size_t len;
    User_t *users;
    FILE *fp;
    char *file_name;
} Table_t;

Table_t *new_Table(char *file_name);
int add_User(Table_t *table, User_t *user);
int archive_table(Table_t *table);
User_t* get_User(Table_t *table, size_t idx);

#endif
