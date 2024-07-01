# GDK pixbuf loader module for Krita documents

This module enables Krita image support in the default Gnome image viewer, [Eye of Gnome (eog)](https://wiki.gnome.org/Apps/EyeOfGnome).

<img src="https://github.com/vurentjie/kra-gdk-pixbuf/blob/main/screenshot.png?raw=true" style="max-width:500px" />

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

3. Update the query loader cache
  ```sh
  sudo gdk-pixbuf-query-loaders --update-cache
  ```

  ### **Caveats**:
  If `gdk-pixbuf-query-loaders` is not found, you may need to locate it with
  pkg-config and add it to your path.

  Using `pkg-config gdk-pixbuf-2.0 --variable gdk_pixbuf_query_loaders` you can
  narrow the location.

  If this gives `/usr/lib/x86_64-linux-gnu/gdk-pixbuf-2.0/gdk-pixbuf-query-loaders`.
  Then check if `/usr/lib/x86_64-linux-gnu/gdk-pixbuf-2.0/gdk-pixbuf-query-loaders` exists.

  After that link it to a binary directory on your path:
  ```
  sudo ln -s /usr/lib/x86_64-linux-gnu/gdk-pixbuf-2.0/gdk-pixbuf-query-loaders /usr/local/bin/gdk-pixbuf-query-loaders
  sudo gdk-pixbuf-query-loaders --update-cache
  ```

## Screencast

https://github.com/vurentjie/kra-gdk-pixbuf/assets/639806/9d7dabb2-547b-4d39-9712-3a0b94672f08
