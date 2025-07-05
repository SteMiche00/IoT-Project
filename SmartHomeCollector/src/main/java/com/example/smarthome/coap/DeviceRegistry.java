package com.example.smarthome.coap;

import java.util.Collections;
import java.util.HashMap;
import java.util.Map;

public class DeviceRegistry {
    private static final Map<String, String> deviceMap = new HashMap<>();

    private DeviceRegistry() {
    }

    public static void registerDevice(String type, String ip) {
        deviceMap.put(type, ip);
        System.out.printf("[DeviceRegistry] Device registered %s with IP %s%n", type, ip);
    }

    public static void unregisterDevice(String type) {
        String removed = deviceMap.remove(type);
        if (removed != null) {
            System.out.printf("[DeviceRegistry] Device Removed %s%n", type);
        }
    }

    public static String getDeviceIP(String type) {
        return deviceMap.get(type);
    }

    public static Map<String, String> getAll() {
        return Collections.unmodifiableMap(deviceMap);
    }

    public static boolean contains(String type) {
        return deviceMap.containsKey(type);
    }

    public static void clear() {
        deviceMap.clear();
    }
}
