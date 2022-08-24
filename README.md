# libgmobile

Libgmobile carries some helpers for GNOME on mobile devices. Those
parts should move to glib or libgnome-desktop eventually. It is
meant to be used as a git submodule by projects. There aren't
any API stability guarantees at this point in time.

## License

libgmobile is licensed under the LGPLv2.1-or-later.

## Getting the source

```sh
    git clone https://gitlab.gnome.org/guidog/gmobile.git
    cd gmobile.git
```

The `main` branch has the current development version.

## Dependencies
See `meson.build` for required dependencies.

## Building

We use the meson (and thereby Ninja) build system for libgmobile.  The quickest
way to get going is to do the following:

    meson . _build
    ninja -C _build

# API docs
API documentation is available at https://guidog.pages.gitlab.gnome.org/gmobile/

# Getting in Touch
* Issue tracker: https://gitlab.gnome.org/guidog/gmobile/-/issues
* Matrix: https://im.puri.sm/#/room/#phosh:talk.puri.sm

