package com.example.smarthome;

import com.example.smarthome.coap.CoapRegistrationServer;
import com.example.smarthome.coap.SensorPoller;

import java.io.BufferedReader;
import java.io.InputStreamReader;

public class SmartHomeCollector {
    public static void main(String[] args) {
        System.out.println("=== SMART HOME COLLECTOR ===");

        // 1. Avvia il server CoAP principale (registrazione sensori reali)
        CoapRegistrationServer coapServer = new CoapRegistrationServer();
        coapServer.start();
        System.out.println("[INFO] CoAP server started on port 5683");

        // 2. Avvia il poller per interrogare i sensori registrati
        SensorPoller sensorPoller = new SensorPoller();
        sensorPoller.startPolling();

        // 3. Mostra comandi disponibili
        printHelp();

        // 4. CLI per comandi da utente
        BufferedReader br = new BufferedReader(new InputStreamReader(System.in));
        String input;

        /*
        while (true) {
            try {
                System.out.print("> ");
                input = br.readLine().trim();

                switch (input) {
                    case "exit":
                        System.out.println("Closing server...");
                        coapServer.stop();
                        return;
                    default:
                        System.out.println("Use 'help'");
                }
            } catch (Exception e) {
                e.printStackTrace();
            }
        }
        */
    }

    private static void printHelp() {
        System.out.println("\nCommands:");
        System.out.println("  help                  - show commands");
        System.out.println("  exit                  - close server");
    }
}
