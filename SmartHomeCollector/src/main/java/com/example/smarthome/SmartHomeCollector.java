package com.example.smarthome;

import com.example.smarthome.coap.CoapRegistrationServer;
import com.example.smarthome.commands.ThresholdManager;
import com.example.smarthome.commands.VisualizeManager;
import com.example.smarthome.db.DatabaseManager;
import com.example.smarthome.log.LoggerConfig;
import com.example.smarthome.model.DeviceModel;

import java.io.BufferedReader;
import java.io.InputStreamReader;
import java.util.List;

public class SmartHomeCollector {
    private static final java.util.logging.Logger LOGGER = java.util.logging.Logger.getLogger(SmartHomeCollector.class.getName());
    public static void main(String[] args) {
        LoggerConfig.setup(); 
        DatabaseManager.cleanup(); // Clear previous data
        System.out.println("=== SMART HOME COLLECTOR ===");

        // 1. Start Coap Server
        CoapRegistrationServer coapServer = new CoapRegistrationServer();
        coapServer.start();
        LOGGER.info("[INFO] CoAP server started on port 5683");

        // Show commands
        printCommands();

        
        BufferedReader br = new BufferedReader(new InputStreamReader(System.in));
        String input;

        
        while (true) {
            try {
                System.out.print("> ");
                input = br.readLine().trim();

                switch (input) {
                    case "0":
                        printHelp();
                        break;
                    case "1":
                        ThresholdManager.setThresholds();
                        break;
                    case "2":
                        ThresholdManager.getThresholds();
                        break;
                    case "3":
                        VisualizeManager.visualizeDevices();
                        break;
                    case "4":
                        VisualizeManager.visualizeRecentSensorData();
                        break;
                    case "q":
                        LOGGER.info("Closing server...");
                        coapServer.stop();
                        return;
                    default:
                        System.out.println("Command not available, Use 'help'");
                }
            } catch (Exception e) {
                e.printStackTrace();
            }
        }
        
    }

    private static void printCommands() {
        System.out.println("Available Commands:");
        System.out.println("1. Set thresholds for actuators");
        System.out.println("2. Show current thresholds");
        System.out.println("3. Show registered devices");
        System.out.println("4. Show recent sensor data");
        //System.out.println("5. Avvia/ferma observing dei sensori");
        //System.out.println("6. Esporta dati in CSV");
        System.out.println("0. Show help");
        System.out.println("q. Close the application");
    }

    public static void printHelp() {
    System.out.println("\n=== HELP MENU ===\n");

    System.out.println("1. Set actuator thresholds");
    System.out.println("   → Set the temperature or light threshold that triggers an actuator.\n");

    System.out.println("2. Show current thresholds");
    System.out.println("   → Display the current threshold values configured for each actuator.\n");

    System.out.println("3. Show registered devices");
    System.out.println("   → List all sensors and actuators currently registered in the system.\n");

    System.out.println("4. Show recent sensor data");
    System.out.println("   → Display the most recent values recorded by each sensor.\n");

    System.out.println("5. Start/Stop sensor polling");
    System.out.println("   → Enable or disable periodic polling of sensors.\n");

    System.out.println("6. Export sensor data to CSV");
    System.out.println("   → Export recent sensor readings into a CSV file for analysis.\n");

    System.out.println("0. Show this help menu");
    System.out.println("   → Print this list of available commands.\n");

    System.out.println("q. Exit application");
    System.out.println("   → Shut down the CoAP server and exit the application.\n");
}

}
