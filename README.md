# DDE Launchpad

The "launcher" or "start menu" component for DDE.

## Dependencies

Check `debian/control` for build-time and runtime dependencies, or use `cmake` to check the missing required dependencies.

## Building

Regular CMake building steps applies, in short:

```shell
$ mkdir build && cd build
$ cmake ..
$ cmake --build .
```

After building, a `dde-launchpad` binary can be found inside the build folder. And you can optionally install it by:

```shell
$ cmake --build . --target install # only do this if you know what you are doing
```

A `debian` folder is provided to build the package under the *deepin* linux desktop distribution. To build the package, use the following command:

```shell
$ sudo apt build-dep . # install build dependencies
$ dpkg-buildpackage -uc -us -nc -b # build binary package(s)
```

### Update Translations

Once CMake configuration phase is done, use the `update_translations` target to update the `.ts` files:

```shell
$ cmake --build . --target update_translations
$ # *OR* you can do it manually. Assume you are using GNU Make:
$ make update_translations
```

## Getting Involved

- [Code contribution via GitHub](https://github.com/linuxdeepin/dde-launchpad/)
- [Submit bug or suggestions to GitHub Issues or GitHub Discussions](https://github.com/linuxdeepin/developer-center/issues/new/choose)
- Contribute translation: [deepin-translation-program](https://www.deepin.org/index/zh/docs/sig/sig/deepin-translation-program/README.md)

## License

**dde-launchpad** is licensed under [GPL-3.0-or-later](LICENSE).
