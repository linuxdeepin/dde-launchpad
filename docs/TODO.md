## Features that are not Implemented

### Simply not implemented

- Windowed frame: DnD
- Favorited: Pin to top

### Related to legacy ApplicationManager daemon.Launcher1 DBus APIs

- Apply proxy settings to application

### Related to plugin APIs

- All plugin-related APIs in dde-launcher

## Known Issues Caused by Dependencies

Status:

- { } Still exists in git master branch
- {T} Fixed from upstream, but no new tag released after the fix
- {A} Resolved and tag released from upstream, but Arch Linux have out-dated package that have the issue

### dde-launchpad itself

- { } Fullscreen frame can be tiled by touchpad guesture (and probably shortcut)

### DTK

- { } Qt6's DDE QPA need to load its plugin before Qt's

### deepin-kwin

- { } The window size might still be fullscreen while trying to toggle from fullscreen frame to windowed frame

## Other TODOs

- Optimize blurhash-related logic

