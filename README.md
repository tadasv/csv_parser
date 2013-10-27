csv_parser
==========

This library provides a CSV parser to be used in an event loop for processing large amounts of streaming data.
The parser itself does not use any internal buffers. Whenever field value is available a user's specified callback
will be invoked with field data and CSV location (row and column).

See ``examples`` for more information.
