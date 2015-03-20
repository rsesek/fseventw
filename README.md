# fseventw

This small utility watches for all filesystem write activity using the official CoreServices
FSEvents API. This will report write events on the root filesystem.

Because this uses the CoreServices API rather than the kernel event device, it does not blame the
write to a specific PID.

The output is a continuous stream of messages in the following format:

`event 26fb14f CMRNMOX at /private/var/folders/93/t6gtn1pn4vx9j4wjnzvkzz8m003bw/T/.com.google.Chrome.6oIYcA`

The first HEX number is the event ID. The second string describes the type of change that occurred.
Each position refers to a specific type of modification. In order:

* C - Create
* M - Modify
* R - Remove
* N - Rename
* M - Metadata modify
* O - Owner modify
* X - Xattr modify

And the last string is the path on which the event occured.
