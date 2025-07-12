package com.example.smarthome.model;

import java.sql.Timestamp;

public class DataModel {
    private final int id;
    private final String name;
    private final double value;
    private final Timestamp timestamp;

    public DataModel(int id, String name, double value, Timestamp timestamp) {
        this.id = id;
        this.name = name;
        this.value = value;
        this.timestamp = timestamp;
    }

    public int getId() {
        return id;
    }

    public String getName() {
        return name;
    }

    public double getValue() {
        return value;
    }

    public Timestamp getTimestamp() {
        return timestamp;
    }
}