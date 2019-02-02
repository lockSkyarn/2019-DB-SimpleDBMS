#include <stdio.h>
#include <sys/stat.h>
#include "gtest/gtest.h"
#include "Table.h"
#include "User.h"

TEST(testTable, testNewTable) {
    Table_t *table = new_Table(NULL);
    ASSERT_NE(table, nullptr);
    ASSERT_EQ(table->capacity, MAX_TABLE_SIZE);
    ASSERT_EQ(table->len, 0);
    ASSERT_NE(table->users, nullptr);
    ASSERT_EQ(table->fp, nullptr);
}

TEST(testTable, testNewTableWithFile) {
    char file_name[] = "./test/test.db";
    Table_t *table = new_Table(file_name);
    struct stat st;

    ASSERT_NE(table, nullptr);
    ASSERT_EQ(table->capacity, MAX_TABLE_SIZE);
    ASSERT_EQ(table->len, 0);
    ASSERT_NE(table->users, nullptr);
    ASSERT_NE(table->fp, nullptr);
    ASSERT_STREQ(table->file_name, file_name);
    fclose(table->fp);
    remove(file_name);
    ASSERT_NE(stat(file_name, &st), 0);
}

TEST(testTable, testNewTableWithOldFile) {
    char file_name[] = "./test/test.db";
    FILE *fp;
    Table_t *table;
    struct stat st;
    User_t users[] = {
        { 1, "user1", "user1@example.com", 20},
        { 2, "user2", "user2@example.com", 22},
    };

    fp = fopen(file_name, "wb");
    fwrite((void*)users, sizeof(User_t), 2, fp);
    fclose(fp);
    {
        struct stat st;
        ASSERT_EQ(stat(file_name, &st), 0);
        ASSERT_EQ(st.st_size, sizeof(User_t)*2);
    }

    table = new_Table(file_name);
    ASSERT_NE(table, nullptr);
    ASSERT_EQ(table->capacity, MAX_TABLE_SIZE);
    ASSERT_EQ(table->len, 2);
    ASSERT_NE(table->users, nullptr);
    ASSERT_NE(table->fp, nullptr);
    ASSERT_STREQ(table->file_name, file_name);
    fclose(table->fp);
    remove(file_name);
    ASSERT_NE(stat(file_name, &st), 0);
}

TEST(testTable, testAddUserSuc) {
    Table_t *table = new_Table(NULL);
    ASSERT_NE(table, nullptr);

    User_t user = { 1, "First User", "first@example.com", 21 };

    ASSERT_EQ(add_User(table, &user), 1);
    ASSERT_EQ(table->capacity, MAX_TABLE_SIZE);
    ASSERT_EQ(table->len, 1);
    ASSERT_EQ(table->users[0].id, user.id);
    ASSERT_STREQ(table->users[0].name, user.name);
    ASSERT_STREQ(table->users[0].email, user.email);
    ASSERT_EQ(table->users[0].age, user.age);
}

TEST(testTable, testAddUserFail) {
    Table_t *table = new_Table(NULL);
    User_t user = { 1, "First User", "first@example.com", 21 };
    ASSERT_NE(add_User(NULL, NULL), 1);
    ASSERT_NE(add_User(table, NULL), 1);
    ASSERT_NE(add_User(NULL, &user), 1);
}

TEST(testTable, testAddUserFull) {
    Table_t *table = new_Table(NULL);
    User_t user = { 1, "user", "user@example.com", 20 };
    size_t idx;
    int ret = 0;
    for (idx = 0; idx < MAX_TABLE_SIZE; idx++) {
        ret = add_User(table, &user);
        ASSERT_EQ(ret, 1);
        ASSERT_EQ(table->len, idx+1);
        ASSERT_TRUE(table->len <= MAX_TABLE_SIZE);
    }
    ret = add_User(table, &user);
    ASSERT_NE(ret, 1);
    ASSERT_TRUE(table->len <= MAX_TABLE_SIZE);
}

TEST(testTable, testArchiveTable) {
    char file_name[] = "./test/test.db";
    Table_t *table;
    struct stat st;
    User_t user = { 1, "user", "user@example.com", 20 };
    const size_t insert_count = 5;
    size_t idx;
    int ret;

    ASSERT_NE(stat(file_name, &st), 0);

    table = new_Table(file_name);
    for (idx = 0; idx < insert_count; idx++) {
        ret = add_User(table, &user);
        EXPECT_EQ(ret, 1);
        EXPECT_EQ(table->len, idx+1);
    }
    ret = archive_table(table);
    EXPECT_EQ(ret, insert_count);
    EXPECT_EQ(stat(file_name, &st), 0);
    EXPECT_EQ(st.st_size, sizeof(User_t)*insert_count);

    if (table->fp) {
        fclose(table->fp);
    }
    remove(file_name);
    ASSERT_NE(stat(file_name, &st), 0);
}

TEST(testTable, testArchiveOldTable) {
    char file_name[] = "./test/test.db";
    FILE *fp;
    Table_t *table;
    struct stat st;
    User_t users[] = {
        { 1, "user1", "user1@example.com", 20},
        { 2, "user2", "user2@example.com", 22},
    };
    User_t user = { 3, "user", "user@example.com", 20 };
    const size_t insert_count = 5;
    size_t idx;
    int ret;

    fp = fopen(file_name, "wb");
    fwrite((void*)users, sizeof(User_t), 2, fp);
    fclose(fp);
    {
        struct stat st;
        ASSERT_EQ(stat(file_name, &st), 0);
        ASSERT_EQ(st.st_size, sizeof(User_t)*2);
    }

    table = new_Table(file_name);
    for (idx = 0; idx < insert_count; idx++) {
        ret = add_User(table, &user);
        EXPECT_EQ(ret, 1);
    }
    ret = archive_table(table);
    EXPECT_EQ(ret, (int)insert_count+2);
    EXPECT_EQ(stat(file_name, &st), 0);
    EXPECT_EQ(st.st_size, sizeof(User_t)*(insert_count+2));

    if (table->fp) {
        fclose(table->fp);
    }
    remove(file_name);
    ASSERT_NE(stat(file_name, &st), 0);
}

TEST(testTable, testArchiveTableNULL) {
    Table_t *table = new_Table(NULL);
    int ret;
    ASSERT_NE(table, nullptr);
    ASSERT_EQ(table->capacity, MAX_TABLE_SIZE);
    ASSERT_EQ(table->len, 0);
    ASSERT_NE(table->users, nullptr);
    ASSERT_EQ(table->fp, nullptr);

    ret = archive_table(table);
    ASSERT_EQ(ret, 0);
}

TEST(testTable, testGetUserNoFile) {
    Table_t *table;
    User_t *tmp_user;
    User_t users[] = {
        { 1, "user1", "user1@example.com", 20},
        { 2, "user2", "user2@example.com", 22},
    };
    size_t idx;
    int ret;


    table = new_Table(NULL);
    for (idx = 0; idx < 2; idx++) {
        ret = add_User(table, users+idx);
        EXPECT_EQ(ret, 1);
        EXPECT_EQ(table->len, idx+1);
    }
    tmp_user = get_User(table, 0);
    ASSERT_NE(tmp_user, nullptr);
    ASSERT_EQ(tmp_user->id, users[0].id);
    ASSERT_STREQ(tmp_user->name, users[0].name);
    ASSERT_STREQ(tmp_user->email, users[0].email);
    ASSERT_EQ(tmp_user->age, users[0].age);

    tmp_user = get_User(table, 1);
    ASSERT_NE(tmp_user, nullptr);
    ASSERT_EQ(tmp_user->id, users[1].id);
    ASSERT_STREQ(tmp_user->name, users[1].name);
    ASSERT_STREQ(tmp_user->email, users[1].email);
    ASSERT_EQ(tmp_user->age, users[1].age);
}

TEST(testTable, testGetUserBeforeArchive) {
    char file_name[] = "./test/test.db";
    Table_t *table;
    struct stat st;
    User_t *tmp_user;
    User_t users[] = {
        { 1, "user1", "user1@example.com", 20},
        { 2, "user2", "user2@example.com", 22},
    };
    size_t idx;
    int ret;

    EXPECT_NE(stat(file_name, &st), 0);

    table = new_Table(file_name);
    for (idx = 0; idx < 2; idx++) {
        ret = add_User(table, users+idx);
        EXPECT_EQ(ret, 1);
        EXPECT_EQ(table->len, idx+1);
    }
    tmp_user = get_User(table, 0);
    ASSERT_NE(tmp_user, nullptr);
    EXPECT_EQ(tmp_user->id, users[0].id);
    EXPECT_STREQ(tmp_user->name, users[0].name);
    EXPECT_STREQ(tmp_user->email, users[0].email);
    EXPECT_EQ(tmp_user->age, users[0].age);

    tmp_user = get_User(table, 1);
    ASSERT_NE(tmp_user, nullptr);
    EXPECT_EQ(tmp_user->id, users[1].id);
    EXPECT_STREQ(tmp_user->name, users[1].name);
    EXPECT_STREQ(tmp_user->email, users[1].email);
    EXPECT_EQ(tmp_user->age, users[1].age);

    if (table->fp) {
        fclose(table->fp);
    }
    remove(file_name);
    ASSERT_NE(stat(file_name, &st), 0);
}

TEST(testTable, testGetUserFile) {
    char file_name[] = "./test/test.db";
    Table_t *table;
    struct stat st;
    int ret;
    User_t *tmp_user;
    User_t users[] = {
        { 1, "user1", "user1@example.com", 20},
        { 2, "user2", "user2@example.com", 22},
        { 3, "user3", "user3@example.com", 23},
    };

    {
        FILE *fp = fopen(file_name, "wb");
        fwrite((void*)users, sizeof(User_t), 2, fp);
        fclose(fp);
    }

    table = new_Table(file_name);
    ret = add_User(table, users+2);
    EXPECT_EQ(ret, 1);

    tmp_user = get_User(table, 0);
    ASSERT_NE(tmp_user, nullptr);
    EXPECT_EQ(tmp_user->id, users[0].id);
    EXPECT_STREQ(tmp_user->name, users[0].name);
    EXPECT_STREQ(tmp_user->email, users[0].email);
    EXPECT_EQ(tmp_user->age, users[0].age);

    tmp_user = get_User(table, 1);
    ASSERT_NE(tmp_user, nullptr);
    EXPECT_EQ(tmp_user->id, users[1].id);
    EXPECT_STREQ(tmp_user->name, users[1].name);
    EXPECT_STREQ(tmp_user->email, users[1].email);
    EXPECT_EQ(tmp_user->age, users[1].age);

    tmp_user = get_User(table, 2);
    ASSERT_NE(tmp_user, nullptr);
    EXPECT_EQ(tmp_user->id, users[2].id);
    EXPECT_STREQ(tmp_user->name, users[2].name);
    EXPECT_STREQ(tmp_user->email, users[2].email);
    EXPECT_EQ(tmp_user->age, users[2].age);

    if (table->fp) {
        fclose(table->fp);
    }
    remove(file_name);
    ASSERT_NE(stat(file_name, &st), 0);
}

