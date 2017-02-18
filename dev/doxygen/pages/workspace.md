Workspace Documentation {#doc_workspace}
========================================

[TOC]

# File Structure {#doc_workspace_file_structure}

Libraries are stored as directories in the LibrePCB workspace.

- There can be local libraries and remote libraries in a workspace:
    - Local libraries are writable, but cannot be updated automatically
    - Remote libraries are managed by LibrePCB and are read-only

The whole library is located in the workspace directory "library". This folder contains the following subdirectories:
    - local.lplib:  All local library elements (created by the user or imported)
    - *.lplib       For each downloaded library a separate directory. These directories contain all library elements from the appropriate repository.

# Library Index SQLite Database {#doc_workspace_library_database}

For better performance of the workspace library (searching in the filesystem can be quite slow), a
[SQLite] database is used for caching/indexing (see librepcb::workspace::WorkspaceLibraryDb). From
time to time this database is filled/updated with metadata of all libraries and their elements.
Searching for elements is then done with `SELECT` statements in the database, which is very fast.

The database is stored in the `v0.1/metadata` directory of the workspace (where `v0.1` corresponds
to the file format version).

[SQLite]: https://sqlite.org/ "SQLite"



































