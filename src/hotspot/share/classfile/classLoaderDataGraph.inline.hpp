/*
 * Copyright (c) 2018, 2023, Oracle and/or its affiliates. All rights reserved.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * This code is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 only, as
 * published by the Free Software Foundation.
 *
 * This code is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * version 2 for more details (a copy is included in the LICENSE file that
 * accompanied this code).
 *
 * You should have received a copy of the GNU General Public License version
 * 2 along with this work; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * Please contact Oracle, 500 Oracle Parkway, Redwood Shores, CA 94065 USA
 * or visit www.oracle.com if you need additional information or have any
 * questions.
 *
 */

#ifndef SHARE_CLASSFILE_CLASSLOADERDATAGRAPH_INLINE_HPP
#define SHARE_CLASSFILE_CLASSLOADERDATAGRAPH_INLINE_HPP

#include "classfile/classLoaderDataGraph.hpp"

#include "classfile/javaClasses.hpp"
#include "oops/oop.inline.hpp"
#include "runtime/atomic.hpp"
#include "runtime/orderAccess.hpp"

// SapMachine 2019-09-01: vitals.
#include "runtime/globals.hpp"
#include "vitals/vitals.hpp"

inline ClassLoaderData *ClassLoaderDataGraph::find_or_create(Handle loader) {
  guarantee(loader() != nullptr && oopDesc::is_oop(loader()), "Loader must be oop");
  // Gets the class loader data out of the java/lang/ClassLoader object, if non-null
  // it's already in the loader_data, so no need to add
  ClassLoaderData* loader_data = java_lang_ClassLoader::loader_data_acquire(loader());
  if (loader_data) {
     return loader_data;
  }
  return ClassLoaderDataGraph::add(loader, false);
}

size_t ClassLoaderDataGraph::num_instance_classes() {
  return Atomic::load(&_num_instance_classes);
}

size_t ClassLoaderDataGraph::num_array_classes() {
  return Atomic::load(&_num_array_classes);
}

void ClassLoaderDataGraph::inc_instance_classes(size_t count) {
  Atomic::add(&_num_instance_classes, count, memory_order_relaxed);
  // SapMachine 2019-02-20 : vitals
  if (EnableVitals) {
    sapmachine_vitals::counters::inc_classes_loaded(count);
  }
}

void ClassLoaderDataGraph::dec_instance_classes(size_t count) {
  size_t old_count = Atomic::fetch_then_add(&_num_instance_classes, -count, memory_order_relaxed);
  assert(old_count >= count, "Sanity");
  // SapMachine 2019-02-20 : vitals
  if (EnableVitals) {
    sapmachine_vitals::counters::inc_classes_unloaded(count);
  }
}

void ClassLoaderDataGraph::inc_array_classes(size_t count) {
  Atomic::add(&_num_array_classes, count, memory_order_relaxed);
  // SapMachine 2019-02-20 : vitals
  if (EnableVitals) {
    sapmachine_vitals::counters::inc_classes_loaded(count);
  }
}

void ClassLoaderDataGraph::dec_array_classes(size_t count) {
  size_t old_count = Atomic::fetch_then_add(&_num_array_classes, -count, memory_order_relaxed);
  assert(old_count >= count, "Sanity");
  // SapMachine 2019-02-20 : vitals
  if (EnableVitals) {
    sapmachine_vitals::counters::inc_classes_unloaded(count);
  }
}

bool ClassLoaderDataGraph::should_clean_metaspaces_and_reset() {
  // Only clean metaspaces after full GC.
  bool do_cleaning = _safepoint_cleanup_needed;
#if INCLUDE_JVMTI
  do_cleaning = do_cleaning && (_should_clean_deallocate_lists || InstanceKlass::should_clean_previous_versions());
#else
  do_cleaning = do_cleaning && _should_clean_deallocate_lists;
#endif
  _safepoint_cleanup_needed = false;  // reset
  return do_cleaning;
}

#endif // SHARE_CLASSFILE_CLASSLOADERDATAGRAPH_INLINE_HPP
