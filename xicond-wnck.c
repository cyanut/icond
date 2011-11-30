
#define WNCK_I_KNOW_THIS_IS_UNSTABLE
#define ICON_SIZE 48

#include <libwnck/libwnck.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
typedef unsigned long int CARD32;

Display *dpy;
GtkIconTheme *gicontheme;


void err(char *errmsg){
    printf("%s\n", errmsg);
    exit(1);
}

void asrt(int b, char *errmsg){
    if (!b){
        printf("%s\n", errmsg);
        exit(1);
    }
}


void lower(char *s){
    int i;
    for (i=0; i<strlen(s); i++)
        s[i] = (s[i] > 64 && s[i] < 91) ? s[i] : s[i] + 32;
}

CARD32 *pixbuf2card32(GdkPixbuf *pix){
    int imgw, imgh, n_channels, i, j;
    CARD32 *data;
    guchar *pixels;

    n_channels = gdk_pixbuf_get_n_channels(pix);
    asrt(gdk_pixbuf_get_colorspace(pix) == GDK_COLORSPACE_RGB,
            "image not in RGB!");
    asrt(gdk_pixbuf_get_bits_per_sample(pix), "Not 32 bit RGBA image!");
    imgw = gdk_pixbuf_get_width(pix);
    imgh = gdk_pixbuf_get_height(pix);
    printf("Pixbuf %d x %d\n", imgw, imgh);
    pixels = gdk_pixbuf_get_pixels(pix);
    data = g_new0(CARD32, (imgw * imgh + 2));
    data[0] = imgw;
    data[1] = imgh;
    for (i=2; i < imgw * imgh + 2; i++){
        for (j=2; j>=0; j--)
            data[i] += (int)(*(pixels++)) << j*8; //RGB
        if (n_channels == 4)
            data[i] += (int)(*(pixels++)) << 24; //A 
    }
    return data;
}


void on_window_opened(WnckScreen *screen, WnckWindow *win){
    WnckApplication *app = wnck_window_get_application(win);
    char *icon_name = (char *) wnck_application_get_icon_name(app);
    Window wid = (Window) wnck_window_get_xid(win);
    CARD32 *iconcard32;
    GdkPixbuf *iconpix;
    GError *err = NULL;
    iconpix = gtk_icon_theme_load_icon(gicontheme, icon_name,
            ICON_SIZE, 0, &err);
    if (iconpix == NULL){
        lower(icon_name);
        g_error_free(err);
        err = NULL;
        iconpix = gtk_icon_theme_load_icon(gicontheme, icon_name,
               ICON_SIZE, 0, &err);
    }
    if (iconpix == NULL) {
        g_error_free(err);
        return;
    } 
    printf("icon %s at %p\n", icon_name, (void *)iconpix);
    iconcard32 = pixbuf2card32(iconpix);
    Atom iconatom = XInternAtom(dpy, "_NET_WM_ICON", False); 
    int success = XChangeProperty(
            dpy,
            wid,
            iconatom,
            XA_CARDINAL,
            32,
            PropModeReplace,
            (unsigned char *)iconcard32,
            48 * 48 + 2);
    if (success != 0){
        printf("image: %dx%d\n", (int)*(iconcard32), (int)*(iconcard32+1));
        XFlush(dpy);
    }
    g_free(iconcard32);
    g_object_unref(iconpix);
    //g_free(icon_name);
}    

int main(int argc, char **argv){
    GMainLoop *loop;
    WnckScreen *screen;
    
    gdk_init(&argc, &argv);
    dpy = XOpenDisplay(NULL);
    gicontheme = gtk_icon_theme_get_default();
    loop = g_main_loop_new(NULL, FALSE);
    screen = wnck_screen_get_default();
    g_signal_connect(screen, "window-opened", 
            G_CALLBACK(on_window_opened), NULL);
    g_main_loop_run(loop);
    g_main_loop_unref(loop);
    return 0;
}
