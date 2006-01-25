/* this code only works for Linux kernels */
#ifdef __linux__


#include <linux/input.h>
#include <sys/ioctl.h>

#include <sys/stat.h>
#include <string.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/fcntl.h>
#include <unistd.h>

#include "hid.h"

#define DEBUG(x)
//#define DEBUG(x) x 

/*------------------------------------------------------------------------------
 * from evtest.c from the ff-utils package
 */

#define BITS_PER_LONG (sizeof(long) * 8)
#define NBITS(x) ((((x)-1)/BITS_PER_LONG)+1)
#define OFF(x)  ((x)%BITS_PER_LONG)
#define BIT(x)  (1UL<<OFF(x))
#define LONG(x) ((x)/BITS_PER_LONG)
#define test_bit(bit, array)	((array[LONG(bit)] >> OFF(bit)) & 1)


/*
 * from an email from Vojtech:
 *
 * The application reading the device is supposed to queue all events up to 
 * the SYN_REPORT event, and then process them, so that a mouse pointer
 * will move diagonally instead of following the sides of a rectangle, 
 * which would be very annoying. 
 */


/* ------------------------------------------------------------------------------ */
/* LINUX-SPECIFIC SUPPORT FUNCTIONS */
/* ------------------------------------------------------------------------------ */

void hid_convert_linux_buttons_to_numbers(__u16 linux_code, char *hid_code)
{
    if(linux_code >= 0x100) 
    {
        if(linux_code < BTN_MOUSE)   
            sprintf(hid_code,"btn_%d",linux_code - BTN_MISC);  /* numbered buttons */
        else if(linux_code < BTN_JOYSTICK)
            sprintf(hid_code,"btn_%d",linux_code - BTN_MOUSE);  /* mouse buttons */
        else if(linux_code < BTN_GAMEPAD)
            sprintf(hid_code,"btn_%d",linux_code - BTN_JOYSTICK);  /* joystick buttons */
        else if(linux_code < BTN_DIGI)
            sprintf(hid_code,"btn_%d",linux_code - BTN_GAMEPAD);  /* gamepad buttons */
        else if(linux_code < BTN_WHEEL)
            sprintf(hid_code,"btn_%d",linux_code - BTN_DIGI);  /* tablet buttons */
        else if(linux_code < KEY_OK)
            sprintf(hid_code,"btn_%d",linux_code - BTN_WHEEL);  /* wheel buttons */
    }
}

/* Georg Holzmann: implementation of the keys */
void hid_convert_linux_keys(__u16 linux_code, char *hid_code)
{  
    if(linux_code > 226)
        return;
  
    /*         quick hack to get the keys         */
    /* (in future this should be auto-generated)  */

    char key_names[227][20] =
        { 
            "reserved", "esc", "1", "2", "3", "4", "5", "6", "7",
            "8", "9", "0", "minus", "equal", "backspace", "tab",
            "q", "w", "e", "r", "t", "y", "u", "i", "o", "p",
            "leftbrace", "rightbrace", "enter", "leftctrl", "a",
            "s", "d", "f", "g", "h", "j", "k", "l", "semicolon",
            "apostrophe", "grave", "leftshift", "backslash", "z",
            "x", "c", "v", "b", "n", "m", "comma", "dot", "slash",
            "rightshift", "kpasterisk", "leftalt", "space", "capslock",
            "f1", "f2", "f3", "f4", "f5", "f6", "f7", "f8", "f9", "f10",
            "numlock", "scrolllock", "kp7", "kp8", "kp9", "kpminus",
            "kp4", "kp5", "kp6", "kpplus", "kp1", "kp2", "kp3", "kp3",  "kpdot",
            "103rd", "f13", "102nd", "f11", "f12", "f14", "f15", "f16",
            "f17", "f18", "f19", "f20", "kpenter", "rightctrl", "kpslash",
            "sysrq", "rightalt", "linefeed", "home", "up", "pageup", "left",
            "right", "end", "down", "pagedown", "insert", "delete", "macro",
            "mute", "volumedown", "volumeup", "power", "kpequal", "kpplusminus",
            "pause", "f21", "f22", "f23", "f24", "kpcomma", "leftmeta",
            "rightmeta", "compose",
    
            "stop", "again", "props", "undo", "front", "copy", "open",
            "paste", "find", "cut", "help", "menu", "calc", "setup", "sleep", "wakeup",
            "file", "sendfile", "deletefile", "xfer", "prog1", "prog2", "www",
            "msdos", "coffee", "direction", "cyclewindows", "mail", "bookmarks",
            "computer", "back", "forward", "colsecd", "ejectcd", "ejectclosecd",
            "nextsong", "playpause", "previoussong", "stopcd", "record",
            "rewind", "phone", "iso", "config", "homepage", "refresh", "exit",
            "move", "edit", "scrollup", "scrolldown", "kpleftparen", "kprightparen",
    
            "intl1", "intl2", "intl3", "intl4", "intl5", "intl6", "intl7",
            "intl8", "intl9", "lang1", "lang2", "lang3", "lang4", "lang5",
            "lang6", "lang7", "lang8", "lang9", "playcd", "pausecd", "prog3",
            "prog4", "suspend", "close", "play", "fastforward", "bassboost",
            "print", "hp", "camera", "sound", "question", "email", "chat",
            "search", "connect", "finance", "sport", "shop", "alterase",
            "cancel", "brightnessdown", "brightnessup", "media"
        };
  
    sprintf(hid_code,"key_%s",key_names[linux_code]);
}

void hid_print_element_list(t_hid *x)
{
    DEBUG(post("hid_print_element_list"););
    unsigned long bitmask[EV_MAX][NBITS(KEY_MAX)];
//    char event_type_string[256];
//    char event_code_string[256];
    char *event_type_name = "";
    t_int i, j;
    /* counts for various event types */
    t_int syn_count,key_count,rel_count,abs_count,msc_count,led_count,snd_count,rep_count,ff_count,pwr_count,ff_status_count;

    /* get bitmask representing supported events (axes, keys, etc.) */
    memset(bitmask, 0, sizeof(bitmask));
    ioctl(x->x_fd, EVIOCGBIT(0, EV_MAX), bitmask[0]);
    post("\nSupported events:");
    
/* init all count vars */
    syn_count = key_count = rel_count = abs_count = msc_count = led_count = 0;
    snd_count = rep_count = ff_count = pwr_count = ff_status_count = 0;
    
    /* cycle through all possible event types 
     * i = i   j = j
     */
    for (i = 1; i < EV_MAX; i++) 
    {
        if (test_bit(i, bitmask[0])) 
        {
            /* make pretty names for event types */
            switch(i) 
            {
//            case EV_SYN: event_type_name = "Synchronization"; break;
            case EV_KEY: event_type_name = "Keys/Buttons"; break;
            case EV_REL: event_type_name = "Relative Axis"; break;
            case EV_ABS: event_type_name = "Absolute Axis"; break;
            case EV_MSC: event_type_name = "Miscellaneous"; break;
            case EV_LED: event_type_name = "LEDs"; break;
            case EV_SND: event_type_name = "System Sounds"; break;
            case EV_REP: event_type_name = "Autorepeat Values"; break;
            case EV_FF:  event_type_name = "Force Feedback"; break;
            case EV_PWR: event_type_name = "Power"; break;
            case EV_FF_STATUS: event_type_name = "Force Feedback Status"; break;
            default: event_type_name = "UNSUPPORTED"; 
            }
		 
            /* get bitmask representing supported button types */
            ioctl(x->x_fd, EVIOCGBIT(i, KEY_MAX), bitmask[i]);
		 
            post("");
            post("  TYPE\tCODE\tEVENT NAME");
            post("-----------------------------------------------------------");

            /* cycle through all possible event codes (axes, keys, etc.) 
             * testing to see which are supported.
             * i = i   j = j
             */
            for (j = 0; j < KEY_MAX; j++) 
            {
                if (test_bit(j, bitmask[i])) 
                {
                    if ((i == EV_KEY) && (j >= BTN_MISC) && (j < KEY_OK) )
                    {
                        char hid_code[7];
                        hid_convert_linux_buttons_to_numbers(j,hid_code);
                        post("  %s\t%s\t%s",
                             ev[i] ? ev[i] : "?", 
                             hid_code,
                             event_names[i] ? (event_names[i][j] ? event_names[i][j] : "?") : "?");
                    }
                    else if (i != EV_SYN)
                    {
                        post("  %s\t%s\t%s",
                             ev[i] ? ev[i] : "?", 
                             event_names[i][j] ? event_names[i][j] : "?", 
                             event_type_name);
                        
/* 	  post("    Event code %d (%s)", j, names[i] ? (names[i][j] ? names[i][j] : "?") : "?"); */
                    }
				  
                    switch(i) {
/* 
 * the API changed at some point...  EV_SYN seems to be the new name
 * from "Reset" events to "Syncronization" events
 */
/* #ifdef EV_RST */
/*                     case EV_RST: syn_count++; break; */
/* #else  */
/*                     case EV_SYN: syn_count++; break; */
/* #endif */
                    case EV_KEY: key_count++; break;
                    case EV_REL: rel_count++; break;
                    case EV_ABS: abs_count++; break;
                    case EV_MSC: msc_count++; break;
                    case EV_LED: led_count++; break;
                    case EV_SND: snd_count++; break;
                    case EV_REP: rep_count++; break;
                    case EV_FF:  ff_count++;  break;
                    case EV_PWR: pwr_count++; break;
                    case EV_FF_STATUS: ff_status_count++; break;
                    }
                }
            }
        }        
    }
    
    post("\nDetected:");
//    if (syn_count > 0) post ("  %d Synchronization types",syn_count);
    if (key_count > 0) post ("  %d Key/Button types",key_count);
    if (rel_count > 0) post ("  %d Relative Axis types",rel_count);
    if (abs_count > 0) post ("  %d Absolute Axis types",abs_count);
    if (msc_count > 0) post ("  %d Misc types",msc_count);
    if (led_count > 0) post ("  %d LED types",led_count);
    if (snd_count > 0) post ("  %d System Sound types",snd_count);
    if (rep_count > 0) post ("  %d Key Repeat types",rep_count);
    if (ff_count > 0) post ("  %d Force Feedback types",ff_count);
    if (pwr_count > 0) post ("  %d Power types",pwr_count);
    if (ff_status_count > 0) post ("  %d Force Feedback types",ff_status_count);
}


void hid_print_device_list(void)
{
    DEBUG(post("hid_print_device_list"););
    int i,fd;
    char device_output_string[256] = "Unknown";
    char dev_handle_name[20] = "/dev/input/event0";

    post("");
    for (i=0;i<128;++i) 
    {
        sprintf(dev_handle_name,"/dev/input/event%d",i);
        if (dev_handle_name) 
        {
            /* open the device read-only, non-exclusive */
            fd = open (dev_handle_name, O_RDONLY | O_NONBLOCK);
            /* test if device open */
            if (fd < 0 ) 
            { 
                fd = -1;
            } 
            else 
            {
                /* get name of device */
                ioctl(fd, EVIOCGNAME(sizeof(device_output_string)), device_output_string);
                post("Device %d: '%s' on '%s'", i, device_output_string, dev_handle_name);
			  
                close (fd);
            }
        } 
    }
    post("");	
}

/* ------------------------------------------------------------------------------ */
/*  FORCE FEEDBACK FUNCTIONS */
/* ------------------------------------------------------------------------------ */

/* cross-platform force feedback functions */
t_int hid_ff_autocenter( t_hid *x, t_float value )
{
    return ( 0 );
}


t_int hid_ff_gain( t_hid *x, t_float value )
{
    return ( 0 );
}


t_int hid_ff_motors( t_hid *x, t_float value )
{
    return ( 0 );
}


t_int hid_ff_continue( t_hid *x )
{
    return ( 0 );
}


t_int hid_ff_pause( t_hid *x )
{
    return ( 0 );
}


t_int hid_ff_reset( t_hid *x )
{
    return ( 0 );
}


t_int hid_ff_stopall( t_hid *x )
{
    return ( 0 );
}



// these are just for testing...
t_int hid_ff_fftest ( t_hid *x, t_float value)
{
    return ( 0 );
}


void hid_ff_print( t_hid *x )
{
}



/* ------------------------------------------------------------------------------ */
/* Pd [hid] FUNCTIONS */
/* ------------------------------------------------------------------------------ */

t_int hid_get_events(t_hid *x)
{
    DEBUG(post("hid_get_events"););

/* for debugging, counts how many events are processed each time hid_read() is called */
    DEBUG(t_int event_counter = 0;);

    char hid_code[7];

/* this will go into the generic read function declared in hid.h and
 * implemented in hid_linux.c 
 */
    struct input_event hid_input_event;

    if (x->x_fd < 0) return 0;

    while( read (x->x_fd, &(hid_input_event), sizeof(struct input_event)) > -1 )
    {
        if( hid_input_event.type == EV_KEY )
        {
            hid_convert_linux_buttons_to_numbers(hid_input_event.code,hid_code);
            hid_convert_linux_keys(hid_input_event.code,hid_code);
        }
        else if( hid_input_event.type == EV_SYN )
        {
            // filter out EV_SYN events, they are currently unused
        }
        else if( event_names[hid_input_event.type][hid_input_event.code] != NULL )
        {
            strcpy(hid_code, event_names[hid_input_event.type][hid_input_event.code]);
        }
        else 
        {
            strcpy(hid_code, "unknown");
        }
		
        if( hid_input_event.type != EV_SYN )
            hid_output_event(x, ev[hid_input_event.type], hid_code, 
                             (t_float)hid_input_event.value);
        DEBUG(++event_counter;);
    }
    DEBUG(
        //if (event_counter > 0)
        //post("output %d events",event_counter);
	);
	
    return (0);
}


void hid_print(t_hid* x)
{
    hid_print_device_list();
    hid_print_element_list(x);
}


t_int hid_open_device(t_hid *x, t_int device_number)
{
    DEBUG(post("hid_open_device"););

    char device_name[256] = "Unknown";
    char dev_handle_name[20] = "/dev/input/event0";
    struct input_event hid_input_event;

    x->x_fd = -1;
  
    x->x_device_number = device_number;
    sprintf(dev_handle_name,"/dev/input/event%d",(int)x->x_device_number);

    if (dev_handle_name) 
    {
        /* open the device read-only, non-exclusive */
        x->x_fd = open(dev_handle_name, O_RDONLY | O_NONBLOCK);
        /* test if device open */
        if (x->x_fd < 0 ) 
        { 
            error("[hid] open %s failed",dev_handle_name);
            x->x_fd = -1;
            return 1;
        }
    } 
  
    /* read input_events from the HID_DEVICE stream 
     * It seems that is just there to flush the input event queue
     */
    while (read (x->x_fd, &(hid_input_event), sizeof(struct input_event)) > -1);

    /* get name of device */
    ioctl(x->x_fd, EVIOCGNAME(sizeof(device_name)), device_name);
    post ("[hid] opened device %d (%s): %s",
          x->x_device_number,dev_handle_name,device_name);

    return (0);
}

/*
 * Under GNU/Linux, the device is a filehandle
 */
t_int hid_close_device(t_hid *x)
{
    DEBUG(post("hid_close_device"););
    if (x->x_fd <0) 
        return 0;
    else
        return (close(x->x_fd));
}

t_int hid_build_device_list(t_hid *x)
{
    DEBUG(post("hid_build_device_list"););
    /* the device list should be refreshed here */
/*
 *	since in GNU/Linux the device list is the input event devices 
 *	(/dev/input/event?), nothing needs to be done as of yet to refresh 
 * the device list.  Once the device name can be other things in addition
 * the current t_float, then this will probably need to be changed.
 */

    return (0);
}

void hid_platform_specific_free(t_hid *x)
{
    /* nothing to be done here on GNU/Linux */
}



#endif  /* #ifdef __linux__ */

