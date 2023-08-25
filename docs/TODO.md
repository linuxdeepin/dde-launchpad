## Features that are not Implemented

### Simply not implemented

- Fullscreen frame: DnD
- Windowed frame: DnD
- Favorited: Pin to top

### Related to legacy ApplicationManager daemon.Launcher1 DBus APIs

- Apply proxy settings to application
- Uninstall application

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

- {T} PageIndicator is not visible
- { } Click fallthough Popup component
- { } ListView missing keyboard focus navigation
- {A} dtkwidget DHiDPIHelper usage (this API moved to dtkgui)

### dde-dock

- {T} Missing dock position change signal

### deepin-kwin

- {A} Freeze when launching UI right after launch the program (Appearance1 DBus API caused deepin-kwin freezing, results UI blocking)
- { } The window size might still be fullscreen while trying to toggle from fullscreen frame to windowed frame

## Other TODOs

- Optimize blurhash-related logic

