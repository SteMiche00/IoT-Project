package com.example.smarthome.model;

public class DeviceModel {
    private final int id;
    private final String name;
    private final String ip;
    private final int port;

    public DeviceModel(int id, String name, String ip, int port) {
        this.id = id;
        this.name = name;
        this.ip = ip;
        this.port = port;
    }

    public int getId() {
        return id;
    }

    public String getName() {
        return name;
    }

    public String getIp() {
        return ip;
    }

    public int getPort() {
        return port;
    }

    @Override
    public String toString() {
        return String.format("%s @ %s:%d", name, ip, port);
    }
}
