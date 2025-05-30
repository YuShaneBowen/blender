/* SPDX-FileCopyrightText: 2023 Blender Authors
 *
 * SPDX-License-Identifier: GPL-2.0-or-later */

/** \file
 * \ingroup freestyle
 */

#pragma once

extern "C" {
#include <Python.h>
}

#include "../view_map/ViewMap.h"

///////////////////////////////////////////////////////////////////////////////////////////

extern PyTypeObject ViewMap_Type;

#define BPy_ViewMap_Check(v) (PyObject_IsInstance((PyObject *)v, (PyObject *)&ViewMap_Type))

/*---------------------------Python BPy_ViewMap structure definition----------*/
typedef struct {
  PyObject_HEAD
  Freestyle::ViewMap *vm;
} BPy_ViewMap;

/*---------------------------Python BPy_ViewMap visible prototypes-----------*/

int ViewMap_Init(PyObject *module);

///////////////////////////////////////////////////////////////////////////////////////////
