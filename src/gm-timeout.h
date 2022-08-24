/*
 * Copyright (C) 2022 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 * Author: Guido Günther <agx@sigxcpu.org>
 */

#pragma once

#include <glib.h>

G_BEGIN_DECLS

guint       gm_timeout_add_seconds_once_full (int             priority,
					      gulong          seconds,
					      GSourceFunc     function,
					      gpointer        data,
					      GDestroyNotify  notify);
guint       gm_timeout_add_seconds_once      (int             seconds,
					      GSourceFunc     function,
					      gpointer        data);

G_END_DECLS
