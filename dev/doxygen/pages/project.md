Project Specification {#doc_project}
====================================

[TOC]

This is the documentation of a LibrePCB project. Such a project is represented by an instance of the
class librepcb::project::Project.


# The File Structure of a Project {#doc_project_filestructure}

TBD


# Compatibility between different application versions {#doc_project_compatibility}

See @ref doc_versioning.


# Avoid opening a project multiple times (Directory Lock) {#doc_project_lock}

Opening a project multiple times simultaneously can be dangerous. Even if the application's design
does not allow to open a project multiple times, it's still possible to save a project in a shared
folder and open it from different computers at the same time (for example). This could demage the
project files.

To avoid such problems, the constructor of librepcb::project::Project tries to lock the project's
directory with a librepcb::DirectoryLock object (librepcb::project::Project::mLock). If the project
is already locked by another application instance, the project cannot be opened again (the
constructor of librepcb::project::Project throws an exception). If the lock file exists because the
application was crashed while the project was open, the user gets a message box which asks whether
the last automatic backup should be restored or not (see also @ref doc_project_save). The lock will
be released automatically when the librepcb::project::Project instance is destroyed (RAII, see
librepcb::DirectoryLock).

@see librepcb::DirectoryLock, librepcb::project::Project::Project(), librepcb::project::Project::mLock


# Saving Procedere / Automatic Periodically Saving {#doc_project_save}

It's important to keep all files of a project valid and consistent. If the application crashes, the
project files must not be demaged. Also a crash while saving the project should not demage the project.

Additionally, an automatic backup/restore system is useful to avoid the loose of all the changes
made in a project after the last saving. If the application crashes with unsaved changes, it should
be possible to restore at lease some of these changes while opening that project the next time. To
detect a crash, the librepcb::DirectoryLock class is used (see @ref doc_project_lock).

**To reach these goals, we use temporary copies of the project files while a project is open. This means:**

1. The project is automatically saved to temporary files periodically (autosave).
   Temporary files are indicated by the '~' at the end of the filename. For example the class
   librepcb::project::Circuit saves its content to `core/circuit.xml~` instead of `core/circuit.xml`.
2. If the user wants to save the project, all changes will be saved to the temporary files first.
   Only after *ALL* changes to the whole project were saved *SUCCESSFULLY* to these files, the
   changes can be written to the original files. The temporary files will still exist. If saving to
   the temporary files was not successful, you must *NOT* begin saving the changes to the original
   files. This way, the original files will stay valid even if there is an error in the save algorithm.
3. On (ordinary) closing a project, all temporary files will be removed from the harddisc.
4. If the application crashes while unsaved changes are made in a project, the temporary files won't
   be removed (and also the lock still exist). On opening this project the next time, the user can
   choose whether to restore the last backup or not. If not, the normal, original files will be
   loaded (as if there was no crash). But if the backup should be restored, the backup files will be
   loaded instead. For this purpose, the constructor of classes like librepcb::project::Project,
   librepcb::project::Circuit and so on needs a parameter `bool restore` (or similar).


**Details of #2 of the list above:**

To save a project, all related classes need a method with this signature:

    bool save(int version, bool toOriginal, QStringList& errors) noexcept;

This method will be called to save all the changes of the corresponding object **and all its child
objects**. So, the class librepcb::project::Project provides such a method to save **the whole
project with all its content** (circuit, schematics and so on). If the parameter `toOriginal` is
`false`, all changes will be saved to the temporary files. Otherwise they are saved to the original
files. Error messages must be appended the `QStringList errors` (after translation). All entries in
the list will be printed out in a message box. The return value is `true` if the saving proccess was
successful, and `false` if there was an error. These methods should never throw an exception (`noexcept`).

If an error occurs while saving the project, you should not abort the saving proccess. "Save all
what you can" is the name of the game :-) Simply add the error message to the error list and return
`false` after the job is finished.

The class librepcb::project::editor::ProjectEditor provides the public slot `saveProject()`. This
method will try to save the whole project to the temporary files. **Only if this call has returned
`true` (project successfully saved to temporary files), it will also save the project to the
original files.**


# The undo/redo system (Command Design Pattern) {#doc_project_undostack}

It's very important to have undo and a redo commands for the whole project (and maybe also for
independent parts of the project). For this purpose we use the "Command Design Pattern". Our common
classes librepcb::UndoCommand and librepcb::UndoStack implement this design pattern. Look at their
documentation for more details.

@note The documentation [Overview of Qt's Undo Framework](http://doc.qt.io/qt-5/qundo.html) and/or
other documentations about the Command Design Pattern may be useful to understand the whole
undo/redo system. Our own undo classes are quite similar to Qt's classes `QUndoCommand` and
`QUndoStack`, but provide some additional functionality.

All librepcb::UndoCommand subclasses which are used to modify a librepcb::project::Project are located
in the namespace librepcb::project. But the librepcb::UndoStack is only used in the namespace
librepcb::project::editor, so it is located there: librepcb::project::editor::ProjectEditor::mUndoStack.

All important changes to the project which should appear in the undo stack need to be implemented as
subclasses of librepcb::UndoCommand and must be appended to the project's undo stack
librepcb::project::editor::ProjectEditor::mUndoStack. Changes which do not need an undo action (like
changing the color of a layer) can be done without the use of the undo/redo system. These changes
then cannot be undone. Basically, all changes to the circuit/schematics/boards and so on must have
an undo action. If you are unsure if you should use the project's undo/redo system, try to answer
the question *do the user wants to undo/redo this action with the undo/redo buttons in the editors?*.

It's possible to use seperate librepcb::UndoStack objects for independent parts of the project. For
example, changing the project settings could be done by using an undo/redo system to provide an undo
command. But these undo stacks need their own undo/redo buttons. The undo/redo buttons in the
schematic editor and in the board editor are only connected to the project's undo stack
librepcb::project::editor::ProjectEditor::mUndoStack.

librepcb::UndoCommand objects can also have an unlimited amount of child objects. This is very
useful to group actions together. For example if multiple symbols are selected in the schematic and
the user wants to remove them all, multiple librepcb::UndoCommand objects will be created (one for
each symbol). But if the user now wants to undo the deletion, the undo command (Ctrl+Z) will only
bring back the last symbol, so the user needs to press the undo command multiple times. This is not
good, so we use the parent/child mechanism of librepcb::UndoCommand. This means that we will still
create an librepcb::UndoCommand for each removed symbol. But we will also create a parent
librepcb::UndoCommand for all these child commands. Only the parent command will be added to the
undo stack. If the user now wants to undo the deletion, he only needs to press undo (Ctrl+Z) once.
And with pressing redo (Ctrl+Y) once, all symbols will be restored.

@see librepcb::UndoCommand, librepcb::UndoStack, librepcb::project::editor::ProjectEditor::mUndoStack
@see http://doc.qt.io/qt-5/qundo.html (Overview of Qt's Undo Framework)
