package com.example.smarthome.db;

import java.sql.Connection;
import java.sql.DriverManager;
import java.sql.PreparedStatement;
import java.sql.SQLException;
import java.time.Instant;
import java.sql.Timestamp;

public class DatabaseManager {
    private static final String URL = "jdbc:mysql://localhost:3306/iot_smarthome";
    private static final String USER = "root";
    private static final String PASSWORD = "PASSWORD";

    public static Connection connect() throws SQLException {
        return DriverManager.getConnection(URL, USER, PASSWORD);
    }

    public static void insertSensor(String name, String ip, int port) {

        System.out.printf("[DB] Trying to register device: %s @ %s:%d%n", name, ip, port);

        String sql = "INSERT INTO device_registry (name, ip, port, registered_at) VALUES (?, ?, ?, ?) " +
                     "ON DUPLICATE KEY UPDATE ip = VALUES(ip), port = VALUES(port)";
        try (Connection conn = connect(); PreparedStatement stmt = conn.prepareStatement(sql)) {
            stmt.setString(1, name);
            stmt.setString(2, ip);
            stmt.setInt(3, port);
            stmt.setTimestamp(4, Timestamp.from(Instant.now()));
            stmt.executeUpdate();
            System.out.printf("[DB] Device successfully registered: %s @ %s:%d%n", name, ip, port);
        } catch (SQLException e) {
            System.err.println("[DB] Errore durante inserimento sensore: " + e.getMessage());
        }
    }
}

