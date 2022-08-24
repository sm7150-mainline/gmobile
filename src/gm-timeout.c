/*
 * Copyright (C) 2022 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 * Author: Guido Günther <agx@sigxcpu.org>
 */

#include "gm-timeout.h"

#include <fcntl.h>
#include <unistd.h>
#include <sys/timerfd.h>


typedef struct _GmTimeoutOnce {
  GSource   source;
  int       fd;
  gpointer  tag;
  gulong    timeout_ms;
  gboolean  armed;
} GmTimeoutOnce;


static gboolean
gm_timeout_once_prepare (GSource *source, gint *timeout)
{
  GmTimeoutOnce *timer = (GmTimeoutOnce *)source;
  struct itimerspec time_spec = { 0 };
  int ret;

  if (timer->fd == -1)
    return FALSE;

  if (timer->armed)
    return FALSE;

  g_debug ("Timeout prepare: %ld for %p", timer->timeout_ms / 1000, source);
  time_spec.it_value.tv_sec = timer->timeout_ms / 1000;
  time_spec.it_value.tv_nsec = (timer->timeout_ms % 1000) * 1000;

  ret = timerfd_settime (timer->fd, 0 /* flags */, &time_spec, NULL);
  if (ret)
    g_warning ("Failed to set up timer: %s", strerror(ret));

  g_debug ("Prepared %p", source);
  timer->armed = TRUE;
  *timeout = -1;
  return FALSE;
}


static gboolean
gm_timeout_once_dispatch (GSource     *source,
			     GSourceFunc  callback,
			     void        *data)
{
  if (!callback) {
    g_warning ("Timeout source dispatched without callback. "
	       "You must call g_source_set_callback().");
    return G_SOURCE_REMOVE;
  }

  g_debug ("dispatch %p", source);
  callback(data);

  return G_SOURCE_REMOVE;
}


static void
gm_timeout_once_finalize (GSource *source)
{
  GmTimeoutOnce *timer = (GmTimeoutOnce *) source;

  close(timer->fd);
  timer->fd = -1;
  timer->armed = FALSE;

  g_source_remove_unix_fd (source, timer->tag);
  timer->tag = NULL;

  g_debug ("Finalize %p", source);
}


static GSourceFuncs gm_timeout_once_source_funcs = {
  gm_timeout_once_prepare,
  NULL, /* check */
  gm_timeout_once_dispatch,
  gm_timeout_once_finalize,
};


static GSource *
gm_timeout_source_once_new (gulong timeout_ms)
{
  int fdf, fsf;
  GmTimeoutOnce *timer = (GmTimeoutOnce *) g_source_new (&gm_timeout_once_source_funcs,
						   sizeof (GmTimeoutOnce));

  timer->timeout_ms = timeout_ms;
  g_source_set_name ((GSource *)timer, "[gm] boottime timeout source");
  timer->fd = timerfd_create (CLOCK_BOOTTIME, 0);
  if (timer->fd == -1)
    return (GSource*)timer;

  fdf = fcntl (timer->fd, F_GETFD) | FD_CLOEXEC;
  fcntl (timer->fd, F_SETFD, fdf);
  fsf = fcntl (timer->fd, F_GETFL) | O_NONBLOCK;
  fcntl (timer->fd, F_SETFL, fsf);

  timer->tag = g_source_add_unix_fd (&timer->source, timer->fd, G_IO_IN | G_IO_ERR);
  return (GSource*)timer;
}


/**
 * gm_timeout_add_seconds_once_full: (rename-to gm_timeout_add_seconds_once)
 * @priority: the priority of the timeout source. Typically this will be in
 *   the range between %G_PRIORITY_DEFAULT and %G_PRIORITY_HIGH.
 * @seconds: the timeout in seconds
 * @function: function to call
 * @data: data to pass to @function
 * @notify: (nullable): function to call when the timeout is removed, or %NULL
 *
 * Sets a function to be called after a timeout with priority @priority.
 *
 * This internally creates a main loop source using
 * g_timeout_source_new_seconds() and attaches it to the main loop context
 * using g_source_attach().
 *
 * The timeout given is in terms of `CLOCK_BOOTTIME` time, it hence is also
 * correct across suspend and resume. If that doesn't matter use
 * `g_timeout_add_seconds_full` instead.
 *
 * Returns: the ID (greater than 0) of the event source.
 **/
guint
gm_timeout_add_seconds_once_full (gint           priority,
				  gulong         seconds,
				  GSourceFunc    function,
				  gpointer       data,
				  GDestroyNotify notify)
{
  g_autoptr (GSource) source = NULL;
  guint id;

  g_return_val_if_fail (function != NULL, 0);

  source = gm_timeout_source_once_new (1000L * seconds);

  if (priority != G_PRIORITY_DEFAULT)
    g_source_set_priority (source, priority);

  g_source_set_callback (source, function, data, notify);
  id = g_source_attach (source, NULL);

  return id;
}


/**
 * gm_timeout_add_seconds_once:
 * @seconds: the timeout in seconds
 * @function: function to call
 * @data: data to pass to @function
 *
 * Sets a function to be called after a timeout with the default
 * priority, %G_PRIORITY_DEFAULT.
 *
 * This internally creates a main loop source using
 * g_timeout_source_new_seconds() and attaches it to the main loop context
 * using g_source_attach().
 *
 * The timeout given is in terms of `CLOCK_BOOTTIME` time, it hence is also
 * correct across suspend and resume. If that doesn't matter use
 * `g_timeout_add_seconds` instead.
 *
 * Returns: the ID (greater than 0) of the event source.
 **/
guint
gm_timeout_add_seconds_once (int          seconds,
			     GSourceFunc  function,
			     gpointer     data)
{
  g_return_val_if_fail (function != NULL, 0);

  return gm_timeout_add_seconds_once_full (G_PRIORITY_DEFAULT, seconds, function, data, NULL);
}
