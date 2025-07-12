package com.example.smarthome.coap;

import java.util.Collections;
import java.util.HashMap;
import java.util.Map;

public class DeviceRegistry {
    private static final Map<String, String> deviceMap = new HashMap<>();
    private static final java.util.logging.Logger LOGGER = java.util.logging.Logger.getLogger(DeviceRegistry.class.getName());
    private DeviceRegistry() {
    }

    public static void registerDevice(String name, String ip) {
        deviceMap.put(name, ip);
        LOGGER.info(String.format("[DeviceRegistry] Device registered %s with IP %s%n", name, ip));
    }

    public static void unregisterDevice(String name) {
        String removed = deviceMap.remove(name);
        if (removed != null) {
            LOGGER.info(String.format("[DeviceRegistry] Device Removed %s%n", name));
        }
    }

    public static String getDeviceIP(String name) {
        return deviceMap.get(name);
    }

    public static Map<String, String> getAll() {
        return Collections.unmodifiableMap(deviceMap);
    }

    public static boolean contains(String name) {
        return deviceMap.containsKey(name);
    }

    public static void clear() {
        deviceMap.clear();
    }
}
