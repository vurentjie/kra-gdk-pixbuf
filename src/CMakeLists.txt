find_package(PkgConfig)

pkg_check_modules(GDKPIXBUF2 REQUIRED gdk-pixbuf-2.0)
pkg_check_modules(LIBPNG REQUIRED libpng)
pkg_check_modules(LIBZIP REQUIRED libzip)
pkg_check_modules(GLIB REQUIRED glib-2.0)

execute_process(
  COMMAND ${PKG_CONFIG_EXECUTABLE} gdk-pixbuf-2.0 --variable gdk_pixbuf_moduledir
  --define-variable=prefix=${CMAKE_INSTALL_PREFIX} OUTPUT_VARIABLE GDKPIXBUF2_MODULE_DIR
  OUTPUT_STRIP_TRAILING_WHITESPACE)

add_library(pixbufloader-kra MODULE pixbufloader-kra.c)

target_include_directories(
  pixbufloader-kra
  PRIVATE
  ${GDKPIXBUF2_INCLUDE_DIRS}
  ${GLIB_INCLUDE_DIRS}
  ${LIBPNG_INCLUDE_DIRS}
  ${LIBZIP_INCLUDE_DIRS}
)

target_link_directories(
  pixbufloader-kra
  PRIVATE
  ${GDKPIXBUF2_LIBRARY_DIRS}
)

target_link_libraries(
  pixbufloader-kra
  PUBLIC
  ${GLIB_LIBRARIES}
  ${GDKPIXBUF2_LIBRARIES}
  ${LIBPNG_LIBRARIES}
  ${LIBZIP_LIBRARIES}
)

install(
  TARGETS pixbufloader-kra
  LIBRARY DESTINATION "${GDKPIXBUF2_MODULE_DIR}"
)
