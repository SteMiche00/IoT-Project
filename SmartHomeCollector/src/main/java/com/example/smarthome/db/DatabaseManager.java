package com.example.smarthome.db;

import com.example.smarthome.model.DataModel;
import com.example.smarthome.model.DeviceModel;

import java.util.logging.Logger;
import java.sql.Connection;
import java.sql.DriverManager;
import java.sql.PreparedStatement;
import java.sql.ResultSet;
import java.sql.SQLException;
import java.time.Instant;
import java.util.ArrayList;
import java.util.List;
import java.sql.Timestamp;

public class DatabaseManager {
    private static final String URL = "jdbc:mysql://localhost:3306/iot_smarthome";
    private static final String USER = "root";
    private static final String PASSWORD = "PASSWORD";
    private static final Logger LOGGER = Logger.getLogger(DatabaseManager.class.getName());

    public static Connection connect() throws SQLException {
        return DriverManager.getConnection(URL, USER, PASSWORD);
    }

    public static void insertDevice(String name, String ip, int port) {

        LOGGER.info(String.format("[DB] Trying to register device: %s @ %s:%d%n", name, ip, port));

        String sql = "INSERT INTO device_registry (name, ip, port, registered_at) VALUES (?, ?, ?, ?) " +
                     "ON DUPLICATE KEY UPDATE ip = VALUES(ip), port = VALUES(port)";
        try (Connection conn = connect(); PreparedStatement stmt = conn.prepareStatement(sql)) {
            stmt.setString(1, name);
            stmt.setString(2, ip);
            stmt.setInt(3, port);
            stmt.setTimestamp(4, Timestamp.from(Instant.now()));
            stmt.executeUpdate();
            LOGGER.info(String.format("[DB] Device successfully registered: %s @ %s:%d%n", name, ip, port));
        } catch (SQLException e) {
            LOGGER.severe("[DB] Error during device registration: " + e.getMessage());
            throw new RuntimeException("Database error", e);
        }
    }

    public static List<DeviceModel> getAllDevices() {
        String sql = "SELECT * FROM device_registry";
        List<DeviceModel> devices = new ArrayList<>();

        try (Connection conn = connect();
            PreparedStatement stmt = conn.prepareStatement(sql)) {
    
            try (ResultSet rs = stmt.executeQuery()) {
                while (rs.next()) {
                    int id = rs.getInt("id");
                    String name = rs.getString("name");
                    String ip = rs.getString("ip");
                    int port = rs.getInt("port");
                    devices.add(new DeviceModel(id, name, ip, port));
                }
            }

        } catch (SQLException e) {
            LOGGER.severe("[DB] Error during filtered sensor retrieval: " + e.getMessage());
            throw new RuntimeException("Database error", e);
        }

        return devices;
    }

    public static void insertSensorData(String name, Double value) {
        String table;

        if (name.contains("light")) {
            table = "light_data";
        } else if (name.contains("temp")) {
            table = "temperature_data";
        } else if (name.contains("humidity")) {
            table = "humidity_data";
        } else {
            LOGGER.severe("[DB] Unknown sensor type for name: " + name);
            return;
        }

        String sql = "INSERT INTO " + table + " (name, value, timestamp) VALUES (?, ?, ?)";

        try (Connection conn = connect(); PreparedStatement stmt = conn.prepareStatement(sql)) {
            stmt.setString(1, name);
            stmt.setDouble(2, value/1000);
            stmt.setTimestamp(3, Timestamp.from(Instant.now()));
            stmt.executeUpdate();
            LOGGER.info(String.format("[DB] Data inserted into %s: %s = %.2f%n", table, name, value/1000));
        } catch (SQLException e) {
            LOGGER.severe("[DB] Error inserting sensor data: " + e.getMessage());
            throw new RuntimeException("Database error", e);
        }
    }

    public static void cleanup(){
        String[] tables = {"device_registry", "light_data", "temperature_data", "humidity_data"};
        try (Connection conn = connect()) {
            for (String table : tables) {
                String sql = "DELETE FROM " + table;
                try (PreparedStatement stmt = conn.prepareStatement(sql)) {
                    stmt.executeUpdate();
                    LOGGER.info(String.format("[DB] Cleared table: %s%n", table));
                } catch (SQLException e) {
                    LOGGER.severe(String.format("[DB] Error clearing table %s: %s%n", table, e.getMessage()));
                }
            }
        } catch (SQLException e) {
            LOGGER.severe("[DB] Error during cleanup: " + e.getMessage());
            throw new RuntimeException("Database error", e);
        }
    }

    public static String getNodeUri(String actuatorType) {
        String sql = "SELECT * FROM device_registry WHERE name = ? ORDER BY registered_at DESC LIMIT 1";
        String parts[] = actuatorType.split("_");
         try (Connection conn = connect();
            PreparedStatement stmt = conn.prepareStatement(sql)) {
            stmt.setString(1, actuatorType);
            try (ResultSet rs = stmt.executeQuery()) {
                if (rs.next()) {
                    String ip = rs.getString("ip");
                   
                    return "coap://[" + ip + "]/actuators/" + parts[1].trim() + "_th";
                } else {
                    LOGGER.warning("[DB] No device found for actuator type: " + actuatorType);
                    return null;
                }
            }

        } catch (SQLException e) {
            LOGGER.severe("[DB] Error during filtered sensor retrieval: " + e.getMessage());
            throw new RuntimeException("Database error", e);
        }
    }

    public static List<DataModel> getRecentSensorData(String sensorType, int minutes) {
        String sql = "SELECT * FROM " + sensorType + " WHERE timestamp >= NOW() - INTERVAL ? MINUTE";
        List<DataModel> data = new ArrayList<>();

        try (Connection conn = connect();
            PreparedStatement stmt = conn.prepareStatement(sql)) {
            stmt.setInt(1, minutes);
    
            try (ResultSet rs = stmt.executeQuery()) {
                while (rs.next()) {
                    int id = rs.getInt("id");
                    String name = rs.getString("name");
                    double value = rs.getDouble("value");
                    Timestamp timestamp = rs.getTimestamp("timestamp");
                    data.add(new DataModel(id, name, value, timestamp));
                }
            }

        } catch (SQLException e) {
            LOGGER.severe("[DB] Error during recent sensor data retrieval: " + e.getMessage());
            throw new RuntimeException("Database error", e);
        }

        return data;
    }
}




