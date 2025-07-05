package com.example.smarthome.db;

import java.sql.Connection;
import java.sql.DriverManager;
import java.sql.PreparedStatement;
import java.sql.SQLException;

public class DatabaseManager {
    private static final String URL = "jdbc:mysql://localhost:3306/iot_sensors";
    private static final String USER = "root";
    private static final String PASSWORD = "PASSWORD"; // Lascia vuoto se non hai impostato password

    public static Connection connect() throws SQLException {
        return DriverManager.getConnection(URL, USER, PASSWORD);
    }

    public static void insertSensor(String type, String ip, int port) {

        System.out.printf("[DB] Registrazione sensore: %s @ %s:%d%n", type, ip, port);

        String sql = "INSERT INTO sensor_registry (sensor_type, sensor_ip, port) VALUES (?, ?, ?) " +
                     "ON DUPLICATE KEY UPDATE sensor_ip = VALUES(sensor_ip), port = VALUES(port)";
        try (Connection conn = connect(); PreparedStatement stmt = conn.prepareStatement(sql)) {
            stmt.setString(1, type);
            stmt.setString(2, ip);
            stmt.setInt(3, port);
            stmt.executeUpdate();
            System.out.printf("[DB] Sensore registrato/aggiornato: %s @ %s:%d%n", type, ip, port);
        } catch (SQLException e) {
            System.err.println("[DB] Errore durante inserimento sensore: " + e.getMessage());
        }
    }
}

