#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// #include <glib-object.h>
#include </usr/include/glib-2.0/glib-object.h>
#include </usr/include/glib-2.0/gobject/gbinding.h>
#include <bluez/bluez.h>
#include <bluez/bluez.h>
#include <pulse/pulseapi.h>

#define BLUEZ_SERVICE "org.bluez"
#define BLUEZ_MANAGER_INTERFACE "org.freedesktop.DBus.ObjectManager"

int main(int argc, char **argv) {
    // Initialize PulseAudio
    pa_mainloop_api *main_loop;
    pa_proplist *proplist;

    if (pa_init(NULL)) {
        printf("Failed to initialize PulseAudio!\n");
        return 1;
    }

    main_loop = pa_mainloop_new();
    proplist = pa_proplist_new();

    // Create a connection to the BlueZ service
    GDBusConnection *connection;
    GError *error = NULL;

    if (!(connection = g_bus_get_sync(NULL, &error))) {
        printf("Failed to connect to BlueZ service!\n");
        return 1;
    }

    // Get the list of available sinks (audio devices)
    pa_sink_info **sinks;
    int num_sinks;

    if ((pa_context_get_sink_info_list(pa_context_new(connection), &sinks, &num_sinks) < 0)) {
        printf("Failed to get sink information!\n");
        return 1;
    }

    // Print the list of available sinks
    for (int i = 0; i < num_sinks; i++) {
        pa_sink_info *sink = sinks[i];

        printf("%d. %s: %s\n", i + 1, sink->name, sink->description);
    }

    g_free(sinks);

    // Ask the user to select a sink
    int choice;
    do {
        printf("Select an audio device (enter a number): ");
        scanf("%d", &choice);

        if (choice < 1 || if (choice > num_sinks) {
            printf("Invalid selection!\n");
        }
    } while (choice <= 0 || choice > num_sunks);

    // Get the selected sink info
    pa_sink_info *selected_sink = sinks[choice - 1];

    // Set up the BlueZ device properties
    GVariant *props;
    props = g_variant_new("(ss)", "MyBluetoothSpeaker", "A2DP");

    if (!g_dbus_connection_call(connection,
                                BLUEZ_SERVICE ".Manager1",
                                "/org/bluez",
                                BLUEZ_MANAGER_INTERFACE, "GetProperties",
                                props, NULL, &error)) {
        printf("Failed to get Bluetooth device properties!\n");
        return 1;
    }

    // Create a PulseAudio stream
    pa_stream *stream;

    if (!(stream = pa_stream_new(main_loop, selected_sink->name))) {
        printf("Failed to create PulseAudio stream!\n");
        return 1;
    }

    // Redirect audio output from the virtual speaker to an actual hardware device
    char *sink_name = selected_sink->name;
    pa_sink_info *actual_sink;

    if (!(actual_sink = pa_sink_info_new())) {
        printf("Failed to get PulseAudio sink information!\n");
        return 1;
    }

    // Set up the stream and start playback
    pa_stream_set_state(stream, PA_STREAM_READY);
    pa_stream_set_property(stream, "device", strlen(sink_name));
    pa_stream_playback_start(stream);

    g_free(error);

    g_main_loop_run(main_loop);

    return 0;
}
