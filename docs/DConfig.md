## Overview

This application uses DTK's [DConfig solution](https://github.com/linuxdeepin/deepin-specifications/blob/55f6d0d2bc8cfd4a9611057e2af1033813842201/unstable/%E9%85%8D%E7%BD%AE%E6%96%87%E4%BB%B6%E8%A7%84%E8%8C%83.md) to store OEM-able configurations.

## Usage

### Common

DTK's DConfig requires a base/default configuration template (in DConfig spec it called "Configuration Description File") to be placed in the correct location to work, it also provides the ability to allow OEM manufactures or redistributors override the default configuration when needed.

DConfig has its own service to manage all DConfig-based configuration files, which is also required to make DConfig works. Please ensure [dde-app-service](https://github.com/linuxdeepin/dde-app-services) (or whatever the name is in your distro) is packaged, installed and enabled.

For detailed usage information, please consult the [DConfig spec](https://github.com/linuxdeepin/deepin-specifications/blob/55f6d0d2bc8cfd4a9611057e2af1033813842201/unstable/%E9%85%8D%E7%BD%AE%E6%96%87%E4%BB%B6%E8%A7%84%E8%8C%83.md) and [Dtk::Core::DConfig documentation](https://linuxdeepin.github.io/dtkcore/classDtk_1_1Core_1_1DConfig.html). We will only cover the generic usage/cases here.

### Development

Some heads-up:

- To make DConfig-based settings works, **please ensure the configration template files are installed**.
- `dde-dconfig-editor` is a GUI util to tweak those values.
  - `readonly` fields will be read-only in the editor, but it's override-able!
- `dde-dconfig` cli util can be used to test out the config values when needed:
  - Example usage: `dde-dconfig get -a dde-launchpad -r org.deepin.dde.launchpad.appsmodel -k excludeAppIdList`
- If you just changed the on-disk base configuration template file or override file, dde-dconfig service might still holding a outdated cache!
  - To avoid that, simply restart the service: `systemctl restart dde-dconfig-daemon.service`.
  - `dde-config`'s `reset` option won't clear the default value cache that offered by base configuration template file and override file. i.e. things like `dde-dconfig reset -a dde-launchpad -r org.deepin.dde.launchpad.appsmodel -k excludeAppIdList` won't work as intended.

### OEM Configuration Quick Guide

The base configuration template is **usually** located at `/usr/share/dsg/configs/${app-id}/${conf-desc-file}.json`. Redistributors can override this configuration template by putting files under `/usr/share/dsg/configs/overrides/${app-id}/${conf-desc-file}/${preferred-oem-filename}.json`. Please note the json structure of the base template file and the override file are different. Please consult the related documentations for detail information.

- `${app-id}`: the name of the executable, `dde-launchpad` in this case.
- `${conf-desc-file}`: the name of the base configuration template file, without the `.json` suffix.
- `${preferred-oem-filename}`: can be any name, please notice only latin characters are allowed.

`dde-dconfig-editor` can be used to generate the override file. Currently, if a curtain configurable key's permissions is `readonly`, it won't be editable in the editor, but you can still override the value by putting a override file manually.

## Configurations

### org.deepin.dde.launchpad.appsmodel

#### `excludeAppIdList` (readonly)

The application id list that shouldn't be displayed in dde-launchpad. The application id is its freedeskop.org [`desktop-entry-spec` desktop file id](https://specifications.freedesktop.org/desktop-entry-spec/latest/ar01s02.html#desktop-file-id).

> [!IMPORTANT]
> Think twice before actually using this field. If you are application developer or distro package maintainer, please consider use the [`OnlyShowIn`, `NotShowIn` or `Hidden` field](https://specifications.freedesktop.org/desktop-entry-spec/latest/ar01s06.html) inside the application's desktop file.

#### `compulsoryAppIdList` (readonly)

The application id list that is considered as compulsory for the current desktop environment. The "Uninstall" menu entry will be disabled for these applications. The application id is its freedeskop.org [`desktop-entry-spec` desktop file id](https://specifications.freedesktop.org/desktop-entry-spec/latest/ar01s02.html#desktop-file-id).

> [!IMPORTANT]
> Think twice before actually using this field. If you are application developer or distro package maintainer, please consider ship an AppStream metadata file to mark your application as a compulsory component. See: [`<compulsory_for_desktop/>`](https://www.freedesktop.org/software/appstream/docs/chap-Metadata.html#tag-compulsory_for_desktop).

#### `frequentlyUsedAppIdList` (readonly)

The default application id list that will be displayed in the "my frequency used" section when users haven't interact with any program.

> [!NOTE]
> This section will only show max to 16 (4x4) items, and it will only show 12 (4x3) items when "recently installed" section is visible. So it's suggested to ensure the list is equal or less than 16 items.

#### `categoryType`

The default sort and group type for the app list in windowed mode launchpad. Value 0 for alphabetary, 1 for DDE-style category, 2 for freeform sort. Such value can be override from the UI by the end user.
