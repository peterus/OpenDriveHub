/*
 * Copyright (C) 2026 Peter Buchegger
 *
 * This file is part of OpenDriveHub.
 *
 * OpenDriveHub is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * OpenDriveHub is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with OpenDriveHub. If not, see <https://www.gnu.org/licenses/>.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

/**
 * Native unit tests for the shell tokeniser.
 *
 * Only the free function shellTokenize() is tested here because the full
 * Shell class depends on Arduino Serial which is not available in the
 * native-test environment.
 */

#include <Shell.h>
#include <cstring>
#include <unity.h>

using namespace odh;

/* ── Basic tokenisation ──────────────────────────────────────────────────── */

void test_tokenize_single_word(void) {
    char line[] = "status";
    const char *argv[8];
    int argc = shellTokenize(line, argv, 8);
    TEST_ASSERT_EQUAL_INT(1, argc);
    TEST_ASSERT_EQUAL_STRING("status", argv[0]);
}

void test_tokenize_two_words(void) {
    char line[] = "bind scan";
    const char *argv[8];
    int argc = shellTokenize(line, argv, 8);
    TEST_ASSERT_EQUAL_INT(2, argc);
    TEST_ASSERT_EQUAL_STRING("bind", argv[0]);
    TEST_ASSERT_EQUAL_STRING("scan", argv[1]);
}

void test_tokenize_three_args(void) {
    char line[] = "channel set 1500";
    const char *argv[8];
    int argc = shellTokenize(line, argv, 8);
    TEST_ASSERT_EQUAL_INT(3, argc);
    TEST_ASSERT_EQUAL_STRING("channel", argv[0]);
    TEST_ASSERT_EQUAL_STRING("set", argv[1]);
    TEST_ASSERT_EQUAL_STRING("1500", argv[2]);
}

/* ── Whitespace handling ─────────────────────────────────────────────────── */

void test_tokenize_leading_spaces(void) {
    char line[] = "   status";
    const char *argv[8];
    int argc = shellTokenize(line, argv, 8);
    TEST_ASSERT_EQUAL_INT(1, argc);
    TEST_ASSERT_EQUAL_STRING("status", argv[0]);
}

void test_tokenize_trailing_spaces(void) {
    char line[] = "status   ";
    const char *argv[8];
    int argc = shellTokenize(line, argv, 8);
    TEST_ASSERT_EQUAL_INT(1, argc);
    TEST_ASSERT_EQUAL_STRING("status", argv[0]);
}

void test_tokenize_multiple_spaces(void) {
    char line[] = "channel   set   0   1800";
    const char *argv[8];
    int argc = shellTokenize(line, argv, 8);
    TEST_ASSERT_EQUAL_INT(4, argc);
    TEST_ASSERT_EQUAL_STRING("channel", argv[0]);
    TEST_ASSERT_EQUAL_STRING("set", argv[1]);
    TEST_ASSERT_EQUAL_STRING("0", argv[2]);
    TEST_ASSERT_EQUAL_STRING("1800", argv[3]);
}

void test_tokenize_tabs(void) {
    char line[] = "config\tget";
    const char *argv[8];
    int argc = shellTokenize(line, argv, 8);
    TEST_ASSERT_EQUAL_INT(2, argc);
    TEST_ASSERT_EQUAL_STRING("config", argv[0]);
    TEST_ASSERT_EQUAL_STRING("get", argv[1]);
}

/* ── Edge cases ──────────────────────────────────────────────────────────── */

void test_tokenize_empty_string(void) {
    char line[] = "";
    const char *argv[8];
    int argc = shellTokenize(line, argv, 8);
    TEST_ASSERT_EQUAL_INT(0, argc);
}

void test_tokenize_only_spaces(void) {
    char line[] = "     ";
    const char *argv[8];
    int argc = shellTokenize(line, argv, 8);
    TEST_ASSERT_EQUAL_INT(0, argc);
}

void test_tokenize_max_args_limit(void) {
    char line[] = "a b c d e f g h i j";
    const char *argv[4];
    int argc = shellTokenize(line, argv, 4);
    TEST_ASSERT_EQUAL_INT(4, argc);
    TEST_ASSERT_EQUAL_STRING("a", argv[0]);
    TEST_ASSERT_EQUAL_STRING("d", argv[3]);
}

/* ── Quoted strings ──────────────────────────────────────────────────────── */

void test_tokenize_quoted_string(void) {
    char line[] = "vehicle \"My Vehicle\"";
    const char *argv[8];
    int argc = shellTokenize(line, argv, 8);
    TEST_ASSERT_EQUAL_INT(2, argc);
    TEST_ASSERT_EQUAL_STRING("vehicle", argv[0]);
    TEST_ASSERT_EQUAL_STRING("My Vehicle", argv[1]);
}

void test_tokenize_quoted_with_extra_args(void) {
    char line[] = "config set dev_name \"Dump Truck 1\"";
    const char *argv[8];
    int argc = shellTokenize(line, argv, 8);
    TEST_ASSERT_EQUAL_INT(4, argc);
    TEST_ASSERT_EQUAL_STRING("config", argv[0]);
    TEST_ASSERT_EQUAL_STRING("set", argv[1]);
    TEST_ASSERT_EQUAL_STRING("dev_name", argv[2]);
    TEST_ASSERT_EQUAL_STRING("Dump Truck 1", argv[3]);
}

void test_tokenize_empty_quotes(void) {
    char line[] = "vehicle \"\"";
    const char *argv[8];
    int argc = shellTokenize(line, argv, 8);
    TEST_ASSERT_EQUAL_INT(2, argc);
    TEST_ASSERT_EQUAL_STRING("vehicle", argv[0]);
    TEST_ASSERT_EQUAL_STRING("", argv[1]);
}

/* ── Test runner ─────────────────────────────────────────────────────────── */

// Tests are registered in the central main() in test_protocol.cpp.
// No main() here – just test functions exported for RUN_TEST().
