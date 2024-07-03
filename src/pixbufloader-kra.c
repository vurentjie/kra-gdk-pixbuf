/*
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, see <http://www.gnu.org/licenses/>.
 */

/*
 * This is a highly modified copy of the pixbuf loader that comes with libheif
 * (https://github.com/strukturag/libheif/blob/master/gdk-pixbuf/pixbufloader-heif.c)
 * I have altered significant portions of it to instead extract the
 * mergedimage.png that is included in a Krita document.
 */

#define GDK_PIXBUF_ENABLE_BACKEND

#include <gdk-pixbuf/gdk-pixbuf-io.h>
#include <png.h>
#include <zip.h>

typedef struct {
  GdkPixbufModuleUpdatedFunc update_func;
  GdkPixbufModulePreparedFunc prepare_func;
  GdkPixbufModuleSizeFunc size_func;
  gpointer user_data;
  GByteArray* data;
} KritaPixbufContext;

static void png_read_zip(png_structp png_ptr, png_bytep outBytes, png_size_t byteCountToRead) {
  struct zip_file* zf = (struct zip_file*)png_get_io_ptr(png_ptr);
  zip_fread(zf, outBytes, byteCountToRead);
}

static void gdk_pixbuf_destroy_notify(guchar* pixels, gpointer data) {
  free(pixels);
}

static gboolean extract_png_from_zip(struct zip_file* zf,
                                     guchar** image_data,
                                     gint* width,
                                     gint* height,
                                     gint* channels,
                                     GError** error) {
  png_structp png_ptr;
  png_infop info_ptr;
  png_bytep* row_pointers = NULL;
  png_byte header[8];
  int bytes_read;

  bytes_read = zip_fread(zf, header, 8);
  if (bytes_read != 8 || png_sig_cmp(header, 0, 8)) {
    g_warning("%s", "Not a valid PNG");
    g_set_error(error, G_FILE_ERROR, G_FILE_ERROR_FAILED, "Not a valid PNG file");
    return FALSE;
  }

  png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
  if (!png_ptr) {
    g_warning("%s", "Failed to create read struct");
    g_set_error(error, G_FILE_ERROR, G_FILE_ERROR_FAILED, "Failed to create PNG read struct");
    return FALSE;
  }

  info_ptr = png_create_info_struct(png_ptr);
  if (!info_ptr) {
    png_destroy_read_struct(&png_ptr, NULL, NULL);
    g_warning("%s", "Failed to create info struct");
    g_set_error(error, G_FILE_ERROR, G_FILE_ERROR_FAILED, "Failed to create PNG info struct");
    return FALSE;
  }

  if (setjmp(png_jmpbuf(png_ptr))) {
    png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
    if (row_pointers) {
      free(row_pointers);
    }
    g_warning("%s", "Unknown error reading PNG");
    g_set_error(error, G_FILE_ERROR, G_FILE_ERROR_FAILED, "Error during PNG creation");
    return FALSE;
  }

  png_set_read_fn(png_ptr, zf, png_read_zip);

  png_set_sig_bytes(png_ptr, 8);
  png_read_info(png_ptr, info_ptr);

  *width = png_get_image_width(png_ptr, info_ptr);
  *height = png_get_image_height(png_ptr, info_ptr);
  *channels = png_get_channels(png_ptr, info_ptr);

  size_t row_bytes = png_get_rowbytes(png_ptr, info_ptr);
  *image_data = (guchar*)malloc(row_bytes * (*height) * sizeof(guchar));
  row_pointers = (png_bytep*)malloc((*height) * sizeof(png_bytep));

  for (int y = 0; y < *height; y++) {
    row_pointers[y] = (*image_data) + y * row_bytes;
  }

  png_read_image(png_ptr, row_pointers);

  png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
  free(row_pointers);

  return TRUE;
}

static gpointer begin_load(GdkPixbufModuleSizeFunc size_func,
                           GdkPixbufModulePreparedFunc prepare_func,
                           GdkPixbufModuleUpdatedFunc update_func,
                           gpointer user_data,
                           GError** error) {
  KritaPixbufContext* ctx;

  ctx = g_new0(KritaPixbufContext, 1);
  ctx->data = g_byte_array_new();
  ctx->size_func = size_func;
  ctx->prepare_func = prepare_func;
  ctx->update_func = update_func;
  ctx->user_data = user_data;
  return ctx;
}

static gboolean stop_load(gpointer context, GError** error) {
  KritaPixbufContext* ctx;
  GdkPixbuf* pixbuf = NULL;
  zip_source_t* zs = NULL;
  struct zip* za = NULL;
  struct zip_file* zf = NULL;
  guchar* image_data = NULL;
  gint width, height, channels;
  GError* png_err = NULL;
  gboolean result = FALSE;
  zip_error_t ze;
  GByteArray* data = NULL;

  ctx = (KritaPixbufContext*)context;
  zip_error_init(&ze);
  data = ctx->data;

  zs = zip_source_buffer_create((guchar*)data->data, data->len, 0, &ze);

  if (zs == NULL) {
    result = FALSE;
    goto cleanup;
  }

  za = zip_open_from_source(zs, 0, &ze);

  if (za == NULL) {
    result = FALSE;
    goto cleanup;
  }

  zf = zip_fopen(za, "mergedimage.png", 0);

  if (zf == NULL) {
    result = FALSE;
    goto cleanup;
  }

  if (!extract_png_from_zip(zf, &image_data, &width, &height, &channels, &png_err)) {
    result = FALSE;
    goto cleanup;
  }

  pixbuf = gdk_pixbuf_new_from_data(image_data,
                                    GDK_COLORSPACE_RGB,
                                    channels == 4,
                                    8,
                                    width,
                                    height,
                                    width * channels,
                                    gdk_pixbuf_destroy_notify,
                                    NULL);

  if (!pixbuf) {
    result = FALSE;
    goto cleanup;
  }

  if (ctx->prepare_func) {
    (*ctx->prepare_func)(pixbuf, NULL, ctx->user_data);
  }

  if (ctx->update_func != NULL) {
    (*ctx->update_func)(pixbuf,
                        0,
                        0,
                        gdk_pixbuf_get_width(pixbuf),
                        gdk_pixbuf_get_height(pixbuf),
                        ctx->user_data);
  }

  result = TRUE;

cleanup:

  if (zf) {
    zip_fclose(zf);
  }

  if (za) {
    zip_close(za);
  }

  if (pixbuf) {
    g_clear_object(&pixbuf);
  }

  if (ctx->data) {
    g_byte_array_free(ctx->data, TRUE);
  }

  g_free(ctx);

  return result;
}

static gboolean load_increment(gpointer context, const guchar* buf, guint size, GError** error) {
  KritaPixbufContext* ctx = (KritaPixbufContext*)context;
  g_byte_array_append(ctx->data, buf, size);
  return TRUE;
}

G_MODULE_EXPORT void fill_vtable(GdkPixbufModule* module) {
  module->begin_load = begin_load;
  module->stop_load = stop_load;
  module->load_increment = load_increment;
}

G_MODULE_EXPORT void fill_info(GdkPixbufFormat* info) {
  static GdkPixbufModulePattern signature[] = {{"PK\003\004", "    ", 100}, {NULL, NULL, 0}};
  static const gchar* mime_types[] = {"application/x-krita", "image/openraster", NULL};
  static const gchar* extensions[] = {"kra", "ora", NULL};

  info->name = "krita/kra";
  info->signature = (GdkPixbufModulePattern*)signature;
  info->description = "Krita document";
  info->mime_types = (gchar**)mime_types;
  info->extensions = (gchar**)extensions;
  info->flags = GDK_PIXBUF_FORMAT_THREADSAFE;
  info->disabled = FALSE;
  info->license = "LGPL";
}
