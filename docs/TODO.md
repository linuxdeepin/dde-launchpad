## Features are not Implemented

### Simply not implemented

- Fullscreen frame: DnD
- Windowed frame: DnD
- Favorited: Pin to top

### Related to legacy ApplicationManager daemon.Launcher1 DBus APIs

- Send to desktop/remove from desktop
- Send to dock/remove from dock
- Auto-start/remove auto-start
- Apply proxy settings to application
- Uninstall application

### Related to plugin APIs

- All plugin-related APIs in dde-launcher

## Known Issues Caused by Dependencies

Status:

- { } Still exists in git master branch
- {T} Fixed from upstream, but no new tag released after the fix
- {A} Resolved and tag released from upstream, but Arch Linux have out-dated package that have the issue

### DTK

- { } PageIndicator is not visible
- { } Click fallthough Popup component
- {A} dtkwidget DHiDPIHelper usage (this API moved to dtkgui)

### dde-dock

- {T} Missing dock position change signal

### deepin-kwin

- {A} Freeze when launching UI right after launch the program (Appearance1 DBus API caused deepin-kwin freezing, results UI blocking)

## Other TODOs

- Optimize blurhash-related logic

