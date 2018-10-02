/*
 * File: vfs.h
 * 
 * Copyright (c) 2017-2018 Sydney Erickson, John Davis
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef VFS_H
#define VFS_H

#include <main.h>

// File in VFS (node).
typedef struct {
    char Name[128]; // TODO
    uint16_t Flags;

    // Permission mask, user, group IDs.
    uint32_t PermMask;
    uint32_t UserId;
    uint32_t GroupId;

    // Length in bytes.
    uint64_t Length;
} vfs_node_t;

// Directory entry.
typedef struct {
    char *Name;
    uint32_t Inode;
} vfs_dir_ent_t;

extern vfs_node_t *RootVfsNode;

extern int32_t vfs_open(const char *path, int32_t flags);
extern void vfs_init(void);

#endif