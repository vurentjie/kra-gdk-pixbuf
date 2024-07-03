> (Note: Gnome replaced [Eye of Gnome](https://wiki.gnome.org/Apps/EyeOfGnome) with a new default image viewer called [Loupe](https://welcome.gnome.org/app/Loupe/). This module will not work for the newer viewer. You need to install [Eye of Gnome](https://wiki.gnome.org/Apps/EyeOfGnome) instead. [Eye of Gnome](https://wiki.gnome.org/Apps/EyeOfGnome) is still currently the default viewer for Ubuntu 24.04.)

# GDK pixbuf loader module for Krita documents

This module enables Krita image support in the Gnome image viewer, [Eye of Gnome (eog)](https://wiki.gnome.org/Apps/EyeOfGnome).

Image formats handled are:
- **application/x-krita**: `.kra` files
- **image/openraster**: `.ora` files

<img src="https://github.com/vurentjie/kra-gdk-pixbuf/blob/main/screenshot.png?raw=true" style="width:500px" />

## Installation

You can check the [releases page](https://github.com/vurentjie/kra-gdk-pixbuf/releases) for a `.deb` package. Otherwise you will need to
build it manually.

If you are installing the deb via dpkg, you may need to run `apt -f install` afterwards to fix missing dependencies.

```
sudo dpkg -i /path/to/kra_gdk_pixbuf.deb 
sudo apt -f install
```

## Building

You can build and install the module with these steps:

1. Install the dependencies (assuming you are using apt)
  ```sh
  sudo apt install cmake libzip-dev libgdk-pixbuf-2.0-dev libpng-dev libglib2.0-dev
  ```

2. Clone the repo, build the package, and install it
  ```sh
  git clone https://github.com/vurentjie/kra-gdk-pixbuf
  cd kra-gdk-pixbuf
  mkdir build
  cd build
  cmake -DCMAKE_INSTALL_PREFIX=/usr ..
  cmake ..
  make
  sudo make install
  ```

3. You need to manually update the query loader cache by running:
  ```sh
  sudo gdk-pixbuf-query-loaders --update-cache
  ```

  ### **Caveats**:
  If `gdk-pixbuf-query-loaders` is not found, you may need to locate it with
  pkg-config and add it to your path and manually run it.

  Using `pkg-config gdk-pixbuf-2.0 --variable gdk_pixbuf_query_loaders` you can
  narrow the location.

  If this gives `/usr/lib/x86_64-linux-gnu/gdk-pixbuf-2.0/gdk-pixbuf-query-loaders`.
  Then link it to a binary directory on your path:
  ```
  sudo ln -s /usr/lib/x86_64-linux-gnu/gdk-pixbuf-2.0/gdk-pixbuf-query-loaders /usr/local/bin/gdk-pixbuf-query-loaders
  sudo gdk-pixbuf-query-loaders --update-cache
  ```

## Screencast

https://github.com/vurentjie/kra-gdk-pixbuf/assets/639806/9d7dabb2-547b-4d39-9712-3a0b94672f08

## Additional Resources

If you are looking for thumbnail support then check instructions at the [gnome-kra-ora-thumbnailer release](https://github.com/Deevad/gnome-kra-ora-thumbnailer/releases).
