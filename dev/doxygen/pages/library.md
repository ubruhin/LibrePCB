Library Documentation {#doc_library}
====================================

[TOC]

![overview](library_structure.svg)

# File Structure {#doc_library_file_structure}

A LibrePCB library is represented by a regular directory on the file system. All metadata and
library elements are stored in files and subdirectories of the library's root directory.

Library directories have the suffix `.lplib` to easily recognize directories as LibrePCB libraries.

This tree shows the structure of a library directory:

    MyLibrary.lplib/
    ├── .librepcb-lib
    ├── library.png
    ├── library.xml
    ├── cmp/
    │   └── 670421c3-850b-4d7c-b511-c0e6b7ac40f9/
    │       ├── component.xml
    │       └── .librepcb-cmp
    ├── cmpcat/
    │   └── c49eb199-2cf5-45da-a740-7d444045c08a/
    │       ├── component_category.xml
    │       └── .librepcb-cmpcat
    ├── dev/
    │   └── 6c696128-6e7e-401c-92af-57f1e340da52/
    │       ├── device.xml
    │       └── .librepcb-dev
    ├── pkg/
    │   └── 556c2a2f-8462-4acd-bed1-3218d67c774b/
    │       ├── package.xml
    │       └── .librepcb-pkg
    ├── pkgcat/
    │   └── b66c9cf9-99b6-434e-af72-867cee1a271c/
    │       ├── package_category.xml
    │       └── .librepcb-pkgcat
    └── sym/
        └── ad26f787-cdc9-47f9-963f-ab5a4767cdb4/
            ├── symbol.xml
            └── .librepcb-sym

## .librepcb-lib

This file indicates that the directory is a LibrePCB library and also contains the file format version.
See [File Format Versioning] for more information about LibrePCB's file format versioning system.

## library.png

This (optional) picture is displayed on different locations in the graphical user interface (e.g. in
the library manager).

## library.xml

This file contains some metadata about the library itself (e.g. name, author, dependencies).

## cmp, cmpcat, dev, pkg, pkgcat, sym

These directories contain all the library elements:

- `cmp`: Components (librepcb::library::Component)
- `cmpcat`: Component Categories (librepcb::library::ComponentCategory)
- `dev`: Devices (librepcb::library::Device)
- `pkg`: Packages (librepcb::library::Package)
- `pkgcat`: Package Categories (librepcb::library::PackageCategory)
- `sym`: Symbols (librepcb::library::Symbol)

All of these directories are optional. For example if a library does not provide any Symbols, the
`sym` directory may not exist.

Each library element inside these directories is represented by a subdirectory whose name
corresponds to the element's [UUID]. The element directories contain following files:

- An identification file (e.g. `.librepcb-cmp`) used for [File Format Versioning].
- An XML file (e.g. `component.xml`) containing the actual library element (metadata, geometry, ...).
- Depending on the element type, there may be more files (e.g. 3D models for Packages).


# Identification of Library Elements {#doc_library_identification}

Each library element (and also the library itself) gets a [UUID] at their creation. This [UUID] is
then used to identify these elements. For example a librepcb::library::Device references to a
librepcb::library::Package with the [UUID] of the package. This allows even to combine library
elements across different repositories.

To avoid broken dependencies, two rules needs to be followed:

- The [UUID] of each library element must never change.
- The "interface" of each library element must never change. For example the interface of a
  librepcb::library::Symbol is defined by their pins ([UUID] and meaning of every pin). If the count,
  [UUID] or the meaning of pins changes, all librepcb::library::Component which have references to
  that librepcb::library::Symbol will break their functionality.

If one of these things need to be changed, this must be done by creating a new element (with a new
[UUID]) and (optionally) marking the original element as deprecated.


# Internationalization of Library Elements {#doc_library_i18n}

Each library element must contain English (`en_US`) texts (name, description, keywords, ...), other
languages are optional. The user can define his favorite language in his workspace settings, and
LibrePCB shows all library elements in this language (if available). English is always the fallback
language if the desired language is not available.


# Categorization of Library Elements {#doc_library_categories}

LibrePCB uses category elements to categorize all other library elements. Category elements are like
normal library elements, but cannot be used in schematics or boards. Their only purpose is to
provide a category tree in the libraries, where all the other library elements can be assigned to
unlimited many categories.

This system allows the user to choose other elements (like a librepcb::library::Component in a
librepcb::project::Schematic) by browsing through that category tree.

There exist two types of categories, while all other element types use exactly one of them:

- librepcb::library::ComponentCategory (e.g. "Diodes", "Resistors", ...) are used for:
    - librepcb::library::Symbol
    - librepcb::library::Component
    - librepcb::library::Device
- librepcb::library::PackageCategory (e.g. "DIP", "TQFP", "BGA", ...) are used for:
    - librepcb::library::Package


[UUID]: https://en.wikipedia.org/wiki/Universally_unique_identifier "Universally Unique Identifier"
[File Format Versioning]: @ref doc_versioning






































