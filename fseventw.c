// Copyright 2015 Google Inc. All rights reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <CoreServices/CoreServices.h>
#include <dispatch/dispatch.h>
#include <stdio.h>

void EventStreamCallback(ConstFSEventStreamRef event_stream,
                         void* callback_info,
                         size_t num_events,
                         void* event_paths_void,
                         const FSEventStreamEventFlags event_flags[],
                         const FSEventStreamEventId event_ids[]) {
  dispatch_queue_t log_queue = callback_info;
  const char** event_paths = event_paths_void;

  for (size_t i = 0; i < num_events; ++i) {
    const FSEventStreamEventFlags flags = event_flags[i];
    const FSEventStreamEventId ids = event_ids[i];

    char* path = strdup(event_paths[i]);

    dispatch_async(log_queue, ^{
        printf("event %llx %x at %s\n", ids, flags, path);
        free(path);
    });
  }
}

int main() {
  dispatch_queue_t log_queue = dispatch_queue_create("com.google.fseventw.logq", DISPATCH_QUEUE_SERIAL);
  dispatch_queue_t event_queue = dispatch_queue_create("com.google.fseventw.eventq", DISPATCH_QUEUE_CONCURRENT);

  const CFStringRef kPath = CFSTR("/");
  const void* paths_to_watch_c[] = { kPath };
  CFArrayRef paths_to_watch = CFArrayCreate(NULL, paths_to_watch_c, 1, &kCFTypeArrayCallBacks);

  FSEventStreamContext context = {
    .version = 0,
    .info = log_queue,
    .retain = (const void* (*)(const void*))&dispatch_retain,
    .release = (void (*)(const void*))&dispatch_release,
    .copyDescription = NULL
  };
  FSEventStreamRef event_stream =
      FSEventStreamCreate(NULL,
                          &EventStreamCallback,
                          &context,
                          paths_to_watch,
                          kFSEventStreamEventIdSinceNow,
                          1,
                          kFSEventStreamCreateFlagFileEvents);

  CFRelease(paths_to_watch);

  FSEventStreamSetDispatchQueue(event_stream, event_queue);

  if (!FSEventStreamStart(event_stream)) {
    fprintf(stderr, "Failed to start FSEventStream\n");
    return EXIT_FAILURE;
  }

  dispatch_main();

  FSEventStreamStop(event_stream);
  FSEventStreamSetDispatchQueue(event_stream, NULL);
  FSEventStreamRelease(event_stream);

  dispatch_release(event_queue);
  dispatch_barrier_sync(log_queue, ^{
      dispatch_release(log_queue);
  });
}
