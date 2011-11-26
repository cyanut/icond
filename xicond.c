#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <librsvg/rsvg.h>
#include <gtk/gtk.h>
int verbose = 1;

typedef unsigned long int CARD32;

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


CARD32 *pixbuf2card32(GdkPixbuf *pix){
    int imgw, imgh, n_channels, i, j;
    GError *imgfileerr;
    CARD32 *data;
    guchar *pixels;

    n_channels = gdk_pixbuf_get_n_channels(pix);
    asrt(gdk_pixbuf_get_colorspace(pix) == GDK_COLORSPACE_RGB,
            "image not in RGB!");
    asrt((gdk_pixbuf_get_bits_per_sample(pix) == 8 &&
                gdk_pixbuf_get_has_alpha(pix) &&
                n_channels == 4),
            "Not 32 bit RGBA image!");
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
        data[i] += (int)(*(pixels++)) << 24; //A 
    }
    g_object_unref(pix);
    return data;
}



int main(int argc, char **argv){
    //g_type_init();
    gdk_init(&argc, &argv);
    Display *dpy = XOpenDisplay(NULL);
    Atom actual_type;
    int actual_format;
    unsigned long nitems;
    unsigned long bytes;
    unsigned char *data;
    unsigned char *name;
    int i;
    XEvent e;
    GtkIconTheme *gicontheme;
    GList *giconlist;
    GList *p;
    Window wid;

    if (!dpy) err("cannot get display");

    gicontheme = gtk_icon_theme_get_default();
    giconlist = gtk_icon_theme_list_icons(gicontheme, "Applications");
    /*
    p = giconlist;
    while (p != NULL){
        printf("icon - %s\n", p->data);
        p = p->next;
        //g_free(giconlist);
        giconlist = p;
    }
    
    exit(1);*/
    XSelectInput(dpy, DefaultRootWindow(dpy), SubstructureNotifyMask);
    for (;;){
        XNextEvent(dpy, &e);
        if (e.type == CreateNotify){
            wid = e.xcreatewindow.window;
            int status = XGetWindowProperty(
                    dpy,
                    wid,
                    XInternAtom(dpy, "_NET_WM_ICON_NAME", True),
                    0,
                    (~0L),
                    False,
                    AnyPropertyType,
                    &actual_type,
                    &actual_format,
                    &nitems,
                    &bytes,
                    &name);
            if (name){ 
                printf("icon name = %s\n", name);
                printf("at %d,%d of %d*%d: parent=0x%x, window=0x%x\n", 
                        e.xcreatewindow.x, 
                        e.xcreatewindow.y, 
                        e.xcreatewindow.width, 
                        e.xcreatewindow.height, 
                        (int) e.xcreatewindow.parent, 
                        (int) wid);
                //XFree(name); 
                
                Atom *proplist;
                Atom iconatom;
                int n_prop;
                iconatom = XInternAtom(dpy, "_NET_WM_ICON", 0);
                proplist = XListProperties(dpy, wid, &n_prop);
                for (i=0; i<n_prop && proplist[i] != iconatom; i++)
                    printf("%s vs %s\n", XGetAtomName(dpy, proplist[i]), 
                            XGetAtomName(dpy, iconatom));
                if (i == n_prop){
                    printf("this window has no icon\n");
                    continue;
                }
                XFree(proplist);
                CARD32 *iconcard32;
                GdkPixbuf *iconpix;
                GError *err = NULL;
                iconpix = gtk_icon_theme_load_icon(gicontheme, name, 128, 0, &err);
                printf("icon %s at %p\n", name, iconpix);
                //iconpix = rsvg_pixbuf_from_file_at_size("./abiword.svg", 48, 48, &err);
                printf("iconpix @ %p\n", iconpix);
                if (iconpix == NULL) continue;
                iconcard32 = pixbuf2card32(iconpix);
                int success = XChangeProperty(
                        dpy,
                        wid,
                        iconatom,
                        XA_CARDINAL,
                        32,
                        PropModeReplace,
                        (gchar*)iconcard32,
                        48 * 48 + 2);
                if (success != NULL)
                    printf("image: %dx%d\n", *(iconcard32), *(iconcard32+1));
                printf("#655pixel=%d\n", *(iconcard32+655));
                //g_free(iconcard32);
                
            }
        }
    }
    return 0;
}

