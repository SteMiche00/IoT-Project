package com.example.smarthome.commands;

import java.util.List;
import java.util.Scanner;

import com.example.smarthome.db.DatabaseManager;
import com.example.smarthome.model.DataModel;
import com.example.smarthome.model.DeviceModel;

public class VisualizeManager {
 private static final Scanner scanner = new Scanner(System.in);

    public static void visualizeDevices() {
        List<DeviceModel> devices = DatabaseManager.getAllDevices();
        if(devices.isEmpty()) {
            System.out.println("No devices registered.");
            return;
        }
        List<DeviceModel> sensors = new java.util.ArrayList<>();
        List<DeviceModel> actuators = new java.util.ArrayList<>();
        for (DeviceModel device : devices) {
            if (device.getName().startsWith("sensor")) {
                sensors.add(device);
            } 
            else if (device.getName().startsWith("actuator")) {
                actuators.add(device);
            }
        }
        System.out.println("Registered Devices:");
        if(sensors.isEmpty()) {
            System.out.println("No sensors registered.");
        } 
        else {
            System.out.println("Sensors:");
            for (DeviceModel sensor : sensors) {
                System.out.printf("- " + sensor);
            }
        }
        if(actuators.isEmpty()) {
            System.out.println("No actuators registered.");
        }
        else {
            System.out.println("Actuators:");
            for (DeviceModel actuator : actuators) {
                System.out.printf("- " + actuator);
            }
        }
    }

    public static void visualizeRecentSensorData() {
        System.out.println("Choose sensor to visualize:");
        System.out.println("1. Light");
        System.out.println("2. Temperature");
        System.out.println("3. Humidity");
        System.out.print("> ");

        String sensorType = null;
        String input = scanner.nextLine();

        switch (input) {
            case "1":
                sensorType = "light_data";
                break;
            case "2":
                sensorType = "temperature_data";
                break;
            case "3":
                sensorType = "humidity_data";
                break;
            default:
                System.out.println("Invalid selection.");
                return;
        }

        System.out.print("How many minutes back? ");
        int minutes = scanner.nextInt();
        scanner.nextLine(); // consume newline
        List<DataModel> data = DatabaseManager.getRecentSensorData(sensorType, minutes);
        if(data.isEmpty()) {
            System.out.println("No data found for " + sensorType + " in the last " + minutes + " minutes.");
            return;
        }
        System.out.println("+----+----------------+---------+---------------------+");
        System.out.println("| ID | Name           | Value   | Timestamp           |");
        System.out.println("+----+----------------+---------+---------------------+");
            
        for (DataModel entry : data) {
            System.out.printf("| %-2d | %-14s | %-7.2f | %-19s |\n",
                entry.getId(),
                entry.getName(),
                entry.getValue(),
                entry.getTimestamp().toString()
            );
        }
        
        System.out.println("+----+----------------+---------+---------------------+");

    }
}
    
